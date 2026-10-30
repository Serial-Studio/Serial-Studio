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

#include <cmath>
#include <QTest>

#include "DataModel/FrameBuilder/QuickPlotFormatState.h"

// Quick Plot's format state (spec 0106): the 35-stream corpus of outcomes, channel titles, units
// and values, the rebuild budget, the replay pin and the reset.

using Outcome = DataModel::QuickPlotLineOutcome;

//--------------------------------------------------------------------------------------------------
// Corpus
//--------------------------------------------------------------------------------------------------

struct Fmt {
  int separator    = -1;
  int decimalComma = -1;
  int wrapper      = -1;
  int json         = -1;
};

struct Expect {
  QList<QByteArray> lines;
  QList<Outcome> outcomes;
  int channels = -1;
  QStringList titles;
  QList<double> numbers;
  QList<QByteArray> units;
  Fmt fmt;
};

struct StreamResult {
  QList<Outcome> outcomes;
  QStringList titles;
  QList<double> numbers;
  QList<QByteArray> units;
  DataModel::LineFormat format;
  int channels           = 0;
  int rebuildsAfterFirst = 0;
  bool established       = false;
};

static const double kNaN = qQNaN();

/**
 * @brief Streams 1 to 7 and 23 to 25: plain delimited input and its separators.
 */
[[nodiscard]] static QList<Expect> delimitedCorpus()
{
  const QStringList pos3{"", "", ""};
  const auto R = Outcome::Rebuild;
  QList<Expect> out;
  out.append({
    .lines    = {"23.5,60,1013", "23.5,60,1013"},
    .outcomes = {R, Outcome::Apply},
    .channels = 3,
    .titles   = pos3,
    .numbers  = {23.5, 60, 1013}
  });
  out.append({.lines = {"23.5, 60, 1013"}, .outcomes = {R}, .channels = 3, .titles = pos3});
  out.append({.lines = {"23.5;60;1013"}, .outcomes = {R}, .fmt = {.separator = ';'}});
  out.append({
    .lines    = {"23,5;60;1013,2"},
    .outcomes = {R},
    .numbers  = {23.5, 60, 1013.2},
    .fmt      = {.decimalComma = 1}
  });
  out.append({.lines = {"23.5\t60\t1013"}, .outcomes = {R}, .fmt = {.separator = '\t'}});
  out.append({.lines = {"23.5 60 1013"}, .outcomes = {R}, .fmt = {.separator = ' '}});
  out.append({
    .lines = {"23,5 60,2 1013"},
      .outcomes = {R},
      .numbers = {23.5, 60.2, 1013}
  });
  out.append({
    .lines = {"-12.5,1.5e3,+7"},
      .outcomes = {R},
      .numbers = {-12.5, 1500, 7}
  });
  out.append({
    .lines = {"23.5,60,1013,"},
      .outcomes = {R},
      .channels = 4, .numbers = {23.5, 60, 1013, kNaN}
  });
  out.append({
    .lines = {"12:30:45,23.5,60"},
      .outcomes = {R},
      .channels = 3, .numbers = {kNaN, 23.5, 60}
  });
  return out;
}

/**
 * @brief Streams 8 to 13, 26, 27, 30, 31 and 34: named fields, units and mixed streams.
 */
