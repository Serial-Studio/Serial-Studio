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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QByteArray>
#include <QByteArrayView>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QString>
#include <QStringList>

namespace DataModel {

/**
 * @brief Parameter value type; Json params carry machine-managed structured data.
 */
enum class NativeParamType : quint8 {
  String,
  Char,
  Int,
  Float,
  Bool,
  Enum,
  Json,
};

/**
 * @brief Describes one configurable parameter of a native parser template. Enum parameters
 * carry parallel value/label lists; numeric parameters carry inclusive min/max bounds.
 */
struct NativeParamSpec {
  QString key;
  QString label;
  QString description;
  QJsonValue defaultValue;
  QStringList optionValues;
  QStringList optionLabels;
  double minValue      = 0.0;
  double maxValue      = 0.0;
  NativeParamType type = NativeParamType::String;
};

/**
 * @brief Stateful parser instance created from a template descriptor and a params object.
 * One instance exists per source; latch-style templates keep inter-frame state here.
 */
class INativeParser {
public:
  virtual ~INativeParser() = default;

  [[nodiscard]] virtual QList<QStringList> parseText(const QString& frame)      = 0;
  [[nodiscard]] virtual QList<QStringList> parseBinary(const QByteArray& frame) = 0;

  /**
   * @brief Parses a UTF-8 text frame; the default transcodes once and reuses the text path.
   *        Byte-oriented templates override this to skip the QString round-trip.
   */
  [[nodiscard]] virtual QList<QStringList> parseUtf8(const QByteArray& frame)
  {
    return parseText(QString::fromUtf8(frame));
  }

  /**
   * @brief Allocation-free single-row fast path; -1 = contract unsatisfiable (latch/rewrite).
   */
  [[nodiscard]] virtual qsizetype parseSpans(QByteArrayView frame,
                                             QByteArrayView* out,
                                             qsizetype maxSpans) noexcept
  {
    Q_UNUSED(frame)
    Q_UNUSED(out)
    Q_UNUSED(maxSpans)
    return -1;
  }

  virtual void reset() {}
};

/**
 * @brief Base for parsers that latch one value row between frames: missing keys keep their
 * previous values, matching the persistent parsedValues arrays of the JS/Lua templates.
 */
class NativeLatchParser : public INativeParser {
public:
  explicit NativeLatchParser(int count);

  void reset() override;

protected:
  void storeAt(int index, const QString& value);
  [[nodiscard]] QList<QStringList> latchedFrame() const;
  [[nodiscard]] int latchCount() const noexcept;

private:
  int m_count;
  QStringList m_values;
};

/**
 * @brief Stateless descriptor for a native parser template: stable id, translated metadata,
 * parameter schema and the factory that builds a configured parser instance.
 */
class INativeTemplate {
public:
  virtual ~INativeTemplate() = default;

  [[nodiscard]] virtual QString id() const          = 0;
  [[nodiscard]] virtual QString name() const        = 0;
  [[nodiscard]] virtual QString description() const = 0;

  [[nodiscard]] virtual QList<NativeParamSpec> params() const = 0;

  [[nodiscard]] virtual std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                                  QString& error) const = 0;
};

[[nodiscard]] const QList<const INativeTemplate*>& nativeTemplates();
[[nodiscard]] const INativeTemplate* nativeTemplateById(const QString& id);
[[nodiscard]] QString defaultNativeTemplateId();

[[nodiscard]] QList<const INativeTemplate*> textNativeTemplates();
[[nodiscard]] QList<const INativeTemplate*> binaryNativeTemplates();
[[nodiscard]] QList<const INativeTemplate*> mapNativeTemplates();
[[nodiscard]] QList<const INativeTemplate*> multiFrameNativeTemplates();

[[nodiscard]] QJsonObject nativeTemplateDefaults(const INativeTemplate& tmpl);

[[nodiscard]] NativeParamSpec nativeParam(const char* key,
                                          NativeParamType type,
                                          const char* label,
                                          const char* description,
                                          const QJsonValue& defaultValue);
[[nodiscard]] QStringList nativeKeyList(const QJsonObject& params,
                                        const QString& key,
                                        const QString& fallbackCsv);
[[nodiscard]] QJsonArray nativeParamArray(const QJsonObject& params, const QString& key);
[[nodiscard]] QString nativeParamString(const QJsonObject& params,
                                        const QString& key,
                                        const QString& fallback);
[[nodiscard]] QChar nativeParamChar(const QJsonObject& params, const QString& key, QChar fallback);
[[nodiscard]] int nativeParamInt(const QJsonObject& params, const QString& key, int fallback);
[[nodiscard]] double nativeParamFloat(const QJsonObject& params,
                                      const QString& key,
                                      double fallback);
[[nodiscard]] bool nativeParamBool(const QJsonObject& params, const QString& key, bool fallback);

}  // namespace DataModel
