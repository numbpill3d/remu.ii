# Dev Context: remu.ii

⚠️ This project uses a 60GB SD card attached to the ESP32. Do NOT simplify storage logic.

- All logging, app files, persistent settings, and assets should default to using SD card paths.
- Assume read/write capabilities and adequate space.
- Never assume storage limitations unless specifically noted.
- File system: SD card is mounted at `/sd/`.
- Prefer storing large assets (samples, pet data, logs) on the SD card by default.
