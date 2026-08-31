# SmallTV two-sensor screen: the outdoor and the bedroom temperature side by
# side on one screen. The panel is split down the middle, each half gets an
# icon, a short label, the value in big type, and the unit on its own row, and
# a vertical line separates them.
#
# Import the blueprint first: Settings -> Automations & scenes -> Blueprints
# -> Import blueprint, then paste
# https://github.com/giovi321/smalltv-mod/blob/main/blueprints/automation/smalltv/dual_sensor.yaml
# or drop the file into config/blueprints/automation/smalltv/ yourself.
# Replace the entity ids and the hostname. The slot name must be unique per
# device across every automation publishing to the same hostname.
#
# Both labels are written by hand. Left empty they fall back to the entity's
# friendly name, and "Bedroom 2 temperature" does not fit a 112 px column, so
# it would be cut to "Bedroom 2 temperat".
#
# The units come from each sensor's unit_of_measurement, so they are not set
# here. Both columns cap the value at one decimal place, which is the default:
# a sensor reporting 21.718751234567 draws as "21.7" at text scale 4 instead of
# shrinking to scale 1 to fit all 15 characters. The cap drops trailing zeros
# and leaves non-numeric states alone, so 1234 stays "1234". Set a column's
# decimals to "off" to print whatever the sensor reports.
#
# The right column's colour template turns the bedroom reading red above 24
# degrees and blue below 18. Because that template reads the same entity the
# column already triggers on, watched_entities can stay empty; list an entity
# there only when it appears in a template and nowhere else, or that change
# is only picked up by the one-minute safety timer.
#
# Two icons plus a title put this payload at about 715 bytes, just over the
# ESP8266 limit of ~700 B. On an ESP8266 set the icons to "none" (-132 B),
# turn on unit_inline (-125 B), or clear the title (-75 B). The ESP32 builds
# have ~2 KB and do not care.

automation:
  - alias: SmallTV two temperatures
    use_blueprint:
      path: smalltv/dual_sensor.yaml
      input:
        hostname: smalltv
        slot: duo
        title: Temperatures
        bg: "#000000"
        divider_color: "#444444"
        left_entity: sensor.outside_temperature
        left_label: Outside
        left_icon: sun
        left_color: "#8AB4F8"
        right_entity: sensor.bedroom_2_temperature
        right_label: Bedroom 2
        right_decimals: "1"
        right_icon: home
        right_color: "#FFCC00"
        right_color_tpl: >-
          {% set t = states('sensor.bedroom_2_temperature') | float(0) %}
          {{ '#D50000' if t > 24 else '#8AB4F8' if t < 18 else '#FFCC00' }}
