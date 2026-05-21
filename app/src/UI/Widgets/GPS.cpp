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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/GPS.h"

#include <QCursor>
#include <QLinearGradient>
#include <QNetworkReply>
#include <QPainter>
#include <QPainterPath>

#include "DSP.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/Utilities.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Global parameters
//--------------------------------------------------------------------------------------------------

// clang-format off
constexpr int MIN_ZOOM = 2;
constexpr int WEATHER_MAX_ZOOM = 6;
constexpr int WEATHER_GIBS_MAX_ZOOM = 9;
constexpr auto CLOUD_URL = "https://clouds.matteason.co.uk/images/4096x2048/clouds-alpha.png";
constexpr double kInvTile = 1.0 / 256.0;
constexpr double kInv360  = 1.0 / 360.0;
constexpr double kInv180  = 1.0 / 180.0;
constexpr double kInv120  = 1.0 / 120.0;
constexpr double kInvPi   = 1.0 / M_PI;
// clang-format on

QCache<QString, QImage> Widgets::GPS::s_tileCache;
QHash<QString, QNetworkReply*> Widgets::GPS::s_pending;
bool Widgets::GPS::s_cacheInitialized = false;

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a GPS map widget using ArcGIS Maps tile data.
 */
Widgets::GPS::GPS(const int index, QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_zoom(MIN_ZOOM)
  , m_index(index)
  , m_mapType(0)
  , m_autoCenter(true)
  , m_showWeather(false)
  , m_plotTrajectory(true)
  , m_showNasaWeather(false)
  , m_enableReferenceLayer(false)
  , m_altitude(0)
  , m_latitude(0)
  , m_longitude(0)
{
  // Configure item flags
  setMipmap(true);
  setOpaquePainting(true);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemHasContents, true);

  // Configure shared tile cache (once across all instances)
  if (!s_cacheInitialized) {
    s_tileCache.setMaxCost(2048);
    s_cacheInitialized = true;
  }

  // Set user-friendly map names
  m_mapTypes << tr("Satellite Imagery") << tr("Satellite Imagery with Labels") << tr("Street Map")
             << tr("Topographic Map") << tr("Terrain") << tr("Light Gray Canvas")
             << tr("Dark Gray Canvas") << tr("National Geographic");

  // Set URL/tile server map type IDs
  m_mapIDs << "World_Imagery" << "World_Imagery" << "World_Street_Map"
           << "World_Topo_Map" << "World_Terrain_Base"
           << "Canvas/World_Light_Gray_Base" << "Canvas/World_Dark_Gray_Base"
           << "NatGeo_World_Map";

  // Set max zoom for each map type
  m_mapMaxZoom << 18 << 18 << 18 << 18 << 9 << 16 << 16 << 16;

  // Read settings
  setMapType(m_settings.value("gpsMapType", 0).toInt());
  m_showWeather     = m_settings.value("gpsWeather", false).toBool();
  m_autoCenter      = m_settings.value("gpsAutoCenter", true).toBool();
  m_showNasaWeather = m_settings.value("gpsNasaWeather", false).toBool();
  m_plotTrajectory  = m_settings.value("gpsPlotTrajectory", true).toBool();
  m_zoom            = qBound(
    static_cast<double>(MIN_ZOOM), m_settings.value("gpsZoomLevel", MIN_ZOOM).toDouble(), 18.0);

  // Only enable one of the weather overlays
  if (m_showNasaWeather && m_showWeather)
    m_showWeather = false;

  // Download cloud map
  if (m_showWeather) {
    QUrl cloudUrl(CLOUD_URL);
    QNetworkRequest request(cloudUrl);
    QNetworkReply* reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
      reply->deleteLater();

      if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QImage image;
        if (image.loadFromData(imageData)) {
          m_cloudOverlay = image;
          update();
        }
      }
    });
  }

  // Connect to the theme manager to update the colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &Widgets::GPS::onThemeChanged);

  // Configure signals/slots with the dashboard
  if (VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index)) {
    updateData();
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Widgets::GPS::updateData);
    center();
  }
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the GPS widget onto the screen.
 */
