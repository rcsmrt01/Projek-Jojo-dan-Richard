#include "jamTest.h"
#include "RTClib.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <LCD_I2C.h>
#include <MQUnifiedsensor.h>
#include <NTPClient.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <cstring>
#include <nvs_flash.h>
#include <stdio.h>

// GMT+7 Waktu Indonesia Barat
#define UTC_OFFSET_IN_SECONDS 25200
#define CheckIntervalOneMin 1 * 60 * 1000 // 1 menit x 60 detik x 1000 ms
#define CheckIntervalOneSec 1000
#define CheckIntervalAmonia 10000

// Define EEPROM keys and namespaces
#define MODE_NAMESPACE "modeConfig"
#define MODE_KEY_DAY_BASED "dayBased"
#define MODE_KEY_DYNAMIC "dynamic"
#define DYNAMIC_SETPOINT_KEY "dynSetpoint"
#define LAST_DAY_KEY "lastDay"
#define START_DATE_KEY "startDate" // Key for storing start date

// pin lcd
#define SDA_2 19
#define SCL_2 18
extern TwoWire Wire1;
LCD_I2C lcd(Wire1, 0x27, 16, 2);

// konfigurasi PIN untuk dht-11
#define pinDHT1 17 // dht titik A
#define DHTTYPE DHT11
DHT dht(pinDHT1, DHTTYPE);
float suhu;
float hKalVal = 8.39;
float tKalVal = 1.23;

#define pin_buzzer 23      // pin buzzer
uint32_t start_buzzer = 0; // millis buzzer

// Struct for day ranges and temperature setpoints
struct DayRange {
  int startDay;    // Starting day of the range
  int endDay;      // Ending day of the range
  int temperature; // Temperature setpoint in °C
};

// Temperature setpoints based on day ranges
const DayRange temperatureRangesDefault[] = {
    {1, 1, 33},   // Day 1
    {2, 2, 32},   // Day 2    {3, 3, 31},   // Day 3
    {4, 5, 30},   // Days 4-5
    {6, 8, 29},   // Days 6-8
    {9, 11, 28},  // Days 9-11
    {12, 17, 27}, // Days 12-17
    {18, 20, 26}, // Days 18-20
    {21, 23, 25}, // Days 21-23
    {24, 26, 24}, // Days 24-26
    {27, 29, 23}, // Days 27-29
    {30, 31, 22}, // Days 30-31
    {32, 34, 21}  // day 32-34
};

// preferences satu periode pemeliharaan ayam
void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length);
void setStartDate(int year, int month, int day);
int getTemperatureForDay(int day, const DayRange *ranges, int totalRanges);
int calculateDayNumber(DateTime currentDate);
bool isValidDate(int year, int month, int day);
void writeRangesToPreferences();
String getStartDate();
Preferences preferences;

// interval pembacaan dan pengiriman suhu dan kelembapan
// dilakukan setiap 10 menit
unsigned long previousSuhuMillis = 0;
const long intervalSuhu = 60000;

const int TOTAL_RANGES =
    sizeof(temperatureRangesDefault) / sizeof(temperatureRangesDefault[0]);

// Define temperature setpoint limits
const int MIN_SETPOINT = 16; // Minimum temperature setpoint in °C
const int MAX_SETPOINT = 40; // Maximum temperature setpoint in °C

// pin kipas kandang
int relay_pin_1 = 14;
int relay_pin_2 = 27;
int relay_pin_3 = 26;
// pin kipas amoniak
int relay_pin_4 = 25;
// pin pemanas dan lampu kandang
int relay_pin_5 = 33;
int relay_pin_6 = 32;

// rentang waktu pembacaan ppm amoniak
unsigned long currentMillis = millis();
unsigned long previousAmoniakMillis = 0;
const long intervalAmoniak = 30000;

// logika penyalaan kipas amoniak untuk tambahan pendinginan ruangan
// apabila selisih suhu bernilai 7°C - ke atas
bool penyalaan_kipas_tambahan_dari_amoniak = false;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// deklarasi rtcds3231
RTC_DS3231 rtc;

unsigned long tempo, prev_period_one_sec = 0, prev_period_amonia = 0;
const uint16_t jedacekrelay = 250;

// password wifi 1
// const char *ssid = "JONSIM 81";
// const char *password = "Simarmata_71";

// password wifi 2
const char *ssid = "Galaxy A54 5G B313";
const char *password = "2rj8xuwuh4z2u8y";

WiFiUDP ntpUDP;

// setingann untuk amoniak
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 34                // Analog input 0 of your esp32
#define tipe "MQ-135"         // MQ135
#define ADC_Bit_Resolution 12 // Resolusi ADC untuk esp32 dari library
#define RatioMQ135CleanAir 3.6
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, tipe);

// logika default relay kipas kandang
bool relay_1 = false;
bool relay_2 = false;
bool relay_3 = false;

// logika default relay kipas amoniak
bool relay_4 = false;

// logika default relay lampu kandang dan lampu pemanas
bool relay_5 = false;
bool relay_6 = false;

// flag untuk kontrol manual relay
bool kontrolManual1 = false;
bool kontrolManual2 = false;
bool kontrolManual3 = false;
bool kontrolManual4 = false;
bool kontrolManual5 = false;
bool kontrolManual6 = false;

// variabel penampung timer lampu kandang
byte parjam_hidupkandang = 0;
byte parmenit_hidupkandang = 0;
byte parjam_matikandang = 0;
byte parmenit_matikandang = 0;

// variabel penampung timer lampu pemanas
byte parjam_hiduppemanas = 0;
byte parmenit_hiduppemanas = 0;
byte parjam_matipemanas = 0;
byte parmenit_matipemanas = 0;

// variabel penampung status relay untuk database
String namaKipas;
String idKipas;
String statusMatchKipas;

// variabel untuk pemanas otomatis
bool autoControlPemanasDariSuhu = false;

// penulisan status penyalaan relay di database
bool statusRelaySuhu = false;

unsigned char hour;
unsigned char minute;
unsigned char detik;

byte jamTHidupPemanas, jamTHidupKandang;
byte menitTHidupPemanas, menitTHidupKandang;
byte jamTMatiPemanas, jamTMatiKandang;
byte menitTMatiPemanas, menitTMatiKandang;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", UTC_OFFSET_IN_SECONDS);

