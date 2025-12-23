#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Function declarations
void controlLEDs(float level);
void offLeds();
void testLeds();

// WiFi налаштування
const char *ssid = "Watermelon";   // Змініть на вашу WiFi мережу
const char *password = "44129347"; // Змініть на ваш WiFi пароль

// MQTT налаштування
const char *mqtt_server = "192.168.50.50";     // Змініть на адресу вашого MQTT сервера
// const char *mqtt_topic = "waterpump/distance"; // Топік для отримання даних
const char *mqtt_topic = "waterpump/distance"; // Топік для отримання даних
const char *mqtt_client_id = "ESP8266_WaterLevel";
const char *mqtt_user = "mqtt-user";     // Замініть на ваш MQTT користувач (якщо потрібно)
const char *mqtt_password = "mqtt-user"; // Замініть на ваш MQTT пароль (якщо потрібно)

// Піни для світлодіодів
const int LED_GREEN_1 = D3; // Зелений LED 1 (2-5)
const int LED_GREEN_2 = D4; // Зелений LED 2 (5-10)
const int LED_BLUE_1 = D5;  // Синій LED 1 (10-14)
const int LED_BLUE_2 = D6;  // Синій LED 2 (14-16)
const int LED_RED_1 = D7;   // Червоний LED 1 (16-20)
const int LED_RED_2 = D8;   // Червоний LED 2 (20-22)

enum LedMode
{
  LED_WIFI_CONNECTING, // Мигає 1 раз в секунду
  LED_MQTT_CONNECTING, // Мигає 2 рази в секунду (500ms)
  LED_READY            // Горить постійно
};
LedMode currentLedMode = LED_WIFI_CONNECTING;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  digitalWrite(LED_BLUE_1, HIGH);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BLUE_2, HIGH);
    delay(100);
    digitalWrite(LED_BLUE_2, LOW);
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(LED_BLUE_2, HIGH);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String message = "";
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.print("Received message: ");
  Serial.println(message);

  float waterLevel = message.toFloat();
  controlLEDs(waterLevel);
}

void testLeds()
{
  Serial.println("Testing LEDs...");

  offLeds();

  for (int i = 0; i < 18; i++)
  {
    switch (i)
    {
    case 0:
      digitalWrite(LED_GREEN_1, HIGH);
      break;
    case 1:
      digitalWrite(LED_GREEN_2, HIGH);
      break;
    case 2:
      digitalWrite(LED_BLUE_1, HIGH);
      break;
    case 3:
      digitalWrite(LED_BLUE_2, HIGH);
      break;
    case 4:
      digitalWrite(LED_RED_1, HIGH);
      break;
    case 5:
      digitalWrite(LED_RED_2, HIGH);
      break;
    case 6:
      digitalWrite(LED_GREEN_1, LOW);
      break;
    case 7:
      digitalWrite(LED_GREEN_2, LOW);
      break;
    case 8:
      digitalWrite(LED_BLUE_1, LOW);
      break;
    case 9:
      digitalWrite(LED_BLUE_2, LOW);
      break;
    case 10:
      digitalWrite(LED_RED_1, LOW);
      break;
    case 11:
      digitalWrite(LED_RED_2, LOW);
      break;
    case 12:
      digitalWrite(LED_RED_2, HIGH);
      break;
    case 13:
      digitalWrite(LED_RED_1, HIGH);
      break;
    case 14:
      digitalWrite(LED_BLUE_2, HIGH);
      break;
    case 15:
      digitalWrite(LED_BLUE_1, HIGH);
      break;
    case 16:
      digitalWrite(LED_GREEN_2, HIGH);
      break;
    case 17:
      digitalWrite(LED_GREEN_1, HIGH);
      break;
    default:
      break;
    }

    delay(100);
  }

  offLeds();
  delay(1000);
}

void offLeds()
{
  digitalWrite(LED_GREEN_1, LOW);
  digitalWrite(LED_GREEN_2, LOW);
  digitalWrite(LED_BLUE_1, LOW);
  digitalWrite(LED_BLUE_2, LOW);
  digitalWrite(LED_RED_1, LOW);
  digitalWrite(LED_RED_2, LOW);
}