void Widgets::GPS::paint(QPainter* painter)
{
  // No need to update if widget is not enabled
  if (!isEnabled())
    return;

  // No need to update if widget is invalid
  if (!VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
    return;

  // Configure painter hints
  painter->setRenderHint(QPainter::Antialiasing);

  const QSize viewport = size().toSize();

  // Paint widget data
  paintMap(painter, viewport);
  paintPathData(painter, viewport);
  paintAttributionText(painter, viewport);
}

//--------------------------------------------------------------------------------------------------
// Position & map getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current GPS altitude.
 */
double Widgets::GPS::altitude() const
{
  return m_altitude;
}

/**
 * @brief Returns the current GPS latitude.
 */
double Widgets::GPS::latitude() const
{
  return m_latitude;
}

/**
 * @brief Returns the current GPS longitude.
 */
double Widgets::GPS::longitude() const
{
  return m_longitude;
}

/**
 * @brief Returns the currently selected map type index.
 */
int Widgets::GPS::mapType() const
{
  return m_mapType;
}

/**
 * @brief Returns the current zoom level.
 */
int Widgets::GPS::zoomLevel() const
{
  return qRound(m_zoom);
}

/**
 * @brief Returns the precise (floating-point) zoom level.
 */
double Widgets::GPS::zoomLevelPrecise() const
{
  return m_zoom;
}

/**
 * @brief Returns @c true if the map shall be centered on the coordinate
 *        every time it's updated.
 */
bool Widgets::GPS::autoCenter() const
{
  return m_autoCenter;
}

/**
 * @brief Returns the current state of the weather overlay visibility flag.
 */
bool Widgets::GPS::showWeather() const
{
  return m_showWeather;
}

/**
 * @brief Indicates whether the map should display the GPS trajectory.
 */
bool Widgets::GPS::plotTrajectory() const
{
  return m_plotTrajectory;
}

/**
 * @brief Returns the current state of the weather overlay visibility flag.
 */
bool Widgets::GPS::showNasaWeather() const
{
  return m_showNasaWeather;
}

/**
 * @brief Provides the list of available user-friendly map type names.
 */
const QStringList& Widgets::GPS::mapTypes()
{
  return m_mapTypes;
}

//--------------------------------------------------------------------------------------------------
// Navigation control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-centers the map to the latest GPS coordinates.
 */
void Widgets::GPS::center()
{
  m_centerTile = latLonToTile(m_latitude, m_longitude, m_zoom);
  updateTiles();
  update();
}

//--------------------------------------------------------------------------------------------------
// Map configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the map zoom level, preserving viewport center if auto-centering is disabled.
 */
void Widgets::GPS::setZoomLevel(int zoom)
{
  // Bound zoom to valid zoom level
  const auto z = qBound(MIN_ZOOM, zoom, m_mapMaxZoom[m_mapType]);

  // Skip if zoom is the same
  if (m_zoom == z)
    return;

  // Set zoom level and center map on viewport center
  if (!autoCenter()) {
    // Compute which lat/lon is currently at the center of the widget
    const QPointF pixelCenter(width() * 0.5, height() * 0.5);
    const QPointF tileCenter = m_centerTile
                             + QPointF((pixelCenter.x() - width() * 0.5) * kInvTile,
                                       (pixelCenter.y() - height() * 0.5) * kInvTile);

    // Convert screen center tile to geographic lat/lon
    const QPointF geo = tileToLatLon(tileCenter, m_zoom);

    // Update zoom, then reproject lat/lon to new zoom level
    m_zoom                = z;
    const QPointF newTile = latLonToTile(geo.x(), geo.y(), m_zoom);

    // Set new center tile so that geo stays centered in the widget
    m_centerTile = clampCenterTile(newTile);

    // Redraw the widget
    updateTiles();
    update();
  }

  // Set the zoom level and let center() fix the world for us
  else {
    m_zoom = z;
    center();
  }

  // Persist zoom level
  m_settings.setValue(QStringLiteral("gpsZoomLevel"), m_zoom);

  // Emit signal
  Q_EMIT zoomLevelChanged();
}

/**
 * @brief Sets the zoom level with fractional precision for smooth zooming.
 */
void Widgets::GPS::setZoomLevelPrecise(double zoom)
{
  // Bound zoom to valid zoom level
  const auto z =
    qBound(static_cast<double>(MIN_ZOOM), zoom, static_cast<double>(m_mapMaxZoom[m_mapType]));

  // Skip if zoom is effectively the same (within small epsilon)
  if (qAbs(m_zoom - z) < 0.001)
    return;

  // Set zoom level and center map on viewport center
  if (!autoCenter()) {
    // Compute which lat/lon is currently at the center of the widget
    const QPointF pixelCenter(width() * 0.5, height() * 0.5);
    const QPointF tileCenter = m_centerTile
                             + QPointF((pixelCenter.x() - width() * 0.5) * kInvTile,
                                       (pixelCenter.y() - height() * 0.5) * kInvTile);

    // Convert screen center tile to geographic lat/lon
    const QPointF geo = tileToLatLon(tileCenter, m_zoom);

    // Update zoom, then reproject lat/lon to new zoom level
    m_zoom                = z;
    const QPointF newTile = latLonToTile(geo.x(), geo.y(), m_zoom);

    // Set new center tile so that geo stays centered in the widget
    m_centerTile = clampCenterTile(newTile);

    // Redraw the widget
    updateTiles();
    update();
  }

  // Set the zoom level and let center() fix the world for us
  else {
    m_zoom = z;
    center();
  }

  // Persist zoom level
  m_settings.setValue(QStringLiteral("gpsZoomLevel"), m_zoom);

  // Emit signal
  Q_EMIT zoomLevelChanged();
}

/**
 * @brief Sets the active map type to be rendered.
 */
void Widgets::GPS::setMapType(const int type)
{
  // Clamp the index and apply the new map type if it changed
  auto mapId = qBound(0, type, m_mapTypes.count() - 1);
  if (m_mapType != mapId) {
#ifdef BUILD_COMMERCIAL
    const auto& tk = Licensing::CommercialToken::current();
    const bool proMaps =
      tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Hobbyist;
#else
    const bool proMaps = false;
#endif
    if (!proMaps && mapId > 0) {
      mapId = 0;
      Misc::Utilities::showMessageBox(
        tr("Additional map layers are available only for Pro users."),
        tr("We can't offer unrestricted access because the ArcGIS API key incurs real costs."),
        QMessageBox::Information);
    }

    m_mapType = mapId;
    m_settings.setValue(QStringLiteral("gpsMapType"), mapId);

    if (mapId == 1)
      m_enableReferenceLayer = true;
    else
      m_enableReferenceLayer = false;

    if (m_zoom > m_mapMaxZoom[mapId])
      m_zoom = m_mapMaxZoom[mapId];

    updateTiles();
    precacheWorld();
    updateData();
    update();

    Q_EMIT mapTypeChanged();
  }
}

/**
 * @brief Enables/disables centering the map on the latest coordinate.
 */
void Widgets::GPS::setAutoCenter(const bool enabled)
{
  if (m_autoCenter != enabled) {
    m_autoCenter = enabled;
    m_settings.setValue(QStringLiteral("gpsAutoCenter"), enabled);

    if (enabled)
      center();

    Q_EMIT autoCenterChanged();
  }
}

/**
 * @brief Enables or disables the display of weather overlay image from the Live Cloud Maps API.
 */
void Widgets::GPS::setShowWeather(const bool enabled)
{
  // Toggle the cloud overlay and download the image if needed
  if (m_showWeather != enabled) {
    m_showWeather = enabled;
    m_settings.setValue(QStringLiteral("gpsWeather"), enabled);

    if (enabled && showNasaWeather())
      setShowNasaWeather(false);
    else
      update();

    if (enabled && m_cloudOverlay.isNull()) {
      QUrl cloudUrl(CLOUD_URL);
      QNetworkRequest request(cloudUrl);
      QNetworkReply* reply = m_network.get(request);
      connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
          QByteArray imageData = reply->readAll();
          QImage image;
          if (image.loadFromData(imageData)) {
            m_cloudOverlay = image;
            update();
          }
        }
      });
    }

    Q_EMIT showWeatherChanged();
  }
}