// port http
int portServer = 80;
WiFiClient wifiClient;

// millis untuk ngitung jeda berapa lama
// jika kita tidak mengontrol relay secara manual
// maka sistem akan beralih fungsi ke mode otomatis
unsigned long hitungKontrolRManualMillis1 = 0;
unsigned long manualControlTimeout1 =
    10000; // durasi berapa lama jika tombol websocket tidak dioperasikan maka
           // sistem memasuki mode otomatis
unsigned long hitungKontrolRManualMillis2 = 0;
unsigned long manualControlTimeout2 = 10000;

unsigned long hitungKontrolRManualMillis3 = 0;
unsigned long manualControlTimeout3 = 10000;

unsigned long hitungKontrolRManualMillis4 = 0;
unsigned long manualControlTimeout4 = 10000;

unsigned long hitungKontrolRManualMillis5 = 0;
unsigned long manualControlTimeout5 = 10000;

unsigned long hitungKontrolRManualMillis6 = 0;
unsigned long manualControlTimeout6 = 10000;

StaticJsonDocument<500> doc_sender;
StaticJsonDocument<500> doc_receiver;

// status penyalaan relay yang akan dikirim ke database
unsigned char relid;
String relayName;
String relayVal;
const char *relay;
bool status;

int daytemp_setpoint = 33;
int currentDayNumber;

bool isValidDate(int year, int month, int day) {
  if (month < 1 || month > 12)
    return false;
  if (day < 1)
    return false;
  int daysInMonth;
  switch (month) {
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    daysInMonth = 31;
    break;
  case 4:
  case 6:
  case 9:
  case 11:
    daysInMonth = 30;
    break;
  case 2:
    // Check for leap year
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
      daysInMonth = 29;
    else
      daysInMonth = 28;
    break;
  default:
    return false;
  }
  return day <= daysInMonth;
}

// Function to validate day ranges (no overlaps)
bool validateDayRanges(const DayRange *ranges, int totalRanges) {
  for (int i = 0; i < totalRanges; i++) {
    for (int j = i + 1; j < totalRanges; j++) {
      // Check for overlapping ranges
      if (!(ranges[i].endDay < ranges[j].startDay ||
            ranges[j].endDay < ranges[i].startDay)) {
        Serial.printf("Overlap detected between range %d-%d and range %d-%d\n",
                      ranges[i].startDay, ranges[i].endDay, ranges[j].startDay,
                      ranges[j].endDay);
        return false;
      }
    }
  }
  return true;
}

// Function to read temperature ranges from EEPROM
bool readRangesFromPreferences(DayRange *loadedRanges, int &loadedTotalRanges) {
  preferences.begin("tempConfig", true); // Open in read-only mode
  loadedTotalRanges = 0;

  for (int i = 0; i < TOTAL_RANGES; i++) {
    String key = "range" + String(i);
    size_t bytesAvailable = preferences.getBytesLength(key.c_str());

    if (bytesAvailable != sizeof(DayRange)) {
      Serial.printf("Incomplete or missing data for %s\n", key.c_str());
      continue;
    }

    preferences.getBytes(key.c_str(), &loadedRanges[loadedTotalRanges],
                         sizeof(DayRange));

    // Validate the range
    if (loadedRanges[loadedTotalRanges].startDay >
            loadedRanges[loadedTotalRanges].endDay ||
        loadedRanges[loadedTotalRanges].temperature < MIN_SETPOINT ||
        loadedRanges[loadedTotalRanges].temperature > MAX_SETPOINT) {
      Serial.printf("Invalid data in %s\n", key.c_str());
      continue;
    }

    loadedTotalRanges++;
  }

  preferences.end(); // Close the namespace

  if (loadedTotalRanges > 0) {
    Serial.println("Temperature ranges read from Preferences successfully.");
    return true;
  } else {
    Serial.println("No valid temperature ranges found in Preferences.");
    return false;
  }
}

template <typename T> void sendJson(const String &l_type, T l_value) {
  String jsonString = "";
  doc_sender.clear();
  doc_sender["type"] = l_type;
  doc_sender["value"] = l_value;
  serializeJson(doc_sender, jsonString);
  webSocket.broadcastTXT(jsonString);
  Serial.println("Broadcasting: " + jsonString);
}

void toggleRelaydanChecking(int &relayPin, bool &status,
                            bool &manualControlFlag) {
  if (!manualControlFlag) {

    digitalWrite(relayPin, status ? LOW : HIGH);
    Serial.print("Relay pada pin ");
    Serial.print(relayPin);
    Serial.println(status ? "dinyalakan." : "dimatikan");
  } else {
    Serial.println("Relay dalam kontrol manual, fungsi automasi dimatikan");
  }
}

void simpanSuhukeEEPROM(float suhu) {
  preferences.begin("TEMP", false);
  preferences.putFloat("DT0", suhu);
  preferences.end();
  Serial.println("Suhu disimpan ke EEPROM: " + String(suhu));
}

float bacaSuhuDariEEPROM() {
  float suhuEEPROM; // variabel untuk menyimpan nilai suhu yang akan dibaca dari
                    // eeprom
  preferences.begin("TEMP", true);
  suhuEEPROM = preferences.getFloat("DT0");
  return suhuEEPROM;
}

