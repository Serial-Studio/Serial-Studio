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

#include "UI/Dashboard.h"
#include "UI/Widgets/GPS.h"
#include "Misc/Utilities.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

#include <QCursor>
#include <QPainter>
#include <QPainterPath>
#include <QNetworkReply>
#include <QLinearGradient>

//------------------------------------------------------------------------------
// Global parameters
//------------------------------------------------------------------------------

// clang-format off
constexpr int MIN_ZOOM = 2;
constexpr int WEATHER_MAX_ZOOM = 6;
constexpr int WEATHER_GIBS_MAX_ZOOM = 9;
constexpr auto CLOUD_URL = "https://clouds.matteason.co.uk/images/4096x2048/clouds-alpha.png";
// clang-format on

//------------------------------------------------------------------------------
// Constructor function
//------------------------------------------------------------------------------

/**
 * @brief Constructs a GPS map widget using ArcGIS Maps tile data.
 *
 * Initializes internal state, sets up cache, and binds to dashboard
 * updates.
 *
 * @param index Index in the dashboard model (used for data binding).
 * @param parent Optional QML parent item.
 */
Widgets::GPS::GPS(const int index, QQuickItem *parent)
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
  setFlags(QQuickPaintedItem::ItemHasContents);

  // Configure cache
  m_tileCache.setMaxCost(512);

  // Set user-friendly map names
  m_mapTypes << tr("Satellite Imagery") << tr("Satellite Imagery with Labels")
             << tr("Street Map") << tr("Topographic Map") << tr("Terrain")
             << tr("Light Gray Canvas") << tr("Dark Gray Canvas")
             << tr("National Geographic");

  // Set URL/tile server map type IDs
  m_mapIDs << "World_Imagery" << "World_Imagery" << "World_Street_Map"
           << "World_Topo_Map" << "World_Terrain_Base"
           << "Canvas/World_Light_Gray_Base" << "Canvas/World_Dark_Gray_Base"
           << "NatGeo_World_Map";

  // Set max zoom for each map type
  m_mapMaxZoom << 18 << 18 << 18 << 18 << 9 << 16 << 16 << 16;

  // Read settings
  setMapType(m_settings.value("gpsMapType", 0).toInt());
  m_showWeather = m_settings.value("gpsWeather", false).toBool();
  m_autoCenter = m_settings.value("gpsAutoCenter", true).toBool();
  m_showNasaWeather = m_settings.value("gpsNasaWeather", false).toBool();
  m_plotTrajectory = m_settings.value("gpsPlotTrajectory", true).toBool();

  // Only enable one of the weather overlays
  if (m_showNasaWeather && m_showWeather)
    m_showWeather = false;

  // Download cloud map
  if (m_showWeather)
  {
    QUrl cloudUrl(CLOUD_URL);
    QNetworkRequest request(cloudUrl);
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
      reply->deleteLater();

      if (reply->error() == QNetworkReply::NoError)
      {
        QByteArray imageData = reply->readAll();
        QImage image;
        if (image.loadFromData(imageData))
        {
          m_cloudOverlay = image;
          update();
        }
      }
    });
  }

  // Connect to the theme manager to update the colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::GPS::onThemeChanged);

  // Configure signals/slots with the dashboard
  if (VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
  {
    updateData();
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Widgets::GPS::updateData);
    center();
  }
}

//------------------------------------------------------------------------------
// Painting code
//------------------------------------------------------------------------------

/**
 * @brief Renders the GPS widget onto the screen.
 *
 * This function is called by the Qt framework whenever the widget needs to be
 * repainted. It verifies that the widget is both enabled and properly
 * configured before proceeding. The rendering process is broken into three main
 * steps:
 *
 * - Drawing the map tiles (`paintMap`)
 * - Drawing optional path data (e.g., trajectory lines) (`paintPathData`)
 * - Drawing attribution and copyright labels (`paintAttributionText`)
 *
 * The function ensures anti-aliasing is enabled for smooth visuals and uses
 * the current viewport size for all rendering operations.
 *
 * @param painter Pointer to the QPainter used for rendering the widget.
 */
