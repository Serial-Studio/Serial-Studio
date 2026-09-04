/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QPainter>
#  include <QString>

/**
 * @brief String<->enum mappers for the Canvas2D drawing state.
 *
 * Header-inline on purpose: every one of these runs inside the widget's per-frame paint()
 * whenever a script assigns lineCap/lineJoin/globalCompositeOperation, so the lookups have to
 * stay visible to the caller's translation unit.
 */
namespace Widgets::PainterEnums {

/**
 * @brief Maps Canvas2D line-cap names to Qt::PenCapStyle.
 */
[[nodiscard]] inline Qt::PenCapStyle mapLineCap(const QString& cap)
{
  if (cap == QLatin1String("round"))
    return Qt::RoundCap;

  if (cap == QLatin1String("square"))
    return Qt::SquareCap;

  return Qt::FlatCap;
}

/**
 * @brief Maps Qt::PenCapStyle back to its Canvas2D name.
 */
[[nodiscard]] inline QString unmapLineCap(Qt::PenCapStyle cap)
{
  switch (cap) {
    case Qt::RoundCap:
      return QStringLiteral("round");
    case Qt::SquareCap:
      return QStringLiteral("square");
    case Qt::FlatCap:
    default:
      return QStringLiteral("butt");
  }
}

/**
 * @brief Maps Canvas2D line-join names to Qt::PenJoinStyle.
 */
[[nodiscard]] inline Qt::PenJoinStyle mapLineJoin(const QString& join)
{
  if (join == QLatin1String("round"))
    return Qt::RoundJoin;

  if (join == QLatin1String("bevel"))
    return Qt::BevelJoin;

  return Qt::MiterJoin;
}

/**
 * @brief Maps Qt::PenJoinStyle back to its Canvas2D name.
 */
[[nodiscard]] inline QString unmapLineJoin(Qt::PenJoinStyle join)
{
  switch (join) {
    case Qt::RoundJoin:
      return QStringLiteral("round");
    case Qt::BevelJoin:
      return QStringLiteral("bevel");
    case Qt::MiterJoin:
    default:
      return QStringLiteral("miter");
  }
}

/**
 * @brief Maps a Canvas2D globalCompositeOperation name to QPainter::CompositionMode.
 */
[[nodiscard]] inline QPainter::CompositionMode mapComposite(const QString& op)
{
  static const struct {
    QLatin1String name;
    QPainter::CompositionMode mode;
  } table[] = {
    {     QLatin1String("source-over"),      QPainter::CompositionMode_SourceOver},
    {       QLatin1String("source-in"),        QPainter::CompositionMode_SourceIn},
    {      QLatin1String("source-out"),       QPainter::CompositionMode_SourceOut},
    {     QLatin1String("source-atop"),      QPainter::CompositionMode_SourceAtop},
    {QLatin1String("destination-over"), QPainter::CompositionMode_DestinationOver},
    {  QLatin1String("destination-in"),   QPainter::CompositionMode_DestinationIn},
    { QLatin1String("destination-out"),  QPainter::CompositionMode_DestinationOut},
    {QLatin1String("destination-atop"), QPainter::CompositionMode_DestinationAtop},
    {         QLatin1String("lighter"),            QPainter::CompositionMode_Plus},
    {            QLatin1String("copy"),          QPainter::CompositionMode_Source},
    {             QLatin1String("xor"),             QPainter::CompositionMode_Xor},
    {        QLatin1String("multiply"),        QPainter::CompositionMode_Multiply},
    {          QLatin1String("screen"),          QPainter::CompositionMode_Screen},
    {         QLatin1String("overlay"),         QPainter::CompositionMode_Overlay},
    {          QLatin1String("darken"),          QPainter::CompositionMode_Darken},
    {         QLatin1String("lighten"),         QPainter::CompositionMode_Lighten},
    {     QLatin1String("color-dodge"),      QPainter::CompositionMode_ColorDodge},
    {      QLatin1String("color-burn"),       QPainter::CompositionMode_ColorBurn},
    {      QLatin1String("hard-light"),       QPainter::CompositionMode_HardLight},
    {      QLatin1String("soft-light"),       QPainter::CompositionMode_SoftLight},
    {      QLatin1String("difference"),      QPainter::CompositionMode_Difference},
    {       QLatin1String("exclusion"),       QPainter::CompositionMode_Exclusion},
  };

  for (const auto& e : table)
    if (op == e.name)
      return e.mode;

  return QPainter::CompositionMode_SourceOver;
}

}  // namespace Widgets::PainterEnums

#endif  // BUILD_COMMERCIAL
