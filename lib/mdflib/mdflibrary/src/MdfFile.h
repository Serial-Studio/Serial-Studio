/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/mdffile.h>
#include "mdfattachment.h"
#include "MdfDataGroup.h"
#include "MdfHeader.h"

using namespace System;

namespace MdfLibrary {
public ref class MdfFile {
public:

  property array<MdfAttachment^>^ Attachments {
    array<MdfAttachment^>^ get();
  }

  property array<MdfDataGroup^>^ DataGroups {
    array<MdfDataGroup^>^ get();
  }

  property String^ Name { String^ get(); void set( String^ name); }
  property String^ FileName { String^ get(); void set( String^ filename); }
  property String^ Version { String^ get(); } 
  property int MainVersion { int get(); }
  property int MinorVersion { int get(); void set(int minor); }
  property String^ ProgramId { String^ get(); void set(String^ program_id); }
  
  /// <summary>	True if finalized. </summary>
  property bool Finalized { bool get(); }

  property MdfHeader^ Header { MdfHeader^ get(); }
  property bool IsMdf4 { bool get(); }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// <summary>	Creates the attachment. </summary>
  ///
  /// <returns>	Nullptr if it fails, else the new attachment. </returns>
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  MdfAttachment^ CreateAttachment();

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// <summary>	Creates data group. </summary>
  ///
  /// <returns>	Nullptr if it fails, else the new data group. </returns>
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  MdfDataGroup^ CreateDataGroup();

  MdfDataGroup^ FindParentDataGroup(const MdfChannel^ channel);
  
private:


  MdfFile() {};

internal:
  // Note that the pointer is destroyed by the MdfReader object
  mdf::MdfFile *mdf_file_ = nullptr;
  MdfFile(mdf::MdfFile *mdf_file);
};

} // end namespace 