[[nodiscard]] static QList<Expect> namedCorpus()
{
  const auto R = Outcome::Rebuild;
  const QStringList th{"temp", "hum"};
  const QList<double> tv{23.5, 60};
  const QByteArray degC = QByteArray("\xC2\xB0") + "C";
  QList<Expect> out;
  out.append({.lines = {"temp:23.5,hum:60"}, .outcomes = {R}, .titles = th, .numbers = tv});
  out.append({.lines = {"temp: 23.5, hum: 60"}, .outcomes = {R}, .titles = th, .numbers = tv});
  out.append({.lines = {"temp=23.5;hum=60"}, .outcomes = {R}, .titles = th, .numbers = tv});
  out.append({.lines = {"temp:23.5 hum:60"}, .outcomes = {R}, .titles = th, .numbers = tv});
  out.append({
    .lines    = {"temp:23.5" + degC + ";pressure:1013hPa"},
    .outcomes = {R},
    .titles   = {"temp", "pressure"},
    .units    = {degC, "hPa"}
  });
  out.append({
    .lines    = {"3.3V,25" + degC + ",45%"},
    .outcomes = {R},
    .channels = 3,
    .titles   = {"", "", ""},
    .units    = {"V", degC, "%"}
  });
  out.append({
    .lines = {"Temp:23.5\tHum:60"},
      .outcomes = {R},
      .titles = {"Temp", "Hum"},
      .numbers = tv
  });
  out.append({
    .lines = {"temp:23.5,60"},
      .outcomes = {R},
      .titles = {"temp", ""},
      .numbers = tv
  });
  out.append({.lines = {"temp:23,5;hum:60"}, .outcomes = {R}, .titles = th, .numbers = tv});
  out.append({
    .lines    = {"Temp Sensor: 23.5, Humidity: 60"},
    .outcomes = {R},
    .titles   = {"Temp Sensor", "Humidity"}
  });
  out.append({
    .lines    = {"temp:23.5,status:OK"},
    .outcomes = {R},
    .titles   = {"temp", "status"},
    .numbers  = {23.5, kNaN}
  });
  return out;
}

/**
 * @brief Streams 14 to 17 and 35: JSON objects and the comment wrapper.
 */
[[nodiscard]] static QList<Expect> jsonCorpus()
{
  const auto R = Outcome::Rebuild;
  QList<Expect> out;
  out.append({
    .lines = {"{\"t\":23.5,\"h\":60}"},
      .outcomes = {R},
      .titles = {"t", "h"},
      .fmt = {.json = 1}
  });
  out.append({
    .lines    = {"{\"t\":23.5,\"ok\":true,\"n\":\"x\",\"h\":60}"},
    .outcomes = {R},
    .channels = 2,
    .titles   = {"t", "h"}
  });
  out.append({
    .lines    = {"{\"imu\":{\"ax\":0.1,\"g\":[1,2]},\"t\":5}"},
    .outcomes = {R},
    .titles   = {"imu.ax", "imu.g[0]", "imu.g[1]", "t"},
    .numbers  = {0.1, 1, 2, 5}
  });
  out.append({
    .lines    = {"/*23.5,60,1013*/"},
    .outcomes = {R},
    .channels = 3,
    .titles   = {"", "", ""},
    .fmt      = {.wrapper = 1}
  });
  out.append({
    .lines    = {"{\"acc\":[1,2]}", "{\"acc\":[1,2,3]}"},
    .outcomes = {R, R},
    .channels = 3,
    .titles   = {"acc[0]", "acc[1]", "acc[2]"},
    .numbers  = {1, 2, 3}
  });
  return out;
}

/**
 * @brief Streams 18 to 22, 28, 29, 32 and 33: headers, chatter and re-establishment.
 */
[[nodiscard]] static QList<Expect> sequenceCorpus()
{
  const auto R = Outcome::Rebuild;
  const auto H = Outcome::Header;
  QList<Expect> out;
  out.append({
    .lines    = {"Temperature,Pressure,Humidity", "23.5,1013.2,45"},
    .outcomes = {H, R},
    .titles   = {"Temperature", "Pressure", "Humidity"},
    .numbers  = {23.5, 1013.2, 45}
  });
  out.append({
    .lines    = {"WiFi connected", "IP: 192.168.1.42", "Connected in 1234 ms", "23.5,60,1013"},
    .outcomes = {H, H, H, R},
    .channels = 3,
    .titles   = {"", "", ""}
  });
  out.append({
    .lines    = {"uptime=1234", "temp=23.5,hum=60"},
    .outcomes = {            R,                  R},
    .channels = 2,
    .titles   = {       "temp",              "hum"},
    .numbers  = {         23.5,                 60}
  });
  out.append({
    .lines    = {"23.5,60", "WiFi reconnecting...", "23.6,61"},
    .outcomes = {R, Outcome::Text, Outcome::Apply},
    .channels = 2,
    .numbers  = {23.6, 61}
  });
  out.append({
    .lines    = {"23.5,60", "RSSI -67 dBm", "23.6,61"},
    .outcomes = {R, Outcome::Text, Outcome::Apply},
    .channels = 2,
    .numbers  = {23.6, 61}
  });
  out.append({.lines = {"0x1F,0x20"}, .outcomes = {H}, .channels = 0});
  out.append({
    .lines = {"hello", "world"},
      .outcomes = {      H,       H},
      .channels = 0
  });
  out.append({
    .lines = {"23.5,60", "temp:23.5;hum:60"},
      .outcomes = {        R,                  R},
      .titles = {   "temp",              "hum"}
  });
  out.append({
    .lines = {"60,1013", "23.5,60,1013"},
      .outcomes = {        R,              R},
      .channels = 3
  });
  return out;
}

