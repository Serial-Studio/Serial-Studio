/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <QString>
#include <QStringView>
#include <QtGlobal>
#include <vector>

/**
 * @brief Compiled arithmetic transforms (spec 0060): a per-dataset expression over `v` (this
 *        sample), `t` (seconds), `n` (sample index), `dt`, sibling datasets by name and a
 *        bounded history accessor `sample(name, k)`, compiled once into a flat postfix program
 *        and evaluated per sample with a fixed-depth stack, no allocation and no script engine.
 *        Both lanes share the same evaluator, so results are bit-identical by construction.
 */
namespace DataModel::Expression {

/**
 * @brief Deepest history `sample(name, k)` may reach; the per-slot ring holds this many values.
 */
inline constexpr int kMaxHistory = 256;

/**
 * @brief Upper bound on compiled program length; a longer expression is a compile error.
 */
inline constexpr int kMaxNodes = 512;

/**
 * @brief Evaluation stack depth; a program needing more is a compile error.
 */
inline constexpr int kMaxStackDepth = 64;

/**
 * @brief Postfix opcodes. Binary and unary operators pop their operands and push one result;
 *        `Select` pops condition, then-value, else-value; `Sample` pops k and pushes history.
 */
enum class Op : std::uint8_t {
  PushConst,
  PushV,
  PushT,
  PushN,
  PushDt,
  PushSlot,
  Sample,
  PushTable,
  Neg,
  Not,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Pow,
  Lt,
  Le,
  Gt,
  Ge,
  Eq,
  Ne,
  And,
  Or,
  Select,
  Call1,
  Call2,
  Call3
};

/**
 * @brief One program node: opcode, an integer argument (slot index or function id) and the
 *        constant payload for `PushConst`.
 */
struct Node {
  Op op;
  std::int32_t arg;
  double value;
};

/**
 * @brief A compiled expression. `PushSlot` / `Sample` nodes carry the slot the resolver handed
 *        out at compile time, i.e. an index into the owner's per-source SlotTable.
 */
struct Program {
  std::vector<Node> code;
  int stackDepth;
  int maxSlot;
  bool usesHistory;

  Program() : code(), stackDepth(0), maxSlot(-1), usesHistory(false) {}

  [[nodiscard]] bool valid() const noexcept { return !code.empty(); }
};

/**
 * @brief Maps a dataset name to a SlotTable slot (or -1 when unknown) during compilation. Names
 *        are matched against the sibling datasets of the same source, exact and case-sensitive.
 */
using NameResolver = std::function<int(QStringView name)>;

/**
 * @brief Maps a `table(name, register)` pair to a DataTableStore handle (or -1 when unknown)
 *        during compilation. An empty resolver means tables are unreachable from this lane, and
 *        `table()` is then rejected with its own message instead of an unknown-register error.
 */
using TableResolver = std::function<qint64(QStringView table, QStringView reg)>;

/**
 * @brief Compiles @p text into @p out. On failure returns false, leaves @p out invalid and puts
 *        a one-line reason (with column) into @p error.
 */
[[nodiscard]] bool compile(const QString& text,
                           const NameResolver& resolver,
                           Program& out,
                           QString& error);

/**
 * @brief compile() with read-only data-table access: `table(name, register)` resolves through
 *        @p tables to a store handle, baked into the program the same way slots are.
 */
[[nodiscard]] bool compile(const QString& text,
                           const NameResolver& resolver,
                           const TableResolver& tables,
                           Program& out,
                           QString& error);

/**
 * @brief Read-only view of the values a program can reach while evaluating one sample:
 *        `slotValues[slot]` is the latest published value of that slot and `history(slot, k)`
 *        answers from the owner's rings (NaN when unavailable).
 */
struct Context {
  double v;
  double t;
  double n;
  double dt;
  const double* slotValues;
  std::size_t slotCount;
  const void* historyOwner;
  double (*history)(const void* owner, int slot, int k);
  const void* tableOwner;
  double (*tableValue)(const void* owner, qint64 handle);
};

/**
 * @brief Runs @p program over @p ctx; returns NaN on an inconsistent program (never throws,
 *        never allocates). Bounded by the program length.
 */
[[nodiscard]] double evaluate(const Program& program, const Context& ctx) noexcept;

/**
 * @brief Per-source table of the datasets any expression of that source refers to: the latest
 *        published value per slot plus a bounded history ring. Slots are handed out at compile
 *        time; publish() and sample() are O(1) and allocation-free. Single owner thread.
 */
class SlotTable {
public:
  SlotTable();

  [[nodiscard]] int slotFor(int uniqueId);
  [[nodiscard]] int slotOf(int uniqueId) const noexcept;
  [[nodiscard]] int slotCount() const noexcept;
  [[nodiscard]] int uniqueIdAt(int slot) const noexcept;
  [[nodiscard]] const double* latestValues() const noexcept;
  [[nodiscard]] double sample(int slot, int k) const noexcept;
  [[nodiscard]] bool empty() const noexcept;

  void publish(int uniqueId, double value) noexcept;
  void publishSlot(int slot, double value) noexcept;
  void reset() noexcept;

  [[nodiscard]] static double historyThunk(const void* owner, int slot, int k) noexcept;

private:
  struct Ring {
    std::vector<double> values;
    std::size_t head;
    std::size_t fill;
  };

  std::vector<int> m_slotByUniqueId;
  std::vector<int> m_uniqueIdOfSlot;
  std::vector<double> m_latest;
  std::vector<Ring> m_rings;
};

/**
 * @brief Per-dataset evaluation state shared by both lanes: the program, its `n` counter and
 *        the previous `t` for `dt`. `run()` builds the Context from a SlotTable and evaluates.
 */
struct Runtime {
  Program program;
  double lastT;
  std::uint64_t count;
  const void* tableOwner;
  double (*tableValue)(const void* owner, qint64 handle);

  Runtime() : program(), lastT(0.0), count(0), tableOwner(nullptr), tableValue(nullptr) {}

  [[nodiscard]] double run(double v, double t, const SlotTable& table) noexcept;
  void reset() noexcept;
};

}  // namespace DataModel::Expression
