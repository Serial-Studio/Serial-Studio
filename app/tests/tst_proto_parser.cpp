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

#include <QString>
#include <QTest>

#include "DataModel/Importers/ProtoParser.h"

// The parser is the half of the proto import that owns no session, no project and no dialog, so
// every case below is a plain string in and a model out. tst_proto_importer covers the generated
// project; this suite covers the grammar it is generated from. Sources are written with explicit
// "\n" so an asserted diagnostic line number is readable at the call site.

/**
 * @brief Grammar contract of DataModel::ProtoParser: the proto3 subset it accepts, the shape of
 *        the model it emits, and the line numbers it reports when it refuses a file.
 */
class TstProtoParser : public QObject {
  Q_OBJECT

private slots:
  void scalarKeywordsClassify();
  void packageQualifiesTopLevelMessages();
  void nestedMessagesCarryQualifiedNames();
  void commentsAreTriviaButStillCountLines();
  void fieldModifiersAndTagsAreParsed();
  void oneofMapAndEnumAreFolded();
  void topLevelBoilerplateIsSkipped();
  void nestingDepthIsBounded();

  void malformedSourceReportsLine_data();
  void malformedSourceReportsLine();
};

//--------------------------------------------------------------------------------------------------
// Scalar classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every proto3 scalar keyword maps to its own enumerator; anything else is a type
 *        reference, which is what makes an unknown name resolvable as a message later.
 */
void TstProtoParser::scalarKeywordsClassify()
{
  using DataModel::ProtoScalar;

  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("double")), ProtoScalar::Double);
  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("float")), ProtoScalar::Float);
  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("uint64")), ProtoScalar::UInt64);
  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("sfixed32")), ProtoScalar::SFixed32);
  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("bool")), ProtoScalar::Bool);
  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("bytes")), ProtoScalar::Bytes);

  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("Attitude")), ProtoScalar::MessageRef);
  QCOMPARE(DataModel::classifyProtoScalar(QStringLiteral("Double")), ProtoScalar::MessageRef);
  QCOMPARE(DataModel::classifyProtoScalar(QString()), ProtoScalar::MessageRef);
}

//--------------------------------------------------------------------------------------------------
// Naming
//--------------------------------------------------------------------------------------------------

/**
 * @brief The package prefixes every top-level message's qualified name: the importer resolves
 *        message-typed fields against those names, so a dropped package silently unlinks them.
 */
void TstProtoParser::packageQualifiesTopLevelMessages()
{
  const QString src = QStringLiteral("syntax = \"proto3\";\n"
                                     "package telemetry.v2;\n"
                                     "message Attitude {\n"
                                     "  float roll = 1;\n"
                                     "}\n");

  DataModel::ProtoParser parser(src);
  QVERIFY(parser.parse());
  QCOMPARE(parser.package(), QStringLiteral("telemetry.v2"));
  QCOMPARE(parser.messages().size(), 1);
  QCOMPARE(parser.messages().at(0).name, QStringLiteral("Attitude"));
  QCOMPARE(parser.messages().at(0).qualifiedName, QStringLiteral("telemetry.v2.Attitude"));
}

/**
 * @brief A nested message is collected under its parent and qualified through it, while the
 *        field that references it stays a MessageRef carrying the written type reference.
 */
void TstProtoParser::nestedMessagesCarryQualifiedNames()
{
  const QString src = QStringLiteral("syntax = \"proto3\";\n"
                                     "package sensors;\n"
                                     "message Outer {\n"
                                     "  message Inner {\n"
                                     "    int32 depth = 1;\n"
                                     "  }\n"
                                     "  Inner inner = 1;\n"
                                     "  int32 flat = 2;\n"
                                     "}\n");

  DataModel::ProtoParser parser(src);
  QVERIFY(parser.parse());
  QCOMPARE(parser.messages().size(), 1);

  const auto& outer = parser.messages().at(0);
  QCOMPARE(outer.qualifiedName, QStringLiteral("sensors.Outer"));
  QCOMPARE(outer.nested.size(), 1);
  QCOMPARE(outer.nested.at(0).name, QStringLiteral("Inner"));
  QCOMPARE(outer.nested.at(0).qualifiedName, QStringLiteral("sensors.Outer.Inner"));
  QCOMPARE(outer.nested.at(0).fields.size(), 1);

  QCOMPARE(outer.fields.size(), 2);
  QCOMPARE(outer.fields.at(0).typeRef, QStringLiteral("Inner"));
  QCOMPARE(outer.fields.at(0).scalar, DataModel::ProtoScalar::MessageRef);
  QCOMPARE(outer.fields.at(1).scalar, DataModel::ProtoScalar::Int32);
}

