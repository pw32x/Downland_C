
@echo off

rem Kill NABU Internet Adapter
taskkill /F /IM "NABU-Internet-Adapter-84.exe" 2>nul

taskkill /F /IM "python.exe" 2>nul
taskkill /F /FI "WINDOWTITLE eq Marduk-Log*" /T 2>nul

rem Kill all processes whose names start with Marduk
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "Get-Process | Where-Object { $_.ProcessName -like 'Marduk*' } | Stop-Process -Force"

exit /b
