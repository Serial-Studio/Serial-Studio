/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <qfloat16.h>

#include <cmath>
#include <QImage>
#include <QKeySequence>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickView>
#include <QQuickWindow>
#include <QTest>
#include <QUrl>

#include "Misc/HdrTransfer.h"
#include "UI/Automation/ElementCatalog.h"
#include "UI/Automation/InputSynthesizer.h"
#include "UI/Automation/WindowCapture.h"
#include "UI/Automation/WindowInventory.h"

using namespace UI::Automation;

/**
 * @brief The automation surface's Qt-facing classes (spec 0101) against an offscreen QQuickView:
 *        the window inventory, catalog ids, roles and geometry, the dynamic meta-object name
 *        parsing, modal popup detection, synchronous click and key delivery, the window grab
 *        with its crop and scale, and the inverse HDR transfer on a synthetic FP16 image.
 */
class TstUiAutomation : public QObject {
  Q_OBJECT

public:
  TstUiAutomation() : m_view(nullptr) {}

private slots:
  void initTestCase();
  void cleanupTestCase();

  void windowInventory();
  void catalogRolesAndIds();
  void catalogFilters();
  void componentNameParsing();
  void modalDetection();
  void clickById();
  void clickIgnoredWhenDisabled();
  void typedTextAndSelectAll();
  void grabWindowAndCrop();
  void inverseTransfer();

private:
  [[nodiscard]] QQuickItem* item(const QString& id) const;
  [[nodiscard]] static QRect devicePixels(const QRectF& rect, qreal ratio);

