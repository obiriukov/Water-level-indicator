#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <VL53L0X.h>

// Function declarations
void controlLEDs(int level);
void offLeds();
void testLeds();
float readDistance();
void initSensor();

// VL53L0X сенсор
VL53L0X sensor;

// VL53L0X налаштування
const int MIN_TANK_HEIGHT = 10; // Мінімальна висота бака в мм
const int MAX_TANK_HEIGHT = 250; // Максимальна висота бака в мм
const int MEASUREMENT_INTERVAL = 1000; // Інтервал вимірювань в мс
int lastDistance = -1;
unsigned long lastMeasurement = 0;
bool fillingUp = false; // Чи заповнюється бак

// Сенсорна кнопка для перемикання режиму
const int TOUCH_BUTTON_PIN = D8; // Пін OUT сенсорної кнопки (підключити до D8)
// Підключення: VCC -> 3.3V, GND -> GND, OUT -> D8
bool lastTouchState = LOW;
bool touchState = LOW;
unsigned long lastTouchTime = 0;
const unsigned long touchDebounceDelay = 100; // Трохи більший час для сенсорної кнопки

// WiFi налаштування
// const char *ssid = "Watermelon";   // Змініть на вашу WiFi мережу
// const char *password = "44129347"; // Змініть на ваш WiFi пароль

// // MQTT налаштування
// const char *mqtt_server = "192.168.50.50"; // Змініть на адресу вашого MQTT сервера
// // const char *mqtt_topic = "waterpump/distance"; // Топік для отримання даних
// const char *mqtt_topic = "waterpump/distance"; // Топік для отримання даних
// const char *mqtt_client_id = "ESP8266_WaterLevel";
// const char *mqtt_user = "mqtt-user";     // Замініть на ваш MQTT користувач (якщо потрібно)
// const char *mqtt_password = "mqtt-user"; // Замініть на ваш MQTT пароль (якщо потрібно)

// Піни для 5-сегментного індикатора рівня
const int SEGMENT_1 = D1; // Сегмент 1 (найнижчий рівень)
const int SEGMENT_2 = D2; // Сегмент 2
const int SEGMENT_3 = D3; // Сегмент 3 (середній рівень)
const int SEGMENT_4 = D4; // Сегмент 4
const int SEGMENT_5 = D7; // Сегмент 5 (найвищий рівень)
const int MAX_SEGMENTS = 5; // Кількість сегментів

enum LedMode
{
  LED_WIFI_CONNECTING, // Мигає 1 раз в секунду
  LED_MQTT_CONNECTING, // Мигає 2 рази в секунду (500ms)
  LED_READY            // Горить постійно
};
LedMode currentLedMode = LED_WIFI_CONNECTING;

WiFiClient espClient;
PubSubClient client(espClient);

// void setup_wifi()
// {
//   delay(10);
//   Serial.println();
//   Serial.print("Connecting to WiFi ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password);

//   digitalWrite(SEGMENT_3, HIGH); // Показуємо підключення WiFi

//   while (WiFi.status() != WL_CONNECTED)
//   {
//     digitalWrite(SEGMENT_4, HIGH);
//     delay(100);
//     digitalWrite(SEGMENT_4, LOW);
//     delay(100);
//     Serial.print(".");
//   }

//   Serial.println();
//   Serial.println("WiFi connected!");
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());

//   digitalWrite(SEGMENT_5, HIGH); // Показуємо успішне підключення
// }

// void callback(char *topic, byte *payload, unsigned int length)
// {
//   String message = "";
//   for (unsigned int i = 0; i < length; i++)
//   {
//     message += (char)payload[i];
//   }

//   Serial.print("Received message: ");
//   Serial.println(message);

//   float waterLevel = message.toFloat();
//   controlLEDs(waterLevel);
// }

