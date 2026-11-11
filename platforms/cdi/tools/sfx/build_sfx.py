#!/usr/bin/env python3
"""Convert Downland sound WAVs into gamedata/SFX.BIN and src/sfx_clips.h.

For each clip: boost the 8-bit WAV by --gain (clipping past full scale), encode it
with psxavenc as stereo 4-bit XA-ADPCM at --rate, strip the 8-byte sector subheader
from each 2336-byte sector, and append the 2304-byte audio frames. Frame 0 is silence.
The frame ranges and the buffer size are written to src/sfx_clips.h.

Usage:
  build_sfx.bat (edit the settings at the top), or
  python build_sfx.py --sounds SOUNDS_DIR --psxavenc PATH_TO_PSXAVENC_EXE

Every .wav in the sounds folder is packed, in filename order.

Requires psxavenc https://github.com/WonderfulToolchain/psxavenc
"""
import argparse
import os
import subprocess
import wave

# The game reads SFX.BIN in chunks of this size. Each frame is one block of audio
# that the sound hardware plays, and the game finds clips by counting frames.
FRAME = 2304        # bytes of audio in one frame
SECTOR = 2336       # psxavenc writes each frame inside a disc sector this big
SUBHDR = 8          # the first 8 bytes of each sector are a header we don't want
GROUPS = 18         # a frame is made of 18 small "sound groups"
# Frame 0 is always silence. The game uses it for "no sound", so it has to be
# valid data that plays as nothing: each group gets a 16-byte header of 0x0C and
# 112 zero bytes of audio.
SILENT = bytes([0x0C] * 16 + [0] * 112) * GROUPS

# run.wav is four footstep hits spaced ~200-260ms apart, followed by ~220ms
# of the same quiet noise floor as the gaps between them -- not dead air, but
# the gap the loop needs between the last step and the first step repeating
# (confirmed against platforms/dreamcast/romdisk/run_hq.wav and
# platforms/playdate/Source/run.wav, both the same length -- the playdate one
# byte-for-byte identical -- and both used unmodified where they work).
# Trimming it all off (like everything else) squeezes that one gap to zero
# while the other three stay at their normal spacing, so the loop sounds like
# it skips a beat once per cycle -- too fast/clipped. Leaving the full ~220ms
# in made the loop's one gap longer than the file's other three instead --
# too slow -- so CD-i's sd_loop() apparently adds its own small restart cost
# on top of the content's own gap. keep_ms below is how much of the gap's
# content to leave in per clip, tuned by ear until the loop sounds even;
# clips not listed get the default (trim fully, like everything else).
TRIM_KEEP_MS = {'run': 160}

# Work out where the project is from where this script lives, so the script
# doesn't depend on any particular drive or folder layout.
HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.abspath(os.path.join(HERE, '..', '..'))


def boost(src, dst, gain):
    """Make a louder copy of an 8-bit WAV and return how much of it clipped.

    Samples in an 8-bit WAV are unsigned, with silence at 128. We move them to
    be centred on zero, multiply by the gain, and move them back. Anything that
    goes past the 8-bit limits gets clipped, which is how loud sounds end up
    distorted. The clipped fraction tells us whether the gain is too high.
    """
    w = wave.open(src, 'rb')
    channels, width, rate = w.getnchannels(), w.getsampwidth(), w.getframerate()
    raw = w.readframes(w.getnframes())
    w.close()
    if width != 1:
        raise SystemExit(f"{src}: expected an 8-bit WAV")

    out = bytearray()
    clipped = 0
    for b in raw:
        v = (b - 128) * gain
        if v > 127:
            v = 127
            clipped += 1
        elif v < -128:
            v = -128
            clipped += 1
        out.append(int(round(v)) + 128)

    # Save the boosted audio as a normal WAV so psxavenc can read it.
    o = wave.open(dst, 'wb')
    o.setnchannels(channels)
    o.setsampwidth(1)
    o.setframerate(rate)
    o.writeframes(bytes(out))
    o.close()
    return clipped / len(raw)


