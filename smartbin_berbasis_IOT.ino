#define BLYNK_TEMPLATE_ID "----------" //           [your blynk template id / Erase this comment !]
#define BLYNK_TEMPLATE_NAME "---------" //          [your blynk template name /Erase this comment !]
#define BLYNK_AUTH_TOKEN "-------------------" //   [your blynk auth token / Erase this comment !]
#include <NewPing.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

#define TRIGGER_PIN  12  // Pin trigger sensor ultrasonik
#define ECHO_PIN     13  // Pin echo sensor ultrasonik
#define SERVO_PIN    15  // Pin servo motor
#define TRIGGER_PIN_2  18  // Pin trigger sensor ultrasonik kedua
#define ECHO_PIN_2     19  // Pin echo sensor ultrasonik kedua

NewPing sonar(TRIGGER_PIN, ECHO_PIN);
NewPing sonar2(TRIGGER_PIN_2, ECHO_PIN_2);

Servo myservo;  // Objek untuk mengontrol servo

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "-----"; //    [your ssid name Erase / this comment !]
char pass[] = "------"; //   [your wifi password / Erase this comment !]

LiquidCrystal_I2C lcd(0x27, 16, 2);

int servoPosition = 0;

#define buzzerPin 2 // Ganti dengan pin yang Anda gunakan untuk menghubungkan pin SIG buzzer
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// inisialisasi Bot Token
#define BOTtoken "-----------I"  // [Bot Token from Botfather / Erase this comment !]

// chat id dari @myidbot
#define CHAT_ID "-----" [chat id from myidbot / Erase this comment !]

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 100;
unsigned long lastTimeBotRan;

#include <TinyGPS++.h>
#include <HardwareSerial.h>

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;
void sendTelegramMessage(String message) {
  bot.sendMessage(CHAT_ID, message, "");
}
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    while(GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }
   if (gps.charsProcessed() > 10) {
    float currentLat = gps.location.lat();
    float currentLng = gps.location.lng();

    if (text == "/start") {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan Commands Di Bawah Untuk Monitoring Lokasi GPS\n\n";
      control += "/Lokasi Untuk Mengetahui lokasi saat ini \n";
      bot.sendMessage(chat_id, control, "");
    }

      
  if (text == "/Lokasi"){
     String lokasi = "Lokasi : https://www.google.com/maps/@";
      lokasi +=String(currentLat,6);
      lokasi +=",";
      lokasi +=String(currentLng,6);
      lokasi +=",21z?entry=ttu";
      bot.sendMessage(chat_id, lokasi, "");
  }  
 
   }
 
  }
}
void setup() {
  GPSSerial.begin(115200, SERIAL_8N1, 16, 17);
  // Koneksi Ke Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  pinMode(buzzerPin, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  myservo.attach(SERVO_PIN);
  Blynk.begin(auth, ssid, pass, "blynk.cloud");
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("SMARTBIN ACTIVE!");
  buzzerOn(1); // Bunyikan buzzer 1 ketukan panjang ketika layar menampilkan tulisan "SMARTBIN ACTIVE!"
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("HELLO THERE ^_^");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMARTBIN READY !");
  lcd.setCursor(0, 1);

  myservo.write(servoPosition);
}

unsigned long lastOpenTime = 0;
bool lcdTextDisplayed = false;
bool autoOpen = false;

void loop() {
  Blynk.run();
  delay(0);
if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  int distance1 = sonar.ping_cm();
  int distance2 = sonar2.ping_cm();
  int persentase = 0;

  if (distance1 > 0 && distance1 < 10) {
    int konversi = map(180, 0, 180, 0, 100);
    Blynk.virtualWrite(V0, konversi);
    Blynk.virtualWrite(V1, konversi);
    myservo.write(180);
    autoOpen = true;
    lastOpenTime = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smartbin Open !");
    lcd.setCursor(0, 1);
    lcd.print("");
    lcdTextDisplayed = true;
    buzzerOn(2);
  } else {
    if (millis() - lastOpenTime >= 4500 && !autoOpen && servoPosition == 0) {
      myservo.write(0);
      lcdTextDisplayed = false;
      Blynk.virtualWrite(V0, 0);
      Blynk.virtualWrite(V1, 0);
      buzzerOff();
    }
  }

  if (!autoOpen) {
    if (distance2 > 0 && distance2 <= 3) {
      persentase = 100;
    } else if (distance2 > 1 && distance2 <= 12) {
      persentase = 50;
    } else if (distance2 > 12 && distance2 <= 20) {
      persentase = 10;
    }

    if (persentase == 100) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SMARTBIN FULL !");
      lcd.setCursor(0, 1);
      lcd.print("Persentase: ");
      lcd.print(persentase);
      lcd.print("%");
      buzzerOn(3);
      sendTelegramMessage("Smartbin telah mencapai kapasitas penuh!. Ketik /start untuk mengetahui lokasi smartbin");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SMARTBIN READY !");
      lcd.setCursor(0, 1);
      lcd.print("Persentase: ");
      lcd.print(persentase);
      lcd.print("%");
      buzzerOff();
    }

    Blynk.virtualWrite(V3, persentase);
  }

  if (autoOpen && millis() - lastOpenTime >= 4500) {
    autoOpen = false;
    myservo.write(0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smartbin Closed!");
    lcd.setCursor(0, 1);
    lcd.print("");
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);
    buzzerOn(2);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMARTBIN READY !");
    lcd.setCursor(0, 1);
    buzzerOff();

    if (distance2 < 0 && distance2 <= 3) {
      persentase = 100;
    } else if (distance2 > 1 && distance2 <= 12) {
      persentase = 50;
    } else if (distance2 > 12 && distance2 <= 20) {
      persentase = 10;
    }

    if (persentase == 100) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SMARTBIN FULL !");
      lcd.setCursor(0, 1);
      lcd.print("Persentase: ");
      lcd.print(persentase);
      lcd.print("%");
      buzzerOn(3);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SMARTBIN READY !");
      lcd.setCursor(0, 1);
      lcd.print("Persentase: ");
      lcd.print(persentase);
      lcd.print("%");
      buzzerOff();
    }

    Blynk.virtualWrite(V3, persentase);
  }
}

void buzzerOn(int numOfBeeps) {
  for (int i = 0; i < numOfBeeps; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
}

void buzzerOff() {
  digitalWrite(buzzerPin, LOW);
}

BLYNK_WRITE(V0)
{
  int value = param.asInt();
  int konversi = map(value, 0, 100, 0, 180);
  myservo.write(konversi);
  servoPosition = konversi;
  int persentase = map(value, 0, 100, 0, 100);
  Blynk.virtualWrite(V1, persentase);
}

BLYNK_WRITE(V1)
{
  int value = param.asInt();
  int konversi = map(value, 0, 100, 0, 180);
  myservo.write(konversi);
  Blynk.virtualWrite(V0, konversi);
}

BLYNK_WRITE(V3)
{
  int value = param.asInt();  
}
BLYNK_CONNECTED() {
  Blynk.syncAll();
}
