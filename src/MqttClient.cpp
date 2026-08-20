// MqttClient.cpp — see MqttClient.h for the contract and the wire topics.
#include "MqttClient.h"
#include "config.h"
#if WITH_HA
#include <PubSubClient.h>
#include "Platform.h"   // brings in WiFi.h / ESP8266WiFi.h per target
#include "Net.h"
#include "HaScreens.h"

// Reconnect backoff: 5 s after a failure, doubling up to a 60 s cap; reset on
// a successful connect.
#define MQTT_RETRY_MIN_MS  5000UL
#define MQTT_RETRY_MAX_MS 60000UL

static const Settings* S = nullptr;
static WiFiClient   g_tcp;
static PubSubClient g_mqtt(g_tcp);

static uint32_t g_backoffMs   = MQTT_RETRY_MIN_MS;
static uint32_t g_nextAttempt = 0;

// Snapshot of what the current connection (or attempt) was made with; a save
// from the web UI that changes any of it forces a reconnect.
static char     g_connHost[MAX_HA_HOST_LEN] = "";
static uint16_t g_connPort = 0;
static char     g_connUser[MAX_HA_USER_LEN] = "";
static char     g_connPass[MAX_HA_PASS_LEN] = "";

static bool brokerChanged() {
  return g_connPort != S->ha.brokerPort
      || strncmp(g_connHost, S->ha.brokerHost.c_str(), MAX_HA_HOST_LEN)
      || strncmp(g_connUser, S->ha.brokerUser.c_str(), MAX_HA_USER_LEN)
      || strncmp(g_connPass, S->ha.brokerPass.c_str(), MAX_HA_PASS_LEN);
}

// smalltv/<hostname>/screen/<slot> — the slot is everything after the last
// '/', used as-is (no case folding; the docs' examples are all lowercase).
static void onMessage(char* topic, uint8_t* payload, unsigned int len) {
  const char* slash = strrchr(topic, '/');
  if (!slash || !slash[1]) return;
  haScreensApply(slash + 1, payload, len);
}

void mqttBegin(const Settings& s) {
  S = &s;
  g_mqtt.setCallback(onMessage);
}

bool mqttConnected() { return g_mqtt.connected(); }

bool mqttPublish(const char* topic, const char* payload, bool retained) {
  if (!g_mqtt.connected()) return false;
  return g_mqtt.publish(topic, payload, retained);
}

void mqttLoop() {
  haScreensService();   // debounced persist, even with no broker configured
  if (!S) return;

  // No broker configured: the feature is off. Dropping an existing connection
  // here is what turns a cleared broker host into a clean disconnect.
  if (!S->ha.brokerHost.length()) {
    if (g_mqtt.connected()) g_mqtt.disconnect();
    g_connHost[0] = 0;    // counts as "changed" when a host is set again
    return;
  }

  // Only connect while the station is up (never in AP/setup mode).
  if (netMode() != NET_STA || !netConnected()) {
    if (g_mqtt.connected()) g_mqtt.disconnect();
    return;
  }

  if (g_mqtt.connected()) {
    if (brokerChanged()) g_mqtt.disconnect();   // reconnect below, next tick
    else { g_mqtt.loop(); return; }
  }

  // Disconnected. A settings change retries immediately; failures back off.
  uint32_t now = millis();
  if (brokerChanged()) g_nextAttempt = now;
  if ((int32_t)(now - g_nextAttempt) < 0) return;
  g_nextAttempt = now + g_backoffMs;
  // Double the backoff up to the cap (written out: ESP8266's min() template
  // rejects the mixed uint32_t/unsigned-long types).
  if (g_backoffMs < MQTT_RETRY_MAX_MS / 2) g_backoffMs *= 2;
  else g_backoffMs = MQTT_RETRY_MAX_MS;

  strlcpy(g_connHost, S->ha.brokerHost.c_str(), sizeof(g_connHost));
  g_connPort = S->ha.brokerPort;
  strlcpy(g_connUser, S->ha.brokerUser.c_str(), sizeof(g_connUser));
  strlcpy(g_connPass, S->ha.brokerPass.c_str(), sizeof(g_connPass));

  char avail[96], sub[104];
  snprintf(avail, sizeof(avail), "smalltv/%s/availability", S->hostname.c_str());
  snprintf(sub, sizeof(sub), "smalltv/%s/screen/+", S->hostname.c_str());

  g_mqtt.setServer(g_connHost, g_connPort);
  // PubSubClient::connect() registers the LWT (retained "offline") and does a
  // synchronous TCP connect — a dead broker IP can stall the loop for the
  // TCP timeout here, once per backoff interval.
  if (!g_mqtt.connect(S->hostname.c_str(),
                      g_connUser[0] ? g_connUser : nullptr,
                      g_connPass[0] ? g_connPass : nullptr,
                      avail, 0, true, "offline")) {
    return;
  }

  g_backoffMs = MQTT_RETRY_MIN_MS;
  // Both availability messages are retained, so a `mosquitto_sub -C 1` always
  // gets the current state immediately (docs verify it that way).
  g_mqtt.publish(avail, "online", /*retained=*/true);
  g_mqtt.subscribe(sub);
  g_mqtt.loop();
}

#endif  // WITH_HA
