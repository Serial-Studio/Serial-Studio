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

#include <QByteArray>
#include <QFile>
#include <QSignalSpy>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QVariantList>

#include "Console/Annotations.h"

/**
 * @file tst_console_annotations.cpp
 * @brief The frame annotation layer (spec 0059): interned texts stay bounded, the store trims
 *        with the retained byte window, the filter proxy and CSV export see the same rows, payload
 *        extraction reproduces the annotated bytes byte-for-byte, and a JS decoder runs with
 *        carry-over, is disabled on a throw, and never blocks the caller.
 */
class TstConsoleAnnotations : public QObject {
  Q_OBJECT

private slots:
  void internsTextsAndBoundsThem();
  void trimsToRetainedBytesAndCapacity();
  void filterAndCsvAgree();
  void payloadReproducesAnnotatedBytes();
  void decoderRunsWithCarryOver();
  void decoderIsDisabledOnThrow();
  void decoderRejectsBadLayout();

private:
  [[nodiscard]] static QVariantList twoClasses();
};

/**
 * @brief Two named classes with explicit colours.
 */
QVariantList TstConsoleAnnotations::twoClasses()
{
  QVariantMap header;
  header.insert(QStringLiteral("name"), QStringLiteral("header"));
  header.insert(QStringLiteral("color"), QStringLiteral("#ff0000"));
  QVariantMap payload;
  payload.insert(QStringLiteral("name"), QStringLiteral("payload"));
  payload.insert(QStringLiteral("color"), QStringLiteral("#00ff00"));
  return {header, payload};
}

/**
 * @brief The same text is interned once; past kMaxTexts new strings collapse onto id 0.
 */
void TstConsoleAnnotations::internsTextsAndBoundsThem()
{
  Console::AnnotationModel model;
  model.declareLayout({QStringLiteral("bytes")}, twoClasses());
  QCOMPARE(model.textCount(), 1);

  for (int i = 0; i < 1000; ++i)
    model.annotate(i, i, 0, 0, {QStringLiteral("SYNC"), QStringLiteral("S")});

  model.commitPending();
  QCOMPARE(model.count(), 1000);
  QCOMPARE(model.textCount(), 3);
  QCOMPARE(model.text(999, 0), QStringLiteral("SYNC"));
  QCOMPARE(model.text(999, 2), QStringLiteral("S"));
  QCOMPARE(model.text(999, 1), QStringLiteral("S"));

  for (int i = 0; i < Console::AnnotationModel::kMaxTexts + 50; ++i)
    model.annotate(2000 + i, 2000 + i, 0, 1, {QStringLiteral("unique %1").arg(i)});

  model.commitPending();
  QCOMPARE(model.textCount(), Console::AnnotationModel::kMaxTexts);
  QCOMPARE(model.text(model.count() - 1, 0), QStringLiteral("..."));

  model.annotate(5, 4, 0, 0, {QStringLiteral("bad")});
  model.annotate(5, 6, 3, 0, {QStringLiteral("no such row")});
  model.annotate(5, 6, 0, 9, {QStringLiteral("no such class")});
  model.commitPending();
  QCOMPARE(model.count(), 1000 + Console::AnnotationModel::kMaxTexts + 50);
}

/**
 * @brief Annotations that end before the retained byte window drop with it, and the store trims
 *        a quarter when it reaches kMaxAnnotations.
 */
void TstConsoleAnnotations::trimsToRetainedBytesAndCapacity()
{
  Console::AnnotationModel model;
  model.declareLayout({QStringLiteral("bytes")}, twoClasses());

  const QByteArray chunk(4096, 'x');
  model.ingestBytes(chunk);
  model.ingestBytes(chunk);
  model.annotate(0, 15, 0, 0, {QStringLiteral("early")});
  model.annotate(4100, 4200, 0, 1, {QStringLiteral("late")});
  model.commitPending();
  QCOMPARE(model.retainedStart(), qint64(0));
  QCOMPARE(model.retainedEnd(), qint64(8192));

  const qint64 blocks = Console::AnnotationModel::kMaxRetainedBytes / chunk.size();
  for (qint64 i = 0; i < blocks - 1; ++i)
    model.ingestBytes(chunk);

  QCOMPARE(model.retainedStart(), qint64(4096));
  QCOMPARE(model.count(), 1);
  QCOMPARE(model.text(0, 0), QStringLiteral("late"));

  Console::AnnotationModel big;
  big.declareLayout({QStringLiteral("bytes")}, twoClasses());
  for (int i = 0; i < Console::AnnotationModel::kMaxAnnotations + 10; ++i)
    big.annotate(i, i, 0, 0, {QStringLiteral("x")});

  big.commitPending();
  QVERIFY(big.count() <= Console::AnnotationModel::kMaxAnnotations);
  QVERIFY(big.count() > Console::AnnotationModel::kMaxAnnotations * 3 / 4);
  QCOMPARE(big.at(0).start, qint64(Console::AnnotationModel::kMaxAnnotations / 4));
}

