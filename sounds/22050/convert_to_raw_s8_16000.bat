for %%f in (*.wav) do (
  "C:\Program Files\ShareX\ffmpeg.exe" -y -i "%%f" -ar 16000 -ac 1 -f s8 -c:a pcm_s8 "%%~nf_s8_16000.raw"
)
