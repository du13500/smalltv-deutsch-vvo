# SmallTV screen board: three screens from one blueprint automation. Living
# room temperature with a thermometer icon on slot s1, outdoor weather with a
# sun icon on s2, and the energy plug's power with a plug icon on s3,
# rotating in slot order at the device's dwell time.
#
# Import the blueprint first: Settings -> Automations & scenes -> Blueprints
# -> Import blueprint, then paste
# https://github.com/giovi321/smalltv-mod/blob/main/blueprints/automation/smalltv/screen_board.yaml
# or drop the file into config/blueprints/automation/smalltv/ yourself.
# Replace the entity ids and the hostname. Slot names sort lexicographically
# and must be unique per device across every automation publishing to the
# same hostname. Screen 4 is left empty here, so it is skipped entirely.

automation:
  - alias: SmallTV screen board
    use_blueprint:
      path: smalltv/screen_board.yaml
      input:
        hostname: smalltv
        watched_entities:
          - sensor.living_room_temperature
          - sensor.outdoor_temperature
          - sensor.energy_plug_power
        screen_1_entity: sensor.living_room_temperature
        screen_1_title: Living room
        screen_1_icon: thermometer
        screen_1_color: "#FFCC00"
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
