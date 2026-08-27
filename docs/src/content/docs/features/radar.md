---
title: Plane radar
description: A radar scope of nearby aircraft, centred on your location, from the free adsb.lol or adsb.fi feeds or a LAN webhook.
---

Switch **Display → Mode** to **Radar** and set it up in the **Radar** tab. The screen becomes a radar scope centred on where you are: range rings, a home marker in the middle, nearby aircraft as red heading triangles with a speed vector and a callsign or altitude label, traffic outside the ring as dots on the rim, and any airports you add as small markers.

## Set your location

Enter your home latitude and longitude in decimal degrees, a range ring (5, 10, 15, 20, 25, or 50), and whether to measure in km or miles. Then pick a data source, below.

## Display options

The Radar tab's **What to show** card tunes what appears. Alongside the two settings below it carries independent toggles for the callsign and altitude labels, the speed vectors, and the rim dots for traffic beyond the outer ring.

- **Marker and label size** (Small, Medium, Large) scales the triangles, markers, and text together. Medium is the default. Large reads from across the room; Small keeps a busy sky uncluttered.
- **Hide aircraft below (ft)** drops ground and low traffic. Set it to `500` and planes taxiing at the airport disappear; `0` shows everything.
- Callsigns are placed nearest first, and any label that would overlap one already on screen is skipped, so a crowded scope stays legible. The triangle still shows even when its label was dropped.

## Data sources

Three choices, set in the Radar tab. Two fetch a free open-data feed straight from the device; the third goes through a proxy on your own network.

### adsb.lol, the default on the ESP8266

The device fetches the free [adsb.lol](https://adsb.lol) API directly over HTTPS, one request per refresh:

```
GET https://api.adsb.lol/v2/lat/<lat>/lon/<lon>/dist/<nm>
```

No API key. The device parses the feed and keeps the closest 24 aircraft. The endpoint is rate-limited to about one request a second, so keep the refresh interval sensible. The default is 10 seconds.

### adsb.fi, the default on the ESP32

Same shape, same fields, a different host:

```
GET https://opendata.adsb.fi/api/v3/lat/<lat>/lon/<lon>/dist/<nm>
```

### Which direct feed to pick

On the ESP8266, HTTPS is tight on RAM, and the deciding factor is the TLS record size the server uses. The device probes the server's Maximum Fragment Length support so its buffer can stay at 512 bytes; without MFLN it has to fall back to 4 KB, and if the server then sends a record larger than that, the read fails part-way through and the scope stays empty.

adsb.fi moved behind Cloudflare in August 2026. Cloudflare does not negotiate MFLN and ramps its record size up on large responses, so **direct adsb.fi no longer works on the ESP8266**. A 50 nm response is around 50 KB and the connection breaks mid-stream. adsb.lol still honours MFLN, which is why it is the ESP8266 default.

The ESP32 boards use mbedTLS with dynamically sized buffers and are unaffected, so either feed works there.

If your chosen feed ever goes behind a CDN too, switch to the other one or to the webhook below.

### When the scope stays empty on an ESP8266

Before it opens an HTTPS connection the radar checks that 18,000 bytes of heap are free, and skips the poll if they are not. It fails silently rather than crash, so an empty scope can mean the device never sent a request at all. Read `heap` in the Status tab to tell the two apart.

Two ways out. Install `smalltv-mod-firmware-lean.bin`, which compiles out Home Assistant screens and the usage meter and leaves the heap 8,732 bytes larger, or use the webhook below, since the check only guards the HTTPS path. [Which release file to download](/smalltv-mod/reference/release-assets/) and the [troubleshooting entry](/smalltv-mod/manual/troubleshooting/#the-radar-scope-is-empty-or-a-ticker-is-blank-with-no-error-shown) go into more detail.

### Custom webhook, a LAN proxy

Point a small proxy (n8n, Node-RED, or a short script) at either feed, filter it down to the nearest few aircraft, and return a small JSON over plain HTTP on your LAN. Set the source to **Custom webhook**, give it the URL, and the device calls:

```
GET <webhookUrl>?lat=<lat>&lon=<lon>&dist=<km>
```

It expects the same `{"ac":[ ... ]}` shape both feeds return, with these fields per aircraft: `lat`, `lon`, `track`, `gs`, `flight`, `hex`, `alt_baro`. The proxy does the filtering and the TLS handshake, which is the most reliable setup on the ESP8266.
