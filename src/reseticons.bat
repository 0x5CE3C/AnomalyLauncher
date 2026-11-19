@echo off
echo Stopping Explorer...
taskkill /f /im explorer.exe >nul 2>&1

echo Clearing icon cache...
cd /d "%userprofile%\AppData\Local\Microsoft\Windows\Explorer"
attrib -h -s iconcache_*.db >nul 2>&1
del /f iconcache_*.db >nul 2>&1

echo Starting Explorer...
start %windir%\explorer.exe
echo Done.
pause