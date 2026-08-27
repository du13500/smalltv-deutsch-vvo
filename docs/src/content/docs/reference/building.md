---
title: Building from source
description: Build any of the four board targets with PlatformIO, the lean ESP8266 variant, and the ESP32 toolchain notes.
---

The four board targets share one codebase and build from [PlatformIO](https://platformio.org/). Pick the env that matches your board.

```bash
pio run -e smalltv                 # ESP8266
pio run -e smalltv_lean            # ESP8266, without HA screens or the usage meter
pio run -e smalltv_c2              # ESP32-C2
pio run -e smalltv_esp32           # NM-TV-154 (classic ESP32)
pio run -e smalltv_esp32_8mb       # SmallTV Pro (classic ESP32, 8 MB flash)
pio run -e smalltv_c2 -t upload    # build + flash the C2 over USB-C
pio device monitor -e smalltv_c2   # serial logs @ 115200
pio run -e smalltv_loader          # ESP8266 loader for the SmallTV-ultra
```

Six envs, five published images plus the loader. Which file each env becomes is in [Which release file to download](/smalltv-mod/reference/release-assets/).

## The smalltv_lean env

`smalltv_lean` builds the same ESP8266 code as `smalltv` with Home Assistant screens and the Claude usage meter compiled out. It exists to buy heap, and it is published as `smalltv-mod-firmware-lean.bin`.

```bash
pio run -e smalltv_lean
```

The ESP8266 shares one 80 KB DRAM arena between static allocations and the heap, so a byte of static dropped is a byte the heap gains. On the 2.12.0 build:

| Build | Static RAM | Heap gained |
|-------|-----------|-------------|
| `smalltv` | 55,536 B | baseline |
| `smalltv_lean` | 46,804 B | 8,732 B |

Home Assistant is the bulk of that at 7,668 bytes, and two objects are almost all of Home Assistant: `g_screens`, the four-screen store, at 5,792 bytes, and `g_ic`, the icon cache, at 1,704 bytes. The 768-byte PubSubClient receive buffer also stops being allocated at runtime. The usage meter contributes 1,080 bytes. The two do not sum exactly to 8,732 because they share a little code and string data.

That headroom decides whether two fetch paths run at all. `features/radar/RadarClient.cpp` and `features/ticker/StockClient.cpp` both skip their fetch unless a 16,000-byte contiguous block is available, which is what the BearSSL handshake actually allocates. Both refuse quietly rather than crash, so a device below the threshold shows an empty scope or a blank ticker with no error. On a busy LAN, or over a weak link where retransmissions keep the WiFi queues full, the standard build can sit under both.

Read the current figures from `/api/status`: `heap` is free bytes and `maxblk` is the largest contiguous block.

The env inherits `smalltv` through `extends` and replaces its `build_flags`, so the board, flash layout, and libraries stay identical. Three flags do the work: `-D WITH_HA=0` and `-D WITH_USAGE=0` drop the features, and `-D SMALLTV_LEAN` selects the matching `UPDATE_ASSET` in `src/config.h` so a lean device self-updates to the lean image rather than the standard one.

### Rolling your own combination

The four `WITH_*` flags in `src/config.h` each default to 1 and can be set to 0 in any env, or passed through `PLATFORMIO_BUILD_FLAGS`:

| Flag | Drops | Static RAM saved on the ESP8266 |
|------|-------|--------------------------------|
| `WITH_HA=0` | Home Assistant screens over MQTT | 7,668 B |
| `WITH_TICKER=0` | The stock and crypto ticker | 3,328 B |
| `WITH_RADAR=0` | The plane radar | 2,180 B |
| `WITH_USAGE=0` | The Claude usage meter and its mascot | 1,080 B |

Each figure is that one flag measured on its own against the 55,536-byte standard build of 2.12.0. Combining flags saves slightly less than the sum, because the features share some code and string data.

Every feature is guarded at the module level, so dropping one takes its code, its settings handling, and its web UI tab with it. The tab disappears because `/api/config` reports the compiled-in features and the UI hides what is missing, which means a slim build needs no separate web UI.

Two caveats if you publish your own combination. `UPDATE_ASSET` still points at whichever variant your flags select, so a build that is neither standard nor lean will self-update into one of the published images and pick up the features you removed. And the mascot frame data lives in flash rather than RAM, so `WITH_USAGE=0` saves far more flash than heap.

## The smalltv_loader env

`smalltv_loader` (source `src/loader.cpp`) builds a minimal ESP8266 image: WiFi plus a web OTA endpoint at `/update`, nothing else. Its only job is the two-step [SmallTV-ultra install](/smalltv-mod/getting-started/flashing/#smalltv-ultra-stock-updater-says-not-enough-space), where the stock Ultra layout rejects the full image. The loader is small enough to fit that stock slot, and it uses this firmware's own 4m1m flash layout (`eagle.flash.4m1m.ld`), so its own `/update` slot is large enough to then accept the full `smalltv-mod-firmware.bin`.

```bash
pio run -e smalltv_loader
```

By default the loader opens an open access point named `SmallTV-Loader` at `192.168.4.1`. To have it auto-join an existing network instead, bake the credentials in at build time by adding the two flags to the env's `build_flags` (or via `PLATFORMIO_BUILD_FLAGS`):

```
-DLOADER_SSID='"MyNet"' -DLOADER_PASS='"secret"'
```

Credentials are compile-time only and never live in the repo. The build output is published as the release asset `smalltv-mod-loader.bin`.

## How one codebase builds for all four

Chip differences are centralized so the feature code stays the same on every board.

- `src/Platform.h` holds every chip-specific include, class alias, and small shim. The WiFi stack, web server, HTTPS client, OTA, and reset handling all resolve through it. The ESP32 targets share its Arduino core 3.x branch.
- `src/board_esp8266.h`, `src/board_esp32c2.h`, `src/board_esp32.h`, and `src/board_esp32_pro.h` hold the pin map and panel quirks for each board. `src/config.h` includes the right one based on the build target.
- The three feature modes, the web UI, and the settings layer are identical across all targets.

The target is chosen by a build flag: `SMALLTV_ESP8266`, `SMALLTV_ESP32C2`, or `SMALLTV_ESP32`. The SmallTV Pro defines `SMALLTV_ESP32` (chip-family code paths) plus `SMALLTV_ESP32_PRO` (its pin map and update asset).

## Project layout

```
src/                    shared core (device, net, web, settings)
  main.cpp              setup/loop and the mode registry
  loader.cpp            the standalone smalltv_loader image (built on its own)
  Platform.h            per-chip includes, aliases, and shims
  Mode.h                the DisplayMode interface every feature implements
  board_esp8266.h       ESP8266 pin map and panel quirks
  board_esp32c2.h       ESP32-C2 pin map and panel quirks
  board_esp32.h         NM-TV-154 (classic ESP32) pin map and panel quirks
  board_esp32_pro.h     SmallTV Pro (classic ESP32, 8 MB) pin map and panel quirks
  config.h              limits, feature flags, defaults, board selector
  Settings.*            settings struct and LittleFS persistence
  Net.*                 WiFi station, fallback AP, captive portal, mDNS
  WebPortal.*           web server, REST API, OTA endpoint
  webui.h               the single-page UI (HTML/CSS/JS, served from flash)
  Gfx.*                 shared ST7789 core (Arduino_GFX), colour correction
  Clock.*               SNTP and the night-mode window
  WgClient.*            WireGuard tunnel (no-op stubs without SMALLTV_WIREGUARD)
  BearSslTuning.cpp     ESP8266 TLS cipher/curve pinning
  OtaUpdate.*           GitHub self-update (ESP8266)
  features/
    ticker/             TickerMode + StockClient
    usage/              UsageMode + UsageClient + Mascot
    radar/              RadarMode + RadarClient
    notify/             NotifyMode + its overlay frames (armed over HTTP, never persisted)
partitions/             ESP32 flash layouts (4 MB shared by C2 + NM-TV-154, 8 MB for the Pro)
n8n/                    webhook contract and importable workflows
```

## ESP32 toolchain notes

The ESP32 targets have a few requirements the ESP8266 does not.

- **Platform**: PlatformIO's official espressif32 does not support the C2 and is stuck on Arduino core 2.x. All ESP32 envs use the [pioarduino](https://github.com/pioarduino/platform-espressif32) fork, which tracks Arduino core 3.x on ESP-IDF 5.x (`Platform.h`'s ESP32 branch needs core 3.x). The first build downloads a large toolchain and compiles the IDF from source, so it takes several minutes. Later builds are fast.
- **Display driver**: both use `Arduino_HWSPI` with explicit pins. The register-level `Arduino_ESP32SPI` hangs on the C2, and the software-SPI path in the library does not cover it. `Arduino_HWSPI` uses the stock SPI driver and works.
- **Flashing the C2**: uploads call the system esptool, not the one bundled with PlatformIO, which hangs entering download mode on that board. Install it with `pip install esptool`. The classic ESP32 flashes with either.
- **Partitions**: the 4 MB layout in `partitions/smalltv_4mb_ota.csv` gives two OTA app slots plus about 0.9 MB for LittleFS, and is shared by the C2 and the NM-TV-154. The SmallTV Pro's 8 MB layout in `partitions/smalltv_8mb_ota.csv` doubles the app slots (2.125 MB each) and gives about 3.7 MB of LittleFS, matching the stock firmware's table exactly.

## WireGuard

The optional WireGuard client is compiled into `smalltv_c2` and `smalltv_esp32_8mb`, the two envs whose app slot has room for it. Its switches live in those envs in `platformio.ini`: `-D SMALLTV_WIREGUARD=1` plus `-D CONFIG_WIREGUARD_MAX_PEERS=1` and `-D CONFIG_WIREGUARD_MAX_SRC_IPS=5`, and `droscy/esp_wireguard @ 0.4.5` in `lib_deps`. These have to be `build_flags`, not `build_src_flags`, because they must reach the library's own translation units. Without the flag `src/WgClient.cpp` compiles to no-op stubs, so every other env builds unchanged.

To build it for `smalltv_esp32` anyway, copy those four lines into that env. It links, but at 1,571,195 bytes against a 1,572,864-byte app slot, so it fits only as long as nothing else grows. To give it real room, raise both `app0` and `app1` in `partitions/smalltv_4mb_ota.csv` and take the space from `spiffs` (0xF0000 is generous for one `config.json`). That is a partition-table change, so the device has to be flashed over USB with `firmware.factory.bin`; an over-the-air update cannot install it.

## Footprint

Measured as the flashable `firmware.bin`, which is what an OTA slot has to hold: the ESP8266 build is 694 KB of a 1,020 KB budget and roughly half the RAM at boot, with headroom for OTA, which needs room for two sketch copies. The ESP32-C2 build is 1,469,520 bytes of a 1,572,864-byte app slot with the WireGuard client in it, using around 16 percent of RAM. The classic ESP32 build is 1,501,344 bytes in the same slot, which is why WireGuard is not in it. The SmallTV Pro runs the same code in a 2,228,224-byte slot and comes to 1,573,904 bytes with the client, 71 percent. The mascot frame data lives in flash, not the heap.

The PC-side usage daemon is a separate repo: [clawdmeter-daemon](https://github.com/giovi321/clawdmeter-daemon).