void testLeds()
{
  Serial.println("Testing 5-segment indicator...");

  offLeds();

  // Тест: послідовне включення сегментів
  for (int i = 1; i <= 5; i++)
  {
    Serial.print("Testing segment ");
    Serial.println(i);
    
    switch (i)
    {
    case 1:
      digitalWrite(SEGMENT_1, HIGH);
      break;
    case 2:
      digitalWrite(SEGMENT_2, HIGH);
      break;
    case 3:
      digitalWrite(SEGMENT_3, HIGH);
      break;
    case 4:
      digitalWrite(SEGMENT_4, HIGH);
      break;
    case 5:
      digitalWrite(SEGMENT_5, HIGH);
      break;
    }
    delay(50);
  }
  
  delay(500);
  
  // Тест: послідовне вимкнення
  for (int i = 5; i >= 1; i--)
  {
    switch (i)
    {
    case 1:
      digitalWrite(SEGMENT_1, LOW);
      break;
    case 2:
      digitalWrite(SEGMENT_2, LOW);
      break;
    case 3:
      digitalWrite(SEGMENT_3, LOW);
      break;
    case 4:
      digitalWrite(SEGMENT_4, LOW);
      break;
    case 5:
      digitalWrite(SEGMENT_5, LOW);
      break;
    }
    delay(50);
  }

  offLeds();
  delay(1000);
}

void offLeds()
{
  digitalWrite(SEGMENT_1, LOW);
  digitalWrite(SEGMENT_2, LOW);
  digitalWrite(SEGMENT_3, LOW);
  digitalWrite(SEGMENT_4, LOW);
  digitalWrite(SEGMENT_5, LOW);
}

void controlLEDs(int level)
{
  // Вимкнути всі сегменти
  offLeds();

  Serial.print("Water level: ");
  Serial.println(level);

  // Логіка включення сегментів
  int activeSegments = 0;

  if (level < MIN_TANK_HEIGHT)
    activeSegments = 0;
  else if (level >= MAX_TANK_HEIGHT)
    activeSegments = MAX_SEGMENTS;
  else
  {
    int levelRange = MAX_TANK_HEIGHT - MIN_TANK_HEIGHT;
    int segmentRange = levelRange / MAX_SEGMENTS;
    activeSegments = (int)((level - MIN_TANK_HEIGHT) / segmentRange) + 1;
    if (activeSegments > MAX_SEGMENTS) activeSegments = MAX_SEGMENTS;
  }

  Serial.println("Active segments: " + String(activeSegments));

  // Включаємо сегменти знизу вгору залежно від рівня
  switch (activeSegments)
  {
  case 0:
    Serial.println("Water level too low - no segments");
    break;
  case 1:
    digitalWrite(SEGMENT_1, HIGH);
    Serial.println("Segment 1 ON (Low level)");
    break;
  case 2:
    digitalWrite(SEGMENT_1, HIGH);
    digitalWrite(SEGMENT_2, HIGH);
    Serial.println("Segments 1-2 ON");
    break;
  case 3:
    digitalWrite(SEGMENT_1, HIGH);
    digitalWrite(SEGMENT_2, HIGH);
    digitalWrite(SEGMENT_3, HIGH);
    Serial.println("Segments 1-3 ON (Medium level)");
    break;
  case 4:
    digitalWrite(SEGMENT_1, HIGH);
    digitalWrite(SEGMENT_2, HIGH);
    digitalWrite(SEGMENT_3, HIGH);
    digitalWrite(SEGMENT_4, HIGH);
    Serial.println("Segments 1-4 ON (High level)");
    break;
  case 5:
    digitalWrite(SEGMENT_1, HIGH);
    digitalWrite(SEGMENT_2, HIGH);
    digitalWrite(SEGMENT_3, HIGH);
    digitalWrite(SEGMENT_4, HIGH);
    digitalWrite(SEGMENT_5, HIGH);
    Serial.println("ALL segments ON (Maximum level)");
    break;
  default:
    Serial.println("Value out of range");
    break;
  }
}

// void reconnect()
// {
//   while (!client.connected())
//   {
//     Serial.print("Connecting to MQTT...");
//     digitalWrite(SEGMENT_4, HIGH); // Індикація підключення MQTT
//     if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
//     {
//       Serial.println("connected!");
//       client.subscribe(mqtt_topic);
//       digitalWrite(SEGMENT_5, HIGH); // Успішне підключення
//     }
//     else
//     {
//       Serial.print("failed, error code = ");
//       Serial.print(client.state());
//       Serial.println(" trying again in 5 seconds");

//       digitalWrite(SEGMENT_4, LOW);

//       delay(5000);
//     }
//   }
// }