bool send_to_database = 0;
bool temphum_insert = 0;
bool tCh = 0; // logika pergantian mode waktu untuk kalibrasi kelembapan
void bacaSuhudanBandingkandenganEEPROM() {
  DateTime now = rtc.now();
  byte dhour_time = now.hour();
  byte dmin_time = now.minute();
  byte h_change = 18;
  // byte m_change=0;
  float tempKal;
  float humKal;
  float kelembapandht1;
  float suhudht1;
  float suhuTersimpan;
  float totalSuhu;
  float tempFix;
  float humFix;

  unsigned long currentMillis = millis();
  if (currentMillis - previousSuhuMillis >= intervalSuhu) {
    previousSuhuMillis = currentMillis;
    suhuTersimpan = bacaSuhuDariEEPROM();

    if (dhour_time >= h_change) {
      tCh = 1; // mode malam kalibrasi aktif
    } else {
      tCh = 0;
    }

    if (tCh) {
      tempKal = dht.readTemperature() - tKalVal;
      humKal = dht.readHumidity() - hKalVal;
      Serial.print("Suhu telah dikalibrasi!");
      Serial.print("Kelembapan telah dikalibrasi!");
      totalSuhu = tempKal;
      tempFix = tempKal;
      humFix = humKal;
      // tampilkan nilai kelembapan di lcd
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(tempKal);
      lcd.setCursor(9, 0);
      lcd.print("H:");
      lcd.print(humKal);
      send_to_database = true;
    } else {
      suhudht1 = dht.readTemperature();
      kelembapandht1 = dht.readHumidity();
      Serial.print("Suhu tanpa dikalibrasi");
      Serial.print("Kelembapan tanpa dikalibrasi");
      totalSuhu = suhudht1;
      tempFix = suhudht1;
      humFix = kelembapandht1;
      // tampilkan nilai suhu di lcd
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(suhudht1);
      lcd.setCursor(9, 0);
      lcd.print("H:");
      lcd.print(kelembapandht1);
      send_to_database = true;
    }

    Serial.print("Suhu yang diukur di titik A: ");
    Serial.println(suhudht1);
    Serial.print("Kelembapan Siang di titik A:");
    Serial.println(kelembapandht1);
    Serial.print("Kelembapan Malam di titik A:");
    Serial.println(humKal);

    float rataRataSuhu = totalSuhu;
    Serial.print("Rata-rata Suhu: ");
    Serial.println(rataRataSuhu);

    String format_suhu = String(tempFix, 2);
    sendJson<String>("suhu1", format_suhu);
    sendJson<float>("kelembapan1", humFix);

    int selisih;
    if (suhuTersimpan > 0) {
      selisih = rataRataSuhu - suhuTersimpan;
    } else {
      selisih = rataRataSuhu - daytemp_setpoint;
    }

    lcd.setCursor(9, 1);
    lcd.print("D:");
    lcd.print(selisih);

    Serial.println("Selisih suhu adalah: " + selisih);
    sendJson<int>("selisih_suhu", selisih);
    logikaRelaydariSuhudanKirimkeDatabase(selisih);

    // Membuat string data POST
    String postDataDHT11 = "tempValue1=" + String(tempFix) +
                           "&humValue1=" + String(humFix) +
                           "&temp_store=" + String(suhuTersimpan) +
                           "&temp_diff=" + String(selisih);

    sendJson<int>("suhuTersimpan", suhuTersimpan);

    if (send_to_database && !temphum_insert) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        http.begin("http://192.168.19.201/database_monitoring_kandang/"
                   "monitoring_suhu_kelembapan.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        int httpcode = http.POST(postDataDHT11);
        if (httpcode > 0) {
          Serial.print("httpResponseCode: ");
          Serial.println(httpcode);
          String payload = http.getString();
          Serial.println(payload);
        } else {
          Serial.print("errHttpResponsecode: ");
          Serial.println(httpcode);
        }
        http.end();
        temphum_insert = true;
      } else {
        Serial.println("Wifi Disconnected...");
      }
    }
  }
}

void turnon_buzzer() {
  start_buzzer = millis();
  digitalWrite(pin_buzzer, HIGH);
}

void logikaRelaydariSuhudanKirimkeDatabase(int selisih) {
  // logika pengiriman ke database
  bool timeMatchKipas = false;
  // json untuk kirim data banyak relay sekaligus
  StaticJsonDocument<512> doc_auto_relay;

  auto updateRelay = [&](int relayPin, const char *idKipas,
                         const char *namaKipas, bool statusKipas,
                         bool &manualControlFlag) {
    toggleRelaydanChecking(relayPin, statusKipas, manualControlFlag);
    JsonObject relay = doc_auto_relay.createNestedObject();
    relay["id"] = idKipas;
    relay["name"] = namaKipas;
    relay["status"] = statusKipas ? "ON" : "OFF";
  };

  if (!kontrolManual1 && 1 <= selisih && selisih <= 2) {
    // atur logika untuk relay jika suhu naik
    updateRelay(relay_pin_1, "1", "Kipas Lv.1", true, kontrolManual1);
    timeMatchKipas = true;
    turnon_buzzer(); // bunyikan_buzzer = true;

  } else if (0 <= selisih && selisih < 1) {
    updateRelay(relay_pin_1, "1", "Kipas Lv.1", false, kontrolManual1);
    updateRelay(relay_pin_2, "2", "Kipas Lv.2", false, kontrolManual2);
    updateRelay(relay_pin_3, "3", "Kipas Lv.3", false, kontrolManual3);
    updateRelay(relay_pin_4, "4", "Kipas Lv.4", false, kontrolManual4);
    timeMatchKipas = true;
  }

  if (!kontrolManual2 && selisih >= 3 && selisih <= 4) {
    updateRelay(relay_pin_1, "1", "Kipas Lv.1", true, kontrolManual1);
    updateRelay(relay_pin_2, "2", "Kipas Lv.2", true, kontrolManual2);
    timeMatchKipas = true;
    turnon_buzzer(); // bunyikan_buzzer = true;
  } else if (2 <= selisih && selisih < 3) {
    updateRelay(relay_pin_2, "2", "Kipas Lv.2", false, kontrolManual2);
    updateRelay(relay_pin_3, "3", "Kipas Lv.3", false, kontrolManual3);
    timeMatchKipas = true;
  }

  if (!kontrolManual3 && selisih >= 5 && selisih <= 6) {
    updateRelay(relay_pin_1, "1", "Kipas Lv.1", true, kontrolManual1);
    updateRelay(relay_pin_2, "2", "Kipas Lv.2", true, kontrolManual2);
    updateRelay(relay_pin_3, "3", "Kipas Lv.3", true, kontrolManual3);
    timeMatchKipas = true;
    turnon_buzzer(); // bunyikan_buzzer = true;
  }

  if (!kontrolManual4 && selisih >= 7) {
    updateRelay(relay_pin_1, "1", "Kipas Lv.1", true, kontrolManual1);
    updateRelay(relay_pin_2, "2", "Kipas Lv.2", true, kontrolManual2);
    updateRelay(relay_pin_3, "3", "Kipas Lv.3", true, kontrolManual3);
    updateRelay(relay_pin_4, "4", "Kipas Lv.4", true, kontrolManual4);
    timeMatchKipas = true;
    turnon_buzzer(); // bunyikan_buzzer = true;
    penyalaan_kipas_tambahan_dari_amoniak = true;
  } else if (selisih < 7) {
    updateRelay(relay_pin_4, "4", "Kipas Lv.4", false, kontrolManual4);
    timeMatchKipas = true;
    penyalaan_kipas_tambahan_dari_amoniak = false;
  }

  if (!kontrolManual5 && selisih <= -2 && autoControlPemanasDariSuhu) {
    updateRelay(relay_pin_5, "5", "Lampu Pemanas Auto", true, kontrolManual5);
    updateRelay(relay_pin_1, "1", "Kipas Lv.1", false, kontrolManual1);
    updateRelay(relay_pin_2, "2", "Kipas Lv.2", false, kontrolManual2);
    updateRelay(relay_pin_3, "3", "Kipas Lv.3", false, kontrolManual3);
    timeMatchKipas = true;
    turnon_buzzer(); // bunyikan_buzzer = true;
  } else if (!kontrolManual5 && selisih > -2) {
    updateRelay(relay_pin_5, "5", "Lampu Pemanas Auto", false, kontrolManual5);
    timeMatchKipas = true;
  }

  // kirim status relay auto ke database
  if (timeMatchKipas == true) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin("http://192.168.19.201/database_monitoring_kandang/"
                 "monitoring_relay_auto.php");
      http.addHeader("Content-Type", "application/json");
      String jsonData;
      serializeJson(doc_auto_relay, jsonData);
      Serial.println("isi data: ");
      Serial.println(jsonData); // Check the JSON data in the serial monitor

      int httpcode = http.POST(jsonData);
      if (httpcode > 0) {
        Serial.print("httpResponseCode: ");
        Serial.println(httpcode);
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.print("errHttpResponsecode: ");
        Serial.println(httpcode);
      }
      http.end();
    } else {
      Serial.println("Wifi Disconnected...");
    }
  }
}

