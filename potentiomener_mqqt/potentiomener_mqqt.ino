#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secret.h"

# define DEBUG 0



WiFiClient espClient;
PubSubClient client(espClient);

const int pinLevel = 33;
const int pinSwitch = 19;
const int pinSpeaker = 15;

int calculateMqttVoltage(int input) {
return (input*16);
}

short unsigned int onlineSwitch = 0;
short unsigned int onlineLevel = 0;

int getLevel() {
	return ((analogRead(pinLevel) * 100) / 4095);
}

int getSwitch() {
	return (digitalRead(pinSwitch));
}

void setup() {

	Serial.begin(115200);
	setup_wifi();
	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);
}

void setup_wifi() {
	delay(10);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	// deserialize json
	StaticJsonDocument<244> doc;
	deserializeJson(doc, payload, length);

	short unsigned int idx = doc["idx"];

	Serial.println(idx);
	if (idx == 7) {
		onlineLevel = (int)doc["svalue1"];
		onlineSwitch = (int)doc["nvalue"];
	}

	Serial.println(onlineLevel);
	Serial.println(onlineSwitch);
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		String clientId = "ESP32Client-";
		clientId += String(random(0xffff), HEX);
		if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
			Serial.println("connected");
			// Subscribe
			client.subscribe("esp/out/7");
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

int i = 0;
float value;

void loop() {
	static char buffer[70];
	if (!client.connected()) {
		reconnect();
	}
	sprintf(buffer,"{  \"idx\" : 6,  \"nvalue\" : %i,  \"svalue\" : \"%f\" }", getSwitch(), (float)(getLevel()%16));
	// Serial.println(buffer);
	client.publish("domoticz/in", buffer);
	//Serial.print("Value on a pin changed to: ");
	//Serial.println(getLevel());
	//Serial.println(getSwitch());
	client.loop();
	//Serial.println(calculateMqttVoltage(onlineLevel));
  
  value = (i % 3000);  
  if (i > 1000) {
    i = 0;
  }

	if (getSwitch() == 1) {
    value *= (getLevel() / 255);
	} else {
    value *= (calculateMqttVoltage(onlineLevel) / 255);
  }
  
  Serial.println(value);
    
  analogWrite(pinSpeaker, value);
  i++;
}