//--------------------------------------------------------------------------------------------------
// Trivia
//--------------------------------------------------------------------------------------------------

/**
 * @brief Comments never reach the model, but the newlines inside a block comment still advance
 *        the diagnostic line: an off-by-N here points a user at the wrong line of their schema.
 */
void TstProtoParser::commentsAreTriviaButStillCountLines()
{
  const QString ok = QStringLiteral("syntax = \"proto3\";  // trailing\n"
                                    "/* block\n"
                                    "   comment */\n"
                                    "message A {\n"
                                    "  int32 x = 1;  // field comment\n"
                                    "  /* inline */ int32 y = 2;\n"
                                    "}\n");

  DataModel::ProtoParser parser(ok);
  QVERIFY(parser.parse());
  QCOMPARE(parser.messages().size(), 1);
  QCOMPARE(parser.messages().at(0).fields.size(), 2);
  QCOMPARE(parser.messages().at(0).fields.at(1).name, QStringLiteral("y"));

  const QString bad = QStringLiteral("syntax = \"proto3\";\n"
                                     "// line comment\n"
                                     "/* block\n"
                                     "   comment\n"
                                     "   spanning */\n"
                                     "message A {\n"
                                     "  int32 x = 0;\n"
                                     "}\n");

  DataModel::ProtoParser failing(bad);
  QVERIFY(!failing.parse());
  QCOMPARE(failing.error().line, 7);
}

//--------------------------------------------------------------------------------------------------
// Fields
//--------------------------------------------------------------------------------------------------

/**
 * @brief Labels, dotted type references, option lists and the tag range: only `repeated` marks a
 *        field repeated, and the tag ceiling is the protobuf maximum rather than an int cap.
 */
void TstProtoParser::fieldModifiersAndTagsAreParsed()
{
  const QString src = QStringLiteral("syntax = \"proto3\";\n"
                                     "message A {\n"
                                     "  repeated int32 many = 1;\n"
                                     "  optional int32 maybe = 2;\n"
                                     "  required int32 must = 3;\n"
                                     "  other.pkg.Ref ref = 4;\n"
                                     "  int32 flagged = 5 [deprecated = true, packed = false];\n"
                                     "  int32 ceiling = 536870911;\n"
                                     "}\n");

  DataModel::ProtoParser parser(src);
  QVERIFY(parser.parse());

  const auto& fields = parser.messages().at(0).fields;
  QCOMPARE(fields.size(), 6);
  QVERIFY(fields.at(0).repeated);
  QVERIFY(!fields.at(1).repeated);
  QVERIFY(!fields.at(2).repeated);
  QCOMPARE(fields.at(3).typeRef, QStringLiteral("other.pkg.Ref"));
  QCOMPARE(fields.at(3).scalar, DataModel::ProtoScalar::MessageRef);
  QCOMPARE(fields.at(4).name, QStringLiteral("flagged"));
  QCOMPARE(fields.at(4).tag, 5);
  QCOMPARE(fields.at(5).tag, 536870911);
}

/**
 * @brief oneof members are flattened into the message, a map becomes one repeated bytes field,
 *        and an enum body is skipped while its name stays usable as a field type.
 */
void TstProtoParser::oneofMapAndEnumAreFolded()
{
  const QString src = QStringLiteral("syntax = \"proto3\";\n"
                                     "message A {\n"
                                     "  enum Mode {\n"
                                     "    IDLE = 0;\n"
                                     "    RUN = 1;\n"
                                     "  }\n"
                                     "  oneof choice {\n"
                                     "    int32 first = 1;\n"
                                     "    string second = 2;\n"
                                     "  }\n"
                                     "  map<string, int32> lookup = 3;\n"
                                     "  Mode mode = 4;\n"
                                     "  reserved 9;\n"
                                     "}\n");

  DataModel::ProtoParser parser(src);
  QVERIFY(parser.parse());

  const auto& fields = parser.messages().at(0).fields;
  QCOMPARE(fields.size(), 4);
  QCOMPARE(fields.at(0).name, QStringLiteral("first"));
  QCOMPARE(fields.at(1).scalar, DataModel::ProtoScalar::String);

  QCOMPARE(fields.at(2).name, QStringLiteral("lookup"));
  QCOMPARE(fields.at(2).tag, 3);
  QVERIFY(fields.at(2).repeated);
  QCOMPARE(fields.at(2).scalar, DataModel::ProtoScalar::Bytes);

  QCOMPARE(fields.at(3).name, QStringLiteral("mode"));
  QCOMPARE(fields.at(3).scalar, DataModel::ProtoScalar::MessageRef);
}