// kirim waktu dari  rtc untuk ditampilkan ke webpage
void kirimWaktuDisplayDariRTCkeWebpage() {
  DateTime now = rtc.now();
  char json[256];
  sprintf(json,
          "{\"hour\":%d, \"minute\":%d, \"second\":%d, \"day\":%d, "
          "\"month\":%d, \"year\":%d}",
          now.hour(), now.minute(), now.second(), now.day(), now.month(),
          now.year());
  Serial.println(json);
  server.send(200, "application/json", json);
}

void handleSetTimeLampuKandang() {
  // handle set time
  if (server.hasArg("plain")) {
    String json = server.arg("plain");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, json);

    jamTHidupKandang = doc["houronKandang"];
    menitTHidupKandang = doc["minuteonKandang"];

    JsonArray days = doc["days"];
    for (JsonVariant day : days) {
      Serial.println(day.as<String>());
    }

    server.send(200, "text/plain", "SetTimeKandang");
    Serial.println(
        "lampu kandang dinyalakan pada jam: " + String(jamTHidupKandang) +
        " WIB" + " " + "menit: " + String(menitTHidupKandang) + " WIB");

    parjam_hidupkandang = jamTHidupKandang;
    parmenit_hidupkandang = menitTHidupKandang;
  } else {
    server.send(400, "text/plain", "Invalid request");
    Serial.println("set waktu gagal dikirim");
  }
}

void handleOffTimeLampuKandang() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, json);

    jamTMatiKandang = doc["houroffKandang"];
    menitTMatiKandang = doc["minuteoffKandang"];

    server.send(200, "text/plain", "OffTimeKandang");
    Serial.println(
        "lampu kandang dimatikan pada jam: " + String(jamTMatiKandang) +
        " WIB" + " " + "menit: " + String(menitTMatiKandang) + " WIB");

    parjam_matikandang = jamTMatiKandang;
    parmenit_matikandang = menitTMatiKandang;

  } else {
    server.send(400, "text/plain", "Invalid Request");
    Serial.println("off waktu gagal dikirim");
  }
}

bool dataKandangInserted = false;
void cekRelayKandang(byte parjam_hidupkandang, byte parmenit_hidupkandang,
                     byte parjam_matikandang, byte parmenit_matikandang) {

  DateTime now = rtc.now();
  hour = now.hour();
  minute = now.minute();
  detik = now.second();

  static unsigned char relevt_state = 0, evt_logged = 0;

  bool timeMatch = false;
  String statusMatch;

  if (parjam_hidupkandang == parjam_matikandang &&
      parmenit_hidupkandang == parmenit_matikandang) {
    timeMatch = true;
    statusMatch = "OFF";
    relevt_state = 1;
  }

  else if (kontrolManual6 == true) {
    digitalWrite(relay_pin_6, relay_6 ? LOW : HIGH);
    // Serial.println("Lampu Kandang dioperasikan manual");
    timeMatch = true;
    statusMatch = relay_6 ? "ON" : "OFF";
    relevt_state = 4;
  }

  else if (parjam_hidupkandang == hour && parmenit_hidupkandang == minute &&
           detik == 0) {
    digitalWrite(relay_pin_6, LOW);
    timeMatch = true;
    statusMatch = "ON";
    relevt_state = 2;
  }

  else if (parjam_matikandang == hour && parmenit_matikandang == minute &&
           detik == 0) {
    digitalWrite(relay_pin_6, HIGH);
    timeMatch = true;
    statusMatch = "OFF";
    relevt_state = 3;
  }

  else {
    timeMatch = false;
    statusMatch = "";
    dataKandangInserted = false;
    relevt_state = 5;
  }

  if (evt_logged != relevt_state) { // relevt_state > 0 &&
    evt_logged = relevt_state;
    relevt_state = 0;
    Serial.print("Current time Kandang: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.println(minute);

    Serial.print("Turn on time Kandang: ");
    Serial.print(parjam_hidupkandang);
    Serial.print(":");
    Serial.println(parmenit_hidupkandang);

    Serial.print("Turn off time Kandang: ");
    Serial.print(parjam_matikandang);
    Serial.print(":");
    Serial.println(parmenit_matikandang);

    if (relevt_state == 1) {
      Serial.println("Timer Lampu Hidup dan Mati Sama. Maka tidak dinyalakan");
    } else if (relevt_state == 2) {
      Serial.println("Timer Hidup dan RTC sama. Lampu Kandang menyala");
    } else if (relevt_state == 3) {
      Serial.println("Timer Mati dan RTC sama. Lampu Kandang mati");
    } else if (relevt_state == 4) {
      Serial.println("Lampu Kandang dioperasikan manual");
    } else if (relevt_state == 5) {
      ;
    }
  }

  if (timeMatch == true && !dataKandangInserted) {
    // kirim status relay kandang ke database
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin("http://192.168.19.201/database_monitoring_kandang/"
                 "monitoring_relay.php");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData =
          "relid=6&relayname=Relay Kandang&relayval=" + statusMatch;
      // Serial.println(postData);
      int httpcode = http.POST(postData);
      if (httpcode > 0) {
        Serial.print("httpResponseCode: ");
        Serial.println(httpcode);
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.print("errHttpResponsecode: ");
        Serial.println(httpcode);
      }
      http.end();
      dataKandangInserted = true;
    } else {
      Serial.println("Wifi Disconnected...");
    }
  }
}