/**
 * @brief The whole 35-stream corpus.
 */
[[nodiscard]] static QList<Expect> corpus()
{
  QList<Expect> all = delimitedCorpus();
  all.append(namedCorpus());
  all.append(jsonCorpus());
  all.append(sequenceCorpus());
  return all;
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the values of one line in channel order, NaN for a non-numeric or absent field.
 */
static void captureLine(DataModel::QuickPlotFormatState& st,
                        const DataModel::LineFields& fields,
                        StreamResult& result)
{
  const auto count = static_cast<qsizetype>(st.channels().size());
  result.numbers   = QList<double>(count, kNaN);
  for (qsizetype i = 0; i < fields.count; ++i) {
    const auto& field = fields.items[static_cast<size_t>(i)];
    const int channel = st.indexOf(field, i);
    if (channel >= 0 && field.numeric)
      result.numbers[channel] = field.number;
  }
}

/**
 * @brief Feeds one stream through a fresh state and records outcomes, titles, units and values.
 */
[[nodiscard]] static StreamResult runStream(const QList<QByteArray>& lines,
                                            DataModel::LineFields& fields)
{
  DataModel::QuickPlotFormatState st;
  StreamResult result;
  int rebuilds = 0;
  for (const auto& line : lines) {
    const auto outcome = st.observe(QByteArrayView(line), fields);
    result.outcomes.append(outcome);
    if (outcome == Outcome::Rebuild)
      ++rebuilds;

    if (outcome == Outcome::Rebuild || outcome == Outcome::Apply)
      captureLine(st, fields, result);
  }

  result.channels           = static_cast<int>(st.channels().size());
  result.rebuildsAfterFirst = qMax(0, rebuilds - 1);
  result.established        = st.established();
  result.format             = st.format();
  for (int i = 0; i < result.channels; ++i) {
    result.titles.append(st.channelTitle(i));
    result.units.append(st.channels()[static_cast<size_t>(i)].unit);
  }

  return result;
}

/**
 * @brief Checks the numbers of a stream's last applied line; NaN means the field is not numeric.
 */
static void verifyNumbers(const QList<double>& actual, const QList<double>& expected)
{
  QCOMPARE(actual.size(), expected.size());
  for (qsizetype i = 0; i < expected.size(); ++i)
    if (std::isnan(expected.at(i)))
      QVERIFY(std::isnan(actual.at(i)));
    else
      QVERIFY2(std::abs(actual.at(i) - expected.at(i)) < 1e-9, qPrintable(QString::number(i)));
}

/**
 * @brief Checks the format fields an expectation pins; -1 leaves a field unchecked.
 */
static void verifyFormat(const DataModel::LineFormat& actual, const Fmt& expected)
{
  if (expected.separator >= 0)
    QCOMPARE(static_cast<int>(actual.separator), expected.separator);

  if (expected.decimalComma >= 0)
    QCOMPARE(actual.decimalComma, expected.decimalComma != 0);

  if (expected.wrapper >= 0)
    QCOMPARE(actual.wrapper, expected.wrapper != 0);

  if (expected.json >= 0)
    QCOMPARE(actual.kind == DataModel::LineFormatKind::Json, expected.json != 0);
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class QuickPlotFormatStateTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void corpusStreams_data();
  void corpusStreams();
  void rebuildCountAcrossCorpus();
  void replayPinNeverDetects();
  void resetForgetsEverything();
  void registryCapsAtMaxLineFields();
  void doublyWrappedLineTokenizesOnce();

private:
  DataModel::LineFields m_fields;
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void QuickPlotFormatStateTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief One row per corpus stream, named by its first line.
 */
void QuickPlotFormatStateTest::corpusStreams_data()
{
  QTest::addColumn<int>("index");
  const auto cases = corpus();
  QCOMPARE(cases.size(), 35);
  for (qsizetype i = 0; i < cases.size(); ++i)
    QTest::newRow(cases.at(i).lines.first().constData()) << static_cast<int>(i);
}

/**
 * @brief Outcomes, channel count, titles, units, values and format of one stream.
 */
void QuickPlotFormatStateTest::corpusStreams()
{
  QFETCH(int, index);
  const auto cases   = corpus();
  const auto& expect = cases.at(index);
  const auto result  = runStream(expect.lines, m_fields);
  QCOMPARE(result.outcomes, expect.outcomes);
  QCOMPARE(result.established,
           result.outcomes.contains(Outcome::Rebuild) || result.outcomes.contains(Outcome::Apply));

  if (expect.channels >= 0)
    QCOMPARE(result.channels, expect.channels);

  if (!expect.titles.isEmpty())
    QCOMPARE(result.titles, expect.titles);

  if (!expect.units.isEmpty())
    QCOMPARE(result.units, expect.units);

  if (!expect.numbers.isEmpty())
    verifyNumbers(result.numbers, expect.numbers);

  verifyFormat(result.format, expect.fmt);
}

/**
 * @brief Only streams 20, 32, 33 and 35 re-establish after their first rebuild.
 */
void QuickPlotFormatStateTest::rebuildCountAcrossCorpus()
{
  int total = 0;
  for (const auto& expect : corpus())
    total += runStream(expect.lines, m_fields).rebuildsAfterFirst;

  QCOMPARE(total, 4);
}

/**
 * @brief A pinned state keeps its comma format and channels and never detects another.
 */
void QuickPlotFormatStateTest::replayPinNeverDetects()
{
  DataModel::QuickPlotFormatState st;
  st.pinPositional(3);
  QVERIFY(st.pinned());
  QVERIFY(st.established());
  QCOMPARE(st.channels().size(), size_t(3));

  const auto outcome = st.observe(QByteArrayView("temp:1;hum:2"), m_fields);
  QCOMPARE(outcome, Outcome::Text);
  QCOMPARE(st.format().separator, ',');
  QCOMPARE(st.channels().size(), size_t(3));
}

/**
 * @brief Reset drops the format, the registry and the establishment.
 */
void QuickPlotFormatStateTest::resetForgetsEverything()
{
  DataModel::QuickPlotFormatState st;
  QCOMPARE(st.observe(QByteArrayView("23.5,60"), m_fields), Outcome::Rebuild);
  QVERIFY(st.established());

  st.reset();
  QVERIFY(!st.established());
  QVERIFY(st.channels().empty());
}

/**
 * @brief A stream of ever-new names stops growing the registry at kMaxLineFields and then applies.
 */
void QuickPlotFormatStateTest::registryCapsAtMaxLineFields()
{
  DataModel::QuickPlotFormatState st;
  auto last = Outcome::Text;
  for (int i = 0; i < 600; ++i) {
    const QByteArray line = "n" + QByteArray::number(i) + ":1";
    last                  = st.observe(QByteArrayView(line), m_fields);
  }

  QCOMPARE(st.channels().size(), static_cast<size_t>(DataModel::kMaxLineFields));
  QCOMPARE(last, Outcome::Apply);
}

/**
 * @brief A doubly wrapped line is unwrapped once by detection and by the steady-state path.
 */
void QuickPlotFormatStateTest::doublyWrappedLineTokenizesOnce()
{
  DataModel::QuickPlotFormatState st;
  QCOMPARE(st.observe(QByteArrayView("banner text"), m_fields), Outcome::Header);
  QCOMPARE(st.observe(QByteArrayView("/* /* 1 2 */ */"), m_fields), Outcome::Rebuild);
  QVERIFY(st.established());
  QCOMPARE(st.channels().size(), size_t(4));
}

QTEST_APPLESS_MAIN(QuickPlotFormatStateTest)

#include "tst_quickplot_format_state.moc"
