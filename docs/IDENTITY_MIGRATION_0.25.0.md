# NAU-0250-015: approved Nauclerus identity

- Creator: **Mr. Roboto**.
- Application handshake and package markers: **FM-1_25000**.
- USB MIDI/audio product: **Nauclerus**, UTF-16LE descriptor, 20 bytes.
- Display version: **Nauclerus 0.25.0**.
- USB VID/PID remain 1209:0001; shared loader remains 1209:0002,
  **Felucca Update / ota-FM-1_900**.
- Loader SHA-256 remains
  cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d.

The ID fits the existing 20-byte package markers and 27-byte handshake field. It
retains the FM-1 hardware model and numeric suffix expected by existing parsers.
Both device-discovery filters now include Nauclerus and retain Hortator, Felucca,
FM-1 and loader support. The exact target identity is verified after reboot.

CLI and WebMIDI mock tests exercise RC13 FM-1_900/Hortator -> unchanged shared
loader -> FM-1_25000/Nauclerus, normal Nauclerus updates, rollback to RC13 and
hosts retaining a cached Hortator port label. The real production OTA staging
handler is exercised with both application IDs and the new package. The existing
loader tests verify application writes and project/lock/preset-sector preservation.
No hardware was accessed; physical migration itself is not claimed tested.

Use the updated Nauclerus installer/CLI. Older installers may not recognize the
new USB name after a successful write. DAW port selections may need reselection.
No DSP, sound presets, project/Song records, flash layout, VID/PID or recovery
logic changes. The sole frozen-source difference is the specifically approved
USB product declaration; its old independent baseline remains pinned. H2 permits
only the required compiled identity literal/copy bound, not a protocol exemption.
Historical RC13 identity/hash records remain unchanged.
