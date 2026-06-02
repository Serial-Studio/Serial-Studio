# Qt/std Classes to Avoid (Serial Studio)

Reference for Agent 4. Adapted from The Qt Company's list and annotated with this repo's
actual conventions. Read the "Repo reality" notes before flagging — some Qt-framework
preferences do NOT apply to an application codebase, and flagging them produces noise.

## Qt Classes -> Replacements

| Avoid | Prefer | Rationale |
|-------|--------|-----------|
| Java-style iterators | STL iterators | `QT_NO_JAVA_STYLE_ITERATORS` |
| `Q_FOREACH` / `foreach` | Range-based `for` | `QT_NO_FOREACH`; copies the container |
| `QScopedPointer` | `std::unique_ptr` | Not movable; use `const std::unique_ptr` for scope |
| `QPair` | `std::pair` | `QPair` is a type alias since Qt 6.0 |
| `QAtomic*` | `std::atomic` | Exception: static `QBasicAtomic*` (no runtime init) |
| `count()` / `length()` | `size()` | Consistency with std |
| `qMin`/`qMax`/`qBound` | `(std::min)()` / `(std::max)()` / `std::clamp()` | Mixed-type args; mind arg order |
| `QChar` as an object | `char16_t` | `QChar::isLower()` etc. as a namespace is fine |

## std Classes -> Replacements

| Avoid | Prefer | Rationale |
|-------|--------|-----------|
| `std::mutex` | `QMutex` | `QMutex` uses futexes. Exception: `std::mutex` + `std::condition_variable` beats `QMutex` + `QWaitCondition` |

## Anti-Patterns

| Pattern | Fix | Why |
|---------|-----|-----|
| `std::optional::value()` | `*opt`, `opt->foo`, `if (opt)` | Throws on empty |
| `std::optional{}` default ctor | `std::nullopt` | Avoids a GCC maybe-uninitialized warning |
| `p = realloc(p, ...)` | `tmp = realloc(p, ...); check; p = tmp;` | Leaks on failure |
| `value_or()` with a non-trivial arg | ternary / if-else | The arg is always evaluated |
| `QDateTime::currentDateTime()` | `currentDateTimeUtc()` | ~100x faster, stable across DST |
| Constructing `QRegularExpression` in a loop | Hoist it out | Recompiles every iteration |

## Repo reality — do NOT flag these

- **`std::shared_ptr` / `std::unique_ptr` / `std::weak_ptr` are the house style.** This
  codebase deliberately uses std smart pointers (e.g. `TimestampedFramePtr =
  std::shared_ptr<TimestampedFrame>` in `Frame.h`). Do **not** suggest `QSharedPointer` /
  `QScopedPointer` as replacements — the direction is the opposite.
- **`QSharedDataPointer` / `QExplicitlySharedDataPointer`** are framework-COW concerns; not
  relevant to application code here.
- **Framework-only deprecations** (export-macro hygiene, d-pointer/PIMPL rules, qdoc) do not
  apply — Serial Studio is an application, not a Qt module.

## Genuinely worth flagging in this repo

- `Q_FOREACH` / `foreach` sightings (present in some older drivers, e.g. `BluetoothLE.cpp`) —
  a fair finding; migrate to range-based `for`.
- `QScopedPointer` where a `std::unique_ptr` would match the house style.
- `qMin`/`qMax` where `(std::min)`/`(std::max)`/`std::clamp` is clearer.
- `QRegularExpression` built inside a loop, anywhere near the data path.
