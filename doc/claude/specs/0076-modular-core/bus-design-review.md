# Message bus — design review against outside advice (2026-09-03)

Scope: `core/Core/Bus/` as it stands after stage 2 of spec 0076, read against an outside
advisory on in-process pub/sub. The bus compiles and is tested (`app/tests/tst_message_bus.cpp`,
10 cases) but is **not yet wired**: nothing outside the test calls `setInstance()`, publishes, or
subscribes. The only non-test references in the tree are the CMake source list
(`core/Core/CMakeLists.txt:62`) and the `bus-on-hotpath` lint (`scripts/code-verify.py:2008`).
Everything below is therefore a design review, not a post-mortem on live behaviour.

## 1. How `Core::Bus::MessageBus` works

**Topic = type.** The subscriber table is `unordered_map<std::type_index, vector<Subscriber>>`
(`MessageBus.h:226`); the key is always `std::type_index(typeid(T))`, formed at each call site
(`:124`, `:144`, `:154`, `:171`). No string identifier exists anywhere in the API. The message
vocabulary is `Messages.h` — 8 aggregates of Qt-Core value types, called "the DBC" in the plan
(`plan.md:289`).

**Publish builds exactly one object.** `compose<T>()` (`MessageBus.h:203-208`) brace-initialises
`T message{args...}` and then `make_shared<const T>(std::move(message))` — braced init so an
aggregate topic needs no constructor, and the temporary-then-move so no parenthesised-aggregate
support is required from the compiler. The result is erased to
`ErasedMessage = shared_ptr<const void>` (`:72`), which keeps the original control block, so
`static_pointer_cast<const T>` in the subscribe lambda (`:172-174`) recovers the typed pointer
with no second allocation. There is a second `publish` overload (`:121-125`) taking an
already-built `shared_ptr<const T>`, which is what makes the `MessageArgs` concept (`:83-86`)
necessary to keep the two apart.

**Dispatch: copy under the mutex, run outside it.** `dispatch()` (`MessageBus.cpp:173-189`) locks
`m_mutex`, looks up the topic, copies the `vector<Subscriber>` into a local, releases the lock,
then calls `deliver()` per entry. Consequences, both deliberate and documented at `:167-171`: a
handler may publish, subscribe or unsubscribe re-entrantly (test at `tst_message_bus.cpp:256`),
and a subscriber added *by* a handler joins the next publication, not this one.

**Delivery rule.** `deliver()` (`:196-213`) reads `receiver->thread() == QThread::currentThread()`.
`Qt::DirectConnection`, or `Qt::AutoConnection` with the receiver on the publishing thread, calls
the handler inline on the publisher's stack. Otherwise `QMetaObject::invokeMethod(receiver,
lambda, Qt::QueuedConnection)` — the lambda captures both the handler copy and the
`shared_ptr`, so the message outlives an arbitrarily late drain of the receiver's event loop.
`Qt::BlockingQueuedConnection` is rejected at subscribe time (`:100`): debug aborts, release
downgrades to queued (test at `tst_message_bus.cpp:280`). The stated reason is the right one — a
publisher that blocks on a subscriber's thread deadlocks the moment two threads publish to each
other.

**Retained state.** `publishState<T>` (`MessageBus.h:130-146`) retains the pointer in
`m_retained` (`retain()`, `:222-229`, `insert_or_assign`) *and* dispatches. `latest<T>()`
(`:151-155`) returns the retained pointer or null. `subscribe(..., replayLatest = true)`
(`:177-181`) calls the handler synchronously before returning the handle — it calls `handler`
directly, so replay ignores the connection type and runs on the subscriber's own thread, which is
correct because subscribe is called from there. Plain `publish` never retains
(`tst_message_bus.cpp:186`).

**Lifetime.** `Subscription` (`Subscription.h:43-65`) is move-only, holds `{id, topic,
QPointer<MessageBus>}`; the destructor calls `reset()` (`Subscription.cpp:79-98`) which
unsubscribes and detaches. `release()` (`:103-108`) detaches without unsubscribing, for a
subscription meant to live as long as its receiver. Because the bus is held through a `QPointer`,
a handle that outlives its bus destructs into a no-op (`isActive()`, `:117-120`). Independently,
the bus connects to each receiver's `destroyed` signal once (`MessageBus.cpp:112-118`,
`DirectConnection`) and `purgeReceiver()` (`:146-161`) sweeps both the matching address *and*
every already-null `QPointer` — necessary because `QObject` clears its `QPointer`s before it
emits `destroyed`.

**Ownership.** `s_instance` is a plain file-static (`MessageBus.cpp:34`), set by `setInstance()`
(`:78-82`) and cleared in the destructor (`:55-56`) — deliberately not a Meyers singleton, since
the composition root owns the object. The census counts exactly one accessor
(`scripts/singleton-census.json`: `"Core::Bus::MessageBus": 1`).

**What is not there.** No per-subscriber queue and therefore no overflow policy; no message pool
(every publish allocates); no priority, ordering guarantee beyond registration order, or
filtering beyond the type key; no delivery counters or drop accounting; no `unsubscribe`-by-
receiver public API; no batching or coalescing.

### Sequence: two threads

```
pipeline thread                              GUI thread
---------------                              ----------
                                             sub = bus.subscribe<ConnectionStateChanged>(
                                                     &dashboard, h, AutoConnection);
                                               -> lock; id=7 appended; destroyed-guard connected
