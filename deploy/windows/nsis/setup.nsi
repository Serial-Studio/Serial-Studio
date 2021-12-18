;
; Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
;

Unicode True

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

!define APPNAME                      "Serial Studio"
!define EXECNAME                     "SerialStudio"
!define COMPANYNAME                  "Alex Spataru"
!define DESCRIPTION                  "Dashboard software for serial port devices"
!define VERSIONMAJOR                 1
!define VERSIONMINOR                 1
!define VERSIONBUILD                 5
!define MUI_ABORTWARNING
!define INSTALL_DIR                  "$PROGRAMFILES64\${APPNAME}"
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT      "Run ${APPNAME}"
!define MUI_FINISHPAGE_RUN_FUNCTION  "RunApplication"
!define MUI_FINISHPAGE_LINK          "Visit project website"
!define MUI_FINISHPAGE_LINK_LOCATION "http://github.com/serial-studio/serial-studio"
!define MUI_WELCOMEPAGE_TITLE        "Welcome to the ${APPNAME} installer!"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin"
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740
        quit
${EndIf}
!macroend

Name "${APPNAME}"
ManifestDPIAware true
InstallDir "${INSTALL_DIR}"
RequestExecutionLevel admin
OutFile "${EXECNAME}-${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}-Windows.exe"

Function .onInit
  setShellVarContext all
  !insertmacro VerifyUserIsAdmin

  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" \
  "UninstallString"
  StrCmp $R0 "" done

  ; Uninstall previous version (but save registry settings)
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${APPNAME}.lnk"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
done:
  ; Nothing to do...
FunctionEnd

Section "${APPNAME} (required)" SecDummy
  SectionIn RO
  SetOutPath "$INSTDIR"
  File /r "${APPNAME}\*"

  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"

  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayName"      "${APPNAME}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "UninstallString"  "$INSTDIR\uninstall.exe"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "InstallLocation"  "$INSTDIR"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "Publisher"        "${COMPANYNAME}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayIcon"      "$INSTDIR\icon.ico"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayVersion"   ${VERSIONMAJOR}.${VERSIONMINOR}${VERSIONBUILD}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "VersionMajor"     ${VERSIONMAJOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "VersionMinor"     ${VERSIONMINOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoModify"         1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoRepair"         1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "EstimatedSize"    "$0"
SectionEnd

Section "Start Menu Shortcuts"
  CreateShortCut  "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\bin\${EXECNAME}.exe" "" "$INSTDIR\bin\${EXECNAME}.exe" 0
SectionEnd

Section "Install Visual C++ Redistributable"
  ExecWait "$INSTDIR\bin\vc_redist.x64.exe /quiet /norestart"
  Delete "$INSTDIR\bin\vc_redist.x64.exe"
SectionEnd

Function RunApplication
  ExecShell "" "$INSTDIR\bin\${EXECNAME}.exe"
FunctionEnd

Function un.onInit
        SetShellVarContext all
        MessageBox MB_OKCANCEL|MB_ICONQUESTION "Are you sure that you want to uninstall ${APPNAME}?" IDOK next
                Abort
        next:
        !insertmacro VerifyUserIsAdmin
FunctionEnd

Section "Uninstall"
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${APPNAME}.lnk"
  DeleteRegKey HKCU "Software\${COMPANYNAME}\${APPNAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
SectionEnd
