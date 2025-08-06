for %%f in (*.wav) do (
  "C:\Program Files\ShareX\ffmpeg.exe" -y -i "%%f" -ar 11025 -ac 1 -f u8 -c:a pcm_u8 "%%~nf_u8_11025.raw"
)