  QQuickView* m_view;
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the fixture into a software-rendered view under the Fusion style the application
 *        runs, so every CI leg renders the same and the platform default style never decides a
 *        control's meta-object, then shows it and makes it the active window, which key delivery
 *        needs.
 */
void TstUiAutomation::initTestCase()
{
  QQuickStyle::setStyle(QStringLiteral("Fusion"));
  QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
  m_view = new QQuickView;
  m_view->setObjectName(QStringLiteral("fixture"));
  m_view->setTitle(QStringLiteral("Automation fixture"));
  m_view->setResizeMode(QQuickView::SizeRootObjectToView);
  m_view->setSource(QUrl::fromLocalFile(QStringLiteral(SS_AUTOMATION_FIXTURE)));
  QCOMPARE(static_cast<int>(m_view->status()), static_cast<int>(QQuickView::Ready));
  QVERIFY(m_view->rootObject() != nullptr);

  m_view->resize(400, 300);
  m_view->show();
  QVERIFY(QTest::qWaitForWindowExposed(m_view));
  m_view->requestActivate();
  QTRY_VERIFY(m_view->isActive());
}

void TstUiAutomation::cleanupTestCase()
{
  delete m_view;
  m_view = nullptr;
}

/**
 * @brief The fixture item named @p id, by objectName.
 */
QQuickItem* TstUiAutomation::item(const QString& id) const
{
  if (m_view == nullptr || m_view->rootObject() == nullptr)
    return nullptr;

  return m_view->rootObject()->findChild<QQuickItem*>(id);
}

/**
 * @brief A window-coordinate rect in device pixels, the way the capture crops.
 */
QRect TstUiAutomation::devicePixels(const QRectF& rect, const qreal ratio)
{
  return QRectF(rect.topLeft() * ratio, rect.size() * ratio).toAlignedRect();
}

//--------------------------------------------------------------------------------------------------
// Windows
//--------------------------------------------------------------------------------------------------

/**
 * @brief The view is listed, resolves by its name and as the default, and is described with its
 *        title, scale and frame; no popup is open.
 */
void TstUiAutomation::windowInventory()
{
  auto* window = static_cast<QQuickWindow*>(m_view);
  QVERIFY(WindowInventory::windows().contains(window));
  QCOMPARE(WindowInventory::resolve(QStringLiteral("fixture")), window);
  QCOMPARE(WindowInventory::resolve(QString()), window);
  QVERIFY(WindowInventory::resolve(QStringLiteral("no.such.window")) == nullptr);

  const WindowInfo info = WindowInventory::describe(window);
  QCOMPARE(info.id, QStringLiteral("fixture"));
  QCOMPARE(info.title, QStringLiteral("Automation fixture"));
  QVERIFY(info.scale >= 1.0);
  QVERIFY(!info.frame.isEmpty());
  QVERIFY(!info.modal);
}

//--------------------------------------------------------------------------------------------------
// Catalog
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every fixture control is listed with the role its type maps to, its label, its state
 *        and its scene geometry; a password field reports no text; the unnamed rectangle is not
 *        listed and the named one is a plain item.
 */
void TstUiAutomation::catalogRolesAndIds()
{
  const auto elements = ElementCatalog::collect(m_view);
  QVERIFY(elements.size() >= 8);
  qsizetype plainItems = 0;
  for (const Element& element : elements)
    if (element.role == QStringLiteral("item"))
      ++plainItems;

  QCOMPARE(plainItems, qsizetype(1));
  const auto card = ElementCatalog::byId(m_view, QStringLiteral("fixture.card"));
  QVERIFY(card.has_value());
  QCOMPARE(card->role, QStringLiteral("item"));

  const auto button = ElementCatalog::byId(m_view, QStringLiteral("fixture.button"));
  QVERIFY(button.has_value());
  QCOMPARE(button->role, QStringLiteral("button"));
  QCOMPARE(button->label, QStringLiteral("Press"));
  QVERIFY(button->enabled);
  QCOMPARE(button->rect.size(), QSizeF(120.0, 32.0));
  QQuickItem* buttonItem = item(QStringLiteral("fixture.button"));
  QVERIFY(buttonItem != nullptr);
  const QRectF local(0.0, 0.0, buttonItem->width(), buttonItem->height());
  QCOMPARE(button->rect, buttonItem->mapRectToScene(local));

  const auto check = ElementCatalog::byId(m_view, QStringLiteral("fixture.check"));
  QVERIFY(check.has_value());
  QCOMPARE(check->role, QStringLiteral("checkbox"));
  QVERIFY(check->checked.has_value());
  QVERIFY(!*check->checked);

  const auto field = ElementCatalog::byId(m_view, QStringLiteral("fixture.field"));
  QVERIFY(field.has_value());
  QCOMPARE(field->role, QStringLiteral("editabletext"));
  QCOMPARE(field->label, QStringLiteral("Type here"));

  const auto secret = ElementCatalog::byId(m_view, QStringLiteral("fixture.secret"));
  QVERIFY(secret.has_value());
  QCOMPARE(secret->role, QStringLiteral("editabletext"));
  QVERIFY(secret->text.isEmpty());
  QVERIFY(!secret->label.contains(QStringLiteral("hunter2")));
  QVERIFY(!ElementCatalog::toJson(*secret).contains(QStringLiteral("text")));

  const auto combo = ElementCatalog::byId(m_view, QStringLiteral("fixture.combo"));
  QVERIFY(combo.has_value());
  QCOMPARE(combo->role, QStringLiteral("combobox"));
  QCOMPARE(combo->text, QStringLiteral("Alpha"));

  const auto toolbar = ElementCatalog::byId(m_view, QStringLiteral("fixture.toolbar"));
  QVERIFY(toolbar.has_value());
  QCOMPARE(toolbar->role, QStringLiteral("button"));
  QCOMPARE(toolbar->label, QStringLiteral("Tool"));

  const auto disabled = ElementCatalog::byId(m_view, QStringLiteral("fixture.disabled"));
  QVERIFY(disabled.has_value());
  QVERIFY(!disabled->enabled);
}

/**
 * @brief The id and label filters are case-insensitive substrings.
 */
void TstUiAutomation::catalogFilters()
{
  CatalogFilter byId;
  byId.id = QStringLiteral("CHECK");
  QCOMPARE(ElementCatalog::collect(m_view, byId).size(), qsizetype(1));

  CatalogFilter byLabel;
  byLabel.label      = QStringLiteral("press");
  const auto pressed = ElementCatalog::collect(m_view, byLabel);
  QCOMPARE(pressed.size(), qsizetype(1));
  QCOMPARE(pressed.first().id, QStringLiteral("fixture.button"));
}

/**
 * @brief A QML component with declared properties gets a dynamic meta-object named after its
 *        file plus the `_QMLTYPE_` marker (Qt internal, pinned here), and the catalog reads the
 *        component name back; a Qt control under Fusion, whose Button.qml declares no members,
 *        keeps its C++ class name (a style that does, FluentWinUI3 or Material, renames it).
 */
void TstUiAutomation::componentNameParsing()
{
  QQuickItem* toolbar = item(QStringLiteral("fixture.toolbar"));
  QVERIFY(toolbar != nullptr);
  const QString className = QString::fromLatin1(toolbar->metaObject()->className());
  QVERIFY2(className.contains(QStringLiteral("_QMLTYPE_")), qPrintable(className));
  QCOMPARE(ElementCatalog::componentName(toolbar->metaObject()), QStringLiteral("ToolbarButton"));

  QQuickItem* button = item(QStringLiteral("fixture.button"));
  QVERIFY(button != nullptr);
  QCOMPARE(ElementCatalog::componentName(button->metaObject()), QStringLiteral("QQuickButton"));
}

/**
 * @brief A modal popup is detected through the overlay once open, its controls join the
 *        catalog, and closing it clears the flag.
 */
void TstUiAutomation::modalDetection()
{
  QVERIFY(!WindowInventory::hasModalPopup(m_view));
  QVERIFY(QMetaObject::invokeMethod(m_view->rootObject(), "openPopup"));
  QTRY_VERIFY(WindowInventory::hasModalPopup(m_view));
  QVERIFY(ElementCatalog::byId(m_view, QStringLiteral("fixture.popup.close")).has_value());

  QVERIFY(QMetaObject::invokeMethod(m_view->rootObject(), "closePopup"));
  QTRY_VERIFY(!WindowInventory::hasModalPopup(m_view));
}

//--------------------------------------------------------------------------------------------------
// Input
//--------------------------------------------------------------------------------------------------

/**
 * @brief A synthesized click at a control's centre toggles it before the call returns, the point
 *        resolves back to the same control, and focus follows a click into the field.
 */
void TstUiAutomation::clickById()
{
  const auto check = ElementCatalog::byId(m_view, QStringLiteral("fixture.check"));
  QVERIFY(check.has_value());
  QVERIFY(!*check->checked);
  QVERIFY(
    InputSynthesizer::click(m_view, check->rect.center(), Qt::LeftButton, Qt::NoModifier, false));

  QQuickItem* checkItem = item(QStringLiteral("fixture.check"));
  QVERIFY(checkItem != nullptr);
  QVERIFY(checkItem->property("checked").toBool());

  const auto hit = ElementCatalog::atPoint(m_view, check->rect.center());
  QVERIFY(hit.has_value());
  QCOMPARE(hit->id, QStringLiteral("fixture.check"));

  const auto field = ElementCatalog::byId(m_view, QStringLiteral("fixture.field"));
  QVERIFY(field.has_value());
  QVERIFY(
    InputSynthesizer::click(m_view, field->rect.center(), Qt::LeftButton, Qt::NoModifier, false));
  QTRY_COMPARE(WindowInventory::focusedId(m_view), QStringLiteral("fixture.field"));
}

/**
 * @brief The catalog reports a disabled control as such, and Qt delivers nothing to it.
 */
void TstUiAutomation::clickIgnoredWhenDisabled()
{
  const auto off = ElementCatalog::byId(m_view, QStringLiteral("fixture.disabled"));
  QVERIFY(off.has_value());
  QVERIFY(!off->enabled);

  const bool pressed =
    InputSynthesizer::click(m_view, off->rect.center(), Qt::LeftButton, Qt::NoModifier, false);
  Q_UNUSED(pressed);
  QCOMPARE(m_view->rootObject()->property("disabledClicks").toInt(), 0);
}

/**
 * @brief Typed text lands in the focused field character by character, a Ctrl+A sequence selects
 *        it, and Backspace clears the selection.
 */
void TstUiAutomation::typedTextAndSelectAll()
{
  QQuickItem* field = item(QStringLiteral("fixture.field"));
  QVERIFY(field != nullptr);
  InputSynthesizer::focus(field);
  QTRY_VERIFY(field->hasActiveFocus());

  QVERIFY(InputSynthesizer::text(m_view, QStringLiteral("Hello 42")));
  QCOMPARE(field->property("text").toString(), QStringLiteral("Hello 42"));

  QVERIFY(InputSynthesizer::keys(m_view, QKeySequence(QStringLiteral("Ctrl+A"))));
  QCOMPARE(field->property("selectedText").toString(), QStringLiteral("Hello 42"));

  QVERIFY(InputSynthesizer::keys(m_view, QKeySequence(Qt::Key_Backspace)));
  QCOMPARE(field->property("text").toString(), QString());
}

//--------------------------------------------------------------------------------------------------
// Capture
//--------------------------------------------------------------------------------------------------

/**
 * @brief The spike AC16 asks for: a grab of the offscreen view decodes to the view size times the
 *        device pixel ratio, a crop equals the element rect in device pixels, and a scale halves
 *        the image. A platform that grabs nothing skips, and the GUI pytest tier carries AC4.
 */
void TstUiAutomation::grabWindowAndCrop()
{
  const auto full = WindowCapture::capture(m_view, QRectF(), 1.0);
  if (!full.has_value())
    QSKIP("grabWindow() returned no image on this platform; the GUI pytest tier carries AC4");

  const qreal ratio = m_view->devicePixelRatio();
  QCOMPARE(full->width, qRound(m_view->width() * ratio));
  QCOMPARE(full->height, qRound(m_view->height() * ratio));
  QCOMPARE(full->scale, 1.0);
  const QImage decoded = QImage::fromData(full->png, "PNG");
  QVERIFY(!decoded.isNull());
  QCOMPARE(decoded.size(), QSize(full->width, full->height));

  const auto button = ElementCatalog::byId(m_view, QStringLiteral("fixture.button"));
  QVERIFY(button.has_value());
  const auto crop = WindowCapture::capture(m_view, button->rect, 1.0);
  QVERIFY(crop.has_value());
  const QRect expected = devicePixels(button->rect, ratio);
  QCOMPARE(crop->width, expected.width());
  QCOMPARE(crop->height, expected.height());

  const auto half = WindowCapture::capture(m_view, QRectF(), 0.5);
  QVERIFY(half.has_value());
  QCOMPARE(half->scale, 0.5);
  QCOMPARE(QSize(half->width, half->height), QSize(full->width, full->height) * 0.5);
}

/**
 * @brief The inverse of the HDR output transform: scene-referred floats divided by the SDR white
 *        scale, clamped at SDR white, encoded to sRGB through the shared transfer header; an
 *        8-bit image passes through untouched.
 */
void TstUiAutomation::inverseTransfer()
{
  QImage hdr(4, 1, QImage::Format_RGBA16FPx4);
  QVERIFY(!hdr.isNull());
  auto* pixels                 = reinterpret_cast<qfloat16*>(hdr.scanLine(0));
  const float values[4]        = {0.0f, 1.0f, 2.0f, 4.0f};
  constexpr double kWhiteScale = 2.0;
  for (int x = 0; x < 4; ++x) {
    pixels[x * 4 + 0] = qfloat16(values[x]);
    pixels[x * 4 + 1] = qfloat16(values[x]);
    pixels[x * 4 + 2] = qfloat16(values[x]);
    pixels[x * 4 + 3] = qfloat16(1.0f);
  }

  const QImage out = WindowCapture::toDisplayReferred(hdr, kWhiteScale);
  QCOMPARE(out.format(), QImage::Format_RGB32);
  QCOMPARE(out.size(), hdr.size());
  QCOMPARE(qRed(out.pixel(0, 0)), 0);
  const int mid = static_cast<int>(std::lround(Misc::HdrTransfer::oetfExt(0.5f) * 255.0f));
  QVERIFY(std::abs(qRed(out.pixel(1, 0)) - mid) <= 1);
  QCOMPARE(qRed(out.pixel(2, 0)), 255);
  QCOMPARE(qRed(out.pixel(3, 0)), 255);
  QCOMPARE(qGreen(out.pixel(1, 0)), qRed(out.pixel(1, 0)));
  QCOMPARE(qBlue(out.pixel(1, 0)), qRed(out.pixel(1, 0)));

  QImage sdr(2, 2, QImage::Format_ARGB32);
  sdr.fill(Qt::blue);
  QCOMPARE(WindowCapture::toDisplayReferred(sdr, kWhiteScale), sdr);
  QVERIFY(WindowCapture::isFloatFormat(QImage::Format_RGBA16FPx4));
  QVERIFY(WindowCapture::isFloatFormat(QImage::Format_RGBA32FPx4_Premultiplied));
  QVERIFY(!WindowCapture::isFloatFormat(QImage::Format_ARGB32));
}

QTEST_MAIN(TstUiAutomation)

#include "tst_ui_automation.moc"
