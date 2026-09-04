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

#include "Misc/ContextRegistry.h"

#include <QObject>
#include <QQmlContext>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Name tables
//--------------------------------------------------------------------------------------------------

/**
 * @brief The QML globals that hand a C++ QObject to QML. A package's context shadows exactly these,
 *        so a name reaching QML without appearing here stays reachable from an extension by name.
 */
[[nodiscard]] static QStringList buildObjectNames()
{
  QStringList names = {
    QStringLiteral("Cpp_AppState"),
    QStringLiteral("Cpp_Updater"),
    QStringLiteral("Cpp_IO_Serial"),
    QStringLiteral("Cpp_CSV_Export"),
    QStringLiteral("Cpp_CSV_Player"),
    QStringLiteral("Cpp_IO_Manager"),
    QStringLiteral("Cpp_IO_Network"),
    QStringLiteral("Cpp_MDF4_Export"),
    QStringLiteral("Cpp_MDF4_Player"),
    QStringLiteral("Cpp_Misc_ModuleManager"),
    QStringLiteral("Cpp_UI_Dashboard"),
    QStringLiteral("Cpp_UI_TaskbarSettings"),
    QStringLiteral("Cpp_Console_Export"),
    QStringLiteral("Cpp_NativeWindow"),
    QStringLiteral("Cpp_API_Server"),
    QStringLiteral("Cpp_API_Mirror"),
    QStringLiteral("Cpp_Misc_Utilities"),
    QStringLiteral("Cpp_IO_Bluetooth_LE"),
    QStringLiteral("Cpp_ThemeManager"),
    QStringLiteral("Cpp_Console_Handler"),
    QStringLiteral("Cpp_Misc_Translator"),
    QStringLiteral("Cpp_JSON_ProjectModel"),
    QStringLiteral("Cpp_JSON_ProjectEditor"),
    QStringLiteral("Cpp_ControlScript"),
    QStringLiteral("Cpp_JSON_ProtoImporter"),
    QStringLiteral("Cpp_JSON_FrameBuilder"),
    QStringLiteral("Cpp_Notifications"),
    QStringLiteral("Cpp_Misc_TimerEvents"),
    QStringLiteral("Cpp_Misc_CommonFonts"),
    QStringLiteral("Cpp_IO_FileTransmission"),
    QStringLiteral("Cpp_Misc_WorkspaceManager"),
    QStringLiteral("Cpp_Examples"),
    QStringLiteral("Cpp_HelpCenter"),
    QStringLiteral("Cpp_ExtensionManager"),
    QStringLiteral("Cpp_Misc_IconEngine"),
    QStringLiteral("Cpp_Misc_IconRegistry"),
    QStringLiteral("Cpp_Misc_ProblemCenter"),
    QStringLiteral("Cpp_Misc_ConnectionDiagnostics"),
    QStringLiteral("Cpp_UI_CommandRegistry"),
    QStringLiteral("Cpp_UI_WidgetExtensions"),
    QStringLiteral("Cpp_Misc_GraphicsBackend"),
    QStringLiteral("Cpp_Misc_HighDpiScaling"),
    QStringLiteral("Cpp_Misc_CrashTracker"),
    QStringLiteral("Cpp_Misc_BackupManager"),
    QStringLiteral("Cpp_Benchmark_Runner"),
  };

#ifdef BUILD_COMMERCIAL
  names += QStringList{
    QStringLiteral("Cpp_IO_Audio"),
    QStringLiteral("Cpp_IO_CANBus"),
    QStringLiteral("Cpp_IO_Modbus"),
    QStringLiteral("Cpp_IO_OpcUa"),
    QStringLiteral("Cpp_IO_S7"),
    QStringLiteral("Cpp_IO_Eip"),
    QStringLiteral("Cpp_IO_Iec104"),
    QStringLiteral("Cpp_IO_USB"),
    QStringLiteral("Cpp_IO_HID"),
    QStringLiteral("Cpp_IO_Process"),
    QStringLiteral("Cpp_IO_Mqtt"),
    QStringLiteral("Cpp_MQTT_Publisher"),
    QStringLiteral("Cpp_JSON_DBCImporter"),
    QStringLiteral("Cpp_JSON_ModbusMapImporter"),
    QStringLiteral("Cpp_Licensing_Trial"),
    QStringLiteral("Cpp_Licensing_LemonSqueezy"),
    QStringLiteral("Cpp_Licensing_OfflineLicense"),
    QStringLiteral("Cpp_Sessions_Export"),
    QStringLiteral("Cpp_InfluxDB_Export"),
    QStringLiteral("Cpp_Sessions_Player"),
    QStringLiteral("Cpp_Sessions_Manager"),
    QStringLiteral("Cpp_ShortcutGenerator"),
    QStringLiteral("Cpp_AI_Assistant"),
    QStringLiteral("Cpp_Image_Export"),
  };
#endif

#ifdef ENABLE_GRPC
  names += QStringLiteral("Cpp_GRPC_Server");
#endif

  return names;
}

/**
 * @brief The QML globals that hand a plain build-metadata value to QML.
 */
[[nodiscard]] static QStringList buildValueNames()
{
  return {
    QStringLiteral("Cpp_AppName"),
    QStringLiteral("Cpp_ScreenList"),
    QStringLiteral("Cpp_AppVersion"),
    QStringLiteral("Cpp_PrimaryScreen"),
    QStringLiteral("Cpp_AppUpdaterUrl"),
    QStringLiteral("Cpp_AppOrganization"),
    QStringLiteral("Cpp_UpdaterEnabled"),
    QStringLiteral("Cpp_CommercialBuild"),
    QStringLiteral("Cpp_GrpcAvailable"),
    QStringLiteral("Cpp_HasWebEngine"),
  };
}

//--------------------------------------------------------------------------------------------------
// Registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stages one QObject global; @p name must be in the object table.
 */
void Misc::ContextRegistry::add(const QString& name, QObject* object)
{
  SS_ASSERT(!name.isEmpty(), return);
  SS_ASSERT_LOG(objectNames().contains(name));

  m_objects.append({name, object});
}

/**
 * @brief Stages one plain-value global; @p name must be in the value table.
 */
void Misc::ContextRegistry::add(const QString& name, const QVariant& value)
{
  SS_ASSERT(!name.isEmpty(), return);
  SS_ASSERT_LOG(valueNames().contains(name));

  m_values.append({name, value});
}

/**
 * @brief Registers everything staged, in one loop per kind. Called AFTER the modules are wired and
 *        BEFORE the QML load, so no binding can evaluate against a half-built object.
 */
void Misc::ContextRegistry::apply(QQmlContext* context) const
{
  SS_ASSERT(context != nullptr, return);
  SS_ASSERT_LOG(!m_objects.isEmpty() || !m_values.isEmpty());

  for (const auto& [name, object] : m_objects)
    context->setContextProperty(name, object);

  for (const auto& [name, value] : m_values)
    context->setContextProperty(name, value);
}

/**
 * @brief Every QML global this build registers, objects first.
 */
const QStringList& Misc::ContextRegistry::names()
{
  static const QStringList all = objectNames() + valueNames();
  return all;
}

/**
 * @brief The globals that hand a QObject to QML; an extension's context shadows exactly these.
 */
const QStringList& Misc::ContextRegistry::objectNames()
{
  static const QStringList names = buildObjectNames();
  return names;
}

/**
 * @brief The globals that hand a plain build-metadata value to QML.
 */
const QStringList& Misc::ContextRegistry::valueNames()
{
  static const QStringList names = buildValueNames();
  return names;
}
