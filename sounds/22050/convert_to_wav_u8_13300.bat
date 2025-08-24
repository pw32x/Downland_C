for %%f in (*.wav) do (
  "C:\Program Files\ShareX\ffmpeg.exe" -y -i "%%f" -ar 13300 -ac 1 -c:a pcm_u8 "%%~nf_u8_133000.wav"
)