void initSensor()
{
  Serial.println("Initializing VL53L0X sensor...");
  
  // Ініціалізація I2C
  Wire.begin(D6, D5); // SDA=D6, SCL=D5
  
  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println(F("Failed to detect and initialize VL53L0X sensor!"));
    // Показати помилку через мигання всіх сегментів
    for (int i = 0; i < 10; i++) {
      digitalWrite(SEGMENT_1, HIGH);
      digitalWrite(SEGMENT_2, HIGH);
      digitalWrite(SEGMENT_3, HIGH);
      digitalWrite(SEGMENT_4, HIGH);
      digitalWrite(SEGMENT_5, HIGH);
      delay(200);
      offLeds();
      delay(200);
    }
    while(1); // Зупинити виконання
  }
  
  // Налаштування таймінгу для високої точності
  sensor.setMeasurementTimingBudget(200000); // 200ms
  
  Serial.println(F("VL53L0X sensor initialized successfully!"));
}

float readDistance()
{
  uint16_t rawDistance = sensor.readRangeSingleMillimeters();
  
  if (sensor.timeoutOccurred()) {
    Serial.println("Distance sensor: TIMEOUT");
    lastDistance = -1;
    return lastDistance;
  }

  if(fillingUp) {
    if(rawDistance < lastDistance) {
      lastDistance = rawDistance;
    }
  } else {
    if(rawDistance > lastDistance) {
      lastDistance = rawDistance;
    }
  }
  
  return lastDistance;
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("Water Level Indicator v1.0");
  Serial.println("==========================");
  Serial.println("ESP8266 started!");

  // Ініціалізація пінів 5-сегментного індикатора
  Serial.println("Initializing 5-segment indicator...");
  pinMode(SEGMENT_1, OUTPUT);
  pinMode(SEGMENT_2, OUTPUT);
  pinMode(SEGMENT_3, OUTPUT);
  pinMode(SEGMENT_4, OUTPUT);
  pinMode(SEGMENT_5, OUTPUT);

  // Вимкнути всі сегменти на початку
  digitalWrite(SEGMENT_1, LOW);
  digitalWrite(SEGMENT_2, LOW);
  digitalWrite(SEGMENT_3, LOW);
  digitalWrite(SEGMENT_4, LOW);
  digitalWrite(SEGMENT_5, LOW);
  Serial.println("5-segment indicator initialized!");

  // Ініціалізація сенсорної кнопки
  pinMode(TOUCH_BUTTON_PIN, INPUT);
  Serial.println("Touch button initialized!");

  // Тест 5-сегментного індикатора
  testLeds();

  // Ініціалізація VL53L0X сенсора
  initSensor();

  Serial.println("Water Level Indicator with VL53L0X ready!");
}

void loop()
{
  // Читання стану сенсорної кнопки
  int reading = digitalRead(TOUCH_BUTTON_PIN);
  
  // Обробка сенсорної кнопки (реагуємо на HIGH сигнал)
  if (reading == HIGH && lastTouchState == LOW && (millis() - lastTouchTime > touchDebounceDelay)) {
    fillingUp = !fillingUp;
    Serial.print("Touch button pressed! fillingUp mode changed to: ");
    Serial.println(fillingUp ? "FILLING" : "DRAINING");
    
    // Візуальна індикація перемикання режиму
    for (int i = 0; i < 3; i++) {
      digitalWrite(SEGMENT_3, HIGH);
      delay(100);
      digitalWrite(SEGMENT_3, LOW);
      delay(100);
    }
    
    lastTouchTime = millis();
  }
  
  lastTouchState = reading;
    
  // Періодичне вимірювання відстані
  if (millis() - lastMeasurement > MEASUREMENT_INTERVAL) {
    int distance = readDistance();
    Serial.print("Final distance: ");
    Serial.print(distance);
    Serial.println(" mm");

    if(distance == -1) {
      Serial.println("Invalid distance reading, skipping...");
      if(fillingUp) {
        fillingUp = false;
        Serial.println("Switching to draining mode.");
      }

      return;
    } else {
      float waterLevel = MAX_TANK_HEIGHT - distance; // Рівень води в мм
      controlLEDs(waterLevel);
    }
    
    lastMeasurement = millis();
  }
  
  // Підтримка MQTT з'єднання (опціонально)
  // if (!client.connected()) {
  //   // Можна залишити для дистанційного моніторингу
  //   // reconnect();
  // }
  // client.loop();
}