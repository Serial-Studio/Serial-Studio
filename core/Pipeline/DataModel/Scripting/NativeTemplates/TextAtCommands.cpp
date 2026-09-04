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

#include "DataModel/Scripting/NativeTemplates/TextAtCommands.h"

#include <QHash>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// AT command responses
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching AT response extractor: +CMD: p1,p2 routed via a command-to-index table.
 */
class AtCommandsParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the command-to-indices routing table.
   */
  AtCommandsParser(const QHash<QString, QList<int>>& commands, int count)
    : NativeLatchParser(count), m_commands(commands)
  {
    SS_ASSERT_LOG(!m_commands.isEmpty());
  }

  /**
   * @brief Routes the response parameters into their mapped channel indices.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!m_commands.isEmpty(), return latchedFrame());

    const QString trimmed = frame.trimmed();
    const qsizetype colon = trimmed.indexOf(QLatin1Char(':'));
    if (colon < 0)
      return latchedFrame();

    QString command = trimmed.left(colon).trimmed();
    command.remove(QLatin1Char('+'));

    const auto it = m_commands.constFind(command);
    if (it == m_commands.constEnd())
      return latchedFrame();

    const auto values = trimmed.mid(colon + 1).split(QLatin1Char(','));
    const auto& idx   = it.value();
    for (qsizetype i = 0; i < values.size() && i < idx.size(); ++i)
      storeAt(idx.at(i), values.at(i).trimmed());

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QHash<QString, QList<int>> m_commands;
};

/**
 * @brief Descriptor for the AT command responses template.
 */
class AtCommandsTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("at_commands"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("AT command responses"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Maps modem responses like +CSQ: 25,99 into channels using a command "
                            "routing table with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto commands = DataModel::nativeParam(
      "commands",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Command routing table"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Semicolon-separated entries of NAME:index list, e.g. "
                        "CSQ:0,1;CREG:2,3;CGATT:4."),
      QStringLiteral("CSQ:0,1;CREG:2,3;CGATT:4;COPS:5,6,7"));

    return {commands};
  }

  /**
   * @brief Parses the routing table and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString table = DataModel::nativeParamString(
      params, QStringLiteral("commands"), QStringLiteral("CSQ:0,1;CREG:2,3;CGATT:4;COPS:5,6,7"));

    QHash<QString, QList<int>> commands;
    int max_index = -1;

    const auto entries = table.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const auto& entry : entries) {
      const qsizetype colon = entry.indexOf(QLatin1Char(':'));
      const QString name    = (colon < 0) ? QString() : entry.left(colon).trimmed();
      if (name.isEmpty())
        continue;

      QList<int> indices;
      const auto parts = entry.mid(colon + 1).split(QLatin1Char(','), Qt::SkipEmptyParts);
      for (const auto& part : parts) {
        bool ok         = false;
        const int index = part.trimmed().toInt(&ok);
        if (!ok || index < 0)
          continue;

        indices.append(index);
        max_index = qMax(max_index, index);
      }

      if (!indices.isEmpty())
        commands.insert(name, indices);
    }

    if (commands.isEmpty() || max_index < 0) {
      error =
        trNativeTemplate("The command routing table must contain at least one NAME:index entry.");
      return nullptr;
    }

    return std::make_unique<AtCommandsParser>(commands, max_index + 1);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide AT command responses template descriptor.
 */
const DataModel::INativeTemplate& DataModel::atCommandsTemplate()
{
  static const AtCommandsTemplate s_atCommands;
  return s_atCommands;
}
