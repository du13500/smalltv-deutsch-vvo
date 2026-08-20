---
title: Home Assistant screens
description: Full screens pushed from Home Assistant over MQTT as small JSON draw lists, one retained message per slot, rotating in the device carousel.
---

The other features are things the device fetches itself. This one is the opposite: you publish a small JSON document to an MQTT topic, and the device draws it as a full 240×240 screen and keeps it in the carousel. A temperature warning, a door-left-open reminder, the day's energy total — anything Home Assistant can template, the panel can show.

Because every screen is a retained message, the broker holds the last copy. The device can reboot, Home Assistant can reboot, and the screens come back on their own.

## Pointing the device at your broker

Open the settings page and find the MQTT card. Enter your broker's host, port (1883 by default), and an optional username and password, then save. The connection is plain TCP, not TLS, so keep the broker on your LAN.

The device identifies itself by its hostname — the same name you see in the web UI and browse to as `<hostname>.local`. That name appears in every topic below; the examples use `smalltv`.

## The MQTT contract

| Topic | Direction | What it carries |
|---|---|---|
| `smalltv/<hostname>/availability` | device → broker | `online`, retained, when the device connects. The broker holds a retained `offline` as the device's last will, so the topic always tells you whether the panel is really there. |
| `smalltv/<hostname>/screen/<slot>` | broker → device | One screen, as retained JSON. `<slot>` is a name you pick, such as `window` or `energy`. |

Screen messages must be published with the retain flag. That is not a nicety: it is how the screen survives a device reboot, and it is how a screen that was published while the device was off appears when it comes back.

Publishing an **empty retained payload** to a screen topic deletes that slot, from both the broker and the device.

When several slots are set, the device rotates through them in slot order, dwelling on each for the time configured on the device (15 seconds by default). A screen whose `ttl` has expired drops out of the rotation without anything needing to republish.

### Limits

Keep payloads small. The numbers that matter:

| Limit | ESP32 builds | ESP8266 build |
|---|---|---|
| Payload size | ~2 KB | ~700 B |
| Screens (slots) | 8 | 4 |
| Draw primitives per screen | 48 | 24 |

## Drawing on the screen

A screen is a JSON object with an optional background colour, an optional time-to-live, and a list of draw primitives, painted in order onto the 240×240 panel:

```json
{
  "bg": "#00AA00",
  "ttl": 0,
  "draw": [
    {"t":"fill","c":"#003300"},
    {"t":"rect","x":10,"y":10,"w":220,"h":60,"c":"#FFFFFF","r":8},
    {"t":"circle","x":120,"y":120,"r":40,"c":"#FF0000"},
    {"t":"line","x":0,"y":200,"x2":240,"y2":200,"c":"#888888"},
    {"t":"text","x":120,"y":60,"s":2,"c":"#FFFFFF","a":"c","v":"Open the window"},
    {"t":"text","x":120,"y":120,"s":3,"c":"#FFFF00","a":"c","v":"21.5 in / 18.2 out"}
  ]
}
```

| Field | Meaning |
|---|---|
| `bg` | Background colour, default black. |
| `ttl` | Seconds before the screen drops out of the rotation. `0` means sticky: it stays until you delete or replace it. |
| `draw` | List of primitives, painted in order. |

The primitives:

| `t` | Fields | Draws |
|---|---|---|
| `fill` | `c` | Flood the whole panel with a colour. |
| `rect` | `x`, `y`, `w`, `h`, `c` | A rectangle. |
| `rrect` | `x`, `y`, `w`, `h`, `c`, `r` | A rectangle with corner radius `r`. |
| `circle` | `x`, `y`, `r`, `c` | A circle at centre `x`,`y`. |
| `line` | `x`, `y`, `x2`, `y2`, `c` | A straight line. |
| `text` | `x`, `y`, `s`, `c`, `a`, `v` | A string in the built-in 6×8 font. |

Colours are `#RRGGBB`. Text `s` is an integer scale of the 6×8 font, and `a` aligns the string left (`l`), centre (`c`), or right (`r`) around `x`. The string is capped at 64 characters; stick to plain ASCII, as anything else disappears rather than drawing.

Parsing is forgiving in a specific direction. Unknown fields are ignored, so you can add your own annotations. A malformed primitive is skipped and the rest of the list still draws. But a payload that is not valid JSON at all leaves the slot exactly as it was, so a templating mistake in Home Assistant cannot blank a working screen.

## The window screen from Home Assistant

The flagship use: compare indoor and outdoor temperature and show a green or red full-screen answer to "should I open the window?". One automation, one `mqtt.publish`:

