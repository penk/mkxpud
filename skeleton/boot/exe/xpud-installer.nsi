# xpud-installer.nsi - xPUD Installer script (NSIS)
#
# Copyright 2009 by Ping-Hsun Chen <penkia@gmail.com>

!include "MUI.nsh"
!define NAME "xPUD"
!define VERSION "0.9.5-dev"
!define DRIVE "C:"
!define XPUDDIR "xpud"

Name "${NAME} ${VERSION}"
# mkxpud script expects that ouput file is named exactly like below
OutFile "xpud-installer.exe"

# ShowInstDetails show 
InstallDir "${DRIVE}\${XPUDDIR}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.txt"
#  !insertmacro MUI_PAGE_COMPONENTS
#  !insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "TradChinese"

Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd


Section "${NAME}"

    SetOutPath "${DRIVE}"
	File "grldr"
	File "grldr.mbr"
	File "menu.lst"
	
	CreateDirectory "$INSTDIR"
	SetOutPath "$INSTDIR"
	File "bzImage"
	File "core"
	File /nonfatal "font"
	File /nonfatal "apps"
	File /nonfatal "scim"

    File "license.txt"
    File "install.bat"
    #File "restore.bat"
    File "libiconv2.dll"
    File "libintl3.dll"
    File "sed.exe"

	ExecWait "$INSTDIR\install.bat"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    SetOutPath "$INSTDIR"
    CreateDirectory "$SMPROGRAMS\${NAME}"
    CreateShortCut "$SMPROGRAMS\${NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

Section "un.Uninstaller Section"

	ExecWait "$INSTDIR\restore.bat"
	
	Delete "${DRIVE}\grldr"
	Delete "${DRIVE}\grldr.mbr"
	Delete "${DRIVE}\menu.lst"
	
	Delete "$INSTDIR\bzImage"
	Delete "$INSTDIR\core"
	Delete "$INSTDIR\font"
	Delete "$INSTDIR\apps"
	Delete "$INSTDIR\scim"
	
	Delete "$INSTDIR\bcdguid.txt"
	Delete "$INSTDIR\install.bat"
	Delete "$INSTDIR\libiconv2.dll"
	Delete "$INSTDIR\libintl3.dll"
	Delete "$INSTDIR\sed.exe"
	
    Delete "$INSTDIR\Uninstall.exe"
    Delete "$INSTDIR\license.txt"
    Delete "$INSTDIR\restore.bat"

    RMDir "$INSTDIR"

    Delete "$SMPROGRAMS\${NAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${NAME}"

SectionEnd
