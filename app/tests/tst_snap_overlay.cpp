/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QRect>
#include <QSignalSpy>
#include <QSize>
#include <QTest>

#include "UI/SnapGuides.h"
#include "UI/WindowManager/SnapOverlay.h"

// Every test function here builds its own overlay: no state is carried between slots, so Qt
// Test's declaration-order execution is never load-bearing.

/**
 * @brief Pins the publication contract of the gesture overlay: what each publisher stores, which
 *        change signal it raises, and that re-publishing an unchanged value stays silent.
 */
class TstSnapOverlay : public QObject {
  Q_OBJECT

private slots:
  void idleState();

  void manualGesture_publishesOnce();
  void manualGesture_clearResets();

  void snapGuides_publishVisuals();
  void snapGuides_republishIsSilent();
  void snapGuides_clearEmitsOnce();

  void fractionPreview_disabledStaysEmpty();
  void fractionPreview_labelsBothAxes();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a snap result carrying one guide line, one spacing gap and a size match.
 */
[[nodiscard]] static UI::Snap::SnapResult sampleResult()
{
  UI::Snap::SnapResult result;
  result.rect      = QRect(10, 10, 100, 100);
  result.sizeMatch = QRect(200, 10, 100, 100);
  result.guides.append({QRect(10, 0, 1, 400), UI::Snap::GuideKind::Center});
  result.spacings.append({QRect(110, 40, 90, 1), 90});
  return result;
}

//--------------------------------------------------------------------------------------------------
// Idle state
//--------------------------------------------------------------------------------------------------

/**
 * @brief A fresh overlay publishes nothing: QML binds these properties before any gesture runs.
 */
void TstSnapOverlay::idleState()
{
  UI::SnapOverlay overlay;

  QVERIFY(!overlay.manualGestureActive());
  QVERIFY(!overlay.sizeMatchVisible());
  QVERIFY(overlay.alignmentGuides().isEmpty());
  QVERIFY(overlay.spacingIndicators().isEmpty());
  QVERIFY(overlay.snapSiblings().isEmpty());
  QVERIFY(overlay.fractionPreviewLabel().isEmpty());
  QVERIFY(!overlay.fractionPreviewRect().isValid());
}

//--------------------------------------------------------------------------------------------------
// Manual gesture badge
//--------------------------------------------------------------------------------------------------

/**
 * @brief The badge follows the gesture, and a move that resolves to the same geometry must not
 *        re-notify: this runs per mouse-move event.
 */
void TstSnapOverlay::manualGesture_publishesOnce()
{
  UI::SnapOverlay overlay;
  QSignalSpy spy(&overlay, &UI::SnapOverlay::manualGestureChanged);

  overlay.publishManualGesture(QRect(5, 5, 50, 50));
  QCOMPARE(spy.count(), 1);
  QVERIFY(overlay.manualGestureActive());
  QCOMPARE(overlay.manualGestureGeometry(), QRect(5, 5, 50, 50));

  overlay.publishManualGesture(QRect(5, 5, 50, 50));
  QCOMPARE(spy.count(), 1);

  overlay.publishManualGesture(QRect(6, 5, 50, 50));
  QCOMPARE(spy.count(), 2);
  QCOMPARE(overlay.manualGestureGeometry(), QRect(6, 5, 50, 50));
}

/**
 * @brief Ending a gesture hides the badge exactly once; a second clear is silent.
 */
void TstSnapOverlay::manualGesture_clearResets()
{
  UI::SnapOverlay overlay;
  overlay.publishManualGesture(QRect(5, 5, 50, 50));

  QSignalSpy spy(&overlay, &UI::SnapOverlay::manualGestureChanged);
  overlay.clearManualGesture();
  QCOMPARE(spy.count(), 1);
  QVERIFY(!overlay.manualGestureActive());

  overlay.clearManualGesture();
  QCOMPARE(spy.count(), 1);
}

//--------------------------------------------------------------------------------------------------
// Snap guides
//--------------------------------------------------------------------------------------------------

/**
 * @brief A snap resolution is serialized into the variant shape the QML overlay consumes, one
 *        entry per guide and per spacing gap.
 */
void TstSnapOverlay::snapGuides_publishVisuals()
{
  UI::SnapOverlay overlay;
  overlay.publishSnapGuides(sampleResult());

  QCOMPARE(overlay.alignmentGuides().size(), 1);
  QCOMPARE(overlay.spacingIndicators().size(), 1);
  QVERIFY(overlay.sizeMatchVisible());
  QCOMPARE(overlay.sizeMatchRect(), QRect(200, 10, 100, 100));

  const auto guide = overlay.alignmentGuides().first().toMap();
  QCOMPARE(guide.value(QStringLiteral("x")).toInt(), 10);
  QCOMPARE(guide.value(QStringLiteral("height")).toInt(), 400);
  QCOMPARE(guide.value(QStringLiteral("center")).toBool(), true);

  const auto spacing = overlay.spacingIndicators().first().toMap();
  QCOMPARE(spacing.value(QStringLiteral("gap")).toInt(), 90);
}

/**
 * @brief Re-publishing an identical resolution notifies nobody, which is what keeps a drag that
 *        holds one snap from re-evaluating the overlay bindings on every mouse move.
 */
void TstSnapOverlay::snapGuides_republishIsSilent()
{
  UI::SnapOverlay overlay;
  overlay.publishSnapGuides(sampleResult());

  QSignalSpy guides(&overlay, &UI::SnapOverlay::alignmentGuidesChanged);
  QSignalSpy spacings(&overlay, &UI::SnapOverlay::spacingIndicatorsChanged);
  QSignalSpy sizeMatch(&overlay, &UI::SnapOverlay::sizeMatchRectChanged);

  overlay.publishSnapGuides(sampleResult());
  QCOMPARE(guides.count(), 0);
  QCOMPARE(spacings.count(), 0);
  QCOMPARE(sizeMatch.count(), 0);
}

/**
 * @brief Clearing drops all three visuals with one notification each, and a second clear is
 *        silent because there is nothing left to drop.
 */
void TstSnapOverlay::snapGuides_clearEmitsOnce()
{
  UI::SnapOverlay overlay;
  overlay.publishSnapGuides(sampleResult());

  QSignalSpy guides(&overlay, &UI::SnapOverlay::alignmentGuidesChanged);
  QSignalSpy spacings(&overlay, &UI::SnapOverlay::spacingIndicatorsChanged);
  QSignalSpy sizeMatch(&overlay, &UI::SnapOverlay::sizeMatchRectChanged);

  overlay.clearSnapGuides();
  QCOMPARE(guides.count(), 1);
  QCOMPARE(spacings.count(), 1);
  QCOMPARE(sizeMatch.count(), 1);
  QVERIFY(overlay.alignmentGuides().isEmpty());
  QVERIFY(overlay.spacingIndicators().isEmpty());
  QVERIFY(!overlay.sizeMatchVisible());

  overlay.clearSnapGuides();
  QCOMPARE(guides.count(), 1);
  QCOMPARE(spacings.count(), 1);
  QCOMPARE(sizeMatch.count(), 1);
}

//--------------------------------------------------------------------------------------------------
// Fraction preview
//--------------------------------------------------------------------------------------------------

/**
 * @brief With smart guides off the preview never appears, whatever the geometry resolves to.
 */
void TstSnapOverlay::fractionPreview_disabledStaysEmpty()
{
  UI::SnapOverlay overlay;
  QSignalSpy spy(&overlay, &UI::SnapOverlay::fractionPreviewChanged);

  overlay.publishFractionPreview(QRect(0, 0, 500, 400), QSize(1000, 800), false);
  QCOMPARE(spy.count(), 0);
  QVERIFY(overlay.fractionPreviewLabel().isEmpty());
  QVERIFY(!overlay.fractionPreviewRect().isValid());
}

/**
 * @brief A window sitting on a canvas fraction on both axes gets a footprint and a label naming
 *        both of them; the label describes the size, never the position.
 */
void TstSnapOverlay::fractionPreview_labelsBothAxes()
{
  UI::SnapOverlay overlay;
  QSignalSpy spy(&overlay, &UI::SnapOverlay::fractionPreviewChanged);

  overlay.publishFractionPreview(QRect(0, 0, 500, 400), QSize(1000, 800), true);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(overlay.fractionPreviewRect(), QRect(0, 0, 500, 400));
  QVERIFY(overlay.fractionPreviewLabel().contains(QStringLiteral("1/2")));

  overlay.publishFractionPreview(QRect(0, 0, 500, 400), QSize(1000, 800), true);
  QCOMPARE(spy.count(), 1);

  overlay.clearSnapGuides();
  QCOMPARE(spy.count(), 2);
  QVERIFY(overlay.fractionPreviewLabel().isEmpty());
}

QTEST_GUILESS_MAIN(TstSnapOverlay)

#include "tst_snap_overlay.moc"
