# SmallTV screen board: three screens from one blueprint automation. Screen 1
# is template-driven: it combines the indoor and outdoor temperature into one
# value line and switches icon and colour with the window contact, green
# window-open when the window is open, red window-closed when it is shut.
# Screens 2 and 3 stay static: outdoor weather with a sun icon on s2, and the
# energy plug's power with a plug icon on s3. The screens rotate in slot
# order at the device's dwell time.
#
# Import the blueprint first: Settings -> Automations & scenes -> Blueprints
# -> Import blueprint, then paste
# https://github.com/giovi321/smalltv-mod/blob/main/blueprints/automation/smalltv/screen_board.yaml
# or drop the file into config/blueprints/automation/smalltv/ yourself.
# Replace the entity ids and the hostname. Slot names sort lexicographically
# and must be unique per device across every automation publishing to the
# same hostname. Screen 4 is left empty here, so it is skipped entirely.
#
# watched_entities lists every entity used anywhere, including the ones only
# named inside screen 1's templates. Without the window contact in that list,
# screen 1 would only pick up window changes on the one-minute timer.

automation:
  - alias: SmallTV screen board
    use_blueprint:
      path: smalltv/screen_board.yaml
      input:
        hostname: smalltv
        watched_entities:
          - sensor.living_room_temperature
          - sensor.outdoor_temperature
          - binary_sensor.living_room_window
          - sensor.energy_plug_power
        screen_1_entity: sensor.living_room_temperature
        screen_1_title: Living room
        screen_1_value_tpl: >-
          {{ states('sensor.living_room_temperature') }}° in /
          {{ states('sensor.outdoor_temperature') }}° out
        screen_1_icon: thermometer
        screen_1_icon_tpl: >-
          {{ 'window-open' if is_state('binary_sensor.living_room_window', 'on')
             else 'window-closed' }}
        screen_1_color: "#FFCC00"
        screen_1_color_tpl: >-
          {{ '#00C853' if is_state('binary_sensor.living_room_window', 'on')
             else '#D50000' }}
        screen_1_bg: "#003366"
        screen_1_slot: s1
        screen_2_entity: sensor.outdoor_temperature
        screen_2_title: Outside
        screen_2_icon: sun
        screen_2_color: "#FFFFFF"
        screen_2_bg: "#1A1A1A"
        screen_2_slot: s2
        screen_3_entity: sensor.energy_plug_power
        screen_3_title: Dishwasher
        screen_3_icon: plug
        screen_3_color: "#00FF88"
        screen_3_bg: "#000000"
        screen_3_slot: s3
