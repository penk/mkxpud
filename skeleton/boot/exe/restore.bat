@echo off

ver | find "XP" > nul
if %ERRORLEVEL% == 0 goto ver_xp

goto ver_vista

:: Windows XP
:ver_xp

attrib -R -H -S "c:\boot.ini"
sed -ri "/grldr/d" "c:\boot.ini"

:: Windows Vista
:ver_vista

Set BCDEDIT=C:\windows\System32\bcdedit.exe
set /p GUID= < bcdguid.txt
if not "%GUID%" == "" (
	%BCDEDIT% /delete %GUID%
)

goto END

:END