def trim_trailing_silence(wav_path, threshold=4, keep_ms=0):
    """Cut trailing near-silence off the end of an 8-bit WAV, in place.

    Several of the source clips (run, climb_up, climb_down) have a stretch of
    near-silence recorded after the actual sound before the file ends. These
    are the ones the game loops, and that gap plays back every time the loop
    wraps around -- audible as a stutter/pause each repeat. Trimming it makes
    the loop seamless. For one-shot clips it just makes them a bit shorter,
    which is harmless.

    keep_ms lets some of the trailing gap stay instead of cutting all the way
    to the last loud sample -- used for run.wav, where the gap is content
    (even spacing between the file's footstep hits, confirmed by comparing
    byte-for-byte against platforms/playdate/Source/run.wav, which plays the
    same file unmodified), not junk, so a hardware-loop restart cost on this
    platform needs splitting the difference rather than removing it outright.

    Returns (samples removed, the WAV's own sample rate), for reporting --
    the removed count is in terms of this file's rate, not the output rate
    passed to psxavenc.
    """
    w = wave.open(wav_path, 'rb')
    channels, width, rate = w.getnchannels(), w.getsampwidth(), w.getframerate()
    raw = bytearray(w.readframes(w.getnframes()))
    w.close()
    if width != 1:
        raise SystemExit(f"{wav_path}: expected an 8-bit WAV")

    last_loud = -1
    for i in range(len(raw) - 1, -1, -1):
        if abs(raw[i] - 128) > threshold:
            last_loud = i
            break
    if last_loud < 0:
        return 0, rate  # whole clip is silent; leave it alone

    keep_samples = int(keep_ms * rate / 1000) * channels

    # Round up to a whole frame so a stereo file doesn't get cut mid-sample-pair.
    cut_len = last_loud + 1 + keep_samples
    cut_len = (cut_len + channels - 1) // channels * channels
    removed = len(raw) - cut_len
    if removed <= 0:
        return 0, rate

    o = wave.open(wav_path, 'wb')
    o.setnchannels(channels)
    o.setsampwidth(1)
    o.setframerate(rate)
    o.writeframes(bytes(raw[:cut_len]))
    o.close()
    return removed // channels, rate


def encode(psxavenc, wav, xa, rate):
    """Run psxavenc to compress the WAV into XA-ADPCM.

    We ask for stereo, 4 bits per sample, at the chosen rate. That's the setup
    that played back at the right pitch in the game. -q keeps psxavenc quiet.
    """
    subprocess.run([psxavenc, '-q', '-t', 'xa', '-f', str(rate), '-c', '2', '-b', '4',
                    wav, xa], check=True)


def audio_frames(xa):
    """Cut the audio frames out of the psxavenc output.

    psxavenc writes a series of disc sectors. Each one starts with a small header
    that the game doesn't need, followed by the 2304 bytes of audio. We keep only
    the audio from each sector and throw the rest away.
    """
    data = open(xa, 'rb').read()
    if len(data) % SECTOR:
        raise SystemExit(f"{xa}: not a whole number of sectors")
    return [data[i + SUBHDR:i + SUBHDR + FRAME] for i in range(0, len(data), SECTOR)]


