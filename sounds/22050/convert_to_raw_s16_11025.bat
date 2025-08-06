for %%f in (*.wav) do (
  "C:\Program Files\ShareX\ffmpeg.exe" -y -i "%%f" -ar 11025 -ac 1 -f s16be -c:a pcm_s16be "%%~nf_s16_11025.raw"
)