/**
 * @brief Enables or disables the trajectory visualization on the map.
 */
void Widgets::GPS::setPlotTrajectory(const bool enabled)
{
  if (m_plotTrajectory != enabled) {
    m_plotTrajectory = enabled;
    m_settings.setValue(QStringLiteral("gpsPlotTrajectory"), enabled);

    update();
    Q_EMIT plotTrajectoryChanged();
  }
}

/**
 * @brief Enables or disables the display of weather overlay tiles from NASA GIBS.
 */
void Widgets::GPS::setShowNasaWeather(const bool enabled)
{
  if (m_showNasaWeather != enabled) {
    m_showNasaWeather = enabled;
    m_settings.setValue(QStringLiteral("gpsNasaWeather"), enabled);

    updateTiles();
    if (enabled && showWeather())
      setShowWeather(false);
    else
      update();

    Q_EMIT showNasaWeatherChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates GPS data from the dashboard model.
 */
void Widgets::GPS::updateData()
{
  // No need to update if widget is not enabled
  if (!isEnabled())
    return;

  // No need to update if widget is invalid
  if (!VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
    return;

  // Obtain series data
  const auto& series = UI::Dashboard::instance().gpsSeries(m_index);
  if (series.latitudes.empty() || series.longitudes.empty() || series.altitudes.empty())
    return;

  // Grab most recent GPS values
  const double alt = series.altitudes.back();
  const double lat = series.latitudes.back();
  const double lon = series.longitudes.back();

  // Stop update if data is invalid
  if (std::isnan(alt) && std::isnan(lat) && std::isnan(lon))
    return;

  // Stop update if data did not change
  if (DSP::almostEqual(lat, m_latitude) && DSP::almostEqual(lon, m_longitude)
      && DSP::almostEqual(alt, m_altitude))
    return;

  // Update tracking values
  if (!std::isnan(lat))
    m_latitude = lat;

  if (!std::isnan(lon))
    m_longitude = lon;

  if (!std::isnan(alt))
    m_altitude = alt;

  // Auto-center (redraws widget)
  if (autoCenter())
    center();

  // Redraw widget
  else {
    update();
    updateTiles();
  }

  // Update UI
  Q_EMIT updated();
}

//--------------------------------------------------------------------------------------------------
// Tile management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Downloads a tile if it is not already cached or pending.
 */
void Widgets::GPS::requestTileIfNeeded(const QString& url)
{
  if (s_tileCache.contains(url) || s_pending.contains(url))
    return;

  QNetworkReply* reply = m_network.get(QNetworkRequest(QUrl(url)));
  s_pending[url]       = reply;
  connect(reply, &QNetworkReply::finished, this, [=, this]() { onTileFetched(reply); });
}

/**
 * @brief Preloads the four sub-tiles for the next zoom level.
 */
void Widgets::GPS::preloadNextZoomTiles(int tx, int ty, int baseZoom)
{
  const int nextZoom = baseZoom + 1;
  if (nextZoom > m_mapMaxZoom[m_mapType])
    return;

  const int tx_next = tx * 2;
  const int ty_next = ty * 2;
  for (int sx = 0; sx < 2; ++sx)
    for (int sy = 0; sy < 2; ++sy)
      requestTileIfNeeded(tileUrl(tx_next + sx, ty_next + sy, nextZoom));
}

/**
 * @brief Requests all visible map tiles for the current view and zoom level.
 */
void Widgets::GPS::updateTiles()
{
  // Only download tiles at integer zoom levels
  const int baseZoom          = qFloor(m_zoom);
  const double fractionalZoom = m_zoom - baseZoom;
  const double scale          = qPow(2.0, fractionalZoom);

  // Tile size in pixels
  const int tileSize          = 256;
  const double scaledTileSize = tileSize * scale;

  // Current map center in tile coordinates at base zoom
  const QPointF center = m_centerTile / scale;

  // Viewport size in pixels
  const QSize viewSize = size().toSize();

  // Number of tiles visible horizontally and vertically (based on scaled size)
  const int tilesX = qCeil(viewSize.width() / scaledTileSize) + 1;
  const int tilesY = qCeil(viewSize.height() / scaledTileSize) + 1;

  for (int dx = -tilesX / 2; dx <= tilesX / 2; ++dx) {
    for (int dy = -tilesY / 2; dy <= tilesY / 2; ++dy) {
      // Compute tile X/Y coordinates relative to the map center
      const int tx = static_cast<int>(center.x()) + dx;
      const int ty = static_cast<int>(center.y()) + dy;

      // Maximum valid tile index for the current zoom level
      const int maxTiles = 1 << baseZoom;

      // Skip tiles outside vertical and horizontal bounds
      if (tx < 0 || ty < 0 || tx >= maxTiles || ty >= maxTiles)
        continue;

      // Construct the tile URL (use baseZoom for actual tile requests)
      const auto url = tileUrl(tx, ty, baseZoom);

      // Request base map tile
      requestTileIfNeeded(url);

      // Request tiles for weather and reference layers
      if (m_showNasaWeather && baseZoom <= WEATHER_GIBS_MAX_ZOOM)
        requestTileIfNeeded(nasaWeatherUrl(tx, ty, baseZoom));

      if (m_enableReferenceLayer)
        requestTileIfNeeded(referenceUrl(tx, ty, baseZoom));

      // Preload next zoom level tiles when fractional zoom is near transition
      if (fractionalZoom > 0.7)
        preloadNextZoomTiles(tx, ty, baseZoom);
    }
  }
}

/**
 * @brief Preloads all world tiles at the minimum zoom level.
 */
void Widgets::GPS::precacheWorld()
{
  // Request all tiles at the minimum zoom level for a coarse overview
  for (int tx = 0; tx < (1 << MIN_ZOOM); ++tx) {
    for (int ty = 0; ty < (1 << MIN_ZOOM); ++ty) {
      const QString url = tileUrl(tx, ty, MIN_ZOOM);
      if (!s_tileCache.contains(url) && !s_pending.contains(url)) {
        QNetworkReply* reply = m_network.get(QNetworkRequest(QUrl(url)));
        s_pending[url]       = reply;
        connect(reply, &QNetworkReply::finished, this, [=, this]() { onTileFetched(reply); });
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Theme & tile event handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates colors based on the current theme.
 */
void Widgets::GPS::onThemeChanged()
{
  // Obtain color for latest line data
  const auto color = SerialStudio::getDatasetColor(m_index + 1);
  m_lineHeadColor  = color;

  // Create gradient based on widget index
  QColor midCurve(m_lineHeadColor);
  m_lineTailColor = midCurve.darker(130);
  m_lineHeadColor = midCurve.lighter(130);
  m_lineTailColor.setAlpha(156);
}

/**
 * @brief Called when a tile download finishes.
 */
void Widgets::GPS::onTileFetched(QNetworkReply* reply)
{
  // Cache the downloaded tile image and trigger a repaint
  const QString url = reply->url().toString();
  if (reply->error() == QNetworkReply::NoError) {
    QImage* image = new QImage();
    if (image->loadFromData(reply->readAll())) {
      s_tileCache.insert(url, image);
      update();
    }

    else
      delete image;
  }

  reply->deleteLater();
  s_pending.remove(url);
}

//--------------------------------------------------------------------------------------------------
// Tile rendering helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders a scaled ancestor tile when the target tile is not yet cached.
 */
bool Widgets::GPS::renderFallbackTile(
  QPainter* painter, int tx, int ty, int baseZoom, const QRect& targetRect, double scaledTileSize)
{
  const int tileSize = 256;
  for (int z = baseZoom - 1; z >= MIN_ZOOM; --z) {
    const int dz              = baseZoom - z;
    const int tileScale       = 1 << dz;
    const int parentTx        = tx >> dz;
    const int parentTy        = ty >> dz;
    const int wrappedParentTx = (parentTx % (1 << z) + (1 << z)) % (1 << z);
    const QString parentUrl   = tileUrl(wrappedParentTx, parentTy, z);

    if (!s_tileCache.contains(parentUrl))
      continue;

    auto* src = s_tileCache.object(parentUrl);
    const QRect sourceRect((tx % tileScale) * (tileSize / tileScale),
                           (ty % tileScale) * (tileSize / tileScale),
                           tileSize / tileScale,
                           tileSize / tileScale);

    const auto cropped    = src->copy(sourceRect);
    const int roundedSize = qRound(scaledTileSize);

    QImage finalTile;
    if (qAbs(baseZoom - z) > 5) {
      QImage img = cropped.scaled(1, 1, Qt::IgnoreAspectRatio, Qt::FastTransformation);
      finalTile  = QImage(roundedSize, roundedSize, QImage::Format_ARGB32);
      finalTile.fill(img.pixelColor(0, 0));
    } else {
      finalTile =
        cropped.scaled(roundedSize, roundedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    painter->drawImage(targetRect, finalTile);
    return true;
  }

  return false;
}

/**
 * @brief Draws the NASA GIBS weather overlay tile if available in the cache.
 */
void Widgets::GPS::renderWeatherOverlay(
  QPainter* painter, int wrappedTx, int ty, int baseZoom, const QRect& targetRect)
{
  if (!m_showNasaWeather || baseZoom > WEATHER_GIBS_MAX_ZOOM)
    return;

  const QString gibsUrl = nasaWeatherUrl(wrappedTx, ty, baseZoom);
  if (!s_tileCache.contains(gibsUrl))
    return;

  painter->save();
  painter->setCompositionMode(QPainter::CompositionMode_Screen);
  painter->drawImage(targetRect, *s_tileCache.object(gibsUrl));
  painter->restore();
}

/**
 * @brief Draws the reference layer tile on top of the base tile.
 */
void Widgets::GPS::renderReferenceOverlay(
  QPainter* painter, int wrappedTx, int ty, int baseZoom, const QRect& targetRect)
{
  if (!m_enableReferenceLayer)
    return;

  const QString refUrl = referenceUrl(wrappedTx, ty, baseZoom);
  if (s_tileCache.contains(refUrl))
    painter->drawImage(targetRect, *s_tileCache.object(refUrl));
}

//--------------------------------------------------------------------------------------------------
// Paint operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the visible portion of the map using cached tiles.
 */
void Widgets::GPS::paintMap(QPainter* painter, const QSize& view)
{
  if (!painter) {
    qWarning() << "GPS::paintMap: null painter";
    return;
  }

  // Disable antialiasing for tile rendering to prevent seams
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter->fillRect(painter->viewport(), QColor(0x0C, 0x47, 0x5C));

  // Fractional zoom handling for smooth zooming
  const int baseZoom          = qFloor(m_zoom);
  const double fractionalZoom = m_zoom - baseZoom;
  const double scale          = qPow(2.0, fractionalZoom);
  const int tileSize          = 256;
  const double scaledTileSize = tileSize * scale;

  // Convert m_centerTile from fractional zoom to base zoom for tile fetching
  const QPointF centerTileBase = m_centerTile / scale;
  const int centerX            = static_cast<int>(centerTileBase.x());
  const int centerY            = static_cast<int>(centerTileBase.y());

  // Pixel offset rounded to integer pixels to avoid seams during auto-tracking
  const int offsetX = qRound((centerTileBase.x() - centerX) * scaledTileSize);
  const int offsetY = qRound((centerTileBase.y() - centerY) * scaledTileSize);

  // Number of tiles needed horizontally and vertically (including margins)
  const int tilesX = qCeil(view.width() / scaledTileSize) + 1;
  const int tilesY = qCeil(view.height() / scaledTileSize) + 1;

  // Render visible tiles
  for (int dx = -tilesX / 2; dx <= tilesX / 2; ++dx) {
    for (int dy = -tilesY / 2; dy <= tilesY / 2; ++dy) {
      const int tx        = centerX + dx;
      const int ty        = centerY + dy;
      const int wrappedTx = (tx % (1 << baseZoom) + (1 << baseZoom)) % (1 << baseZoom);

      // Skip if tile is outside vertical bounds (no wrapping in Y)
      if (ty < 0 || ty >= (1 << baseZoom))
        continue;

      // Compute pixel position on screen for drawing this tile
      const QString url = tileUrl(wrappedTx, ty, baseZoom);
      const int drawX   = view.width() / 2 + qRound(dx * scaledTileSize) - offsetX;
      const int drawY   = view.height() / 2 + qRound(dy * scaledTileSize) - offsetY;
      const QRect targetRect(drawX, drawY, qRound(scaledTileSize), qRound(scaledTileSize));

      // Draw tile if it's already in cache, otherwise fall back to scaled lower-zoom tiles
      if (s_tileCache.contains(url))
        painter->drawImage(targetRect, *s_tileCache.object(url));

      else
        renderFallbackTile(painter, tx, ty, baseZoom, targetRect, scaledTileSize);

      // Overlay global cloud image and NASA GIBS reference layers
      renderCloudOverlay(painter, wrappedTx, ty, baseZoom, targetRect, scaledTileSize);
      renderWeatherOverlay(painter, wrappedTx, ty, baseZoom, targetRect);
      renderReferenceOverlay(painter, wrappedTx, ty, baseZoom, targetRect);
    }
  }
}

/**
 * @brief Overlays the equirectangular cloud image onto the given tile rect.
 */
void Widgets::GPS::renderCloudOverlay(QPainter* painter,
                                      int wrappedTx,
                                      int ty,
                                      int baseZoom,
                                      const QRect& targetRect,
                                      double scaledTileSize)
{
  if (!m_showWeather || m_cloudOverlay.isNull() || baseZoom > WEATHER_MAX_ZOOM)
    return;

  // Cloud image uses equirectangular projection (typically 4096x2048)
  const int cloudWidth  = m_cloudOverlay.width();
  const int cloudHeight = m_cloudOverlay.height();
  const double n        = 1 << baseZoom;

  // Compute geographic longitude bounds and Web Mercator latitude bounds
  const double lon0 = (wrappedTx / n) * 360.0 - 180.0;
  const double lon1 = ((wrappedTx + 1) / n) * 360.0 - 180.0;
  const double lat0 = 180.0 * kInvPi * std::atan(std::sinh(M_PI * (1 - 2.0 * ty / n)));
  const double lat1 = 180.0 * kInvPi * std::atan(std::sinh(M_PI * (1 - 2.0 * (ty + 1) / n)));

  // Normalize lon/lat to [0,1] UV range
  const double u0 = (lon0 + 180.0) * kInv360;
  const double u1 = (lon1 + 180.0) * kInv360;
  const double v0 = (90.0 - lat0) * kInv180;
  const double v1 = (90.0 - lat1) * kInv180;

  // Convert UVs to source pixel rectangle in the cloud image
  const QRect sourceRect(int(u0 * cloudWidth),
                         int(v0 * cloudHeight),
                         int((u1 - u0) * cloudWidth),
                         int((v1 - v0) * cloudHeight));

  // Crop and scale the source image tile to the target size
  const int roundedSize = qRound(scaledTileSize);
  const QImage cropped =
    m_cloudOverlay.copy(sourceRect)
      .scaled(roundedSize, roundedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  painter->drawImage(targetRect, cropped);
}

/**
 * @brief Paints the GPS trajectory and position marker overlay.
 */
void Widgets::GPS::paintPathData(QPainter* painter, const QSize& view)
{
  if (!painter) {
    qWarning() << "GPS::paintPathData: null painter";
    return;
  }

  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
    return;

  // Obtain series data
  const auto& series = UI::Dashboard::instance().gpsSeries(m_index);
  if (series.latitudes.empty() || series.longitudes.empty() || series.altitudes.empty())
    return;

  // Enable antialiasing for smooth path and indicator rendering
  painter->setRenderHint(QPainter::Antialiasing, true);

  // Convert center tile to base zoom for consistent calculations
  const int baseZoom           = qFloor(m_zoom);
  const double fractionalZoom  = m_zoom - baseZoom;
  const double scale           = qPow(2.0, fractionalZoom);
  const QPointF centerTileBase = m_centerTile / scale;

  // Render trajectory polyline (if enabled) and the wrapped position indicator
  if (m_plotTrajectory)
    renderTrajectoryPath(painter, view, baseZoom, scale, centerTileBase);

  renderPositionIndicator(painter, view, baseZoom, scale, centerTileBase);
}

/**
 * @brief Renders the historical GPS trajectory polyline with tail-to-head gradient.
 */
void Widgets::GPS::renderTrajectoryPath(
  QPainter* painter, const QSize& view, int baseZoom, double scale, const QPointF& centerTileBase)
{
  const auto& series  = UI::Dashboard::instance().gpsSeries(m_index);
  const int tileSize  = 256;
  const double world  = qPow(2.0, baseZoom);
  const QPointF cTile = centerTileBase;

  // Project lat/lon to on-screen pixel
  auto project = [&](double lat, double lon) -> QPointF {
    QPointF tile = latLonToTile(lat, lon, baseZoom);

    double dx  = tile.x() - cTile.x();
    dx        -= std::round(dx / world) * world;

    QPointF delta(dx, tile.y() - cTile.y());
    return {view.width() * 0.5 + delta.x() * tileSize * scale,
            view.height() * 0.5 + delta.y() * tileSize * scale};
  };

  // Generate a path with historical samples plus the current location
  QVector<QPointF> path;
  auto pushIfValid = [&](double lat, double lon) {
    if (std::isnan(lat) || std::isnan(lon) || DSP::isZero(lat) || DSP::isZero(lon))
      return;

    path.append(project(lat, lon));
  };
  for (size_t i = 0; i < series.latitudes.size(); ++i)
    pushIfValid(series.latitudes[i], series.longitudes[i]);

  pushIfValid(m_latitude, m_longitude);

  // Skip if nothing on screen
  const QRectF vp(0, 0, view.width(), view.height());
  const bool v =
    std::any_of(path.cbegin(), path.cend(), [&](const QPointF& p) { return vp.contains(p); });

  if (!v || path.size() <= 1)
    return;

  // Draw the path with tail-to-head gradient
  QLinearGradient grad(path.first(), path.last());
  grad.setColorAt(0.0, m_lineTailColor);
  grad.setColorAt(1.0, m_lineHeadColor);

  QPen pen(QBrush(grad), 2);
  painter->setPen(pen);

  for (int i = 1; i < path.size(); ++i) {
    const QPointF& p1 = path[i - 1];
    const QPointF& p2 = path[i];
    painter->drawLine(p1, p2);
  }
}

/**
 * @brief Renders the current-position indicator (halo, border, dot), wrapped 3x.
 */
void Widgets::GPS::renderPositionIndicator(
  QPainter* painter, const QSize& view, int baseZoom, double scale, const QPointF& centerTileBase)
{
  const int tileSize      = 256;
  const QPointF gpsTile   = latLonToTile(m_latitude, m_longitude, baseZoom);
  const QPointF deltaBase = gpsTile - centerTileBase;
  const double world      = qPow(2.0, baseZoom);

  // Render the indicator three times: centre plus one wrap left / right
  for (int wrap = -1; wrap <= 1; ++wrap) {
    const QPointF delta = deltaBase + QPointF(wrap * world, 0);
    const QPoint drawPos(view.width() / 2 + delta.x() * tileSize * scale,
                         view.height() / 2 + delta.y() * tileSize * scale);

    constexpr int dotRadius   = 5;
    constexpr int haloRadius  = 14;
    constexpr int outerRadius = 7;

    auto dotColor    = m_lineHeadColor;
    auto haloColor   = m_lineHeadColor;
    auto borderColor = QColor(0xff, 0xff, 0xff);
    haloColor.setAlpha(0x3f);

    // Halo, white border, inner dot
    painter->setPen(Qt::NoPen);
    painter->setBrush(haloColor);
    painter->drawEllipse(drawPos, haloRadius, haloRadius);

    painter->setBrush(borderColor);
    painter->drawEllipse(drawPos, outerRadius, outerRadius);

    painter->setBrush(dotColor);
    painter->drawEllipse(drawPos, dotRadius, dotRadius);
  }
}

/**
 * @brief Renders the map attribution text in the bottom-right corner.
 */
void Widgets::GPS::paintAttributionText(QPainter* painter, const QSize& view)
{
  if (!painter) {
    qWarning() << "GPS::paintAttributionText: null painter";
    return;
  }

  // Attribution string to display
  const int margin    = 6;
  // code-verify off
  QString attribution = "Copyright © Esri, Maxar, Earthstar Geographics";
  // code-verify on
  if (m_showNasaWeather && m_zoom <= WEATHER_GIBS_MAX_ZOOM)
    attribution += " | NASA EOSDIS GIBS, Suomi NPP VIIRS";
  else if (m_showWeather && m_zoom <= WEATHER_MAX_ZOOM)
    attribution += " | Matt Eason";

  // Font styling for the label
  const QFont font = Misc::CommonFonts::instance().customUiFont(0.85);
  painter->setFont(font);

  // Measure the size of the text
  const QFontMetrics metrics(font);
  const QSize textSize = metrics.size(Qt::TextSingleLine, attribution);

  // Define the background rectangle at bottom-right, tight to the text
  QRect backgroundRect(view.width() - textSize.width() - 2 * margin,
                       view.height() - textSize.height() - 2 * margin,
                       textSize.width() + 2 * margin,
                       textSize.height() + 2 * margin);

  // Draw semi-transparent solid background
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 128));
  painter->drawRect(backgroundRect);

  // Draw white attribution text with slight transparency
  painter->setPen(QColor(255, 255, 255, 220));
  painter->drawText(backgroundRect.adjusted(margin, margin, -margin, -margin),
                    Qt::AlignLeft | Qt::AlignVCenter,
                    attribution);
}

//--------------------------------------------------------------------------------------------------
// Coordinate conversion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clamps the center tile coordinate to valid map bounds.
 */
QPointF Widgets::GPS::clampCenterTile(QPointF tile) const
{
  // World size at fractional zoom level (tile param is at fractional zoom)
  const double maxTile = qPow(2.0, m_zoom);

  // Wrap X coordinate for horizontal panning
  double x = std::fmod(tile.x(), maxTile);
  if (x < 0)
    x += maxTile;

  // Compute how many tiles fit vertically in the view at current zoom
  const double fractionalZoom = m_zoom - qFloor(m_zoom);
  const double scale          = qPow(2.0, fractionalZoom);
  const double scaledTileSize = 256.0 * scale;
  const double tilesInViewY   = static_cast<double>(height()) / scaledTileSize;
  const double marginY        = tilesInViewY * 0.5;

  // Clamp Y so full view fits within tile bounds (no overshooting north/south)
  double y = qBound(marginY, tile.y(), maxTile - marginY);
  return QPointF(x, y);
}

/**
 * @brief Converts tile XY to geographic lat/lon.
 */
QPointF Widgets::GPS::tileToLatLon(const QPointF& tile, double zoom)
{
  double n      = qPow(2.0, zoom);
  double lon    = tile.x() / n * 360.0 - 180.0;
  double latRad = qAtan(sinh(M_PI * (1 - 2 * tile.y() / n)));
  double lat    = qRadiansToDegrees(latRad);
  return QPointF(lat, lon);
}

/**
 * @brief Converts geographic lat/lon to tile XY.
 */
QPointF Widgets::GPS::latLonToTile(double lat, double lon, double zoom)
{
  double latRad = qDegreesToRadians(lat);
  double n      = qPow(2.0, zoom);
  double x      = (lon + 180.0) * kInv360 * n;
  double y      = (1.0 - qLn(qTan(latRad) + 1.0 / qCos(latRad)) * kInvPi) * 0.5 * n;
  return QPointF(x, y);
}

//--------------------------------------------------------------------------------------------------
// URL generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the URL to request a specific map tile.
 */
QString Widgets::GPS::tileUrl(const int tx, const int ty, const int zoom) const
{
#ifdef BUILD_COMMERCIAL
  return QStringLiteral(
           "https://services.arcgisonline.com/ArcGIS/rest/services/%1/MapServer/tile/%2/%3/%4?token=%5")
    .arg(m_mapIDs[m_mapType])
    .arg(zoom)
    .arg(ty)
    .arg(tx)
    .arg(ARCGIS_API_KEY);
#else
  return QStringLiteral(
           "https://services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/%1/%2/%3")
    .arg(zoom)
    .arg(ty)
    .arg(tx);
#endif
}

/**
 * @brief Constructs the URL to request a label (reference) tile overlay.
 */
QString Widgets::GPS::referenceUrl(const int tx, const int ty, const int zoom) const
{
#ifdef BUILD_COMMERCIAL
  return QStringLiteral(
           "https://services.arcgisonline.com/ArcGIS/rest/services/Reference/World_Boundaries_and_Places/MapServer/tile/%1/%2/%3?token=%4")
    .arg(zoom)
    .arg(ty)
    .arg(tx)
    .arg(ARCGIS_API_KEY);
#else
  return QStringLiteral(
           "https://services.arcgisonline.com/ArcGIS/rest/services/Reference/World_Boundaries_and_Places/MapServer/tile/%1/%2/%3")
    .arg(zoom)
    .arg(ty)
    .arg(tx);
#endif
}

/**
 * @brief Constructs the tile URL for NASA GIBS weather imagery.
 */
QString Widgets::GPS::nasaWeatherUrl(const int tx, const int ty, const int zoom) const
{
  const QString date = QDate::currentDate().addDays(-1).toString(Qt::ISODate);
  return QStringLiteral(
           "https://gibs.earthdata.nasa.gov/wmts/epsg3857/best/VIIRS_SNPP_CorrectedReflectance_TrueColor/default/%1/GoogleMapsCompatible_Level9/%2/%3/%4.jpg")
    .arg(date)
    .arg(qMin(zoom, WEATHER_GIBS_MAX_ZOOM))
    .arg(ty)
    .arg(tx);
}

//--------------------------------------------------------------------------------------------------
// Input event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel zooming.
 */
void Widgets::GPS::wheelEvent(QWheelEvent* event)
{
  const int delta = event->angleDelta().y();
  if (delta == 0)
    return;

  event->accept();

  const bool isTouchpad =
    !event->pixelDelta().isNull() || event->source() == Qt::MouseEventSynthesizedBySystem;

  double newZoom;
  if (isTouchpad) {
    const double zoomFactor = 1.05;
    const double deltaNorm  = -delta * kInv120;
    const double factor     = qPow(zoomFactor, -deltaNorm);
    newZoom                 = m_zoom * factor;
  } else {
    const double zoomStep = 0.5;
    newZoom               = m_zoom + (delta > 0 ? zoomStep : -zoomStep);
  }

  newZoom =
    qBound(static_cast<double>(MIN_ZOOM), newZoom, static_cast<double>(m_mapMaxZoom[m_mapType]));

  if (qAbs(newZoom - m_zoom) < 0.001)
    return;

  if (!autoCenter()) {
    const QPointF cursorPos = event->position();
    const QPointF cursorTileOffset((cursorPos.x() - width() * 0.5) * kInvTile,
                                   (cursorPos.y() - height() * 0.5) * kInvTile);

    const QPointF cursorTile = m_centerTile + cursorTileOffset;
    const QPointF geo        = tileToLatLon(cursorTile, m_zoom);
    m_zoom                   = newZoom;

    const QPointF newTile = latLonToTile(geo.x(), geo.y(), m_zoom);
    m_centerTile          = clampCenterTile(newTile - cursorTileOffset);

    updateTiles();
    update();

    Q_EMIT zoomLevelChanged();
  }

  else
    setZoomLevelPrecise(newZoom);
}

/**
 * @brief Handles map dragging via mouse move.
 */
void Widgets::GPS::mouseMoveEvent(QMouseEvent* event)
{
  if (!autoCenter()) {
    const QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos     = event->pos();

    m_centerTile -= QPointF(delta.x() * kInvTile, delta.y() * kInvTile);
    m_centerTile  = clampCenterTile(m_centerTile);

    updateTiles();
    update();
    event->accept();
  }
}

/**
 * @brief Records mouse position for dragging logic.
 */
void Widgets::GPS::mousePressEvent(QMouseEvent* event)
{
  if (!autoCenter()) {
    m_lastMousePos = event->pos();

    grabMouse();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
  }
}

/**
 * @brief Handles mouse release events to stop dragging and reset cursor.
 */
void Widgets::GPS::mouseReleaseEvent(QMouseEvent* event)
{
  unsetCursor();
  ungrabMouse();
  ungrabTouchPoints();
  event->accept();
}
