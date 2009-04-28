@echo off

ver | find "XP" > nul
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
echo C:\grldr="xPUD" >> c:\boot.ini

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
%BCDEDIT% /set %GUID% path \grldr.mbr
%BCDEDIT% /displayorder %GUID% /addlast

goto END

:END