def write_table(path, size, table):
    """Write sfx_clips.h: where each clip lives in SFX.BIN, and the sound list.

    The first part holds the frame numbers and buffer size, and is protected by
    an include guard so it's only defined once. The sound list at the end has no
    guard on purpose: audio.h and audio.c both include this file, and each one
    expands the list in its own way (declare it in audio.h, define it in audio.c).

    One sound is written per WAV file, named after the file. A file called
    run.wav becomes SFX_RUN. SFX_NONE is always included for silence (frame 0).
    """
    lines = [
        '#ifndef SFX_CLIPS_INCLUDE_H',
        '#define SFX_CLIPS_INCLUDE_H',
        '',
        '/* Generated by tools/sfx/build_sfx.py. Do not edit by hand. */',
        '/* gamedata/SFX.BIN: stereo 18.9 kHz, 2304-byte CD-XA ADPCM frames. Frame 0 is silence. */',
        f'#define SFX_SIZE {size} /* Smallest multiple of 2048 greater or equal to SFX.BIN */',
        '',
    ]
    for name, start, count in table:
        clip = name.upper()
        lines.append(f'#define CLIP_{clip}_START {start}')
        lines.append(f'#define CLIP_{clip}_FRAMES {count}')
    lines += ['', '#endif', '']
    lines += [
        '/* Every sound effect the game can play: SFX_CLIP(name, start frame, frame count).',
        '   Only expanded by a file that has defined SFX_CLIP first. */',
        '#ifdef SFX_CLIP',
        'SFX_CLIP(SFX_NONE, 0, 1);',
    ]
    for name, start, count in table:
        clip = name.upper()
        lines.append(f'SFX_CLIP(SFX_{clip}, CLIP_{clip}_START, CLIP_{clip}_FRAMES);')
    lines += ['#endif', '']
    with open(path, 'w') as f:
        f.write('\n'.join(lines) + '\n')


def main():
    # Read the command-line settings. The batch file fills these in, so the
    # script itself doesn't need to know where anything lives on this machine.
    ap = argparse.ArgumentParser(description='Build SFX.BIN and sfx_clips.h from Downland WAVs.')
    ap.add_argument('--sounds', required=True, help='folder containing the source WAV files')
    ap.add_argument('--psxavenc', required=True, help='path to psxavenc.exe')
    ap.add_argument('--gain', type=float, default=1.5, help='volume boost applied before encoding')
    ap.add_argument('--rate', type=int, default=18900, help='sample rate passed to psxavenc')
    ap.add_argument('--project', default=PROJECT, help='project root (default: two folders up)')
    args = ap.parse_args()

    # Intermediate files (boosted WAVs and psxavenc output) go in a temp folder
    # next to this script. They're safe to delete once the build is done.
    work = os.path.join(HERE, 'temp')
    os.makedirs(work, exist_ok=True)

    # Use every WAV in the sounds folder, sorted by name. Sorting keeps the
    # layout the same from one build to the next, so the frame numbers stay put.
    names = sorted(os.path.splitext(f)[0] for f in os.listdir(args.sounds)
                   if f.lower().endswith('.wav'))
    if not names:
        raise SystemExit(f"no WAV files in {args.sounds}")

    # Build the file one clip at a time. The silent frame always goes first, and
    # each clip's start position is just however many frames came before it.
    frames = [SILENT]
    table = []
    for name in names:
        src = os.path.join(args.sounds, name + '.wav')
        boosted = os.path.join(work, name + '.wav')
        xa = os.path.join(work, name + '.xa')

        clipped = boost(src, boosted, args.gain)
        trimmed, trimmed_rate = trim_trailing_silence(boosted, keep_ms=TRIM_KEEP_MS.get(name, 0))
        encode(args.psxavenc, boosted, xa, args.rate)
        clip_frames = audio_frames(xa)

        start = len(frames)
        frames += clip_frames
        table.append((name, start, len(clip_frames)))
        trimmed_ms = trimmed / trimmed_rate * 1000 if trimmed else 0
        print(f"{name:12s} start {start:3d} frames {len(clip_frames):3d} "
              f"clipped {clipped:.4f} trimmed {trimmed_ms:.0f}ms")

    # Join all the frames into one file and round its size up to a multiple of
    # 2048 for the reason described in write_table.
    blob = b''.join(frames)
    size = (len(blob) + 2047) // 2048 * 2048

    # Write the two outputs the game needs: the sound file and the table that
    # describes it.
    sfx_bin = os.path.join(args.project, 'gamedata', 'SFX.BIN')
    with open(sfx_bin, 'wb') as f:
        f.write(blob)

    table_h = os.path.join(args.project, 'src', 'sfx_clips.h')
    write_table(table_h, size, table)

    print(f"wrote {sfx_bin} ({len(blob)} bytes, {len(frames)} frames)")
    print(f"wrote {table_h} (SFX_SIZE {size})")


if __name__ == '__main__':
    main()
