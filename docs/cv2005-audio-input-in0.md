# Microphone is on acodec input IN0 (2026-11-28)

## Finding

The CV2005 module's microphone is wired to the audio codec's input IN0.
Majestic's audio pipeline defaults to input IN1, which carries only noise.
Selecting IN0 makes the microphone work: voice and case taps are clearly
audible in the stream.

This note is intended to be merged into CV2005_RESEARCH_NOTES.md later; it
is a separate file only because the main notes file could not be safely
rewritten from the available tooling at the time of writing.

## Evidence (2026-11-28, camera 192.168.1.171)

- Helper: tools/acodec_mixer/acodec_mixer.c (commit d936b67), built as a
  static ARM musl binary by .github/workflows/build-acodec-mixer.yml
  (run 46127069190).
- Binary SHA-256 (PC copy, matched artifact manifest):
  ef490a8023cabcc423c7910d0ed7bd4283cb66fb00bc94c0cffd4c740ac46c04
- Command: /tmp/acodec_mixer 0 -> "mixer input set to 0".
- /proc/umap/acodec before: mixer_left/mixer_right referenced in1_*.
- /proc/umap/acodec after: mixer_left:in0_left, mixer_right:in0_right.
- Subjective check: microphone audio confirmed by ear in the live stream
  (voice and case taps audible) with input 0.

## Mechanism

The helper issues OT_ACODEC_SET_MIXER_MIC (_IOWR('A', 2, unsigned int)) on
/dev/acodec with the value 0 (IN0), 1 (IN1) or 2 (differential). It changes
only the codec mixer register; nothing is written to flash.

## Caveats

- The setting is volatile: it resets on reboot and whenever Majestic
  restarts its audio pipeline (which reselects IN1).
- Not yet tested: input 2 (differential).
- Not yet solved: making the selection persistent. Options discussed:
  a Majestic post-start hook or an init script that runs the helper after
  Majestic starts, with the binary at a permanent path (for example
  /usr/bin/acodec_mixer). Deferred until the overlay-persistence question
  is resolved, per the standing rules in CV2005_RESEARCH_NOTES.md.

Scope: documentation only. No driver, image, flash or partition change was
made in this checkpoint.
