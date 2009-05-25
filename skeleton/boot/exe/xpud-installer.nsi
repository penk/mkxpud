# xpud-installer.nsi - xPUD Installer script (NSIS)
#
# Copyright 2009 by Ping-Hsun Chen <penkia@gmail.com>

!include "MUI.nsh"
!define NAME "xpud"
!define VERSION "installer"

Name " ${NAME} ${VERSION}"
OutFile "${NAME}-${VERSION}.exe"

# ShowInstDetails show 
InstallDir "$PROGRAMFILES\${NAME}"

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

    SetOutPath "C:\"
	File "grldr"
	File "grldr.mbr"
	File "menu.lst"
	
	File "bzImage"
	File "core.gz"
	File "cjkfont.gz"
	File "plate.gz"
	File "scim.gz"

    SetOutPath "$INSTDIR"
    File "license.txt"
	
    CreateDirectory "$INSTDIR"
    SetOutPath "$INSTDIR"
    File "install.bat"
    File "restore.bat"
    File "libiconv2.dll"
    File "libintl3.dll"
    File "sed.exe"

	Exec "$INSTDIR\install.bat"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    SetOutPath "$INSTDIR"
    CreateDirectory "$SMPROGRAMS\${NAME}"
    CreateShortCut "$SMPROGRAMS\${NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

Section "un.Uninstaller Section"

	Exec "$INSTDIR\restore.bat"
	
	Delete "C:\grldr"
	Delete "C:\grldr.mbr"
	Delete "C:\menu.lst"
	Delete "C:\bzImage"
	Delete "C:\core.gz"
	Delete "C:\cjkfont.gz"
	Delete "C:\plate.gz"
	Delete "C:\scim.gz"
	
    Delete "$INSTDIR\Uninstall.exe"
    Delete "$INSTDIR\license.txt"
    Delete "$INSTDIR\restore.bat"

    RMDir "$INSTDIR"

    Delete "$SMPROGRAMS\${NAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${NAME}"

SectionEnd
