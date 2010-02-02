@echo off

ver | find "XP" > nul
if %ERRORLEVEL% == 0 goto ver_xp

ver | find "2000" > nul
if %ERRORLEVEL% == 0 goto ver_xp

goto ver_vista

:: Windows XP
:ver_xp

:: Backup 
attrib -R -H -S c:\boot.ini
copy c:\boot.ini c:\boot.ini.bak

:: Check if installed
findstr /L grldr "c:\boot.ini" > nul
if %ERRORLEVEL% == 0 goto END

:: Add boot entry
echo C:\grldr="xPUD" >> "c:\boot.ini"

:: create restore.bat file for XP
echo attrib -R -H -S "c:\boot.ini" >> restore.bat
echo %CD%\sed -ri "/grldr/d" "c:\boot.ini" >> restore.bat
echo %CD%\sed -ri "s/$/\r/" "c:\boot.ini" >> restore.bat

goto END

:: Windows Vista
:ver_vista

:: Check if installed
Set BCDEDIT=C:\windows\System32\bcdedit.exe
%BCDEDIT% /enum | findstr /L grldr > nul
if %ERRORLEVEL% == 0 goto END

:: Backup
%BCDEDIT% /export "C:\BCD.bak"
attrib +H +S C:\BCD.bak

:: Add boot entry
Set GUIDFILE=bcdguid.txt
%BCDEDIT% /create /d "xPUD" /application bootsector > %GUIDFILE%
sed -ri "s/[^{]*([^}]+}).*/\1/" %GUIDFILE%
set /p GUID= < %GUIDFILE%

%BCDEDIT% /set %GUID% device boot
%BCDEDIT% /set %GUID% device partition=%SYSTEMDRIVE%
%BCDEDIT% /set %GUID% path \grldr.mbr
%BCDEDIT% /displayorder %GUID% /addlast

:: create restore.bat file for Vista
echo %BCDEDIT% /delete %GUID% >> restore.bat

goto END

:END