void handleSetTimeLampuPemanas() {
  // handle set time
  if (server.hasArg("plain")) {
    String json = server.arg("plain");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, json);

    jamTHidupPemanas = doc["houronPemanas"];
    menitTHidupPemanas = doc["minuteonPemanas"];

    JsonArray days = doc["days"];
    for (JsonVariant day : days) {
      Serial.println(day.as<String>());
    }

    server.send(200, "text/plain", "SetTimePemanas");
    Serial.println(
        "lampu pemanas dinyalakan pada jam: " + String(jamTHidupPemanas) + " " +
        "menit: " + String(menitTHidupPemanas) + " WIB");

    parjam_hiduppemanas = jamTHidupPemanas;
    parmenit_hiduppemanas = menitTHidupPemanas;
  } else {
    server.send(400, "text/plain", "Invalid request");
    Serial.println("set waktu gagal dikirim");
  }
}

void handleOffTimeLampuPemanas() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, json);

    jamTMatiPemanas = doc["houroffPemanas"];
    menitTMatiPemanas = doc["minuteoffPemanas"];

    server.send(200, "text/plain", "OffTimePemanas");
    Serial.println(
        "lampu pemanas dimatikan pada jam: " + String(jamTMatiPemanas) +
        " WIB" + " " + "menit: " + String(menitTMatiPemanas) + " WIB");

    parjam_matipemanas = jamTMatiPemanas;
    parmenit_matipemanas = menitTMatiPemanas;

  } else {
    server.send(400, "text/plain", "Invalid Request");
    Serial.println("off waktu gagal dikirim");
  }
}

bool dataPemanasInserted = false;
void cekRelayPemanas(byte parjam_hiduppemanas, byte parmenit_hiduppemanas,
                     byte parjam_matipemanas, byte parmenit_matipemanas) {
  DateTime now = rtc.now();
  hour = now.hour();
  minute = now.minute();
  detik = now.second();

  static unsigned char relevt_state_pemanas = 0, evt_logged_pemanas = 0;

  /*String relayVal;
  if (status == 0) {
    relayVal == "OFF";
  } else {
    relayVal = "ON";
  }*/

  bool timeMatchPemanas = false;
  String statusMatchPemanas;

  if (parjam_hiduppemanas == parjam_matipemanas &&
      parmenit_hiduppemanas == parmenit_matipemanas) {
    autoControlPemanasDariSuhu = true; // relay dinyalakan secara otomatis
    relevt_state_pemanas = 1;
  }

  else if (kontrolManual5 == true) {
    digitalWrite(relay_pin_5, relay_5 ? LOW : HIGH);
    // Serial.println("Lampu pemanas dioperasikan manual");
    timeMatchPemanas = true;
    statusMatchPemanas = relay_5 ? "ON" : "OFF";
    autoControlPemanasDariSuhu = false;
    relevt_state_pemanas = 4;
  }

  else if (parjam_hiduppemanas == hour && parmenit_hiduppemanas == minute &&
           detik == 0) {
    digitalWrite(relay_pin_5, LOW);
    // Serial.println("Lampu pemanas menyala");
    timeMatchPemanas = true;
    statusMatchPemanas = "ON";
    autoControlPemanasDariSuhu = false;
    relevt_state_pemanas = 2;
  }

  else if (parjam_matipemanas == hour && parmenit_matipemanas == minute &&
           detik == 0) {
    digitalWrite(relay_pin_5, HIGH);
    // Serial.println("Lampu pemanas mati");
    timeMatchPemanas = true;
    statusMatchPemanas = "OFF";
    autoControlPemanasDariSuhu = false;
    relevt_state_pemanas = 3;
  }

  else {
    timeMatchPemanas = false;
    statusMatchPemanas = "";
    dataPemanasInserted = false;
    relevt_state_pemanas = 5;
  }

  if (evt_logged_pemanas != relevt_state_pemanas) { // relevt_state > 0 &&
    evt_logged_pemanas = relevt_state_pemanas;
    relevt_state_pemanas = 0;
    Serial.print("Current time Pemanas: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.println(minute);

    Serial.print("Turn on time Pemanas: ");
    Serial.print(parjam_hiduppemanas);
    Serial.print(":");
    Serial.println(parmenit_hiduppemanas);

    Serial.print("Turn off time Pemanas: ");
    Serial.print(parjam_matipemanas);
    Serial.print(":");
    Serial.println(parmenit_matipemanas);

    if (relevt_state_pemanas == 1) {
      Serial.println("Timer Lampu Pemanas tidak dinyalakan, automasi bekerja");
    } else if (relevt_state_pemanas == 2) {
      Serial.println("Timer Hidup dan RTC sama. Lampu Pemanas menyala");
    } else if (relevt_state_pemanas == 3) {
      Serial.println("Timer Mati dan RTC sama. Lampu Pemanas mati");
    } else if (relevt_state_pemanas == 4) {
      Serial.println("Lampu pemanas dioperasikan manual");
    } else if (relevt_state_pemanas == 5) {
      ;
    }
  }

  if (timeMatchPemanas == true && !dataPemanasInserted) {
    // kirim status relay pemanas ke database
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin("http://192.168.19.201/database_monitoring_kandang/"
                 "monitoring_relay.php");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData =
          "relid=5&relayname=Relay Pemanas&relayval=" + statusMatchPemanas;
      // Serial.println(postData);
      int httpcode = http.POST(postData);
      if (httpcode > 0) {
        Serial.print("httpResponseCode: ");
        Serial.println(httpcode);
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.print("errHttpResponsecode: ");
        Serial.println(httpcode);
      }
      http.end();
      dataPemanasInserted = true;
    } else {
      Serial.println("Wifi Disconnected...");
    }
  }
}

