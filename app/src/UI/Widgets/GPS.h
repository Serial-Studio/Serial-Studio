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

#pragma once

#include <QCache>
#include <QHash>
#include <QImage>
#include <QQuickPaintedItem>
#include <QSettings>

class QNetworkReply;
class QNetworkAccessManager;

namespace Misc {
class ThemeManager;
class CommonFonts;
}  // namespace Misc

namespace UI {
class Dashboard;
}  // namespace UI

namespace Widgets {
/**
 * @brief Custom QML widget for displaying GPS position on a tile-based map.
 */
class GPS : public QQuickPaintedItem {
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
  GPS(const int index = -1, QQuickItem* parent = nullptr);
  ~GPS() override;
  void paint(QPainter* painter) override;

  [[nodiscard]] double altitude() const;
  [[nodiscard]] double latitude() const;
  [[nodiscard]] double longitude() const;

  [[nodiscard]] int mapType() const;
  [[nodiscard]] int zoomLevel() const;
  [[nodiscard]] double zoomLevelPrecise() const;

  [[nodiscard]] bool autoCenter() const;
  [[nodiscard]] bool showWeather() const;
  [[nodiscard]] bool plotTrajectory() const;
  [[nodiscard]] bool showNasaWeather() const;

  [[nodiscard]] const QStringList& mapTypes();

public slots:
  void center();
  void setZoomLevel(int zoom);
  void setZoomLevelPrecise(double zoom);
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

private:
  void fetchCloudOverlay();
  void paintMap(QPainter* painter, const QSize& view);
  void paintPathData(QPainter* painter, const QSize& view);
  void paintAttributionText(QPainter* painter, const QSize& view);

private:
  [[nodiscard]] QPointF clampCenterTile(QPointF tile) const;
  QPointF tileToLatLon(const QPointF& tile, double zoom);
  QPointF latLonToTile(double lat, double lon, double zoom);

  [[nodiscard]] QString tileUrl(const int tx, const int ty, const int zoom) const;
  [[nodiscard]] QString referenceUrl(const int tx, const int ty, const int zoom) const;
  [[nodiscard]] QString nasaWeatherUrl(const int tx, const int ty, const int zoom) const;

  void preloadNextZoomTiles(int tx, int ty, int baseZoom);

  static void requestTileIfNeeded(const QString& url);
  static void onTileFetched(QNetworkReply* reply);

  void renderFallbackTile(QPainter* painter, int tx, int ty, int baseZoom, const QRect& targetRect);
  void renderWeatherOverlay(
    QPainter* painter, int wrappedTx, int ty, int baseZoom, const QRect& targetRect);
  void renderReferenceOverlay(
    QPainter* painter, int wrappedTx, int ty, int baseZoom, const QRect& targetRect);
  void renderCloudOverlay(
    QPainter* painter, int wrappedTx, int ty, int baseZoom, const QRect& targetRect);
  void renderTrajectoryPath(QPainter* painter,
                            const QSize& view,
                            int baseZoom,
                            double scale,
                            const QPointF& centerTileBase);
  void renderPositionIndicator(QPainter* painter,
                               const QSize& view,
                               int baseZoom,
                               double scale,
                               const QPointF& centerTileBase);

protected:
  void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  Misc::ThemeManager& m_themeManager;
  UI::Dashboard& m_dashboard;
  Misc::CommonFonts& m_commonFonts;

  double m_zoom;
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
  QVector<QPointF> m_trajectoryScratch;

  QStringList m_mapIDs;
  QStringList m_mapTypes;

  QList<int> m_mapMaxZoom;
  QList<int> m_weatherDays;

  static QList<GPS*> s_instances;
  static QCache<QString, QImage> s_tileCache;
  static QHash<QString, QNetworkReply*> s_pending;
  static QHash<QString, int> s_retryCount;
  static QNetworkAccessManager* s_network;
  static bool s_cacheInitialized;
};
}  // namespace Widgets
