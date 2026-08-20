// MqttClient.h — MQTT connection for the Home Assistant screens feature.
//
// PubSubClient over a plain WiFiClient (non-TLS, LAN broker). Everything is
// driven from mqttLoop() off the main loop: connect only while WiFi STA is up,
// reconnect with a millis()-based backoff, never block waiting. The wire
// contract (docs: features/ha):
//   LWT  smalltv/<hostname>/availability = "offline", retained
//   on connect: publish "online" retained to the same topic, then subscribe
//   smalltv/<hostname>/screen/+ — one retained JSON screen per slot.
// Incoming screen messages go straight to the HaScreens store.
//
// Broker settings changes (web UI save) are picked up live: the loop compares
// a snapshot of what it connected with against the current settings and
// reconnects when they differ.
#pragma once
#include <Arduino.h>
#include "Settings.h"

void mqttBegin(const Settings& s);   // arm; connects from the loop, not here
void mqttLoop();                     // call every main-loop tick, after netLoop()

bool mqttConnected();                                        // broker link up right now
// Publish from outside the loop's own availability traffic. Returns false when
// not connected. An empty payload with retained=true is exactly how a broker
// deletes a retained message (used by /api/ha/clear to purge zombie screens).
bool mqttPublish(const char* topic, const char* payload, bool retained);
