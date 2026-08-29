/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QtTest>

#include "UI/Widgets/Painter/PainterEnums.h"

using namespace Widgets::PainterEnums;

//--------------------------------------------------------------------------------------------------
// Canvas2D string <-> Qt enum mappers (spec 0070)
//--------------------------------------------------------------------------------------------------
//
// These five mappers run inside the Canvas widget's per-frame paint() every time a script assigns
// lineCap / lineJoin / globalCompositeOperation, which is why they live header-inline. The suite
// pins the exact table they encode: an unknown or differently-cased name must fall back to the
// Canvas2D default, never to a neighbouring mode.
//
//--------------------------------------------------------------------------------------------------

class TstPainterEnums : public QObject {
  Q_OBJECT

private slots:
  void lineCapMapsKnownNames();
  void lineCapFallsBackToButt();
  void lineCapRoundTrips();
  void lineJoinMapsKnownNames();
  void lineJoinFallsBackToMiter();
  void lineJoinRoundTrips();
  void compositeMapsEveryName();
  void compositeFallsBackToSourceOver();
};

//--------------------------------------------------------------------------------------------------
// Line caps
//--------------------------------------------------------------------------------------------------

void TstPainterEnums::lineCapMapsKnownNames()
{
  QCOMPARE(mapLineCap(QStringLiteral("round")), Qt::RoundCap);
  QCOMPARE(mapLineCap(QStringLiteral("square")), Qt::SquareCap);
  QCOMPARE(mapLineCap(QStringLiteral("butt")), Qt::FlatCap);
}

void TstPainterEnums::lineCapFallsBackToButt()
{
  const QStringList rejected = {QStringLiteral(""),
                                QStringLiteral("Round"),
                                QStringLiteral("ROUND"),
                                QStringLiteral(" round"),
                                QStringLiteral("flat"),
                                QStringLiteral("miter")};

  for (const auto& name : rejected)
    QCOMPARE(mapLineCap(name), Qt::FlatCap);
}

void TstPainterEnums::lineCapRoundTrips()
{
  const QStringList canonical = {
    QStringLiteral("butt"), QStringLiteral("round"), QStringLiteral("square")};

  for (const auto& name : canonical)
    QCOMPARE(unmapLineCap(mapLineCap(name)), name);
}

//--------------------------------------------------------------------------------------------------
// Line joins
//--------------------------------------------------------------------------------------------------

void TstPainterEnums::lineJoinMapsKnownNames()
{
  QCOMPARE(mapLineJoin(QStringLiteral("round")), Qt::RoundJoin);
  QCOMPARE(mapLineJoin(QStringLiteral("bevel")), Qt::BevelJoin);
  QCOMPARE(mapLineJoin(QStringLiteral("miter")), Qt::MiterJoin);
}

void TstPainterEnums::lineJoinFallsBackToMiter()
{
  const QStringList rejected = {QStringLiteral(""),
                                QStringLiteral("Bevel"),
                                QStringLiteral("BEVEL"),
                                QStringLiteral("square"),
                                QStringLiteral("mitre")};

  for (const auto& name : rejected)
    QCOMPARE(mapLineJoin(name), Qt::MiterJoin);
}

void TstPainterEnums::lineJoinRoundTrips()
{
  const QStringList canonical = {
    QStringLiteral("miter"), QStringLiteral("round"), QStringLiteral("bevel")};

  for (const auto& name : canonical)
    QCOMPARE(unmapLineJoin(mapLineJoin(name)), name);
}

//--------------------------------------------------------------------------------------------------
// Composition modes
//--------------------------------------------------------------------------------------------------

void TstPainterEnums::compositeMapsEveryName()
{
  struct Row {
    const char* name;
    QPainter::CompositionMode mode;
  };

  static const Row rows[] = {
    {     "source-over",      QPainter::CompositionMode_SourceOver},
    {       "source-in",        QPainter::CompositionMode_SourceIn},
    {      "source-out",       QPainter::CompositionMode_SourceOut},
    {     "source-atop",      QPainter::CompositionMode_SourceAtop},
    {"destination-over", QPainter::CompositionMode_DestinationOver},
    {  "destination-in",   QPainter::CompositionMode_DestinationIn},
    { "destination-out",  QPainter::CompositionMode_DestinationOut},
    {"destination-atop", QPainter::CompositionMode_DestinationAtop},
    {         "lighter",            QPainter::CompositionMode_Plus},
    {            "copy",          QPainter::CompositionMode_Source},
    {             "xor",             QPainter::CompositionMode_Xor},
    {        "multiply",        QPainter::CompositionMode_Multiply},
    {          "screen",          QPainter::CompositionMode_Screen},
    {         "overlay",         QPainter::CompositionMode_Overlay},
    {          "darken",          QPainter::CompositionMode_Darken},
    {         "lighten",         QPainter::CompositionMode_Lighten},
    {     "color-dodge",      QPainter::CompositionMode_ColorDodge},
    {      "color-burn",       QPainter::CompositionMode_ColorBurn},
    {      "hard-light",       QPainter::CompositionMode_HardLight},
    {      "soft-light",       QPainter::CompositionMode_SoftLight},
    {      "difference",      QPainter::CompositionMode_Difference},
    {       "exclusion",       QPainter::CompositionMode_Exclusion},
  };

  for (const auto& row : rows)
    QCOMPARE(mapComposite(QString::fromLatin1(row.name)), row.mode);
}

void TstPainterEnums::compositeFallsBackToSourceOver()
{
  const QStringList rejected = {QStringLiteral(""),
                                QStringLiteral("Multiply"),
                                QStringLiteral("source over"),
                                QStringLiteral("source_over"),
                                QStringLiteral("plus"),
                                QStringLiteral("hue")};

  for (const auto& name : rejected)
    QCOMPARE(mapComposite(name), QPainter::CompositionMode_SourceOver);
}

QTEST_GUILESS_MAIN(TstPainterEnums)
#include "tst_painter_enums.moc"
