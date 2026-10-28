# ACID release-reference correction for Nauclerus 0.25.0

The former golden `8145d3ce` was inherited from the pre-Nauclerus default sound.
That test calls `drum_set_model`, so it renders the current first factory preset,
not a fixed oscillator/filter test vector. The historical first ACID preset is
SQUELCH (UID 144): `{0,50,40,64,64,64,0,56}`. The owner-approved current first preset
is FASTSLOW (UID 152): `{-12,50,80,30,60,64,1,40}`, LEVEL104, MORPH/PWM64.
Earlier accepted RC10 also added variable PWM and the documented 150 Hz high-pass
in the ladder feedback solve. Therefore the old default-sound fingerprint cannot
serve as the accepted RC11–RC13 default-sound reference. The earlier SQUELCH-only
probe was not a reconstruction of the old DSP and did not establish a new regression.

## Independent retained source comparison

Retained pre-edit snapshots identify VERSION.txt as RC11, RC12 and RC13. Using
one identical harness against each snapshot, the original golden procedure was
repeated in model order: host reset, production model initialization, velocity127,
44,096 mono int32 samples at 44.1 kHz, through track_render without mixer/FX.

All three ACID renders are byte-identical:

- FNV-1a used by the unchanged test: **5d5973a5**.
- PCM SHA-256: **594c827e2f06f2775d0931450c46b05be6627ac6d8639b0398a422b0aad9ef96**.
- ACID engine source SHA-256: **49dfdb557095f423414a4456d8bd355b673fb2d8d69633b5a0715454d3876b35**.
- Oscillator source SHA-256: **291cdc7cd4ea8a20df9c8fc2ae68f6f6d500a23d126a63a69a7757bc05b9718a**.

The retained RC package SHA-256 values and snapshot/source manifests are recorded
in [ACID_GOLDEN_0.25.0.json](ACID_GOLDEN_0.25.0.json). RC11 is the owner-reported
physical-acceptance baseline; RC12/RC13 comparisons verify implementation continuity,
not an additional physical listening test. No instruction or subjective audio claim
is inferred from host hashes.

## Exact correction

Only the ACID entry in tests/drum_golden.txt changes, 8145d3ce -> 5d5973a5.
All other golden entries, thresholds, DSP, filter coefficients, preset values,
defaults and persistence remain untouched. This correction was explicitly approved
by the owner conditional on the accepted-source comparison above. GOLDEN_UPDATE
was not used. The release source's firmware directory must remain byte-identical
to the preserved RC13 directory.

The obsolete LFO DEST maximum assertion changes from18 to19. Extra assertions
verify every ID0..19 and displayed name across all45models, including A.MOR only
on ACID and inactive display on other models. No destination is renumbered.

Reproduce a source's render using `tests/acid_release_reference.c`, compiling with
`cc -O2 -Wno-int-to-pointer-cast -Ibuild/gen -Ifirmware/src ... -lm` after normal
header generation. Run in a disposable directory; acid.pcm is test output.

## Self-oscillation assertion investigation

The RC11, RC12 and RC13 retained source snapshots were each compiled with the
same production `acid_render` diagnostic. All three produced byte-identical
measurement logs (SHA-256 e2ccdeffb938a38cff3205c7e3ece32ff9465e18f5b1708d6e96d4437fe5f557).
No accepted firmware source was changed.

Conditions: SAW, TUNE 0, CUTOF 100, RESO 0 or 127, ENVMD 0, MIDI note 0
(step note -60), velocity 96, sample rate 44100 Hz. After one second excitation,
the SAW phase is held at its exact zero-valued half-cycle before each call to
the real renderer. This removes oscillator forcing without replacing the filter,
feedback, envelope, scaling or saturation implementation. Four subsequent seconds
are measured, within the existing voice lifetime. Spectral measurements use the
existing Hann-windowed Goertzel method on the latter half-second of each second.

The former continuously driven measurement is 625.746936 (RESO 127), compared
with 6.725845 (RESO 0). It is not a direct measure of autonomous oscillation.
With zero oscillator input, RESO 127 sustains band magnitude 1660.0804–1660.0827
at 2297.809 Hz, within 3.25% of the nominal 2225.481 Hz cutoff. Settled RMS is
4905.54–4905.79, peak 6939, and DC is between -2.085 and +0.575 counts.
The non-resonant control decays to a constant -1 count (zero AC band magnitude).
This establishes stable, bounded, genuinely self-sustaining oscillation.

The test now measures this autonomous response, retaining the original >1000
amplitude requirement, adding a non-resonant control, an unclipped peak check
and a ±5% long-term RMS stability check. No DSP coefficient, preset, golden
reference, or firmware gain was changed for this assertion.
