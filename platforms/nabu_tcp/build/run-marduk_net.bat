start "NABU Downland Server" py ..\server\nabu_mixed_server.py 127.0.0.1 6666
start "Marduk-Log" cmd /k ..\build\run-marduk.bat
start "Marduk-Log" cmd /k ..\build\run-marduk_port5817.bat
