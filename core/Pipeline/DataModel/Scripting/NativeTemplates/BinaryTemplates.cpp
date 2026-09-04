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

#include "DataModel/Scripting/NativeTemplates/BinaryBase64.h"
#include "DataModel/Scripting/NativeTemplates/BinaryCobs.h"
#include "DataModel/Scripting/NativeTemplates/BinaryHex.h"
#include "DataModel/Scripting/NativeTemplates/BinaryMavlink.h"
#include "DataModel/Scripting/NativeTemplates/BinaryMessagePack.h"
#include "DataModel/Scripting/NativeTemplates/BinaryModbus.h"
#include "DataModel/Scripting/NativeTemplates/BinaryNmea2000.h"
#include "DataModel/Scripting/NativeTemplates/BinaryRaw.h"
#include "DataModel/Scripting/NativeTemplates/BinaryRtcm.h"
#include "DataModel/Scripting/NativeTemplates/BinarySirf.h"
#include "DataModel/Scripting/NativeTemplates/BinarySlip.h"
#include "DataModel/Scripting/NativeTemplates/BinaryTlv.h"
#include "DataModel/Scripting/NativeTemplates/BinaryUbx.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the binary-protocol native templates in display order, followed by the
 *        driver-generated wire-latch family (WireLatchTemplates.cpp), which is empty in a GPL
 *        build. The order is user-visible: the first registry entry is the default template.
 */
QList<const DataModel::INativeTemplate*> DataModel::binaryNativeTemplates()
{
  QList<const DataModel::INativeTemplate*> list  = {&rawBytesTemplate(),
                                                    &hexBytesTemplate(),
                                                    &base64Template(),
                                                    &binaryTlvTemplate(),
                                                    &cobsTemplate(),
                                                    &slipTemplate(),
                                                    &ubxTemplate(),
                                                    &sirfTemplate(),
                                                    &mavlinkTemplate(),
                                                    &nmea2000Template(),
                                                    &rtcmTemplate(),
                                                    &modbusTemplate(),
                                                    &messagePackTemplate()};
  list                                          += wireLatchNativeTemplates();
  return list;
}