/**
 * @brief The filter proxy narrows by row and class, and the CSV holds one line per annotation.
 */
void TstConsoleAnnotations::filterAndCsvAgree()
{
  Console::AnnotationModel model;
  model.declareLayout({QStringLiteral("bytes"), QStringLiteral("packets")}, twoClasses());
  model.annotate(0, 0, 0, 0, {QStringLiteral("h")});
  model.annotate(1, 3, 0, 1, {QStringLiteral("p")});
  model.annotate(0, 3, 1, 1, {QStringLiteral("packet \"1\"")});
  model.commitPending();

  Console::AnnotationFilter filter;
  filter.setSourceModel(&model);
  QCOMPARE(filter.rowCount(), 3);
  filter.setRowFilter(1);
  QCOMPARE(filter.rowCount(), 1);
  filter.setRowFilter(-1);
  filter.setClassFilter(1);
  QCOMPARE(filter.rowCount(), 2);
  filter.setClassFilter(-1);
  QCOMPARE(filter.rowCount(), 3);

  QCOMPARE(
    model.data(model.index(2, Console::AnnotationModel::ClassColumn), Qt::DisplayRole).toString(),
    QStringLiteral("payload"));
  QCOMPARE(model.data(model.index(2, 0), Console::AnnotationModel::RowNameRole).toString(),
           QStringLiteral("packets"));

  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = dir.filePath(QStringLiteral("annotations.csv"));
  QVERIFY(model.exportCsv(path));

  QFile file(path);
  QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
  const QStringList lines =
    QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  QCOMPARE(lines.size(), 4);
  QCOMPARE(lines.at(0), QStringLiteral("start,end,length,row,class,text"));
  QVERIFY(lines.at(3).contains(QStringLiteral("\"packet \"\"1\"\"\"")));
}

/**
 * @brief The payload view concatenates exactly the bytes of one class, in stream order.
 */
void TstConsoleAnnotations::payloadReproducesAnnotatedBytes()
{
  Console::AnnotationModel model;
  model.declareLayout({QStringLiteral("bytes")}, twoClasses());
  model.ingestBytes(QByteArrayLiteral("\xAAhello\xBBworld"));
  model.annotate(0, 0, 0, 0, {QStringLiteral("start")});
  model.annotate(1, 5, 0, 1, {QStringLiteral("hello")});
  model.annotate(6, 6, 0, 0, {QStringLiteral("start")});
  model.annotate(7, 11, 0, 1, {QStringLiteral("world")});
  model.commitPending();

  QCOMPARE(model.payloadBytes(1, 1024), QByteArrayLiteral("helloworld"));
  QCOMPARE(model.payloadBytes(0, 1024), QByteArrayLiteral("\xAA\xBB"));
  QCOMPARE(model.payloadBytes(1, 3), QByteArrayLiteral("hel"));
  QCOMPARE(model.payloadHex(0, 1024), QStringLiteral("AA BB"));
  QCOMPARE(model.payloadText(1, 1024), QStringLiteral("helloworld"));

  const auto spans = model.trackSpans(0, 20, 0, 100);
  QCOMPARE(spans.size(), 4);
  QCOMPARE(spans.at(1).toMap().value(QStringLiteral("text")).toString(), QStringLiteral("hello"));
  QCOMPARE(model.trackSpans(0, 20, 1, 100).size(), 0);
  QCOMPARE(model.trackSpans(0, 20, 0, 2).size(), 2);
}

/**
 * @brief A newline-framed JS decoder annotates across chunk boundaries: bytes it did not consume
 *        carry into the next call at the right absolute offset.
 */