void controlLEDs(float level)
{
  // Вимкнути всі світлодіоди
  offLeds();

  Serial.print("Water level: ");
  Serial.println(level);

  // Логіка включення світлодіодів
  int minLevel = 2;
  int maxLevel = 19+1;
  int maxRange = 6;
  int levelRange = 0;

  if(level < minLevel)
    levelRange = 0;
  else if(level >= maxLevel)
    levelRange = maxRange;
  else
  {
    int coeficient = (maxLevel - minLevel) / maxRange;
    levelRange = (int)((level - minLevel) / coeficient) + 1;
  }
  
  Serial.println("Level Range: " + String(levelRange));

  switch (levelRange)
  {
    case 0:
    Serial.println("Water level below 2");
    offLeds();
    break;
  case 1:
    digitalWrite(LED_GREEN_1, HIGH);
    digitalWrite(LED_GREEN_2, HIGH);
    digitalWrite(LED_BLUE_1, HIGH);
    digitalWrite(LED_BLUE_2, HIGH);
    digitalWrite(LED_RED_1, HIGH);
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Green LED 1 ON");
    break;
  case 2:
    digitalWrite(LED_GREEN_2, HIGH);
    digitalWrite(LED_BLUE_1, HIGH);
    digitalWrite(LED_BLUE_2, HIGH);
    digitalWrite(LED_RED_1, HIGH);
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Green LED 2 ON");
    break;
  case 3:
    digitalWrite(LED_BLUE_1, HIGH);
    digitalWrite(LED_BLUE_2, HIGH);
    digitalWrite(LED_RED_1, HIGH);
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Blue LED 1 ON");
    break;
  case 4:
    digitalWrite(LED_BLUE_2, HIGH);
    digitalWrite(LED_RED_1, HIGH);
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Blue LED 2 ON");
    break;
  case 5:
    digitalWrite(LED_RED_1, HIGH);
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Red LED 1 ON");
    break;
  case 6:
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Red LED 2 ON");
    break;
  default:
    Serial.println("Value out of range (2-22)");
    offLeds();
    break;
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT...");
    digitalWrite(LED_RED_1, HIGH);
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
    {
      Serial.println("connected!");
      client.subscribe(mqtt_topic);
      digitalWrite(LED_RED_2, HIGH);
    }
    else
    {
      Serial.print("failed, error code = ");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");

      digitalWrite(LED_RED_1, LOW);

      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("Water Level Indicator v1.0");
  Serial.println("==========================");
  Serial.println("ESP8266 started!");

  // Ініціалізація пінів світлодіодів
  Serial.println("Initializing LEDs...");
  pinMode(LED_GREEN_1, OUTPUT);
  pinMode(LED_GREEN_2, OUTPUT);
  pinMode(LED_BLUE_1, OUTPUT);
  pinMode(LED_BLUE_2, OUTPUT);
  pinMode(LED_RED_1, OUTPUT);
  pinMode(LED_RED_2, OUTPUT);

  // Вимкнути всі світлодіоди на початку
  digitalWrite(LED_GREEN_1, LOW);
  digitalWrite(LED_GREEN_2, LOW);
  digitalWrite(LED_BLUE_1, LOW);
  digitalWrite(LED_BLUE_2, LOW);
  digitalWrite(LED_RED_1, LOW);
  digitalWrite(LED_RED_2, LOW);
  Serial.println("LEDs initialized!");

  // Тест світлодіодів
  testLeds();

  digitalWrite(LED_GREEN_1, HIGH);
  digitalWrite(LED_GREEN_2, HIGH);

  Serial.println("Attempting WiFi connection...");
  setup_wifi();

  Serial.println("Setting up MQTT...");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Water Level Indicator ready!");
}

void loop()
{
  if (!client.connected())
  {
    Serial.println("MQTT disconnected, attempting reconnection...");
    reconnect();
  }
  client.loop();
}