bus.publishState<ConnectionStateChanged>(3, true, false)
  compose  -> one make_shared<const T>              (1 allocation)
  retain   -> lock; m_retained[typeid] = ptr
  dispatch -> lock; copy vector<Subscriber>; unlock
  deliver  -> receiver->thread() != current;
              invokeMethod(dashboard, [h, ptr]{…}, Queued)  -> QMetaCallEvent posted
  returns (never waits)
                                             loop runs the lambda:
                                               h(static_pointer_cast<const T>(ptr))
                                             ptr released; refcount 0; T destroyed here
```

A later `bus.latest<ConnectionStateChanged>()` from any thread returns the same object under
`m_mutex` (`:234-242`).

## 2. Against the advisory, point by point

**Dispatch / ownership / transport separated, and the pointer handoff.** Agreed, and already the
case. Transport is one process with seven static archives (`plan.md:224-250`); ownership is
`shared_ptr<const T>`; dispatch is the type-keyed table. Nothing crosses a process boundary, so
eCAL / DDS / iceoryx / Zenoh are correctly out of scope — they would add a broker, a daemon and a
serialisation step to what is already one object per publish, with every subscriber seeing the
same `const` pointer (`tst_message_bus.cpp:114-136` asserts that identity).

**"~300 lines over moodycamel with `shared_ptr<const T>`".** Ours is ~370 lines across four files
and lands on the same immutability contract, with two deliberate differences. (a) The subscriber
table is a `std::mutex` + vector copy per publish rather than a lock-free queue: cost is one lock
acquisition plus a vector copy whose per-entry cost is a `QPointer` copy and a `std::function`
copy — the latter heap-allocates when the capture exceeds the SBO, so a publish to *n*
subscribers can cost *n*+1 allocations. (b) Cross-thread transport is Qt's event loop rather than
a ring: `invokeMethod` allocates a `QMetaCallEvent`, takes the receiver thread's post-event lock,
and wakes the loop. Both are fine at command/state/notification rate — a connection change, a
project load, a settings key — and both stop being fine the moment a topic runs at frame rate.
The lint `bus-on-hotpath` (`scripts/code-verify.py:2008-2029`) is what keeps the second case from
happening by accident. If the handler-copy cost ever shows up, the cheap fix is storing
`shared_ptr<const ErasedHandler>` in `Subscriber` so the vector copy becomes refcount bumps.

**CAN analogy and overflow policy.** The analogy holds (topic = arbitration ID, subscribers
filter by type, publisher does not know its audience) and the advisory is right that most naive
buses miss the overflow policy. **Ours has none, on purpose, because it is scoped to command,
state and notification rate** (`MessageBus.h:58-60`) — there is no queue to overflow: same-thread
delivery is a call, cross-thread delivery is a `QMetaCallEvent` on an unbounded Qt event queue.
The important point is that the repo **already implements the advisory's latest-wins-plus-drain-
on-render-tick pattern**, in a dedicated path, for exactly the traffic that needs it:

- Producer side: `DataModel::BlockStager` stages parsed rows into a pooled `DataBlock` and
  flushes on the display tick or a sample cap (`kFrameBlockSampleCap` 64,
  `BlockStager.h:65`; `kStreamBlockSampleCap` 4096, `StreamWorker.h:100`) — spec 0055.
- Fan-out: `DataModel::BlockPublisher` (`BlockPublisher.h`) holds each sink as a bound pointer in
  its `Sinks` struct and runs on the pipeline thread only, the single producer for every sink.
- Consumer side: `UI::Dashboard::onDisplayTick()` (`Dashboard.cpp:377-394`) drains the SPSC ring
  (`PipelineHost.h:112`, `kBlockRingSize = 32` at `:239`) inside
  `drainBlockRing()` (`:423-455`), hard-bounded by ring capacity *and* a wall-clock budget, and —
  precisely the advisory's latest-wins slot — discards everything past the budget **except the
  newest block**, which it still applies so the display stays current.

So the two designs are complementary, not competing: the block path is the lossy 60 Hz lane for
10 kHz+ telemetry, the bus is the lossless-because-rare lane for control and state. The failure
the advisory warns about ("why does the UI lag when logging is on") is already structurally
avoided: the recording sinks fan out on the pipeline thread from one trimmed copy
(`clone_block_trimmed`), never through the dashboard's ring.

**The named libraries, one line each.** All five would be new vendored trees under the
supply-chain gate (`lib/VERSIONS.json`), and every first-party file here is
`GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial`, so a candidate must also be permissive
(MIT/BSD/Apache-2.0) to survive the commercial half.

- *entt::dispatcher / emitter* — closest fit and genuinely no-singleton, but it has no thread
  affinity model, so we would still write the `deliver()` half for ~200 lines of saved code.
- *Boost.Signals2* — thread-safe with automatic connection lifetime, but slower than what we have
  (mutex plus per-slot `shared_ptr` tracking per emit), no receiver affinity, and pulls Boost into
  a tree that has none.
- *CAF* — our modules are already `QObject`s with event loops, i.e. actors with mailboxes;
  adopting CAF means two schedulers in one process.
- *NNG* — `inproc://` hands off `nng_msg` without copying, but it is a C byte-buffer API: we
  would serialise typed structs to get them back out.