void TstConsoleAnnotations::decoderRunsWithCarryOver()
{
  Console::AnnotationModel model;
  Console::AnnotationDecoder decoder(&model);
  decoder.setCode(QStringLiteral(
    "decoder = {\n"
    "  rows: ['bytes', 'lines'],\n"
    "  classes: [{name: 'text', color: '#123456'}, 'newline'],\n"
    "  decode: function(bytes, offset, ctx) {\n"
    "    const b = new Uint8Array(bytes);\n"
    "    let i = 0;\n"
    "    while (i < b.length) {\n"
    "      const start = i;\n"
    "      while (i < b.length && b[i] !== 10) ++i;\n"
    "      if (i >= b.length) return start;\n"
    "      if (i > start) ctx.annotate(offset + start, offset + i - 1, 1, 0, ['line', 'L']);\n"
    "      ctx.annotate(offset + i, offset + i, 0, 1, ['LF']);\n"
    "      ++i;\n"
    "    }\n"
    "    return i;\n"
    "  }\n"
    "};\n"));
  QVERIFY2(decoder.compiled(), qPrintable(decoder.lastError()));
  QVERIFY(!decoder.failed());
  QCOMPARE(model.rowNames(), (QStringList{QStringLiteral("bytes"), QStringLiteral("lines")}));
  QCOMPARE(model.classes().size(), 2);

  decoder.setEnabled(true);
  QVERIFY(decoder.enabled());

  decoder.feed(QByteArrayLiteral("abc\nde"));
  model.commitPending();
  QCOMPARE(model.count(), 2);
  QCOMPARE(model.at(0).start, qint64(0));
  QCOMPARE(model.at(0).end, qint64(2));
  QCOMPARE(model.at(1).start, qint64(3));

  decoder.feed(QByteArrayLiteral("f\n"));
  model.commitPending();
  QCOMPARE(model.count(), 4);
  QCOMPARE(model.at(2).start, qint64(4));
  QCOMPARE(model.at(2).end, qint64(6));
  QCOMPARE(model.at(3).start, qint64(7));
  QCOMPARE(model.retainedEnd(), qint64(8));

  decoder.setEnabled(false);
  decoder.feed(QByteArrayLiteral("ignored\n"));
  model.commitPending();
  QCOMPARE(model.count(), 4);
  QCOMPARE(model.retainedEnd(), qint64(16));
}

/**
 * @brief A decoder that throws is disabled after one failure with the message exposed; a fresh
 *        setCode() clears the failure.
 */
void TstConsoleAnnotations::decoderIsDisabledOnThrow()
{
  Console::AnnotationModel model;
  Console::AnnotationDecoder decoder(&model);
  QSignalSpy spy(&decoder, &Console::AnnotationDecoder::stateChanged);

  decoder.setCode(QStringLiteral("decoder = { rows: ['r'], classes: ['c'],"
                                 " decode: function() { throw new Error('boom'); } };"));
  QVERIFY(decoder.compiled());
  decoder.setEnabled(true);
  decoder.feed(QByteArrayLiteral("x"));

  QVERIFY(decoder.failed());
  QVERIFY(!decoder.enabled());
  QVERIFY(decoder.lastError().contains(QStringLiteral("boom")));
  QCOMPARE(decoder.errorCount(), quint64(1));
  QVERIFY(spy.count() >= 2);

  decoder.feed(QByteArrayLiteral("y"));
  QCOMPARE(decoder.errorCount(), quint64(1));
  QCOMPARE(model.retainedEnd(), qint64(2));

  decoder.setCode(QStringLiteral("decoder = { rows: ['r'], classes: ['c'],"
                                 " decode: function(b) { return b.byteLength; } };"));
  QVERIFY(decoder.compiled());
  QVERIFY(!decoder.failed());
}

/**
 * @brief Missing decoder object, missing decode(), or empty rows/classes are compile errors.
 */
void TstConsoleAnnotations::decoderRejectsBadLayout()
{
  Console::AnnotationModel model;
  Console::AnnotationDecoder decoder(&model);

  decoder.setCode(QStringLiteral("var x = 1;"));
  QVERIFY(!decoder.compiled());
  QVERIFY(decoder.failed());

  decoder.setCode(QStringLiteral("decoder = { rows: ['r'], classes: ['c'] };"));
  QVERIFY(!decoder.compiled());

  decoder.setCode(QStringLiteral("decoder = { rows: [], classes: ['c'],"
                                 " decode: function() { return 0; } };"));
  QVERIFY(!decoder.compiled());

  decoder.setCode(QStringLiteral("this is not javascript ("));
  QVERIFY(!decoder.compiled());
  QVERIFY(!decoder.lastError().isEmpty());

  decoder.setCode(QString());
  QVERIFY(!decoder.compiled());
  QVERIFY(!decoder.failed());
}

QTEST_GUILESS_MAIN(TstConsoleAnnotations)

#include "tst_console_annotations.moc"