void Widgets::GPS::paint(QPainter *painter)
{
  // No need to update if widget is not enabled
  if (!isEnabled())
    return;

  // No need to update if widget is invalid
  if (!VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
    return;

  // Configure painter hints
  painter->setRenderHint(QPainter::Antialiasing);

  // Get the dimenstions of the rendered widget in pixels
  const QSize viewport = size().toSize();

  // Paint widget data
  paintMap(painter, viewport);
  paintPathData(painter, viewport);
  paintAttributionText(painter, viewport);
}

//------------------------------------------------------------------------------
// Getter functions
//------------------------------------------------------------------------------

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
 *
 * The index corresponds to an entry in the internal map type list.
 * This value is used to determine which tile set is used when rendering the
 * map.
 *
 * @return Integer index representing the selected map type.
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
 *
 * This flag determines whether a global weather tile is used for the cloud
 * overlay using the Live Cloud Maps API.
 *
 * @return True if the weather overlay is enabled, false otherwise.
 */
bool Widgets::GPS::showWeather() const
{
  return m_showWeather;
}

/**
 * @brief Indicates whether the map should display the GPS trajectory.
 *
 * When enabled, the widget will render a visual trail representing
 * the past GPS positions received over time.
 *
 * @return @c true if the trajectory line should be drawn; otherwise @c false.
 */
bool Widgets::GPS::plotTrajectory() const
{
  return m_plotTrajectory;
}

/**
 * @brief Returns the current state of the weather overlay visibility flag.
 *
 * This flag determines whether weather tiles from NASA GIBS are being
 * requested and rendered over the base map.
 *
 * @return True if the weather overlay is enabled, false otherwise.
 */
bool Widgets::GPS::showNasaWeather() const
{
  return m_showNasaWeather;
}

/**
 * @brief Provides the list of available user-friendly map type names.
 *
 * These names are suitable for display in a UI element (e.g., dropdown menu).
 *
 * @return Reference to the list of localized map type display names.
 */
const QStringList &Widgets::GPS::mapTypes()
{
  return m_mapTypes;
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

/**
 * @brief Re-centers the map to the latest GPS coordinates.
 *
 * Converts the current GPS lat/lon to tile coordinates and updates the center
 * of the map accordingly. Triggers a redraw.
 */
void Widgets::GPS::center()
{
  m_centerTile = latLonToTile(m_latitude, m_longitude, m_zoom);
  updateTiles();
  update();
}

/**
 * @brief Sets the map zoom level, preserving viewport center if auto-centering
 *        is disabled.
 *
 * If auto-centering is off, the map is re-centered so that the geographic
 * location currently at the center of the viewport remains centered after
 * the zoom level change.
 *
 * If auto-centering is on, the map recenters on the current GPS position.
 *
 * @param zoom Desired zoom level. It will be clamped to the valid range
 *             for the current map type.
 */
void Widgets::GPS::setZoomLevel(int zoom)
{
  // Bound zoom to valid zoom level
  const auto z = qBound(MIN_ZOOM, zoom, m_mapMaxZoom[m_mapType]);

  // Skip if zoom is the same
  if (m_zoom == z)
    return;

  // Set zoom level and center map on viewport center
  if (!autoCenter())
  {
    // Compute which lat/lon is currently at the center of the widget
    const QPointF pixelCenter(width() / 2.0, height() / 2.0);
    const QPointF tileCenter
        = m_centerTile
          + QPointF((pixelCenter.x() - width() / 2.0) / 256.0,
                    (pixelCenter.y() - height() / 2.0) / 256.0);

    // Convert screen center tile to geographic lat/lon
    const QPointF geo = tileToLatLon(tileCenter, m_zoom);

    // Update zoom, then reproject lat/lon to new zoom level
    m_zoom = z;
    const QPointF newTile = latLonToTile(geo.x(), geo.y(), m_zoom);

    // Set new center tile so that geo stays centered in the widget
    m_centerTile = clampCenterTile(newTile);

    // Redraw the widget
    updateTiles();
    update();
  }

  // Set the zoom level and let center() fix the world for us
  else
  {
    m_zoom = z;
    center();
  }
}

/**
 * @brief Sets the active map type to be rendered.
 *
 * Updates the current map type, clears cached tiles, and triggers
 * reloading of appropriate tiles for the new map type.
 * If the current zoom exceeds the maximum allowed for this map type,
 * it will be adjusted automatically.
 *
 * @param type Index of the new map type from the mapTypes() list.
 */
void Widgets::GPS::setMapType(const int type)
{
  auto mapId = qBound(0, type, m_mapTypes.count() - 1);
  if (m_mapType != mapId)
  {
    if (!SerialStudio::activated() && mapId > 0)
    {
      mapId = 0;
      Misc::Utilities::showMessageBox(
          tr("Additional map layers are available only for Pro users."),
          tr("We can't offer unrestricted access because the ArcGIS API key "
             "incurs real costs."),
          QMessageBox::Information);
    }

    m_mapType = mapId;
    m_tileCache.clear();
    m_settings.setValue(QStringLiteral("gpsMapType"), mapId);

    if (mapId == 1)
      m_enableReferenceLayer = true;
    else
      m_enableReferenceLayer = false;

    if (m_zoom > m_mapMaxZoom[type])
      m_zoom = m_mapMaxZoom[type];

    updateTiles();
    precacheWorld();
    updateData();
    update();

    Q_EMIT mapTypeChanged();
  }
}

/**
 * @brief Enables/disables centering the map on the latest coordinate.
 * @param enabled boolean indicating if auto center is enabled.
 */
void Widgets::GPS::setAutoCenter(const bool enabled)
{
  if (m_autoCenter != enabled)
  {
    m_autoCenter = enabled;
    m_settings.setValue(QStringLiteral("gpsAutoCenter"), enabled);

    if (enabled)
      center();

    Q_EMIT autoCenterChanged();
  }
}

/**
 * @brief Enables or disables the display of weather overlay image from the
 *        Live Cloud Maps API.
 *
 * When enabled, this flag downloads an image from the Live Cloud Maps API with
 * the latest available global cloud imagery.
 *
 * @param enabled True to show the weather overlay, false to hide it.
 */
void Widgets::GPS::setShowWeather(const bool enabled)
{
  if (m_showWeather != enabled)
  {
    m_showWeather = enabled;
    m_settings.setValue(QStringLiteral("gpsWeather"), enabled);

    if (enabled && showNasaWeather())
      setShowNasaWeather(false);
    else
      update();

    if (enabled && m_cloudOverlay.isNull())
    {
      QUrl cloudUrl(CLOUD_URL);
      QNetworkRequest request(cloudUrl);
      QNetworkReply *reply = m_network.get(request);
      connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError)
        {
          QByteArray imageData = reply->readAll();
          QImage image;
          if (image.loadFromData(imageData))
          {
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
 *
 * When set to @c true, the GPS widget draws a trail that represents the
 * historical path of the tracked GPS location. This value is persisted
 * in the application settings and takes effect immediately.
 *
 * @param enabled @c true to display the trajectory; @c false to hide it.
 */
void Widgets::GPS::setPlotTrajectory(const bool enabled)
{
  if (m_plotTrajectory != enabled)
  {
    m_plotTrajectory = enabled;
    m_settings.setValue(QStringLiteral("gpsPlotTrajectory"), enabled);

    update();
    Q_EMIT plotTrajectoryChanged();
  }
}

/**
 * @brief Enables or disables the display of weather overlay tiles from NASA
 *        GIBS.
 *
 * When enabled, this flag triggers additional tile requests and rendering
 * of real-time weather imagery sourced from NASA's Global Imagery Browse
 * Services (GIBS). This setting is persisted to user configuration under the
 * "gpsNasaWeather" key.
 *
 * Changing this value forces a tile refresh, re-fetching visible tiles and
 * updating all related map data and cache state.
 *
 * @param enabled True to show the weather overlay, false to hide it.
 */
void Widgets::GPS::setShowNasaWeather(const bool enabled)
{
  if (m_showNasaWeather != enabled)
  {
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

//------------------------------------------------------------------------------
// Data model updating
//------------------------------------------------------------------------------

/**
 * @brief Updates GPS data from the dashboard model.
 *
 * Reads lat/lon/alt from the current dashboard group and updates internal
 * values. Emits `updated()` if any value changed.
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
  const auto &series = UI::Dashboard::instance().gpsSeries(m_index);
  if (series.latitudes.empty() || series.longitudes.empty()
      || series.altitudes.empty())
    return;

  // Grab most recent GPS values
  const double alt = series.altitudes.back();
  const double lat = series.latitudes.back();
  const double lon = series.longitudes.back();

  // Stop update if data is invalid
  if (std::isnan(alt) && std::isnan(lat) && std::isnan(lon))
    return;

  // Stop update if data did not change
  if (qFuzzyCompare(lat, m_latitude) && qFuzzyCompare(lon, m_longitude)
      && qFuzzyCompare(alt, m_altitude))
    return;

  // Update tracking values
  if (!std::isnan(lat))
    m_latitude = lat;
  if (!std::isnan(lon))
    m_longitude = lon;
  if (std::isnan(alt))
    m_altitude = alt;

  // Auto-center (redraws widget)
  if (autoCenter())
    center();

  // Redraw widget
  else
  {
    update();
    updateTiles();
  }

  // Update UI
  Q_EMIT updated();
}

//------------------------------------------------------------------------------
// Tile management
//------------------------------------------------------------------------------

/**
 * @brief Requests all visible map tiles for the current view and zoom level.
 *
 * This method calculates which tiles are currently visible on screen based on
 * the center tile coordinate and the widget's size. It then checks which of
 * those tiles are missing from the cache and issues network requests to fetch
 * them from the tile server.
 *
 * Only tiles within valid bounds (non-negative and less than 2^zoom) are
 * requested. Horizontal wrapping is handled during painting, not here.
 *
 * Downloaded tiles are tracked via m_pending to avoid duplicate requests.
 */
void Widgets::GPS::updateTiles()
{
  // Tile size in pixels (standard slippy map tile size)
  const int tileSize = 256;

  // Current map center in tile coordinates
  const QPointF center = m_centerTile;

  // Viewport size in pixels
  const QSize viewSize = size().toSize();

  // Number of tiles visible horizontally and vertically
  const int tilesX = viewSize.width() / tileSize + 2;
  const int tilesY = viewSize.height() / tileSize + 2;

  // Loop through the visible tile range
  for (int dx = -tilesX / 2; dx <= tilesX / 2; ++dx)
  {
    for (int dy = -tilesY / 2; dy <= tilesY / 2; ++dy)
    {
      // Compute tile X/Y coordinates relative to the map center
      const int tx = static_cast<int>(center.x()) + dx;
      const int ty = static_cast<int>(center.y()) + dy;

      // Maximum valid tile index for the current zoom level
      const int maxTiles = 1 << m_zoom;

      // Skip tiles outside vertical and horizontal bounds
      if (tx < 0 || ty < 0 || tx >= maxTiles || ty >= maxTiles)
        continue;

      // Construct the tile URL
      const auto url = tileUrl(tx, ty, m_zoom);

      // Request tile if it's not already cached or being downloaded
      if (!m_tileCache.contains(url) && !m_pending.contains(url))
      {
        QNetworkReply *reply = m_network.get(QNetworkRequest(QUrl(url)));
        m_pending[url] = reply;

        // When finished, call the tile fetched handler
        connect(reply, &QNetworkReply::finished, this,
                [=, this]() { onTileFetched(reply); });
      }

      // Request tiles for weather layer
      if (m_showNasaWeather && m_zoom <= WEATHER_GIBS_MAX_ZOOM)
      {
        const auto gibsUrl = nasaWeatherUrl(tx, ty, m_zoom);
        if (!m_tileCache.contains(gibsUrl) && !m_pending.contains(gibsUrl))
        {
          QNetworkReply *reply = m_network.get(QNetworkRequest(QUrl(gibsUrl)));
          m_pending[gibsUrl] = reply;

          connect(reply, &QNetworkReply::finished, this,
                  [=, this]() { onTileFetched(reply); });
        }
      }

      // Request tiles for reference layer
      if (m_enableReferenceLayer)
      {
        const auto refUrl = referenceUrl(tx, ty, m_zoom);
        if (!m_tileCache.contains(refUrl) && !m_pending.contains(refUrl))
        {
          QNetworkReply *reply = m_network.get(QNetworkRequest(QUrl(refUrl)));
          m_pending[refUrl] = reply;

          connect(reply, &QNetworkReply::finished, this,
                  [=, this]() { onTileFetched(reply); });
        }
      }
    }
  }
}

/**
 * @brief Preloads all world tiles at the minimum zoom level.
 *
 * Used to populate the tile cache for a coarse overview of the world,
 * ensuring a quick rendering experience when initially loading the map.
 * Only requests tiles not already cached or pending.
 */
void Widgets::GPS::precacheWorld()
{
  for (int tx = 0; tx < (1 << MIN_ZOOM); ++tx)
  {
    for (int ty = 0; ty < (1 << MIN_ZOOM); ++ty)
    {
      const QString url = tileUrl(tx, ty, MIN_ZOOM);
      if (!m_tileCache.contains(url) && !m_pending.contains(url))
      {
        QNetworkReply *reply = m_network.get(QNetworkRequest(QUrl(url)));
        m_pending[url] = reply;
        connect(reply, &QNetworkReply::finished, this,
                [=, this]() { onTileFetched(reply); });
      }
    }
  }
}

/**
 * @brief Updates colors based on the current theme.
 *
 * Sets line head/tail colors as a brightness gradient around the base color.
 * Also updates grid and axis colors from the theme manager.
 */
void Widgets::GPS::onThemeChanged()
{
  // Obtain color for latest line data
  const auto &colors = Misc::ThemeManager::instance().widgetColors();
  if (colors.count() > m_index)
    m_lineHeadColor = colors.at(m_index);
  else
    m_lineHeadColor = colors.at(m_index % colors.count());

  // Create gradient based on widget index
  QColor midCurve(m_lineHeadColor);
  m_lineTailColor = midCurve.darker(130);
  m_lineHeadColor = midCurve.lighter(130);
  m_lineTailColor.setAlpha(156);
}

/**
 * @brief Called when a tile download finishes.
 * @param reply Pointer to the completed QNetworkReply.
 *
 * Parses image data, stores in cache, and triggers repaint.
 */
void Widgets::GPS::onTileFetched(QNetworkReply *reply)
{
  const QString url = reply->url().toString();
  if (reply->error() == QNetworkReply::NoError)
  {
    QImage *image = new QImage();
    if (image->loadFromData(reply->readAll()))
    {
      m_tileCache.insert(url, image);
      update();
    }

    else
      delete image;
  }

  reply->deleteLater();
  m_pending.remove(url);
}

//------------------------------------------------------------------------------
// Widget painting functions
//------------------------------------------------------------------------------

/**
 * @brief Renders the visible portion of the map using cached tiles.
 *
 * This function draws the background map tiles based on the current center
 * tile coordinates and zoom level. It first fills the viewport with a solid
 * color, then attempts to draw each visible tile. If a tile is missing at
 * the current zoom level, it falls back to a parent tile from a lower zoom,
 * scaling it appropriately. If the parent tile is too coarse (difference > 5),
 * it fills the tile with the tile's dominant color to avoid visual noise.
 *
 * Horizontal wrapping is handled to allow seamless east-west panning. Vertical
 * wrapping is not allowed.
 *
 * @param painter Pointer to the QPainter used for rendering.
 * @param view The current size of the widget viewport in pixels.
 */
void Widgets::GPS::paintMap(QPainter *painter, const QSize &view)
{
  // Validate arguments
  Q_ASSERT(painter);

  // Fill the painter with a background
  painter->fillRect(painter->viewport(), QColor(0x0C, 0x47, 0x5C));

  // Tile sizing constants
  const int tileSize = 256;
  const QPointF centerTile = m_centerTile;

  // Integer tile coordinates of the center tile
  const int centerX = static_cast<int>(centerTile.x());
  const int centerY = static_cast<int>(centerTile.y());

  // Pixel offset inside the center tile (sub-tile precision)
  const double offsetX = (centerTile.x() - centerX) * tileSize;
  const double offsetY = (centerTile.y() - centerY) * tileSize;

  // Number of tiles needed horizontally and vertically (including margins)
  const int tilesX = view.width() / tileSize + 2;
  const int tilesY = view.height() / tileSize + 2;

  // Render visible tiles
  for (int dx = -tilesX / 2; dx <= tilesX / 2; ++dx)
  {
    for (int dy = -tilesY / 2; dy <= tilesY / 2; ++dy)
    {
      // Compute absolute tile coordinates relative to map center
      const int tx = centerX + dx;
      const int ty = centerY + dy;

      // Wrap X to allow seamless east/west panning around the globe
      const int wrappedTx
          = (tx % (1 << m_zoom) + (1 << m_zoom)) % (1 << m_zoom);

      // Skip if tile is outside vertical bounds (no wrapping in Y)
      if (ty < 0 || ty >= (1 << m_zoom))
        continue;

      // Generate tile URL for current zoom level
      const QString url = tileUrl(wrappedTx, ty, m_zoom);

      // Compute pixel position on screen for drawing this tile
      const QPoint drawPoint(view.width() / 2 + dx * tileSize - offsetX,
                             view.height() / 2 + dy * tileSize - offsetY);

      // Draw tile if it's already in cache
      if (m_tileCache.contains(url))
        painter->drawImage(drawPoint, *m_tileCache.object(url));

      // Tile not found: fallback to scaled lower-zoom tiles if possible
      else
      {
        for (int z = m_zoom - 1; z >= MIN_ZOOM; --z)
        {
          const int dz = m_zoom - z;
          const int scale = 1 << dz;

          // Compute coordinates of parent tile at this lower zoom level
          const int parentTx = tx >> dz;
          const int parentTy = ty >> dz;

          // Wrap parent X coordinate for global map continuity
          const int wrappedParentTx
              = (parentTx % (1 << z) + (1 << z)) % (1 << z);

          // Generate URL for the lower-zoom parent tile
          const QString parentUrl = tileUrl(wrappedParentTx, parentTy, z);

          // Skip if tile isn't in the cache
          if (!m_tileCache.contains(parentUrl))
            continue;

          // Extract subregion of the parent tile corresponding to target tile
          auto *src = m_tileCache.object(parentUrl);
          const QRect sourceRect((tx % scale) * (tileSize / scale),
                                 (ty % scale) * (tileSize / scale),
                                 tileSize / scale, tileSize / scale);

          // Get parent tile
          QImage finalTile;
          const auto cropped = src->copy(sourceRect);

          // Fill with the dominant color if parent zoom level is too coarse
          if (qAbs(m_zoom - z) > 5)
          {
            QImage img = cropped.scaled(1, 1, Qt::IgnoreAspectRatio,
                                        Qt::FastTransformation);
            QColor dominant = img.pixelColor(0, 0);

            finalTile = QImage(tileSize, tileSize, QImage::Format_ARGB32);
            finalTile.fill(dominant);
          }

          // Otherwise, scale up the subregion to tile size
          else
          {
            finalTile = cropped.scaled(tileSize, tileSize, Qt::KeepAspectRatio,
                                       Qt::FastTransformation);
          }

          // Render the fallback tile
          painter->drawImage(drawPoint, finalTile);

          // Stop as soon as we successfully rendered one fallback tile
          break;
        }
      }

      // Overlay global cloud image tile
      if (m_showWeather && !m_cloudOverlay.isNull()
          && m_zoom <= WEATHER_MAX_ZOOM)
      {
        // Get dimensions of the cloud image (equirectangular: 4096x2048)
        const int cloudWidth = m_cloudOverlay.width();
        const int cloudHeight = m_cloudOverlay.height();

        // Number of tiles at the current zoom level
        const double n = 1 << m_zoom;

        // Compute geographic longitude bounds of the tile
        double lon0 = (wrappedTx / n) * 360.0 - 180.0;
        double lon1 = ((wrappedTx + 1) / n) * 360.0 - 180.0;

        // Convert tile Y to latitude using inverse Web Mercator
        double lat0
            = 180.0 / M_PI * std::atan(std::sinh(M_PI * (1 - 2.0 * ty / n)));
        double lat1 = 180.0 / M_PI
                      * std::atan(std::sinh(M_PI * (1 - 2.0 * (ty + 1) / n)));

        // Normalize longitude to [0,1] horizontal UV range
        double u0 = (lon0 + 180.0) / 360.0;
        double u1 = (lon1 + 180.0) / 360.0;

        // Normalize latitude to [0,1] vertical UV range (top = 0, bottom = 1)
        double v0 = (90.0 - lat0) / 180.0;
        double v1 = (90.0 - lat1) / 180.0;

        // Convert UVs to source pixel rectangle in the cloud image
        QRect sourceRect(int(u0 * cloudWidth), int(v0 * cloudHeight),
                         int((u1 - u0) * cloudWidth),
                         int((v1 - v0) * cloudHeight));

        // Crop and scale the source image tile to screen tile size
        QImage cropped = m_cloudOverlay.copy(sourceRect)
                             .scaled(tileSize, tileSize, Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);

        // Draw the cloud overlay with partial transparency
        painter->drawImage(drawPoint, cropped);
      }

      // Overlay weather tile on top of the base tile
      if (m_showNasaWeather && m_zoom <= WEATHER_GIBS_MAX_ZOOM)
      {
        const QString gibsUrl = nasaWeatherUrl(wrappedTx, ty, m_zoom);

        if (m_tileCache.contains(gibsUrl))
        {
          QImage *weatherTile = m_tileCache.object(gibsUrl);

          painter->save();
          painter->setCompositionMode(QPainter::CompositionMode_Screen);
          painter->drawImage(drawPoint, *weatherTile);
          painter->restore();
        }
      }

      // Overlay reference tile on top of the base tile
      if (m_enableReferenceLayer)
      {
        const QString refUrl = referenceUrl(wrappedTx, ty, m_zoom);
        if (m_tileCache.contains(refUrl))
          painter->drawImage(drawPoint, *m_tileCache.object(refUrl));
      }
    }
  }
}

/**
 * @brief Paints the GPS trajectory and position marker overlay.
 *
 * This function draws the cached image containing the GPS path data,
 * including the historical trajectory and current location indicator.
 * It assumes that the path image (`m_gpsDataImage`) has already been
 * updated via `updateData()` or a similar method and is aligned with
 * the current view transformation.
 *
 * @param painter Pointer to the QPainter used for rendering.
 * @param view The current size of the widget viewport (unused).
 */
void Widgets::GPS::paintPathData(QPainter *painter, const QSize &view)
{
  // Validate arguments
  Q_ASSERT(painter);

  // No need to update if widget is not enabled
  if (!isEnabled())
    return;

  // No need to update if widget is invalid
  if (!VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
    return;

  // Obtain series data
  const auto &series = UI::Dashboard::instance().gpsSeries(m_index);
  if (series.latitudes.empty() || series.longitudes.empty()
      || series.altitudes.empty())
    return;

  // Initialize parameters
  constexpr int tileSize = 256;
  const QPointF centerTile = m_centerTile;

  // Plot the trajectory of the tracked device
  if (m_plotTrajectory)
  {
    // Initialize parameters
    const double world = 1 << m_zoom;
    const QPointF cTile = m_centerTile;

    // Project lat/lon to on-screen pixel
    auto project = [&](double lat, double lon) -> QPointF {
      QPointF tile = latLonToTile(lat, lon, m_zoom);

      double dx = tile.x() - cTile.x();
      dx -= std::round(dx / world) * world;

      QPointF delta(dx, tile.y() - cTile.y());
      return {view.width() * 0.5 + delta.x() * tileSize,
              view.height() * 0.5 + delta.y() * tileSize};
    };

    // Generate a path with historical samples
    QVector<QPointF> path;
    for (size_t i = 0; i < series.latitudes.size(); ++i)
    {
      const double lat = series.latitudes[i];
      const double lon = series.longitudes[i];
      if (!std::isnan(lat) && !std::isnan(lon))
      {
        if (!qFuzzyIsNull(lat) && !qFuzzyIsNull(lon))
          path.append(project(lat, lon));
      }
    }

    // Append current location to path
    if (!std::isnan(m_latitude) && !std::isnan(m_longitude))
    {
      if (!qFuzzyIsNull(m_latitude) && !qFuzzyIsNull(m_longitude))
        path.append(project(m_latitude, m_longitude));
    }

    // Skip if nothing on screen
    const QRectF vp(0, 0, view.width(), view.height());
    auto v = std::any_of(path.cbegin(), path.cend(),
                         [&](const QPointF &p) { return vp.contains(p); });

    // Draw the path
    if (v && path.size() > 1)
    {
      QLinearGradient grad(path.first(), path.last());
      grad.setColorAt(0.0, m_lineTailColor);
      grad.setColorAt(1.0, m_lineHeadColor);

      QPen pen(QBrush(grad), 2);
      painter->setPen(pen);

      for (int i = 1; i < path.size(); ++i)
      {
        const QPointF &p1 = path[i - 1];
        const QPointF &p2 = path[i];
        painter->drawLine(p1, p2);
      }
    }
  }

  // Convert current lat/lon to tile-space at current zoom
  const QPointF gpsTile = latLonToTile(m_latitude, m_longitude, m_zoom);
  const QPointF deltaBase = gpsTile - centerTile;

  // Render the indicator three times: centre plus one wrap left / right
  for (int wrap = -1; wrap <= 1; ++wrap)
  {
    // Shift X by ±(2^zoom) tiles to handle east/west wrap-around
    const QPointF delta = deltaBase + QPointF(wrap * (1 << m_zoom), 0);

    // Convert tile delta to on-screen pixel position
    const QPoint drawPos(view.width() / 2 + delta.x() * tileSize,
                         view.height() / 2 + delta.y() * tileSize);

    // Set indicator sizes
    constexpr int dotRadius = 5;
    constexpr int haloRadius = 14;
    constexpr int outerRadius = 7;

    // Set indicator colors
    auto dotColor = m_lineHeadColor;
    auto haloColor = m_lineHeadColor;
    auto borderColor = QColor(0xff, 0xff, 0xff);
    haloColor.setAlpha(0x3f);

    // Draw outer halo
    painter->setPen(Qt::NoPen);
    painter->setBrush(haloColor);
    painter->drawEllipse(drawPos, haloRadius, haloRadius);

    // Draw white border
    painter->setBrush(borderColor);
    painter->drawEllipse(drawPos, outerRadius, outerRadius);

    // Draw inner blue dot
    painter->setBrush(dotColor);
    painter->drawEllipse(drawPos, dotRadius, dotRadius);
  }
}

/**
 * @brief Renders the map attribution text in the bottom-right corner.
 *
 * Draws a semi-transparent black background rectangle with white text
 * on top, indicating the copyright and attribution for map tile sources.
 * The text is anchored to the bottom-right of the current viewport.
 *
 * This function ensures compliance with usage requirements from tile
 * providers like Esri, Maxar, and Earthstar Geographics.
 *
 * @param painter Pointer to the QPainter used for rendering.
 * @param view Size of the current viewport, used to position the text.
 */
void Widgets::GPS::paintAttributionText(QPainter *painter, const QSize &view)
{
  // Validate arguments
  Q_ASSERT(painter);

  // Attribution string to display
  const int margin = 6;
  QString attribution = "Copyright © Esri, Maxar, Earthstar Geographics";
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
                    Qt::AlignLeft | Qt::AlignVCenter, attribution);
}

//------------------------------------------------------------------------------
// Coordinate conversion
//------------------------------------------------------------------------------

/**
 * @brief Clamps the center tile coordinate to valid map bounds.
 *
 * Ensures the map center stays within the allowable tile range for the current
 * zoom level. This prevents the map from showing blank (black) areas near the
 * poles by accounting for the visible viewport height in tile units.
 *
 * - X is wrapped around the global map width to allow infinite horizontal
 *   panning.
 * - Y is clamped so the view remains entirely within the valid tile grid,
 *   preventing overshooting into non-existent map areas (north/south).
 *
 * @param tile Unbounded tile coordinate representing the desired map center.
 * @return A QPointF representing the adjusted (wrapped/clamped) center tile
 * coordinate.
 */
QPointF Widgets::GPS::clampCenterTile(QPointF tile) const
{
  const double maxTile = static_cast<double>(1 << m_zoom);

  // Wrap X coordinate for horizontal panning
  double x = std::fmod(tile.x(), maxTile);
  if (x < 0)
    x += maxTile;

  // Compute how many tiles fit vertically in the view
  const double tilesInViewY = static_cast<double>(height()) / 256.0;
  const double marginY = tilesInViewY / 2.0;

  // Clamp Y so full view fits within tile bounds (no overshooting north/south)
  double y = qBound(marginY, tile.y(), maxTile - marginY);
  return QPointF(x, y);
}

/**
 * @brief Converts tile XY to geographic lat/lon.
 * @param tile Tile coordinate.
 * @param zoom Zoom level.
 * @return Coordinate in degrees (lat, lon).
 */
QPointF Widgets::GPS::tileToLatLon(const QPointF &tile, int zoom)
{
  double n = qPow(2.0, zoom);
  double lon = tile.x() / n * 360.0 - 180.0;
  double latRad = qAtan(sinh(M_PI * (1 - 2 * tile.y() / n)));
  double lat = qRadiansToDegrees(latRad);
  return QPointF(lat, lon);
}

/**
 * @brief Converts geographic lat/lon to tile XY.
 * @param lat Latitude in degrees.
 * @param lon Longitude in degrees.
 * @param zoom Zoom level.
 * @return Tile coordinate.
 */
QPointF Widgets::GPS::latLonToTile(double lat, double lon, int zoom)
{
  double latRad = qDegreesToRadians(lat);
  double n = qPow(2.0, zoom);
  double x = (lon + 180.0) / 360.0 * n;
  double y = (1.0 - qLn(qTan(latRad) + 1.0 / qCos(latRad)) / M_PI) / 2.0 * n;
  return QPointF(x, y);
}

//------------------------------------------------------------------------------
// Tile URLs
//------------------------------------------------------------------------------

/**
 * @brief Constructs the URL to request a specific map tile.
 *
 * Supports both commercial and open versions of ArcGIS. In commercial
 * builds, an API key is appended to authenticate requests.
 *
 * @param tx Tile X coordinate (column).
 * @param ty Tile Y coordinate (row).
 * @param zoom Zoom level.
 * @return Fully constructed tile URL.
 */
QString Widgets::GPS::tileUrl(const int tx, const int ty, const int zoom) const
{
#ifdef BUILD_COMMERCIAL
  return QStringLiteral(
             "https://services.arcgisonline.com/ArcGIS/rest/services/"
             "%1/MapServer/tile/%2/%3/%4?token=%5")
      .arg(m_mapIDs[m_mapType])
      .arg(zoom)
      .arg(ty)
      .arg(tx)
      .arg(ARCGIS_API_KEY);
#else
  return QStringLiteral(
             "https://services.arcgisonline.com/ArcGIS/rest/services/"
             "World_Imagery/MapServer/tile/%1/%2/%3")
      .arg(zoom)
      .arg(ty)
      .arg(tx);
#endif
}

/**
 * @brief Constructs the URL to request a label (reference) tile overlay.
 *
 * This overlay adds geographic labels (boundaries, place names) on top of the
 * base map, typically used with the World_Imagery map to create a satellite map
 * with labels. In commercial builds, an API key is appended for authenticated
 * access.
 *
 * @param tx Tile X coordinate (column).
 * @param ty Tile Y coordinate (row).
 * @param zoom Zoom level.
 * @return Fully constructed URL for the reference tile.
 */
QString Widgets::GPS::referenceUrl(const int tx, const int ty,
                                   const int zoom) const
{
#ifdef BUILD_COMMERCIAL
  return QStringLiteral(
             "https://services.arcgisonline.com/ArcGIS/rest/services/"
             "Reference/World_Boundaries_and_Places/MapServer/tile/%1/%2/"
             "%3?token=%4")
      .arg(zoom)
      .arg(ty)
      .arg(tx)
      .arg(ARCGIS_API_KEY);
#else
  return QStringLiteral(
             "https://services.arcgisonline.com/ArcGIS/rest/services/"
             "Reference/World_Boundaries_and_Places/MapServer/tile/%1/%2/%3")
      .arg(zoom)
      .arg(ty)
      .arg(tx);
#endif
}

/**
 * @brief Constructs the tile URL for NASA GIBS weather imagery.
 *
 * This function returns the URL to fetch a Composite VIIRS Corrected
 * Reflectance tile from NASA's Global Imagery Browse Services (GIBS),
 * using the current UTC date.
 *
 * The tiles follow the Google Maps compatible scheme (EPSG:3857).
 *
 * Note: This layer supports zoom levels up to 9. Higher requested zoom levels
 * are clamped to 9 to avoid invalid tile requests.
 *
 * @param tx Tile X coordinate (column).
 * @param ty Tile Y coordinate (row).
 * @param zoom Requested zoom level.
 *
 * @return Fully constructed weather tile URL for the given coordinates and
 * date.
 */
QString Widgets::GPS::nasaWeatherUrl(const int tx, const int ty,
                                     const int zoom) const
{
  const QString date = QDate::currentDate().addDays(-1).toString(Qt::ISODate);
  return QStringLiteral("https://gibs.earthdata.nasa.gov/wmts/epsg3857/best/"
                        "VIIRS_SNPP_CorrectedReflectance_TrueColor/default/%1/"
                        "GoogleMapsCompatible_Level9/%2/%3/%4.jpg")
      .arg(date)
      .arg(qMin(zoom, WEATHER_GIBS_MAX_ZOOM))
      .arg(ty)
      .arg(tx);
}

//------------------------------------------------------------------------------
// Mouse events
//------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel zooming.
 * @param event Pointer to the wheel event.
 *
 * Zooms in/out based on scroll direction and recenters the map to keep the
 * current cursor location fixed in geo space.
 */
void Widgets::GPS::wheelEvent(QWheelEvent *event)
{
  const int delta = event->angleDelta().y();
  const int newZoom = qBound(MIN_ZOOM, m_zoom + (delta > 0 ? 1 : -1),
                             m_mapMaxZoom[m_mapType]);

  if (newZoom != m_zoom)
  {
    if (!autoCenter())
    {
      const QPointF cursorPos = event->position();
      const QPointF cursorTileOffset((cursorPos.x() - width() / 2.0) / 256.0,
                                     (cursorPos.y() - height() / 2.0) / 256.0);

      const QPointF cursorTile = m_centerTile + cursorTileOffset;
      const QPointF geo = tileToLatLon(cursorTile, m_zoom);
      m_zoom = newZoom;

      const QPointF newTile = latLonToTile(geo.x(), geo.y(), m_zoom);
      m_centerTile = clampCenterTile(newTile - cursorTileOffset);

      updateTiles();
      update();

      Q_EMIT zoomLevelChanged();
    }

    else
      setZoomLevel(newZoom);
  }

  event->accept();
}

/**
 * @brief Handles map dragging via mouse move.
 * @param event Pointer to the mouse event.
 *
 * Shifts the center tile based on mouse movement to simulate map panning.
 */
void Widgets::GPS::mouseMoveEvent(QMouseEvent *event)
{
  if (!autoCenter())
  {
    const QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();

    m_centerTile -= QPointF(delta.x() / 256.0, delta.y() / 256.0);
    m_centerTile = clampCenterTile(m_centerTile);

    updateTiles();
    update();
    event->accept();
  }
}

/**
 * @brief Records mouse position for dragging logic.
 * @param event Pointer to the mouse event.
 */
void Widgets::GPS::mousePressEvent(QMouseEvent *event)
{
  if (!autoCenter())
  {
    m_lastMousePos = event->pos();

    grabMouse();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
  }
}

/**
 * @brief Handles mouse release events to stop dragging and reset cursor.
 * @param event The mouse release event.
 */
void Widgets::GPS::mouseReleaseEvent(QMouseEvent *event)
{
  unsetCursor();
  ungrabMouse();
  ungrabTouchPoints();
  event->accept();
}
