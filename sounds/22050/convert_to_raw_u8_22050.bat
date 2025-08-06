for %%f in (*.wav) do (
  "C:\Program Files\ShareX\ffmpeg.exe" -y -i "%%f" -ar 22050 -ac 1 -f u8 -c:a pcm_u8 "%%~nf_u8_22050.raw"
)