- *iceoryx2 / Zenoh / eCAL* — multi-process only; relevant only if the acquisition pipeline ever
  becomes its own process, which is not on the 0076 roadmap.

**`std::pmr` memory pool.** The advisory's own escape clause applies: mimalloc is already the
global allocator (`SS_USE_MIMALLOC`, default `ON`, `CMakeLists.txt:86`, `cmake/MiMalloc.cmake`),
which is the "bigger win" it mentions. And the frame path is not served by a general pool — it is
served by a purpose-built one: `BlockStager` reserves `kBlockPoolSlots` = 64 slots
(`BlockStager.h:67`), probes a free slot with `use_count() == 1` (exact because every alias lives
on the pipeline thread, `FrameBuilder.cpp:317-327`), and hands out an *aliasing* `shared_ptr` over
the slot so there is no per-block control block at all (`BlockStager.cpp:278`). That is strictly
better than `pmr` for this shape. For the bus, a pool would be premature: at command rate the one
`make_shared` per publish is not measurable.

**Qt integration warning ("don't emit a queued signal per message").** Our bus does exactly the
thing warned about — one `invokeMethod` per cross-thread subscriber per publish
(`MessageBus.cpp:210-211`). That is acceptable only because the traffic is bounded below roughly
1 kHz; the warning is real and the mitigation is the `bus-on-hotpath` lint plus the header's
"NEVER on the per-frame path" (`MessageBus.h:58`). If a topic ever gets hot, the recommended
pattern is the one the dashboard already uses: give the GUI-side subscriber its own latest-wins
slot, have the bus handler only store into it and set a dirty flag, and let the existing display
tick (`Misc::TimerEvents::uiTimeout` → `onDisplayTick`, `Dashboard.cpp:331`) read it — one
coalesced wake-up per frame instead of one per message. Do not add a second timer.