```yaml
automation:
  - alias: SmallTV window screen
    mode: restart
    triggers:
      - trigger: state
        entity_id: sensor.living_room_temperature
      - trigger: state
        entity_id: sensor.outdoor_temperature
      - trigger: time_pattern
        minutes: "/5"   # safety refresh, in case a change was missed
      - trigger: homeassistant
        event: start    # republish the retained screen after an HA restart
    conditions:
      - condition: template
        value_template: >-
          {{ states('sensor.living_room_temperature') not in ('unknown', 'unavailable')
             and states('sensor.outdoor_temperature') not in ('unknown', 'unavailable') }}
    actions:
      - action: mqtt.publish
        data:
          topic: smalltv/smalltv/screen/window
          retain: true
          payload: >-
            {% set ind = states('sensor.living_room_temperature') | float(0) -%}
            {% set out = states('sensor.outdoor_temperature') | float(0) -%}
            {% set open = out <= ind - 0.5 -%}
            {
              "bg": "{{ '#007A1F' if open else '#B00020' }}",
              "ttl": 0,
              "draw": [
                {"t":"text","x":120,"y":60,"s":2,"c":"#FFFFFF","a":"c",
                 "v":{{ ('Open the window' if open else 'Keep it closed') | tojson }}},
                {"t":"line","x":20,"y":110,"x2":220,"y2":110,"c":"#FFFFFF"},
                {"t":"text","x":120,"y":150,"s":2,"c":"#FFFFFF","a":"c",
                 "v":"{{ '%.1f in / %.1f out' | format(ind, out) }}"}
              ]
            }
```

Every time either sensor moves, the screen republishes; the retained message keeps the broker's copy current. The time-pattern trigger republishes within five minutes even if a state change slipped past, and the startup trigger puts the screen back after a Home Assistant restart.

The same automation exists as an importable blueprint, with the sensors, delta, hostname, slot, and labels as inputs: [`blueprints/automation/smalltv/temp_compare.yaml`](https://github.com/giovi321/smalltv-mod/blob/main/blueprints/automation/smalltv/temp_compare.yaml) in the repository. Import it under **Settings → Automations & scenes → Blueprints → Import blueprint** by pasting that URL. Ready-made YAML for all of these examples also lives in [`examples/ha/`](https://github.com/giovi321/smalltv-mod/tree/main/examples/ha/).

## More than one screen

Slots are independent, so several automations — or one automation with several publish actions — build a carousel. These three publishes give you a weather line, the window advice above, and today's energy total, rotating at the device's dwell time:

```yaml
- action: mqtt.publish
  data:
    topic: smalltv/smalltv/screen/weather
    retain: true
    payload: >-
      {"bg":"#003366","draw":[
        {"t":"text","x":120,"y":80,"s":2,"c":"#FFFFFF","a":"c","v":"Outside"},
        {"t":"text","x":120,"y":130,"s":4,"c":"#FFFF00","a":"c",
         "v":"{{ states('sensor.outdoor_temperature') }} C"}]}

- action: mqtt.publish
  data:
    topic: smalltv/smalltv/screen/window
    retain: true
    payload: >-
      {"bg":"#007A1F","draw":[
        {"t":"text","x":120,"y":120,"s":2,"c":"#FFFFFF","a":"c","v":"Open the window"}]}

- action: mqtt.publish
  data:
    topic: smalltv/smalltv/screen/zenergy
    retain: true
    payload: >-
      {"bg":"#1A1A1A","draw":[
        {"t":"text","x":120,"y":80,"s":2,"c":"#FFFFFF","a":"c","v":"Today"},
        {"t":"text","x":120,"y":130,"s":3,"c":"#00FF88","a":"c",
         "v":"{{ states('sensor.energy_today') }} kWh"}]}
```

Slot order is the carousel order, so the leading `z` on `zenergy` is deliberate — it keeps that screen last. Stay under the per-board screen count: 8 slots on the ESP32 builds, 4 on the ESP8266.

## Deleting a screen

Publish an empty retained payload to the slot's topic. The broker drops its retained copy and the device drops the screen:

```yaml
- action: mqtt.publish
  data:
    topic: smalltv/smalltv/screen/window
    retain: true
    payload: ""
```

## Trying it without Home Assistant

Any MQTT client works. With `mosquitto_pub`, one line sets a screen and one line deletes it:

```bash
# set a screen (note -r for retain)
mosquitto_pub -h broker.local -t smalltv/smalltv/screen/hello -r -m \
  '{"bg":"#003366","draw":[{"t":"text","x":120,"y":120,"s":2,"c":"#FFFFFF","a":"c","v":"Hello"}]}'

# delete it: -n sends an empty payload
mosquitto_pub -h broker.local -t smalltv/smalltv/screen/hello -r -n

# is the panel actually connected?
mosquitto_sub -h broker.local -t smalltv/smalltv/availability -C 1 -W 2
```

If the screen does not appear, check the payload size against the limits table above first — an overlong payload is the most common cause, and a JSON syntax error is silently ignored by design.
