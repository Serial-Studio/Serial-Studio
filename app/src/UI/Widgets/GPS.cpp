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

#include <QCursor>
#include <QPainter>
#include <QSvgRenderer>
#include <QNetworkReply>

//------------------------------------------------------------------------------
// Global parameters
//------------------------------------------------------------------------------

constexpr int MIN_ZOOM = 2;

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
  m_tileCache.setMaxCost(256);

  // Set user-friendly map names
  m_mapTypes << tr("Satellite Imagery") << tr("Street Map")
             << tr("Topographic Map") << tr("Terrain")
             << tr("Light Gray Canvas") << tr("Dark Gray Canvas")
             << tr("National Geographic");

  // Set URL/tile server map type IDs
  m_mapIDs << "World_Imagery"
           << "World_Street_Map"
           << "World_Topo_Map"
           << "World_Terrain_Base"
           << "Canvas/World_Light_Gray_Base"
           << "Canvas/World_Dark_Gray_Base"
           << "NatGeo_World_Map";

  // Set max zoom for each map type
  m_mapMaxZoom << 18 << 18 << 18 << 9 << 16 << 16 << 16;

  // Read settings
  setMapType(m_settings.value("gpsMapType", 0).toInt());
  setAutoCenter(m_settings.value("gpsAutoCenter", true).toBool());

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
 * @brief Paints the GPS map widget using tile data and overlays.
 *
 * This method renders the visible portion of the world map using pre-cached
 * Esri World Imagery tiles. It calculates which tiles intersect the view,
 * attempts to draw them from cache, and falls back to scaled-down parent tiles
 * from lower zoom levels when data is missing.
 *
 * Additionally, it renders a crosshair marker at the current GPS coordinates.
 * To support horizontal wrap-around of the map, the crosshair is also drawn
 * offset by ±360° (i.e., one world width) so it remains visible near the map's
 * left/right edges.
 *
 * Tile rendering supports:
 * - Sub-tile positioning with pixel offsets for smooth panning.
 * - Wrapping of X coordinates for seamless horizontal panning.
 * - Dynamic fallback to parent tiles when zoomed-in tiles are missing.
 *
 * @param painter Pointer to the QPainter used for rendering the map.
 */
void Widgets::GPS::paint(QPainter *painter)
{
  // Configure painter hints
  painter->setRenderHint(QPainter::Antialiasing);
  painter->fillRect(painter->viewport(), QColor(0x0C, 0x47, 0x5C));

  // Tile sizing constants
  constexpr int tileSize = 256;

  // Dimensions of the rendered widget in pixels
  const QSize viewSize = size().toSize();

  // Current center of the map in fractional tile coordinates
  const QPointF centerTile = m_centerTile;

  // Integer tile coordinates of the center tile
  const int centerX = static_cast<int>(centerTile.x());
  const int centerY = static_cast<int>(centerTile.y());

  // Pixel offset inside the center tile (sub-tile precision)
  const double offsetX = (centerTile.x() - centerX) * tileSize;
  const double offsetY = (centerTile.y() - centerY) * tileSize;

  // Number of tiles needed horizontally and vertically (including margins)
  const int tilesX = viewSize.width() / tileSize + 2;
  const int tilesY = viewSize.height() / tileSize + 2;

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
      const QPoint drawPoint(viewSize.width() / 2 + dx * tileSize - offsetX,
                             viewSize.height() / 2 + dy * tileSize - offsetY);

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

            QImage finalTile(tileSize, tileSize, QImage::Format_ARGB32);
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
    }
  }

  // Attribution string to display
  const int margin = 6;
  const QString attribution = "Copyright © Esri, Maxar, Earthstar Geographics";

  // Font styling for the label
  const QFont font = Misc::CommonFonts::instance().customUiFont(0.85);
  painter->setFont(font);

  // Measure the size of the text
  const QFontMetrics metrics(font);
  const QSize textSize = metrics.size(Qt::TextSingleLine, attribution);

  // Define the background rectangle at bottom-right, tight to the text
  QRect backgroundRect(viewSize.width() - textSize.width() - 2 * margin,
                       viewSize.height() - textSize.height() - 2 * margin,
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

  // Convert current lat/lon to tile-space at current zoom
  const QPointF gpsTile = latLonToTile(m_latitude, m_longitude, m_zoom);
  const QPointF deltaBase = gpsTile - centerTile;

  // Render the cross-hair three times: centre plus one wrap left / right
  for (int wrap = -1; wrap <= 1; ++wrap)
  {
    // Shift X by ±(2^zoom) tiles to handle east/west wrap-around
    const QPointF delta = deltaBase + QPointF(wrap * (1 << m_zoom), 0);

    // Convert tile delta to on-screen pixel position
    const QPoint drawPos(viewSize.width() / 2 + delta.x() * tileSize,
                         viewSize.height() / 2 + delta.y() * tileSize);

    // Set indicator sizes
    constexpr int dotRadius = 5;
    constexpr int haloRadius = 14;
    constexpr int outerRadius = 7;

    // Set indicator colors
    constexpr QColor dotColor(0x00, 0x7a, 0xff);
    constexpr QColor borderColor(0xff, 0xff, 0xff);
    constexpr QColor haloColor(0x00, 0x7a, 0xff, 0x3f);

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
 * @brief Sets the zoom level.
 * @param zoom New zoom level.
 *
 * Updates the zoom and triggers repaint if it changed.
 */
void Widgets::GPS::setZoomLevel(int zoom)
{
  const auto z = qBound(MIN_ZOOM, zoom, m_mapMaxZoom[m_mapType]);
  if (m_zoom != z)
  {
    m_zoom = z;
    update();
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
             "incurs real costs."));
    }

    m_mapType = mapId;
    m_tileCache.clear();
    m_settings.setValue(QStringLiteral("gpsMapType"), mapId);

    if (m_zoom > m_mapMaxZoom[type])
      m_zoom = m_mapMaxZoom[type];

    updateTiles();
    precacheWorld();
    updateData();

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
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardGPS, m_index);

    double lat = 0, lon = 0, alt = 0;
    for (int i = 0; i < group.datasetCount(); ++i)
    {
      const auto &dataset = group.getDataset(i);
      if (dataset.widget() == QStringLiteral("lat"))
        lat = dataset.value().toDouble();
      else if (dataset.widget() == QStringLiteral("lon"))
        lon = dataset.value().toDouble();
      else if (dataset.widget() == QStringLiteral("alt"))
        alt = dataset.value().toDouble();
    }

    if (!qFuzzyCompare(lat, m_latitude) || !qFuzzyCompare(lon, m_longitude)
        || !qFuzzyCompare(alt, m_altitude))
    {
      m_latitude = lat;
      m_altitude = alt;
      m_longitude = lon;

      if (autoCenter())
        center();

      else
      {
        updateTiles();
        update();
      }

      Q_EMIT updated();
    }
  }
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
                [=]() { onTileFetched(reply); });
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
                [=]() { onTileFetched(reply); });
      }
    }
  }
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
    const QPointF cursorPos = event->position();
    const QPointF cursorTileOffset((cursorPos.x() - width() / 2.0) / 256.0,
                                   (cursorPos.y() - height() / 2.0) / 256.0);

    const QPointF cursorTile = m_centerTile + cursorTileOffset;
    const QPointF geo = tileToLatLon(cursorTile, m_zoom);
    setZoomLevel(newZoom);

    const QPointF newTile = latLonToTile(geo.x(), geo.y(), m_zoom);
    m_centerTile = clampCenterTile(newTile - cursorTileOffset);

    updateTiles();
    update();
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
