# Copyright 2023 Ingemar Hedvall
# SPDX-License-Identifier: MIT 

!include MUI2.nsh
!include x64.nsh
!include FileFunc.nsh
!define APP_BUILD_DIR "..\cmake-build-release" ; Path to executable and release library
!define APP_BUILD_DIR_DEBUG "..\cmake-build-debug" ; Path to the MDF debug library
!define NET_BUILD_DIR "..\x64" ; Path to 64-bit .NET library
!define NET32_BUILD_DIR ".." ; Path to 32-bit .NET library
Name "MDF Applications and Library 2.3"
OutFile "${APP_BUILD_DIR}\mdflib.exe"
Unicode True

RequestExecutionLevel admin ; Request for admin login

Var StartMenuFolder

InstallDir "$LOCALAPPDATA\MdfLib"
InstallDirRegKey HKLM "Software\MdfLib" ""


!define MUI_ABORTWARNING
!define ARP "Software\Microsoft\Windows\CurrentVersion\Uninstall\MdfLib"
!define MSVS_DIR "d:\msvs" ; Path where the MS Visual Studio Run-Time libraries are downloaded

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_DEFAULTFOLDER "MDF Apps & Libs"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\MdfLib"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
   
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
  
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Visual Studio Runtime" MSCRT
  SectionIn RO ; Must be included
  SetShellVarContext all
  SetRegView 64
  SetOutPath "$INSTDIR\bin"
  File "${MSVS_DIR}\VC_redist.x64.exe"
  ExecWait '"$INSTDIR\bin\VC_redist.x64.exe" /passive /norestart'
;  Delete "$INSTDIR\bin\VC_redist.x64.exe"
SectionEnd

Section "Applications" APP
;  SectionIn RO
  SetRegView 64	
  SetShellVarContext all

  SetOutPath "$INSTDIR\bin"
  File "${APP_BUILD_DIR}\mdfviewer\mdfview.exe"
  File "${APP_BUILD_DIR}\mdf2csv\*.exe"
  SetOutPath "$INSTDIR\img"
  File "..\img\*.*"
  
  ;Store installation folder
  WriteRegStr HKLM "Software\MdfLib" "" $INSTDIR
  
  WriteUninstaller "$INSTDIR\Uninstall.exe"  
  
  ; Add /Remove Programs
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0


  WriteRegNone HKLM "${ARP}" "" 
  WriteRegStr HKLM "${ARP}" "InstallLocation" $INSTDIR
  WriteRegStr HKLM "${ARP}" "DisplayIcon" "$INSTDIR\img\mdflib.ico"
  WriteRegStr HKLM "${ARP}" "DisplayName" "MDF Apps & Libs 2.3"
  WriteRegStr HKLM "${ARP}" "DisplayVersion" "2.3.0"
  WriteRegStr HKLM "${ARP}" "Publisher" "Ingemar Hedvall" 
  WriteRegDWORD HKLM "${ARP}" "NoModify" 1 
  WriteRegDWORD HKLM "${ARP}" "NoRepair" 1 
  WriteRegDWORD HKLM "${ARP}" "VersionMajor" 2
  WriteRegDWORD HKLM "${ARP}" "VersionMinor" 3
  WriteRegDWORD HKLM "${ARP}" "EstimatedSize" "$0"			 
  WriteRegStr HKLM "${ARP}"  "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
				 

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortcut "$SMPROGRAMS\$StartMenuFolder\MDF Viewer.lnk" "$INSTDIR\bin\mdfview.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section /o "MDF C++ Library" LIB
  SetRegView 64
  
  SetOutPath "$INSTDIR\lib"
  File "${APP_BUILD_DIR}\mdflib\mdf.lib"
  File "${APP_BUILD_DIR_DEBUG}\mdflib\mdfd.lib"
  
  SetOutPath "$INSTDIR\include\mdf"
  File "..\include\mdf\*.*"
  
  SetOutPath "$INSTDIR\doc\mdf"
  File /r "..\docs\manual\html\*.*"
  
  CreateShortcut "$SMPROGRAMS\$StartMenuFolder\MDF Library Documentation.lnk" \
	"$INSTDIR\doc\mdf\index.html" "" "$INSTDIR\img\document.ico" \
	0 SW_SHOWNORMAL
	
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "${ARP}" "EstimatedSize" "$0"
SectionEnd

Section /o "MDF .NET Library" NET
  SetRegView 64

  SetOutPath "$INSTDIR\net\Release"
  File "${NET_BUILD_DIR}\Release\mdflibrary.*"
  SetOutPath "$INSTDIR\net\Debug"
  File "${NET_BUILD_DIR}\Debug\mdflibrary.*"
  
  SetOutPath "$INSTDIR\net32\Release"
  File "${NET32_BUILD_DIR}\Release\mdflibrary.*"
  SetOutPath "$INSTDIR\net32\Debug"
  File "${NET_BUILD_DIR}\Debug\mdflibrary.*"
  
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "${ARP}" "EstimatedSize" "$0"
SectionEnd

LangString DESC_CRT ${LANG_ENGLISH} "Microsoft Visual Studio Runtime."
LangString DESC_APP ${LANG_ENGLISH} "All executables."
LangString DESC_LIB ${LANG_ENGLISH} "MDF C++ Library"
LangString DESC_NET ${LANG_ENGLISH} "MDF .NET Library"


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${MSCRT} $(DESC_CRT)
  !insertmacro MUI_DESCRIPTION_TEXT ${APP} $(DESC_APP)
  !insertmacro MUI_DESCRIPTION_TEXT ${LIB} $(DESC_LIB)
  !insertmacro MUI_DESCRIPTION_TEXT ${NET} $(DESC_NET)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  SetRegView 64	  
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  
  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR"  
  RMDir /r "$SMPROGRAMS\$StartMenuFolder"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MdfLib"
  DeleteRegKey HKLM "Software\MdfLib"

SectionEnd