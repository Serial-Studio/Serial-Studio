/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QImage>
#  include <QJSEngine>
#  include <QJSValue>
#  include <QQuickPaintedItem>
#  include <QString>
#  include <QVariantList>

#  include "DataModel/Frame.h"
#  include "DataModel/Scripting/JsWatchdog.h"

namespace Widgets {

class PainterContext;
class PainterDataBridge;

/**
 * @brief Pro-tier user-scripted widget. Renders a JS paint(ctx, w, h) callback.
 */
class Painter : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(bool runtimeOk READ runtimeOk NOTIFY runtimeOkChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

signals:
  void runtimeOkChanged();
  void lastErrorChanged();
  void consoleLine(int level, const QString& text);

public:
  explicit Painter(int index = -1, QQuickItem* parent = nullptr);
  ~Painter() override;

  void paint(QPainter* painter) override;

  [[nodiscard]] bool runtimeOk() const noexcept;
  [[nodiscard]] QString lastError() const;

public slots:
  void setUserCode(const QString& code);
  void setPreviewMode(bool enabled);
  void setSimulatedDatasets(const QVariantList& datasets);
  void setPreviewGroupTitle(const QString& title);

private slots:
  void updateData();
  void tickPreview();

private:
  [[nodiscard]] bool compile(const QString& code);
  [[nodiscard]] static QString fallbackTemplate();
  void installBootstrap();
  void installTheme();
  void invalidateCompilation();
  void renderFrame();

  void setRuntimeOk(bool ok);
  void setLastError(const QString& error);

  int m_index;
  QString m_userCode;
  bool m_compileDirty;
  bool m_runtimeOk;
  bool m_bootstrapOk;
  bool m_hasOnFrame;
  bool m_previewMode;
  bool m_slowPaintWarned;
  int m_slowPaintStreak;
  qulonglong m_frameSeq;
  DataModel::Group m_previewGroup;

  QImage m_cache;
  QSize m_cacheSize;

  QJSEngine m_engine;
  DataModel::JsWatchdog m_watchdog;
  QJSValue m_paintFn;
  QJSValue m_onFrameFn;
  QJSValue m_ctxValue;

  PainterContext* m_ctx;
  PainterDataBridge* m_bridge;

  QString m_lastError;
};

}  // namespace Widgets

#endif  // BUILD_COMMERCIAL