**Plugin/ABI warning.** N/A. The five partition libraries plus the executable are one build with
one compiler, so `std::shared_ptr`/`std::function` in the boundary is fine. The extension system
is not a C++ ABI boundary either: `core/Ui/Misc/Extensions/` installs QML/JS packages
(`ExtensionCatalog`, `ExtensionInstaller`) and runs external plugins as child processes
(`PluginRunner` owns `QProcess` lifetimes), so nothing ever `dlopen`s a foreign C++ object into
our address space. The bus stays a C++-internal API and must not be exposed to extensions.

**No singletons / the `ModuleContext` snippet.** This is the one place I would change the plan.
Today the bus is reached through `MessageBus::instance()` (`MessageBus.cpp:70-73`), and the plan
adopts it as a tenth `SessionContext` slot (`plan.md:309`) — `SessionContext` currently owns nine
modules (`app/src/SessionContext.h:85-93`, spec 0039). That is defensible as a migration
scaffold, but it means every migrated call site trades `X::instance().foo()` for
`MessageBus::instance()->publish<T>(...)`: the coupling to a concrete class becomes a coupling to
a struct, which is the real win, but the global stays. The advisory's injection form is strictly
better and costs little here: a `Core::Bus::Context { MessageBus& bus; const Clock& clock; }`
handed to a subsystem's constructor once the composition root adopts the bus, with each subsystem
holding its `Subscription` members. Recommendation: keep `instance()` as the transitional
accessor so the follow-up specs can migrate incrementally, but require every *new* subscriber to
take the bus by reference in its constructor, and treat `instance()` calls as debt to be counted.

**How the census would measure the shift.** `scripts/code-verify.py --singleton-census` currently
records 1554 reaches over 93 classes, bucketed `static-cache` 1071 / `root` 160 / `ctor-capture`
123 / `accessor` 70 / `loose` 96 / `deferred` 34. The migration's success signal is *total*
falling while `Core::Bus::MessageBus` rises from its current 1 — the plan already says the shift
is the metric rather than new debt (`plan.md:311`, `handoff.md:170`). Under the injection path,
the `MessageBus` count should stay near 1 and the drop should be uncompensated: that is a
stronger, unambiguous signal, and it is the argument for injecting rather than adding a tenth
slot. Worth deciding before the first follow-up spec, because it sets the shape of ~1091 call
sites (`handoff.md:96`).

## 3. Recommended next steps, ranked

1. **Decide injection vs. tenth `SessionContext` slot before spec 0076's first follow-up.**
   The choice determines the constructor signature of every migrating subsystem, and reversing it
   later is a mechanical edit across the same ~1091 call sites the census counts.
2. **Wire the bus into the composition root and migrate one edge end-to-end**
   (`OperationModeChanged` / `LicenseStateChanged` / `LanguageChanged`, the plan's step 1) —
   nothing currently calls `setInstance()`, so the design is unexercised outside unit tests and
   the first real subscriber will surface whatever the tests did not.
3. **Write the `Messages.h` growth rules into the file's `@file` block as a checklist**
   (aggregate of Qt-Core value types only; no enums that live above `Core`; a field change is a
   wire break for every reader; added only through a spec) — the header states the policy
   (`Messages.h:26-38`) but not the checklist a reviewer applies.
4. **Add a `--json` census of publish/subscribe sites to `code-verify.py` once migration starts**,
   so a topic that quietly becomes hot, or a topic with publishers and no subscribers, is visible
   without reading the tree.
5. **Design per-subscriber `Policy::LatestWins` / `Policy::Lossless{N}` mailboxes only when a
   topic actually exceeds command rate** — the block path already covers the hotpath, so adding
   mailboxes now would be an unexercised second queueing system; the trigger to build them is the
   first topic that needs backpressure, and the design should follow `drainBlockRing()`'s shape.
6. **Cheap hardening, if the first migration exposes it:** store the handler as
   `shared_ptr<const ErasedHandler>` so `dispatch()`'s vector copy stops copying `std::function`s;
   and note that `QPointer` is not thread-safe, so a receiver destroyed on its own thread
   concurrently with a cross-thread `deliver()` is a narrow race the current code does not close
   (`MessageBus.cpp:198-202`). Neither matters until the bus carries real traffic.
