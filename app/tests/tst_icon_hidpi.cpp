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

#include <QColor>
#include <QFile>
#include <QImage>
#include <QQuickView>
#include <QTemporaryDir>
#include <QTest>

/**
 * @brief Pins spec 0116 AC5: on every way the application loads an icon, Qt displays the @2x file
 *        above 1x scale and the plain file at 1x. The fixture is red at 1x and blue at 2x, and
 *        ctest runs the binary once per scale factor.
 */
class TstIconHidpi : public QObject {
  Q_OBJECT

private slots:
  void selectsTheFileForTheScale_data();
  void selectsTheFileForTheScale();
};

//--------------------------------------------------------------------------------------------------
// Load paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief The QML shapes the interface uses to show an icon.
 */
void TstIconHidpi::selectsTheFileForTheScale_data()
{
  QTest::addColumn<QString>("item");

  const QString source = QStringLiteral("source: \"qrc:/fixture/x.png\"");
  QTest::newRow("image") << QStringLiteral("Image { %1 }").arg(source);
  QTest::newRow("image with source size")
    << QStringLiteral("Image { %1; sourceSize: Qt.size(32, 32) }").arg(source);
  QTest::newRow("image with geometry")
    << QStringLiteral("Image { %1; width: 32; height: 32 }").arg(source);
  QTest::newRow("button icon") << QStringLiteral(
    "Button { padding: 0; background: null; display: AbstractButton.IconOnly; "
    "icon.source: \"qrc:/fixture/x.png\"; icon.width: 32; icon.height: 32; "
    "icon.color: \"transparent\" }");
}

/**
 * @brief Shows one icon, grabs the window and reads the colour at the centre of the artwork.
 */
void TstIconHidpi::selectsTheFileForTheScale()
{
  QFETCH(QString, item);

  QTemporaryDir dir;
  QVERIFY(dir.isValid());

  QFile qml(dir.filePath(QStringLiteral("probe.qml")));
  QVERIFY(qml.open(QFile::WriteOnly));
  qml.write(QStringLiteral("import QtQuick\nimport QtQuick.Controls.Basic\n"
                           "Rectangle { width: 64; height: 64; color: \"white\"\n%1\n}\n")
              .arg(item)
              .toUtf8());
  qml.close();

  QQuickView view;
  view.setSource(QUrl::fromLocalFile(qml.fileName()));
  QCOMPARE(view.status(), QQuickView::Ready);

  view.show();
  QVERIFY(QTest::qWaitForWindowExposed(&view));

  const auto shot  = view.grabWindow();
  const auto ratio = static_cast<double>(shot.width()) / view.width();
  QVERIFY(ratio > 0.0);

  const int centre  = static_cast<int>(16 * ratio);
  const auto colour = shot.pixelColor(centre, centre);
  const auto wanted = ratio > 1.0 ? QColor(Qt::blue) : QColor(Qt::red);
  QCOMPARE(colour.name(), wanted.name());
}

QTEST_MAIN(TstIconHidpi)

#include "tst_icon_hidpi.moc"
