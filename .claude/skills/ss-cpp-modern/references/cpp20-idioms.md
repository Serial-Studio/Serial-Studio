# C++20 idioms (Serial Studio style)

Snippets in this repo's naming and conventions: `CamelCase` types, `camelCase` functions,
`lower_case` locals/public members, `m_camelCase` private members, `kCamelCase`
constants, `std` smart pointers, `static_cast`, ASCII-only, 2-space indent. These show the
*shape* of an idiom; `scripts/code-verify.py` remains the formatting authority.

## Concept-constrained template

A named concept beats `std::enable_if` for readability and diagnostics.

```cpp
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<Numeric T>
[[nodiscard]] T clampValue(T value, T lo, T hi)
{
  return std::clamp(value, lo, hi);
}
```

Concept-based overloading replaces tag dispatch:

```cpp
template<std::integral T>
[[nodiscard]] QString formatScalar(T value);

template<std::floating_point T>
[[nodiscard]] QString formatScalar(T value);
```

## RAII handle wrapper

Wrap every raw OS/library handle so cleanup is exception-safe. Non-copyable, movable via
`std::exchange`. Note: a member handle that lives for the owner's lifetime is simpler as a
`const std::unique_ptr` with a custom deleter; reach for a bespoke class only when the move
semantics matter.

```cpp
class FileHandle
{
public:
  explicit FileHandle(const QString &path)
    : m_handle(std::fopen(path.toLocal8Bit().constData(), "rb"))
  {
    Q_ASSERT(!path.isEmpty());
  }

  ~FileHandle()
  {
    if (m_handle)
      std::fclose(m_handle);
  }

  FileHandle(const FileHandle &) = delete;
  FileHandle &operator=(const FileHandle &) = delete;

  FileHandle(FileHandle &&other) noexcept
    : m_handle(std::exchange(other.m_handle, nullptr))
  {
  }

  [[nodiscard]] std::FILE *get() const noexcept { return m_handle; }

private:
  std::FILE *m_handle;
};
```

## Smart pointer ownership

```cpp
auto buffer = std::make_unique<std::array<std::byte, kBufferSize>>();
```

Shared ownership only when something is genuinely held by several owners (e.g. a frame fanned
out to async sinks). On the dashboard hotpath, acquire from the slot pool instead — see
[[ss-hotpath]].

```cpp
auto frame = std::make_shared<DataModel::TimestampedFrame>();
```

## Move-and-swap assignment

A resource-owning type: disable copy, implement move via swap, document the moved-from state
(here: valid, empty, reusable).

```cpp
class Buffer
{
public:
  explicit Buffer(std::size_t size)
    : m_size(size)
    , m_data(std::make_unique<std::byte[]>(size))
  {
  }

  Buffer(Buffer &&other) noexcept { swap(other); }

  Buffer &operator=(Buffer &&other) noexcept
  {
    if (this != &other)
      Buffer(std::move(other)).swap(*this);

    return *this;
  }

  void swap(Buffer &other) noexcept
  {
    std::swap(m_size, other.m_size);
    std::swap(m_data, other.m_data);
  }

private:
  std::size_t m_size = 0;
  std::unique_ptr<std::byte[]> m_data;
};
```

NRVO note: `return local;` not `return std::move(local);`. A `const` local also blocks the
implicit move on return.

## Ranges pipeline (OFF the hotpath)

Lazy view chain for a transformation that is not on the parse path:

```cpp
auto squaredEvens = numbers
  | std::views::filter([](int n) { return n % 2 == 0; })
  | std::views::transform([](int n) { return n * n; });
```

Do not introduce range pipelines into the frame extractors or per-dataset transforms; those are
fixed-bound hand-tuned loops (Power of Ten). Benchmark with `--benchmark-hotpath` before any
hotpath change.

## Atomics with explicit ordering

Publish-then-observe across threads uses `release` on the writer and `acquire` on the reader.
The SPSC ring (`CircularBuffer` + `FrameReader`) is main-thread and needs none of this — this
is for the cases that genuinely cross threads (watchdog flag, shared counters).

```cpp
void publish(std::atomic<int> &data, std::atomic<bool> &ready)
{
  data.store(42, std::memory_order_relaxed);
  ready.store(true, std::memory_order_release);
}

[[nodiscard]] int observe(std::atomic<int> &data, std::atomic<bool> &ready)
{
  while (!ready.load(std::memory_order_acquire))
    std::this_thread::yield();

  return data.load(std::memory_order_relaxed);
}
```

A shared counter is atomic, not a plain `int++` from multiple threads:

```cpp
m_operationCount.fetch_add(1, std::memory_order_relaxed);
```

## Spaceship comparison

Default `operator<=>` for value types that need ordering; spell it out for custom precedence.

```cpp
struct Version
{
  int major = 0;
  int minor = 0;
  int patch = 0;

  auto operator<=>(const Version &) const = default;
};
```
