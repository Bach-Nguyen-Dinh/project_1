#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "CongB5";
const char* pass = "Bach00Hai03";
const char* mqtt_server = "192.168.100.15";
const uint16_t mqtt_port = 8883;
const char* mqttUser = "bach";
const char* mqttPassword = "bach291020";

const char caCert[] PROGMEM = R"EOF(
  -----BEGIN CERTIFICATE-----
  MIID/zCCAuegAwIBAgIUJlLvrhM08LOEQF2GXGSgKhV7S7UwDQYJKoZIhvcNAQEL
  BQAwgY4xCzAJBgNVBAYTAlZOMQ4wDAYDVQQIDAVoYW5vaTEOMAwGA1UEBwwFaGFu
  b2kxFjAUBgNVBAoMDWNhY2VydGlmaWNhdGUxDTALBgNVBAsMBHRlc3QxFzAVBgNV
  BAMMDjE5Mi4xNjguMTAwLjE1MR8wHQYJKoZIhvcNAQkBFhB0ZXN0Y2FAZ21haWwu
  Y29tMB4XDTIxMDUxNjE3MTcxMFoXDTI2MDUxNjE3MTcxMFowgY4xCzAJBgNVBAYT
  AlZOMQ4wDAYDVQQIDAVoYW5vaTEOMAwGA1UEBwwFaGFub2kxFjAUBgNVBAoMDWNh
  Y2VydGlmaWNhdGUxDTALBgNVBAsMBHRlc3QxFzAVBgNVBAMMDjE5Mi4xNjguMTAw
  LjE1MR8wHQYJKoZIhvcNAQkBFhB0ZXN0Y2FAZ21haWwuY29tMIIBIjANBgkqhkiG
  9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1KqUMvOAZIYoIX6h2WSvbacge5ai2+dRxFkd
  zAgHRSehpJQOqOSVFV296H+IjMis5FlrFSEFvbEh96zqPryjgE8H+JVQCAWRMIpK
  ISxpJyYr3Lj4iN/s8Kx6loBQqwGOlngp84HIDHFin/HmKzFEfbIkBjokaCET4XEm
  zxPco9VFkiPRmwmOGEfxbHZaQy/0eEQLWxhseeAwdGwPy8L10XBxGWQyMPRGvKDR
  hXU8VDcREiZV7Qh5QOkzDRwEbKDAEjZaJRmDZNKhkGStCXeXByV/irC8gqOnO6TS
  AKeflCalYesGc1pKdbBE+VlP9iuZaIFcpwZ0cf7LTFYAPuQ0HQIDAQABo1MwUTAd
  BgNVHQ4EFgQU1k9h1dS/A7KzSIt/htJE9upSkHEwHwYDVR0jBBgwFoAU1k9h1dS/
  A7KzSIt/htJE9upSkHEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC
  AQEALpQ1r565zjtmR5kh+iu1U+SK4Bc9PCjDUZHF9BCK9E25smYmtC8SCdDCe7HN
  /WLtKOGEraYJflGA3l5N2E/cg6tZWwc1CFB1Kr1qVmoGs71zMH9dh6oNlLvIljjI
  EgG8kYDeOsQct4DEDZYrt+sbOH6mpHptPzXo2R95HnMo/u5WZ1xxQiCxTUpmosfh
  aSoMqrjCVQMuEwdUv6HXJR7iZFifxDpKpwzPXTs1YcBZGvHD3RzpULTVdLhNAYYb
  SFwg2Y6Yn1hoFT5VwACQS4k2g5+gjAO/xYcJuYu91URjePdzMJ/g/DLSFyjkbFEh
  WnkgnWgbLb6z2OztqD865AiDFw==
  -----END CERTIFICATE-----
)EOF";

const uint8_t serverCertFingerprint[] = {0x04,0x6F,0xDA,0x31,0xFD,0xD0,0xA3,0x24,0x1F,0x88,0x91,0xE5,0x4A,0x4A,0x10,0xA5,0xB2,0xDC,0x8C,0x6E};


X509List caCertX509(caCert);
WiFiClientSecure espClient;
PubSubClient client(espClient);

const byte ledPin = D1;
const int DHTPIN = 2;
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);

float temperature = 0.0;
float humididity = 0.0;

void setup_wifi(void);
void callback(char*, byte*, unsigned int);
void reconnect(void);

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  setup_wifi();

  //thiet lap server va ham callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  dht.begin(); //khoi chay DHT11 sensor
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.println();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.println(t);
  //publish du lieu nhiet do len topic
  client.publish("testtopic/dht/temperature", String(t).c_str());

  Serial.print("Humidity: ");
  Serial.println(h);
  //publish du lieu do am len topic
  client.publish("testtopic/dht/humidity", String(h).c_str());

  Serial.println();

  //publish trang thai led len topic
  if (digitalRead(ledPin) == LOW) {
    client.publish("testtopic/led/state", "OFF");
  }
  if (digitalRead(ledPin) == HIGH) {
    client.publish("testtopic/led/state", "ON");
  }
  delay(2000);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); //thiet lap che do lam viec la Station Mode
  WiFi.begin(ssid, pass); //ket noi voi wifi   

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("...");
    delay(500);
  }
  Serial.println();
  Serial.println("Wifi Connected");

  //xac nhan thuc hien bao mat ket noi bang SSL/TLS
  espClient.setTrustAnchors(&caCertX509); //thiet lap CA certificate
  espClient.allowSelfSignedCerts();
  espClient.setFingerprint(serverCertFingerprint); //thiet lap server certificate
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    Serial.println("OFF");
    digitalWrite(ledPin, LOW);  
  }

  if ((char)payload[0] == '0') {
    Serial.println("ON");
    digitalWrite(ledPin, HIGH);  
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    //ket noi voi broker su dung ten nguoi dung va mat khau xac nhan
    if (client.connect("ESP_client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.publish("testtopic/status", "esp is connected");

      client.subscribe("testtopic/led/switch"); //subscribe topic dung de dieu khien led

      if (dht.readHumidity() && dht.readTemperature()) {
        Serial.println();
        Serial.println("dht is working");
        client.publish("testtopic/status", "dht is working");
      }
      else {
        Serial.println("dht is not working");
      }
    } 
    else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(", try again in 5 seconds");
      delay(5000);
    }
  }
}