//--------------------------------------------------------------------------------------------------
// File scope
//--------------------------------------------------------------------------------------------------

/**
 * @brief Imports, file-level options, services and standalone enums are accepted and skipped, so
 *        a real-world schema still reaches the importer with its messages intact.
 */
void TstProtoParser::topLevelBoilerplateIsSkipped()
{
  const QString src = QStringLiteral("syntax = \"proto3\";\n"
                                     "import \"google/protobuf/timestamp.proto\";\n"
                                     "option java_package = \"com.example\";\n"
                                     "enum Level {\n"
                                     "  LOW = 0;\n"
                                     "}\n"
                                     "service Telemetry {\n"
                                     "  rpc Stream (A) returns (A);\n"
                                     "}\n"
                                     ";\n"
                                     "message A {\n"
                                     "  int32 x = 1;\n"
                                     "}\n");

  DataModel::ProtoParser parser(src);
  QVERIFY(parser.parse());
  QVERIFY(parser.package().isEmpty());
  QCOMPARE(parser.messages().size(), 1);
  QCOMPARE(parser.messages().at(0).name, QStringLiteral("A"));
}

/**
 * @brief Nesting is capped so a pathological schema cannot recurse the parser off the stack.
 */
void TstProtoParser::nestingDepthIsBounded()
{
  QString src = QStringLiteral("syntax = \"proto3\";\n");
  for (int i = 0; i < 70; ++i)
    src += QStringLiteral("message M%1 {\n").arg(i);

  for (int i = 0; i < 70; ++i)
    src += QStringLiteral("}\n");

  DataModel::ProtoParser parser(src);
  QVERIFY(!parser.parse());
  QVERIFY(!parser.error().message.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Diagnostics
//--------------------------------------------------------------------------------------------------

void TstProtoParser::malformedSourceReportsLine_data()
{
  QTest::addColumn<QString>("source");
  QTest::addColumn<int>("line");

  QTest::newRow("missing semicolon") << QStringLiteral("syntax = \"proto3\";\n"
                                                       "message A {\n"
                                                       "  int32 x = 1\n"
                                                       "}\n")
                                     << 4;

  QTest::newRow("missing tag") << QStringLiteral("syntax = \"proto3\";\n"
                                                 "message A {\n"
                                                 "  int32 x = ;\n"
                                                 "}\n")
                               << 3;

  QTest::newRow("zero tag") << QStringLiteral("syntax = \"proto3\";\n"
                                              "message A {\n"
                                              "  int32 x = 0;\n"
                                              "}\n")
                            << 3;

  QTest::newRow("tag above ceiling") << QStringLiteral("syntax = \"proto3\";\n"
                                                       "message A {\n"
                                                       "  int32 x = 536870912;\n"
                                                       "}\n")
                                     << 3;

  QTest::newRow("missing message name") << QStringLiteral("syntax = \"proto3\";\n"
                                                          "message {\n"
                                                          "}\n")
                                        << 2;

  QTest::newRow("unknown top-level keyword") << QStringLiteral("syntax = \"proto3\";\n"
                                                               "\n"
                                                               "banana Foo {\n"
                                                               "}\n")
                                             << 3;

  QTest::newRow("stray token at file scope") << QStringLiteral("syntax = \"proto3\";\n"
                                                               "}\n")
                                             << 2;

  QTest::newRow("unterminated message") << QStringLiteral("syntax = \"proto3\";\n"
                                                          "message A {\n"
                                                          "  int32 x = 1;\n")
                                        << 4;
}

/**
 * @brief A refused file leaves the model empty and reports the line the user has to edit; the
 *        importer puts that number straight into its error dialog.
 */
void TstProtoParser::malformedSourceReportsLine()
{
  QFETCH(QString, source);
  QFETCH(int, line);

  DataModel::ProtoParser parser(source);
  QVERIFY(!parser.parse());
  QCOMPARE(parser.error().line, line);
  QVERIFY(!parser.error().message.isEmpty());
}

QTEST_APPLESS_MAIN(TstProtoParser)

#include "tst_proto_parser.moc"
