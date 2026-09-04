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

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/NativeTemplates/TextAtCommands.h"
#include "DataModel/Scripting/NativeTemplates/TextDelimited.h"
#include "DataModel/Scripting/NativeTemplates/TextFixedWidth.h"
#include "DataModel/Scripting/NativeTemplates/TextIniConfig.h"
#include "DataModel/Scripting/NativeTemplates/TextJsonData.h"
#include "DataModel/Scripting/NativeTemplates/TextKeyValue.h"
#include "DataModel/Scripting/NativeTemplates/TextNmea0183.h"
#include "DataModel/Scripting/NativeTemplates/TextUrlEncoded.h"
#include "DataModel/Scripting/NativeTemplates/TextXmlData.h"
#include "DataModel/Scripting/NativeTemplates/TextYamlData.h"

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the text-oriented native templates in display order. The order is user-visible:
 *        the first entry of this family is the first entry of the whole registry, and therefore
 *        the default template.
 */
QList<const DataModel::INativeTemplate*> DataModel::textNativeTemplates()
{
  return {&delimitedTextTemplate(),
          &fixedWidthTemplate(),
          &keyValueTemplate(),
          &iniConfigTemplate(),
          &atCommandsTemplate(),
          &nmea0183Template(),
          &urlEncodedTemplate(),
          &jsonDataTemplate(),
          &xmlDataTemplate(),
          &yamlDataTemplate()};
}
