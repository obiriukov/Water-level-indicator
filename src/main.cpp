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
  Serial.print("Підключення до WiFi ");
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
  Serial.println("WiFi підключено!");
  Serial.print("IP адреса: ");
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

  Serial.print("Отримано повідомлення: ");
  Serial.println(message);

  float waterLevel = message.toFloat();
  controlLEDs(waterLevel);
}

void testLeds()
{
  Serial.println("Тест світлодіодів...");
  // Послідовно включити всі світлодіоди для тестування
  const float testLevels[] = {2, 5, 10, 14, 16, 20};
  const int numLevels = sizeof(testLevels) / sizeof(testLevels[0]);

  for (int i = 0; i < numLevels; i++)
  {
    controlLEDs(testLevels[i]);
    delay(100);
  }

  // Показати всі світлодіоди одночасно на 1 секунду
  digitalWrite(LED_GREEN_1, HIGH);
  digitalWrite(LED_GREEN_2, HIGH);
  digitalWrite(LED_BLUE_1, HIGH);
  digitalWrite(LED_BLUE_2, HIGH);
  digitalWrite(LED_RED_1, HIGH);
  digitalWrite(LED_RED_2, HIGH);
  delay(500);
  offLeds();
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

  Serial.print("Рівень води: ");
  Serial.println(level);

  // Логіка включення світлодіодів
  if (level >= 2 && level < 5)
  {
    digitalWrite(LED_GREEN_1, HIGH);
    Serial.println("Зелений LED 1 включено");
  }
  else if (level >= 5 && level < 10)
  {
    digitalWrite(LED_GREEN_2, HIGH);
    Serial.println("Зелений LED 2 включено");
  }
  else if (level >= 10 && level < 14)
  {
    digitalWrite(LED_BLUE_1, HIGH);
    Serial.println("Синій LED 1 включено");
  }
  else if (level >= 14 && level < 16)
  {
    digitalWrite(LED_BLUE_2, HIGH);
    Serial.println("Синій LED 2 включено");
  }
  else if (level >= 16 && level < 20)
  {
    digitalWrite(LED_RED_1, HIGH);
    Serial.println("Червоний LED 1 включено");
  }
  else if (level >= 20 && level <= 22)
  {
    digitalWrite(LED_RED_2, HIGH);
    Serial.println("Червоний LED 2 включено");
  }
  else
  {
    Serial.println("Значення поза діапазоном (2-22)");
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Підключення до MQTT...");
    digitalWrite(LED_RED_1, HIGH);
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
    {
      Serial.println("підключено!");
      client.subscribe(mqtt_topic);
      digitalWrite(LED_RED_2, HIGH);
    }
    else
    {
      Serial.print("невдача, код помилки = ");
      Serial.print(client.state());
      Serial.println(" спробую знову через 5 секунд");

      digitalWrite(LED_RED_1, LOW);

      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  Serial.println("\n\n=== ПОЧАТОК ПРОГРАМИ ===");
  Serial.println("Water Level Indicator v1.0");
  Serial.println("==========================");
  Serial.println("ESP8266 запущений!");

  // Ініціалізація пінів світлодіодів
  Serial.println("Ініціалізація світлодіодів...");
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
  Serial.println("Світлодіоди ініціалізовані!");

  // Тест світлодіодів
  testLeds();

  digitalWrite(LED_GREEN_1, HIGH);
  digitalWrite(LED_GREEN_2, HIGH);

  Serial.println("Спроба підключення до WiFi...");
  setup_wifi();

  Serial.println("Налаштування MQTT...");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Water Level Indicator готовий до роботи!");
}

void loop()
{
  if (!client.connected())
  {
    Serial.println("MQTT відключений, спроба перепідключення...");
    reconnect();
  }
  client.loop();
}