void bacappmAmoniakdankontroldankirimkedatabase() {
  float ppm;
  float RL;
  float RO = MQ135.getR0();

  MQ135.update(); // Update data, the esp32 will read the voltage from the
                  // analog pin
  ppm =
      MQ135.readSensor(); // Sensor will read PPM concentration using the model,
                          // a and b values set previously or from the setup
  MQ135.serialDebug();

  // variabel nilai ppm
  // ppm = MQ135._PPM;
  RL = MQ135.getRL();

  String formatPPM = String(ppm, 2);
  lcd.setCursor(0, 1);
  lcd.print("NH3:");
  lcd.print(formatPPM);

  Serial.print("Nilai RO adalah: ");
  Serial.println(RO);
  Serial.print("Nilai RL adalah: ");
  Serial.println(RL);
  Serial.print("Nilai PPM Amoniak adalah: ");
  Serial.println(ppm);

  sendJson<String>("Amoniak1", formatPPM);

  if (ppm >= 5 && penyalaan_kipas_tambahan_dari_amoniak == false) {
    digitalWrite(relay_pin_4, LOW);
  } else if (ppm < 5 && penyalaan_kipas_tambahan_dari_amoniak == false) {
    digitalWrite(relay_pin_4, HIGH);
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.19.201/database_monitoring_kandang/"
               "monitoring_amoniak.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postDataAmoniak = "amoniakValue=" + String(ppm);
    // Serial.println(postData);
    int httpcode = http.POST(postDataAmoniak);
    if (httpcode > 0) {
      Serial.print("httpResponseCode: ");
      Serial.println(httpcode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("errHttpResponsecode: ");
      Serial.println(httpcode);
    }
    http.end();
  } else {
    Serial.println("Wifi Disconnected...");
  }
}

// Function to calculate day number based on current date and start date
int calculateDayNumber(DateTime currentDate) {
  String startDateStr = getStartDate();
  if (startDateStr.length() == 0) {
    Serial.println("Start date not set.");
    return 0; // Day 0 indicates start date not set
  }

  int year, month, day;
  if (sscanf(startDateStr.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
    Serial.println("Invalid start date format in EEPROM.");
    return 0; // Or handle as needed
  }

  DateTime startDate(year, month, day, 0, 0, 0);

  // Calculate the difference in seconds
  long diff = currentDate.unixtime() - startDate.unixtime();

  if (diff < 0) {
    Serial.println("Current date is before the start date.");
    return 0; // Or handle as needed
  }

  return (diff / 86400) + 1; // +1 to make start date Day 1
}

// Function to get start date from EEPROM
String getStartDate() {
  Preferences prefs;
  prefs.begin(MODE_NAMESPACE, true); // Read-only mode
  String dateStr = prefs.getString(START_DATE_KEY, "");
  prefs.end();
  return dateStr;
}

// Function to get temperature for the current day
int getTemperatureForDay(int day, const DayRange *ranges, int totalRanges) {
  for (int i = 0; i < totalRanges; i++) {
    if (day >= ranges[i].startDay && day <= ranges[i].endDay) {
      return ranges[i].temperature;
    }
  }
  // Handle days beyond defined ranges
  Serial.println("Day number exceeds defined ranges. Using default setpoint.");
  return 25; // Default temperature setpoint
}

// Function to write temperature ranges to EEPROM
void writeRangesToPreferences() {
  preferences.begin("tempConfig", false); // Open in read-write mode

  for (int i = 0; i < TOTAL_RANGES; i++) {
    String key = "range" + String(i); // e.g., range0, range1, ...
    preferences.putBytes(key.c_str(), &temperatureRangesDefault[i],
                         sizeof(DayRange));
  }

  preferences.end(); // Close the namespace
  Serial.println("Temperature ranges written to Preferences successfully.");
}

// Function to set start date (called from WebSocket)
void setStartDate(int year, int month, int day) {
  // Validate date
  if (!isValidDate(year, month, day)) {
    Serial.println("Invalid date provided. Start date not set.");
    return;
  }

  // Convert to string
  char dateBuffer[11]; // "YYYY-MM-DD" + null terminator
  snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", year, month, day);
  String dateStr = String(dateBuffer);

  // Store in EEPROM
  Preferences prefs;
  prefs.begin(MODE_NAMESPACE, false); // Read-write mode
  prefs.putString(START_DATE_KEY, dateStr);
  prefs.end();

  // Reset day counter to 1 (optional based on implementation)
  prefs.begin(MODE_NAMESPACE, false);
  prefs.putInt("dayCounter", 1);
  prefs.end();

  Serial.printf("Start Date set to %s and day counter reset to 1.\n",
                dateStr.c_str());
}

void statusRelayKontrolManual(unsigned char relay, String relayName,
                              String relayVal) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin("http://192.168.19.201/database_monitoring_kandang/"
               "monitoring_relay.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "relid=" + String(relay) + "&relayname=" + relayName +
                      "&relayval=" + relayVal;
    int httpcode = http.POST(postData);
    if (httpcode > 0) {
      Serial.print("httpResponseCode: ");
      Serial.println(httpcode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("errHttpResponsecode: ");
      Serial.println(httpcode);
    }
    http.end();
  } else {
    Serial.println("Wifi Disconnected...");
  }
}

bool validasiSuhuInput(int suhuTarget) { return suhuTarget != 0; }

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length) {
  int todaySetTemperature = getTemperatureForDay(
      currentDayNumber, temperatureRangesDefault, TOTAL_RANGES);
  switch (type) {
  case WStype_DISCONNECTED: // enum that read status that used for
                            // debugging needs
    Serial.println("Client" + String(num) + "Disconnected");
    break;

  case WStype_CONNECTED: // check if a websocket client is connected or
                         // not
    Serial.println("Client" + String(num) + "Connected");
    sendJson<bool>("statrelay_1", String(relay_1));
    sendJson<bool>("statrelay_2", String(relay_2));
    sendJson<bool>("statrelay_3", String(relay_3));
    sendJson<bool>("statrelay_4", String(relay_4));
    sendJson<bool>("statrelay_5", String(relay_5));
    sendJson<bool>("statrelay_6", String(relay_6));
    sendJson<String>("message", "This is day " + String(currentDayNumber) +
                                    "." + " Temperature for today is: " +
                                    String(todaySetTemperature) + " °C");
    break;

  case WStype_TEXT: // checking client's response
    DeserializationError error = deserializeJson(doc_receiver, payload);
    Serial.println("Received: " + String((char *)payload));
    if (error) {
      Serial.print("Deserialization Failed!");
      return;
    } // to check if there is an error
    else {
      // if Json string received succesfully
      // untuk ngecek payload yang akan dikirimkan berhubungan dengan
      // relay
      if (doc_receiver.containsKey("relay")) {
        const char *relay = doc_receiver["relay"];
        bool status = doc_receiver["status"];
        Serial.println("Status Relay: " + String(relay));
        Serial.println("Nilai: " + String(status));
        sendJson(relay, String(status));
        // unsigned char relid;
        // String relayName;
        // String relayVal;
        if (status == 0) {
          relayVal = "OFF";
        } else {
          relayVal = "ON";
        }
        if (strcmp(relay, "relay_1") == 0) {
          relay_1 = status;
          kontrolManual1 = true;
          hitungKontrolRManualMillis1 = millis();
          digitalWrite(relay_pin_1, relay_1 ? LOW : HIGH);
          relid = 1;
          relayName = "Relay 1";
        }

        if (strcmp(relay, "relay_2") == 0) {
          relay_2 = status;
          kontrolManual2 = true;
          hitungKontrolRManualMillis2 = millis();
          digitalWrite(relay_pin_2, relay_2 ? LOW : HIGH);
          relid = 2;
          relayName = "Relay 2";
        }

        if (strcmp(relay, "relay_3") == 0) {
          relay_3 = status;
          kontrolManual3 = true;
          hitungKontrolRManualMillis3 = millis();
          digitalWrite(relay_pin_3, relay_3 ? LOW : HIGH);
          relid = 3;
          relayName = "Relay 3";
        }

        if (strcmp(relay, "relay_4") == 0) {
          relay_4 = status;
          kontrolManual4 = true;
          hitungKontrolRManualMillis4 = millis();
          digitalWrite(relay_pin_4, relay_4 ? LOW : HIGH);
          relid = 4;
          relayName = "Relay 4";
        }

        if (strcmp(relay, "relay_5") == 0) {
          relay_5 = status;
          kontrolManual5 = true;
          hitungKontrolRManualMillis5 = millis();
          digitalWrite(relay_pin_5, relay_5 ? LOW : HIGH);
          relid = 5;
          relayName = "Relay 5";
        }

        if (strcmp(relay, "relay_6") == 0) {
          relay_6 = status;
          kontrolManual6 = true;
          hitungKontrolRManualMillis6 = millis();
          digitalWrite(relay_pin_6, relay_6 ? LOW : HIGH);
          relid = 6;
          relayName = "Relay 6";
        }
        statusRelayKontrolManual(relid, relayName, relayVal);
      }

      // cek payload nilai suhu dijaga yang dikirimkan dari website
      if (doc_receiver.containsKey("suhu")) {
        suhu = doc_receiver["suhu"];
        Serial.println("Received: Suhu = " + String(suhu));
        if (validasiSuhuInput(suhu)) {
          sendJson<float>("suhu", suhu);
          simpanSuhukeEEPROM(suhu);
          // konfirmasi kirim info ke website
          sendJson<String>("konfirmasiSuhu",
                           "Suhu " + String(suhu) + "°C disimpan");
        }
      }

      if (doc_receiver.containsKey("startdate")) {
        String datestr = doc_receiver["startdate"];
        Serial.printf("Received Start Date: %s\n", datestr.c_str());

        // Parse the date string
        int year, month, day;
        if (sscanf(datestr.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
          // Validate the date
          if (isValidDate(year, month, day)) {
            // Check if the start date is not in the future
            DateTime startDate(year, month, day, 0, 0, 0);
            DateTime currentDate = rtc.now();
            if (startDate.unixtime() > currentDate.unixtime()) {
              webSocket.sendTXT(num, "Start date cannot be in the future.");
              Serial.println(
                  "Received start date is in the future. Command rejected.");
            } else {
              // Set start date
              setStartDate(year, month, day);

              // Notify the client
              webSocket.sendTXT(num, "Start date set successfully.");
              webSocket.sendTXT(num, "Date is set on " + String(year) + "-" +
                                         String(month) + "-" + String(day) +
                                         ".");
              webSocket.sendTXT(num,
                                "This is day: " + String(currentDayNumber) +
                                    "." + "The temperature is set at " +
                                    String(todaySetTemperature) + "°C");
              Serial.println(
                  "Start date stored in EEPROM and day counter reset to 1.");
            }
          } else {
            webSocket.sendTXT(num, "Invalid date provided.");
            Serial.println("Received invalid date values.");
          }
        } else {
          webSocket.sendTXT(num, "Date parsing failed. Use format YYYY-MM-DD.");
          Serial.println("Date parsing failed.");
        }
      }

      break;
    }
  }
}

void setup() {
  // Read temperature ranges from Preferences
  DayRange loadedRanges[TOTAL_RANGES];
  int loadedTotalRanges = 0;

  // LCD
  Wire1.begin(SDA_2, SCL_2, 100000);
  lcd.begin(19, 18);
  lcd.backlight();
  Serial.begin(115200);

  // setup amoniak
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(102.2);
  MQ135.setB(-2.473);
  MQ135.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update(); // Update data kalibrasi, the esp32 will read the voltage
                    // from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit "
                   "detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin "
                   "shorts to ground) please check your wiring and supply");
    while (1)
      ;
  }
  MQ135.serialDebug(true);

  // pin setup
  pinMode(relay_pin_1, OUTPUT);
  pinMode(relay_pin_2, OUTPUT);
  pinMode(relay_pin_3, OUTPUT);
  pinMode(relay_pin_4, OUTPUT);
  pinMode(relay_pin_6, OUTPUT);
  pinMode(relay_pin_5, OUTPUT);
  digitalWrite(relay_pin_1, relay_1 ? LOW : HIGH);
  digitalWrite(relay_pin_2, relay_2 ? LOW : HIGH);
  digitalWrite(relay_pin_3, relay_3 ? LOW : HIGH);
  digitalWrite(relay_pin_4, relay_4 ? LOW : HIGH);
  digitalWrite(relay_pin_5, relay_5 ? LOW : HIGH);
  digitalWrite(relay_pin_6, relay_6 ? LOW : HIGH);

  // buzzer
  pinMode(pin_buzzer, OUTPUT);
  digitalWrite(pin_buzzer, LOW);

  // wifi setup
  WiFi.begin(ssid, password);
  Serial.println("Estabilishing connection to WiFi with SSIS : " +
                 String(ssid));

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.print("\nConnected to network with IP adress: ");
  Serial.println(WiFi.localIP());

  // timeClient.begin();
  if (readRangesFromPreferences(loadedRanges, loadedTotalRanges)) {
    Serial.println("Loaded Temperature Ranges:");
    for (int i = 0; i < loadedTotalRanges; i++) {
      Serial.print("Range ");
      Serial.print(loadedRanges[i].startDay);
      Serial.print(" - ");
      Serial.print(loadedRanges[i].endDay);
      Serial.print(" hari: ");
      Serial.print(loadedRanges[i].temperature);
      Serial.println("°C");
    }

    // Validate day ranges
    if (!validateDayRanges(loadedRanges, loadedTotalRanges)) {
      Serial.println("Invalid day ranges detected. Re-initializing EEPROM with "
                     "default ranges.");
      writeRangesToPreferences();
      readRangesFromPreferences(loadedRanges, loadedTotalRanges);
    }
  } else {
    Serial.println(
        "Failed to load temperature ranges. Initializing with default ranges.");
    writeRangesToPreferences();
    readRangesFromPreferences(loadedRanges, loadedTotalRanges);
  }

  // rtcsetup
  if (!rtc.begin()) {
    Serial.println("RTC tidak ditemukan");
    while (1)
      ;
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Temporary time setting
  }

  // update waktu rtc secara otomatis dengan bantuan internet NTP
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // set rtc time dari ntp time
  rtc.adjust(DateTime(timeClient.getEpochTime()));

  server.on("/", []() { server.send(200, "text/html", webPage); });
  server.on("/readWebRTC", kirimWaktuDisplayDariRTCkeWebpage);
  server.on("/setTimeKandang", HTTP_POST, handleSetTimeLampuKandang);
  server.on("/stopTimeKandang", HTTP_POST, handleOffTimeLampuKandang);
  server.on("/setTimePemanas", HTTP_POST, handleSetTimeLampuPemanas);
  server.on("/stopTimePemanas", HTTP_POST, handleOffTimeLampuPemanas);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // other looping
  // sudah ada timing milis tiap 1 detik
  // bacappmAmoniakdankontroldankirimkedatabase();
  // delay(1000);

  server.handleClient();
  webSocket.loop();
  // sudah ada timing milis tiap 10 menit
  bacaSuhudanBandingkandenganEEPROM();

  unsigned long currentMillis = millis();

  if (currentMillis - tempo > jedacekrelay) {
    cekRelayKandang(parjam_hidupkandang, parmenit_hidupkandang,
                    parjam_matikandang, parmenit_matikandang);

    cekRelayPemanas(parjam_hiduppemanas, parmenit_hiduppemanas,
                    parjam_matipemanas, parmenit_matipemanas);
    tempo = currentMillis;
  }

  if (currentMillis - prev_period_one_sec > CheckIntervalOneSec) {
    prev_period_one_sec = currentMillis;
    DateTime now = rtc.now();
    Serial.print("RTC time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.println(now.second(), DEC);

    Serial.print("NTP time: ");
    Serial.println(timeClient.getFormattedTime());

    // Static variable to keep track of last processed day
    static int lastProcessedDay = 0;
    currentDayNumber = calculateDayNumber(now);
    // Check if a new day has arrived
    if (currentDayNumber != lastProcessedDay && currentDayNumber != 0) {
      lastProcessedDay = currentDayNumber;
      Serial.printf("Processing Day %d\n", currentDayNumber);

      // Get the setpoint for the current day
      int setpoint = getTemperatureForDay(
          currentDayNumber, temperatureRangesDefault, TOTAL_RANGES);
      Serial.printf("Current Setpoint: %d°C\n", setpoint);

      // Set the temperature setpoint
      // setTemperatureSetpoint(setpoint);
      daytemp_setpoint = setpoint;
      preferences.begin("TEMP", false);
      if (preferences.getFloat("DT0") != 0) {
        preferences.putFloat("DT0", 0);
      }
      preferences.end();
    }

    // tempo = currentMillis;
  }

  if (currentMillis - prev_period_amonia >= CheckIntervalAmonia) {
    prev_period_amonia = currentMillis;
    bacappmAmoniakdankontroldankirimkedatabase();
  }

  // Batas waktu switching dari kontrol manual websocket dengan sistem
  // automasi yang telah dibuat
  if (kontrolManual1 &&
      currentMillis - hitungKontrolRManualMillis1 > manualControlTimeout1) {
    kontrolManual1 = false;
    Serial.println("manual 1 off");
  }

  if (kontrolManual2 &&
      currentMillis - hitungKontrolRManualMillis2 > manualControlTimeout2) {
    kontrolManual2 = false;
    Serial.println("manual 2 off");
  }

  if (kontrolManual3 &&
      currentMillis - hitungKontrolRManualMillis3 > manualControlTimeout3) {
    kontrolManual3 = false;
    Serial.println("manual 3 off");
  }

  if (kontrolManual4 &&
      currentMillis - hitungKontrolRManualMillis4 > manualControlTimeout4) {
    kontrolManual4 = false;
    Serial.println("manual 4 off");
  }

  if (kontrolManual5 &&
      currentMillis - hitungKontrolRManualMillis5 > manualControlTimeout5) {
    kontrolManual5 = false;
    Serial.println("manual 5 off");
  }

  if (kontrolManual6 &&
      currentMillis - hitungKontrolRManualMillis6 > manualControlTimeout6) {
    kontrolManual6 = false;
    Serial.println("manual 6 off");
  }

  delay(1);
}
