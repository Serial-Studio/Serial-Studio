# Qt6 Code Review Checklist (Serial Studio)

Distilled from The Qt Company's "Things To Look Out For In Reviews", pruned to the rules that
apply to an *application* (Serial Studio) rather than a Qt module. Framework-only rules —
binary compatibility, `Q_*_EXPORT`, d-pointers/PIMPL, qdoc, QML import versioning — are
omitted because this repo ships none of them.

Each rule has a short ID for cross-referencing in review reports. Repo-specific invariants
(hotpath, threading, style, Power of Ten) live in `serial-studio-rules.md`.

## API & Naming

- **API-3**: Naming consistent with similar Qt classes (`timeout` not `timeOut`, `size()` not
  `count()`).
- **API-5**: `get`-prefix means user interaction or out-param decomposition, NOT a plain
  getter.

## Includes

- **INC-4**: Include everything you use directly (Lakos). Don't rely on transitive includes.

## Variables

- **VAR-4**: Never use a dynamically-sized container for statically-sized data. Use
  `std::array` or a C array.

## Methods

- **MTH-1**: Inline methods: prefer defining in the class body (skip the `inline` keyword).

## Properties

- **PRP-1**: New classes: `Q_PROPERTY ... FINAL` unless intended for override.
- **PRP-3**: Avoid a property with the same name as a meta-method (shadows in QML).

## Timeouts

- **TMO-1**: No raw `int`/`qint64` for timeouts or intervals — use `QDeadlineTimer` or
  `std::chrono`.

## Polymorphic Classes

- **PLY-2**: `Q_DISABLE_COPY_MOVE` on polymorphic classes.
- **PLY-4**: A virtual function is marked by exactly ONE of `virtual` / `override` / `final`.
- **PLY-6**: Virtual access level: public if callable, private if a reimpl shouldn't call
  base, protected if a reimpl should call base.

## QObject Subclasses

- **QOB-1**: Always include the `Q_OBJECT` macro.
- **QOB-4**: Idiomatic element order. (Serial Studio fixes this exactly — see
  `serial-studio-rules.md` § Style invariants, which supersedes the generic Qt order.)

## RAII Classes

- **RAI-2**: `Q_DISABLE_COPY`. Make movable, or comment why not.
- **RAI-3**: Move-assignment via move-and-swap.

## Enums

- **ENM-1**: Trailing comma on the last enumerator (reduces diff noise).
- **ENM-2**: Scoped, or explicit underlying type.
- **ENM-5**: `{}` (value 0) should mean "default".
- **ENM-7**: `switch` over an enum: no `default:` label; list all enumerators explicitly.

## Exceptions / noexcept

- **NXC-1**: A `noexcept` function must not have a clear throwing path (allocation,
  precondition-style `Q_ASSERT`, calls to throwing functions). Flag only when a throwing path
  is visible.
- **NXC-3**: `Q_ASSERT` checking a *caller obligation* (precondition) is incompatible with
  `noexcept`; one verifying an *internal invariant* is acceptable. Unclear -> investigation
  target.

## Functions — Returning Data

- **RET-1**: Prefer returning by value over out-params.
- **RET-2**: Enable RVO/NRVO; don't mix named and unnamed returns in one function. Flag only
  when mixed return paths are visible in the source.

## Move Semantics

- **MOV-1**: rvalue refs use `std::move`; universal refs use `std::forward`.
- **MOV-2**: Document the moved-from state.

## Ternary Operator

- **TRN-1**: Nested ternaries: one condition per line.
- **TRN-3**: Never use a ternary just to convert to bool.

## Relational Operators

- **REL-2**: Never convert unsigned to signed for comparison.

## Model Contracts (QAbstractItemModel)

- **MDL-1**: Structural changes (add/remove/move rows) use proper begin/end signals, not
  `layoutChanged`.
- **MDL-2**: `dataChanged` passes the specific changed roles, not an empty vector.
- **MDL-3**: `setData()` emits `dataChanged` before returning `true`.
- **MDL-4**: `beginRemoveRows(parent, 0, count-1)` with `count == 0` violates the contract
  (first > last).
- **MDL-5**: `flags()` returns appropriate flags per item type.
- **MDL-6**: begin/end signal pairs are balanced within every code path.
- **MDL-7**: `data()` switch lists all role cases explicitly; avoid `default:`.
- **MDL-8**: `roleNames()` return matches `data()`'s switch cases (missing cases return
  `QVariant()` silently).
- **MDL-9**: Proxy/filter models go through the source model's `data()`/`index()` API, not raw
  struct pointers.
- **MDL-10**: `roleNames()` caches its `QHash` (static local or member), not rebuilds per call.

## Error Handling & Validation

- **ERR-1**: Check `QFile::open()` before reading/writing.
- **ERR-2**: Check `QJsonDocument::fromJson()` with `isNull()`/`isObject()` before use.
- **ERR-3**: Check `QNetworkReply::error()` before `readAll()`.
- **ERR-4**: `https://` not `http://` for network URLs.
- **ERR-5**: Set `QNetworkRequest::setTransferTimeout()` on network requests.
- **ERR-6**: `QString::arg()` placeholder count matches the `.arg()` call count.
- **ERR-7**: Check `QXmlStreamWriter::hasError()` after writing.
- **ERR-8**: Validate negative values in integer setters (not just zero).
- **ERR-9**: Handle `QNetworkAccessManager::sslErrors`.
- **ERR-10**: Validate schema/version on imported data.
- **ERR-11**: Validate input lengths from untrusted sources (imported JSON, downloads, parsed
  frames).
- **ERR-12**: Consistent error-reporting pattern across sibling methods.

## Resource Lifecycle

- **LCY-1**: `deleteLater()` on `QNetworkReply` in every finished handler.
- **LCY-2**: `QObject`-derived `new` has a parent or explicit lifecycle management.
- **LCY-3**: Never put a side-effectful expression inside `Q_ASSERT` (compiled out in
  release).
- **LCY-4**: Don't use `Q_ASSERT(ptr)` as the sole null guard before a dereference.
- **LCY-5**: Cap unbounded container growth.
- **LCY-6**: Destructor cleans up owned children recursively.

## Thread Safety

- **THR-1**: Never write `QObject` members from `QtConcurrent::run()` without sync.
- **THR-2**: Never emit a signal from a worker with `Qt::DirectConnection` to a main-thread
  receiver. (Serial Studio hotpath inverts this *within* the main thread — see
  `serial-studio-rules.md`.)
- **THR-3**: Never mutate a `QAbstractItemModel` from a background thread.
- **THR-4**: Protect a shared container consistently across all code paths.
- **THR-5**: `std::atomic` or mutex for shared counters across threads.

## Performance & Code Quality

- **PRF-1**: Don't construct `QRegularExpression` inside a loop.
- **PRF-2**: Cache `roleNames()`'s `QHash`.
- **PRF-3**: `const auto&` in range-for over shared containers (avoid COW detach).
- **PRF-4**: `.value()` not `operator[]` for reads on a shared `QHash`/`QMap`.
- **PRF-5**: Cheap early-exit before expensive work.
- **PRF-6**: Flag likely dead code. Out-of-scope callers possible -> investigation target.
- **PRF-7**: Extract magic numbers to named constants.
- **PRF-8**: Don't use `QMap` for small fixed constant data.
- **PRF-9**: Invalidate caches when underlying data changes.
- **PRF-10**: Re-entrancy guard on methods emitting signals that can recurse.
