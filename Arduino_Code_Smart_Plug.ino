#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// --- CONFIGURARE SERVER ---
const char* mqtt_server = "broker.hivemq.com";
const char* topic_cmd = "bia_smart_12340/priza/comenzi";
const char* topic_status = "bia_smart_12340/priza/status";

WiFiClient espClient;
PubSubClient client(espClient);

// Pinii releelor
const int pin1 = 14;
const int pin2 = 25;
const int pin3 = 33;

bool state1 = false, state2 = false, state3 = false;

void setup() {
  Serial.begin(115200);
  
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);

  // Stare inițială oprit
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);

  WiFiManager wm;

  // 1. Încercare rapidă de conectare la rețeaua salvată
  wm.setConnectTimeout(3); 
  
  //The Password is not Password, but I changed it before uploading the file for security
  if(!wm.autoConnect("Configurare_Priza_Smart", "Password")) {
      Serial.println("Eroare critică portal. Reîncercare...");
      delay(2000);
      ESP.restart();
  }

  // 3. Odată ce utilizatorul a introdus datele corecte în portal, codul continuă aici:
  Serial.println("WiFi configurat și conectat cu succes!");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) message += (char)payload[i];
  
  Serial.println("Comanda primită: " + message);

  if (message == "1_TOGGLE") {
    state1 = !state1;
    digitalWrite(pin1, state1 ? HIGH : LOW);
    client.publish(topic_status, state1 ? "1:ON" : "1:OFF");
  }
  if (message == "2_TOGGLE") {
    state2 = !state2;
    digitalWrite(pin2, state2 ? HIGH : LOW);
    client.publish(topic_status, state2 ? "2:ON" : "2:OFF");
  }
  if (message == "3_TOGGLE") {
    state3 = !state3;
    digitalWrite(pin3, state3 ? HIGH : LOW);
    client.publish(topic_status, state3 ? "3:ON" : "3:OFF");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Încercare conectare MQTT...");
    String clientId = "ESP32-Priza-Bia-"; 
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("CONECTAT!");
      client.subscribe(topic_cmd);
      client.publish(topic_status, state1 ? "1:ON" : "1:OFF");
      client.publish(topic_status, state2 ? "2:ON" : "2:OFF");
      client.publish(topic_status, state3 ? "3:ON" : "3:OFF");
    } else {
      Serial.print("Eroare: ");
      Serial.print(client.state());
      Serial.println(" reîncercare în 5 secunde...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}