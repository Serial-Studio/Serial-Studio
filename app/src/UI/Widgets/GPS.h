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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QCache>
#include <QImage>
#include <QSettings>
#include <QQuickPaintedItem>
#include <QNetworkAccessManager>

namespace Widgets
{
/**
 * @class Widgets::GPS
 * @brief A custom QML widget for displaying GPS position on a tile-based map.
 *
 * This class implements a standalone, embeddable GPS map viewer using
 * ArcGIS/ESRI tile servers. It handles map tile rendering, zooming,
 * panning, and dynamic GPS data updates. The widget is based on
 * QQuickPaintedItem, requiring no external plugins or QtLocation support.
 *
 * Features:
 * - Fetches and caches tiles during runtime
 * - Supports zoom and drag interaction
 * - Draws an iOS-style indicator at the current GPS location
 * - Integrates with Serial Studio's dashboard system
 *
 * Designed to be lightweight, dependency-free (beyond QtNetwork),
 * and fully embeddable into any QML/QtQuick scene.
 */
class GPS : public QQuickPaintedItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int mapType
             READ mapType
             WRITE setMapType
             NOTIFY mapTypeChanged)
  Q_PROPERTY(bool autoCenter
             READ autoCenter
             WRITE setAutoCenter
             NOTIFY autoCenterChanged)
  Q_PROPERTY(bool showWeather
             READ showWeather
             WRITE setShowWeather
             NOTIFY showWeatherChanged)
  Q_PROPERTY(bool showNasaWeather
             READ showNasaWeather
             WRITE setShowNasaWeather
             NOTIFY showNasaWeatherChanged)
  Q_PROPERTY(bool plotTrajectory
             READ plotTrajectory
             WRITE setPlotTrajectory
             NOTIFY plotTrajectoryChanged)
  Q_PROPERTY(int zoomLevel
             READ zoomLevel
             WRITE setZoomLevel
             NOTIFY zoomLevelChanged)
  Q_PROPERTY(QStringList mapTypes
             READ mapTypes
             CONSTANT)
  // clang-format on

signals:
  void updated();
  void mapTypeChanged();
  void zoomLevelChanged();
  void autoCenterChanged();
  void showWeatherChanged();
  void plotTrajectoryChanged();
  void showNasaWeatherChanged();

public:
  GPS(const int index = -1, QQuickItem *parent = nullptr);
  void paint(QPainter *painter) override;

  [[nodiscard]] double altitude() const;
  [[nodiscard]] double latitude() const;
  [[nodiscard]] double longitude() const;

  [[nodiscard]] int mapType() const;
  [[nodiscard]] int zoomLevel() const;

  [[nodiscard]] bool autoCenter() const;
  [[nodiscard]] bool showWeather() const;
  [[nodiscard]] bool plotTrajectory() const;
  [[nodiscard]] bool showNasaWeather() const;

  [[nodiscard]] const QStringList &mapTypes();

public slots:
  void center();
  void setZoomLevel(int zoom);
  void setMapType(const int type);
  void setAutoCenter(const bool enabled);
  void setShowWeather(const bool enabled);
  void setPlotTrajectory(const bool enabled);
  void setShowNasaWeather(const bool enabled);

private slots:
  void updateData();
  void updateTiles();
  void precacheWorld();
  void onThemeChanged();
  void onTileFetched(QNetworkReply *reply);

private:
  void paintMap(QPainter *painter, const QSize &view);
  void paintPathData(QPainter *painter, const QSize &view);
  void paintAttributionText(QPainter *painter, const QSize &view);

private:
  QPointF clampCenterTile(QPointF tile) const;
  QPointF tileToLatLon(const QPointF &tile, int zoom);
  QPointF latLonToTile(double lat, double lon, int zoom);

  QString tileUrl(const int tx, const int ty, const int zoom) const;
  QString referenceUrl(const int tx, const int ty, const int zoom) const;
  QString nasaWeatherUrl(const int tx, const int ty, const int zoom) const;

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  int m_zoom;
  int m_index;
  int m_mapType;

  bool m_autoCenter;
  bool m_showWeather;
  bool m_plotTrajectory;
  bool m_showNasaWeather;
  bool m_enableReferenceLayer;

  double m_altitude;
  double m_latitude;
  double m_longitude;

  QPointF m_centerTile;
  QPoint m_lastMousePos;
  QColor m_lineHeadColor;
  QColor m_lineTailColor;

  QSettings m_settings;
  QImage m_cloudOverlay;

  QStringList m_mapIDs;
  QStringList m_mapTypes;

  QList<int> m_mapMaxZoom;
  QList<int> m_weatherDays;

  QNetworkAccessManager m_network;
  QCache<QString, QImage> m_tileCache;
  QHash<QString, QNetworkReply *> m_pending;
};
} // namespace Widgets
