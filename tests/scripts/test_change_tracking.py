"""
Logic tests for the change-driven-transform predicate.

This models the C++ mechanism (DataTableStore per-slot version + write clock,
per-dataset union read-set + last-run clock, and the skip decision) in Python and
asserts the contract: a virtual dataset's transform re-runs exactly when one of the
slots it has read changed since it last ran, and never otherwise — including a
chained dataset-reads-dataset case (which is just a read of the upstream dataset's
'final:' slot).

No app or Node needed; this pins the design's logic independently of the build.
"""


class Store:
    """Mirror of DataTableStore's version/capture surface."""

    def __init__(self, n_slots):
        self.version = [0] * n_slots
        self.write_clock = 0
        self._capture = None

    def write(self, slot):
        self.write_clock += 1
        self.version[slot] = self.write_clock

    def set_capture_target(self, read_set):
        self._capture = read_set

    def read(self, slot):
        if self._capture is not None and slot not in self._capture:
            self._capture.append(slot)
        return self.version[slot]  # value stand-in

    def changed_since(self, slots, since_clock):
        return any(self.version[s] > since_clock for s in slots)


class Dataset:
    """A virtual dataset: a read-set (union-over-history) + last-run clock."""

    def __init__(self, reads):
        self.reads = reads  # slots its transform reads when it runs
        self.read_slots = []  # discovered union read-set
        self.last_run_clock = 0
        self.runs = 0

    def should_run(self, store):
        # empty read-set -> always run (R7); else run iff an input changed
        return not self.read_slots or store.changed_since(
            self.read_slots, self.last_run_clock
        )

    def run(self, store):
        store.set_capture_target(self.read_slots)
        for s in self.reads:
            store.read(s)
        store.set_capture_target(None)
        self.last_run_clock = store.write_clock
        self.runs += 1


def frame(store, datasets):
    """One declaration-order pass: run only datasets whose inputs changed."""
    for d in datasets:
        if d.should_run(store):
            d.run(store)


# ---------------------------------------------------------------------------
# R2 / R3 — re-run iff a read slot changed
# ---------------------------------------------------------------------------


def test_first_frame_runs_then_skips_when_unchanged():
    store = Store(4)
    d = Dataset(reads=[0])
    frame(store, [d])  # cold: empty read-set -> runs, captures slot 0
    assert d.runs == 1
    assert d.read_slots == [0]
    frame(store, [d])  # nothing changed -> skip
    frame(store, [d])
    assert d.runs == 1


def test_reruns_when_its_input_changes():
    store = Store(4)
    d = Dataset(reads=[0])
    frame(store, [d])
    store.write(0)  # its input changed
    frame(store, [d])
    assert d.runs == 2


def test_unrelated_write_does_not_rerun():
    store = Store(4)
    d = Dataset(reads=[0])
    frame(store, [d])
    store.write(3)  # a slot it does not read
    frame(store, [d])
    assert d.runs == 1


def test_union_read_set_grows_and_is_deduped():
    store = Store(4)
    d = Dataset(reads=[0, 1, 0])
    frame(store, [d])
    assert d.read_slots == [0, 1]  # deduped
    store.write(1)
    frame(store, [d])
    assert d.runs == 2


# ---------------------------------------------------------------------------
# R10 — chained dataset-reads-dataset (a read of the upstream 'final:' slot)
# ---------------------------------------------------------------------------


def test_chained_dataset_reruns_when_upstream_value_changes():
    # slot 0 = source register, slot 1 = A's final mirror, slot 2 = B reads A
    store = Store(3)
    a = Dataset(reads=[0])  # A reads source register (slot 0)
    b = Dataset(reads=[1])  # B reads A's final slot (slot 1)

    def a_run(store):
        store.set_capture_target(a.read_slots)
        store.read(0)
        store.set_capture_target(None)
        store.write(1)  # A writes its final value -> bumps slot 1
        a.last_run_clock = store.write_clock
        a.runs += 1

    def a_frame(store):
        if a.should_run(store):
            a_run(store)
        if b.should_run(store):
            b.run(store)

    a_frame(store)  # cold: A runs (writes slot 1), B runs (reads slot 1)
    assert a.runs == 1 and b.runs == 1
    a_frame(store)  # nothing upstream changed -> both skip
    assert a.runs == 1 and b.runs == 1

    store.write(0)  # source changes -> A re-runs -> bumps slot 1 -> B re-runs
    a_frame(store)
    assert a.runs == 2 and b.runs == 2


def test_chained_skips_when_upstream_unchanged():
    store = Store(3)
    a = Dataset(reads=[0])
    b = Dataset(reads=[1])
    a.read_slots = [0]
    b.read_slots = [1]
    # A already ran; B already ran. A's final (slot 1) static.
    a.last_run_clock = store.write_clock
    b.last_run_clock = store.write_clock
    store.write(2)  # unrelated
    assert not a.should_run(store)
    assert not b.should_run(store)
