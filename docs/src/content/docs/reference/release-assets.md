---
title: Which release file to download
description: Every firmware asset attached to a release, what each name means, and which one your board needs.
---

Nine files are attached to every release, and the name tells you which one you need. Read it in three parts: the board, whether the image is an app image or a factory image, and, on the ESP8266, which feature set it carries.

Get them from the [Releases page](https://github.com/giovi321/smalltv-mod/releases). Untagged builds of the same nine files are on the [Actions tab](https://github.com/giovi321/smalltv-mod/actions) under the latest `build` run.

## Pick by board, then by install method

| File | Board | Use it for |
|------|-------|-----------|
| `smalltv-mod-firmware.bin` | SmallTV and SmallTV-ultra (ESP8266) | The normal install and every later update |
| `smalltv-mod-firmware-lean.bin` | Same ESP8266 boards | The same device when it needs more heap. Home Assistant screens and the usage meter are compiled out |
| `smalltv-mod-loader.bin` | SmallTV-ultra (ESP8266) | One-time first install when the stock updater rejects the full image |
| `smalltv-mod-firmware-c2.bin` | SmallTV (ESP32-C2 / ESP8684) | Updates, once this firmware is already running |
| `smalltv-mod-firmware-c2.factory.bin` | SmallTV (ESP32-C2 / ESP8684) | The first install, over USB-C |
| `smalltv-mod-firmware-esp32.bin` | NM-TV-154 (classic ESP32) | Updates, once this firmware is already running |
| `smalltv-mod-firmware-esp32.factory.bin` | NM-TV-154 (classic ESP32) | The first install, over USB |
| `smalltv-mod-firmware-esp32-pro.bin` | SmallTV Pro (classic ESP32, 8 MB) | The first install over the stock web UI, and every later update |
| `smalltv-mod-firmware-esp32-pro.factory.bin` | SmallTV Pro (classic ESP32, 8 MB) | A direct install or a recovery over the internal UART header |

Not sure which board you have? [Hardware and variants](/smalltv-mod/getting-started/hardware/) has photos and the tell-tale signs for each.

## How to read a file name

Every name follows the same pattern:

```
smalltv-mod-<image>[-<target>][.factory].bin
```

The `<image>` part is `firmware` for the real thing and `loader` for the minimal ESP8266 installer.

The `<target>` suffix names the board. No suffix at all means the original ESP8266:

| Suffix | Board |
|--------|-------|
| none | SmallTV and SmallTV-ultra (ESP8266), all features |
| `-lean` | The same ESP8266 boards, without Home Assistant screens or the usage meter |
| `-c2` | SmallTV with the ESP32-C2 / ESP8684 chip |
| `-esp32` | NM-TV-154, classic ESP32, 4 MB flash |
| `-esp32-pro` | SmallTV Pro, classic ESP32, 8 MB flash |

`.factory` marks a merged image. It contains the bootloader, the partition table, and the app, and it gets written to flash offset `0x0` over a cable. A name without `.factory` is an app image: just the application, sized to drop into an OTA slot. The ESP32 boards need the factory image for their first install and the app image for updates after that. The ESP8266 needs no factory variant, because its single image already carries everything.

## Check what a device is running

The System tab shows the running variant next to the version, for example `smalltv-mod 2.12.0 (esp8266-lean)`. The same string is in `/api/status` as `variant`.

That name also decides what a self-update downloads. `UPDATE_ASSET` in `src/config.h` maps each build to its own file, so a lean device fetches `smalltv-mod-firmware-lean.bin` and stays lean. It will not quietly move to the standard image and take its features back.

## Switch between the standard and lean ESP8266 images

Upload the other file in the System tab. Settings survive the swap, since both images read the same `config.json` from the same LittleFS partition, and neither one touches the layout.

One thing does not survive. The lean image has no Home Assistant module, so screens already pushed to the device are dropped and their MQTT topics are no longer subscribed. Your retained messages on the broker are untouched, so going back to the standard image and letting it resubscribe brings the screens back.

## Why the ESP8266 gets two images and the other boards get one

The ESP8266 shares a single 80 KB DRAM arena between static allocations and the heap, so anything compiled in costs heap that TLS then cannot have. Two fetch paths refuse to start a handshake when memory runs short: the plane radar wants 18,000 bytes of free heap, and a cash.ch quote wants a 16,000-byte contiguous block. Below those thresholds the affected screen goes quiet with nothing on it to explain why.

The lean image gives that back. Measured on the 2.12.0 build, static RAM drops from 55,536 bytes to 46,804, so the heap starts 8,732 bytes larger. Almost all of it is Home Assistant: 7,668 bytes, of which the four-screen store (`g_screens`, 5,792 bytes) and the icon cache (`g_ic`, 1,704 bytes) are the bulk, plus the 768-byte PubSubClient receive buffer that no longer gets allocated at runtime. The usage meter contributes 1,080 bytes.

The ESP32 boards have several times the RAM and manage TLS buffers dynamically through mbedTLS, so they never come near these limits and need only one image each.

See [Building from source](/smalltv-mod/reference/building/#the-smalltv_lean-env) to build either ESP8266 image yourself, or a slimmer combination of your own.
