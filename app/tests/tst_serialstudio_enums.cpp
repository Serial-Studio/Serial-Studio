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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QJsonValue>
#include <QMetaEnum>
#include <QTest>
#include <QVariant>

#include "Core/SerialStudio.h"

/**
 * @brief Pins the persisted ordinals of the SerialStudio vocabulary now that it is a Core
 *        namespace (spec 0077): a moved enumerator would silently re-key every saved workspace,
 *        FFT window and widget-settings entry.
 */
class TstSerialStudioEnums : public QObject {
  Q_OBJECT

private slots:
  void fftWindowOrdinalsAreStable();
  void dashboardWidgetPinsAreStable();
  void busTypeOrdinalsAreStable();
  void operationModeOrdinalsAreStable();
  void enumsAreReflectedThroughTheNamespaceMetaObject();
  void widgetIdRoundTrips();
  void textEncodingNamesRoundTrip();
  void toDoubleUnwrapsAVariantHoldingAJsonValue();
};

/**
 * @brief The FFT window list is append-only: every ordinal is persisted.
 */
void TstSerialStudioEnums::fftWindowOrdinalsAreStable()
{
  QCOMPARE(static_cast<int>(SerialStudio::FFTWindowRectangular), 0);
  QCOMPARE(static_cast<int>(SerialStudio::FFTWindowHann), 2);
  QCOMPARE(static_cast<int>(SerialStudio::FFTWindowBlackmanHarris), 5);
  QCOMPARE(static_cast<int>(SerialStudio::FFTWindowParzen), 14);
}

/**
 * @brief BarPanel and Extension keep their explicit values in both build configurations, the LED
 *        Panel group widget rides the persisted LED dashboard type, and its GroupWidget ordinal
 *        (the JSON API's widgetType) stays appended after BarPanel.
 */
void TstSerialStudioEnums::dashboardWidgetPinsAreStable()
{
  QCOMPARE(static_cast<int>(SerialStudio::DashboardTerminal), 0);
  QCOMPARE(static_cast<int>(SerialStudio::DashboardLED), 8);
  QCOMPARE(static_cast<int>(SerialStudio::DashboardNoWidget), 17);
  QCOMPARE(static_cast<int>(SerialStudio::DashboardBarPanel), 90);
  QCOMPARE(static_cast<int>(SerialStudio::DashboardExtension), 100);
  QCOMPARE(static_cast<int>(SerialStudio::BarPanel), 10);
  QCOMPARE(static_cast<int>(SerialStudio::LEDPanel), 11);
}

/**
 * @brief The three GPL bus types keep their ordinals ahead of the commercial block.
 */
void TstSerialStudioEnums::busTypeOrdinalsAreStable()
{
  QCOMPARE(static_cast<int>(SerialStudio::BusType::UART), 0);
  QCOMPARE(static_cast<int>(SerialStudio::BusType::Network), 1);
  QCOMPARE(static_cast<int>(SerialStudio::BusType::BluetoothLE), 2);
}

/**
 * @brief The operation mode ordinals are what QSettings stores.
 */
void TstSerialStudioEnums::operationModeOrdinalsAreStable()
{
  QCOMPARE(static_cast<int>(SerialStudio::ProjectFile), 0);
  QCOMPARE(static_cast<int>(SerialStudio::ConsoleOnly), 1);
  QCOMPARE(static_cast<int>(SerialStudio::QuickPlot), 2);
}

/**
 * @brief Q_ENUM_NS keeps the enums reflectable, which the API layer and QML rely on.
 */
void TstSerialStudioEnums::enumsAreReflectedThroughTheNamespaceMetaObject()
{
  const QMetaEnum busType = QMetaEnum::fromType<SerialStudio::BusType>();
  QVERIFY(busType.isValid());
  QCOMPARE(QString::fromLatin1(busType.valueToKey(0)), QStringLiteral("UART"));

  const QMetaEnum mode = QMetaEnum::fromType<SerialStudio::OperationMode>();
  QVERIFY(mode.isValid());
  QCOMPARE(mode.keyToValue("QuickPlot"), static_cast<int>(SerialStudio::QuickPlot));
}

/**
 * @brief The persisted widget id strings survive the move to a namespace.
 */
void TstSerialStudioEnums::widgetIdRoundTrips()
{
  QCOMPARE(SerialStudio::groupWidgetId(SerialStudio::Gyroscope), QStringLiteral("gyro"));
  QCOMPARE(SerialStudio::groupWidgetFromId(QStringLiteral("gyroscope")), SerialStudio::Gyroscope);
  QCOMPARE(SerialStudio::groupWidgetFromId(QStringLiteral("map")), SerialStudio::GPS);
  QCOMPARE(SerialStudio::datasetWidgetId(SerialStudio::Compass), QStringLiteral("compass"));
  QCOMPARE(SerialStudio::datasetWidgetFromId(QStringLiteral("meter")), SerialStudio::Meter);
  QCOMPARE(SerialStudio::datasetWidgetFromId(QStringLiteral("nope")),
           SerialStudio::NoDatasetWidget);
}

/**
 * @brief Encoding names map both ways, including the legacy aliases.
 */
void TstSerialStudioEnums::textEncodingNamesRoundTrip()
{
  QCOMPARE(SerialStudio::textEncodingName(SerialStudio::EncShiftJis), QStringLiteral("Shift_JIS"));
  QCOMPARE(SerialStudio::textEncodingFromName(QStringLiteral("cp932")), SerialStudio::EncShiftJis);
  QCOMPARE(SerialStudio::textEncodingFromName(QStringLiteral("latin1")), SerialStudio::EncLatin1);
  QCOMPARE(SerialStudio::textEncodingFromName(QString()), SerialStudio::EncUtf8);
}

/**
 * @brief A QVariant wrapping a QJsonValue must reach the JSON overload, never re-enter the
 *        QVariant overload through implicit conversion: the namespace form of the helpers made
 *        that recursion possible once (a stack overflow at startup, 2026-09-08).
 */
void TstSerialStudioEnums::toDoubleUnwrapsAVariantHoldingAJsonValue()
{
  bool ok                = false;
  const QVariant wrapped = QVariant::fromValue(QJsonValue(1.5));
  QCOMPARE(SerialStudio::toDouble(wrapped, &ok), 1.5);
  QVERIFY(ok);
  QCOMPARE(SerialStudio::toDouble(QVariant::fromValue(QJsonValue(QStringLiteral("2.25")))), 2.25);
  QCOMPARE(SerialStudio::toDouble(QJsonValue(QStringLiteral("x")), 7.0), 7.0);
}

QTEST_GUILESS_MAIN(TstSerialStudioEnums)
#include "tst_serialstudio_enums.moc"
