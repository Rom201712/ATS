#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <SPIFFS.h>
#include <SPIFFSEditor.h>
#include "Preferences.h"
#include "SoftwareSerial.h"
#include <ESP32Ticker.h>
#include <SimpleModbusSlave_DUE.h>
#include <string>
#include <RtcDS3231.h>
#include <Adafruit_ADS1X15.h>

#include "COUNTER.h"
#include "TARRING.h"
#include "TANK.h"
#include "NEXTION.h"
#include "Out.h"

#define nPRINTDEBUG

#define PLATE_v1 // PLATE_v1 - плата вер1, PLATE_TEST - тестовая плата

#define DATA_LENGTH 8 // длина протокола передачи LLS

#ifdef PLATE_TEST
#define OUT_PUMP 12           // вывод управления насосом
#define IN_KCOUNT 9           // вход счетчика топлива
static const int RXDNEX = 19; //
static const int TXDNEX = 23; //
static const int RXLS = 25;
static const int TXLS = 33;
#endif

#ifdef PLATE_v1
static const uint INDI_F_PIN_ = 25; // индикатор включения частотного ДУТа
#define OUT_PUMP 12           // вывод управления насосом
#define IN_KCOUNT 5           // вход счетчика топлива
static const int RXDNEX = 23; //
static const int TXDNEX = 19; //
static const int RXLS = 27;
static const int TXLS = 14;
#endif

// #define I2C_SDA 13
// #define I2C_SCL 14

const int SIZE = 22;

// режимы работы
enum type
{
  CALIBR,      // Калибровка счетчика
  PUMPINGAUTO, // Выдача топлива
  TAR,         // Тарировка
  SETTING,     // Настройка тарировки
  MENU,        // Меню
  COUNT,       // Счетчик
  PUMPINGOUT,  // Откачка топлива автоматом
  END_TAR,
  MESSAGE,
  SEARCH_BLE,
  PAUSE
};

union
{
  struct
  {
    unsigned int mode;          //  1 режим работы станции
    unsigned int kRefillNum;    //  2 номер пролива
    unsigned int v_full;        //  3 объем залитого топлива всего в 0,1 литра 1-2 байт
    unsigned int k_v_full;      //  3.1 объем залитого топлива всего в 0,1 литра 3-4 байт
    unsigned int resultNProliv; //  5 N тарируемого ДУТа зафиксированный станцией в проливах
    unsigned int id1;           //  6 номер автомобиля 1-2 байт
    unsigned int id2;           //  6.1 номер автомобиля 3-4 байт
    unsigned int resultN;       //  8 N тарируемого ДУТА постоянно получаемы данные
    unsigned int adress;        //  9 сетевой адресс ДУТа 1-2 байт
    unsigned int k_adress;      //  9.1 сетевой адресс ДУТа 3-4 байт
    unsigned int vtank;         //  11 объем тарируемого бака, литр
    unsigned int kRefill;       //  12  количество проливов
    unsigned int flowRate;      //  13  скорость потока, литр/мин
    unsigned int pause;         //  14  длительность паузы между проливами, сек
    unsigned int k_in_Litr;     //  15  количество импульсов на 10 литров
    unsigned int timetarring;   //  16  время выполнения тарировки
    unsigned int typells;       //  17  тип ДУТ , 0 - аналоговый_U , 1 - аналоговый_F , 2 - цифровой по rs485 , 3 - цифровой BLE
    unsigned int typetarring;   //  19  режим тарировки 1 - автоматический, 0 - ручной
    unsigned int error;         //  19  код ошибки
    unsigned int rssi;          //  20  RSSI ДУТ BLE
    unsigned int llsATP;        //  21  N ДУТа емкости АПТ (lls adr=100)
  };
  unsigned int au16data[SIZE];
} datemod;

volatile int counter_display_resetring = 0;
long duratiom_counter_imp = 0;
const uint MIN_DURATION = 5;
const uint TIME_UPDATE_LLS = 1000;
const uint TIME_PAUSE_END_TAR = 10000; // пауза в конце тарировки для передаче данных в систему мониторинга

unsigned long time_display_update, start_pause, worktime, speedPumptime, time_start_refill;
bool autostop = false;
bool flag_dell_lls = false; // флаг необходимости удаления lls

const char *LOG_FILE_NAME = "log.csv";
const char *SSID = "ATS";
const char *PASWD = "12_04_19";

AsyncWebServer server(80);

void rpmFun();
void updateLS();

void analyseString(String incStr);
void readNextion();
void modeMenu();
void modePumpOut();
void modeTarring();
void modeCounter();
void modePumpAuto();
void modeCalibr();
void endTarring();
void endRefill();
void proceedTarring();
#ifdef PRINTDEBUG
void printDebugLog();
#endif
void modeSetting();
void flowRate();
void modeEndTar();
void errors();
void modbus();
void digitalpause();
void updateNextion();
void startPump();
void stopPump();
String makeLlsDateToDisplay(ILEVEL_SENSOR *_lls);
void exitTarring();
String saveLog();
void wifiInit();

void wifiInit();
void delete_lls();
String saveLog();
String deleteLog();

String listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void getDataLog(AsyncWebServerRequest *request, String file);

RtcDS3231<TwoWire> Rtc(Wire);
Adafruit_ADS1115 ads; /* Use this for the 16-bit version */

Preferences flash;
SoftwareSerial serialNextion;
SoftwareSerial serialLS;
Ticker tickermodbus;        // обновление данных modbus
Ticker tickerupdateLS;      // обновление данных ДУТ
Ticker tickerspeedPump;     //  вычисление скорости насоса
Ticker tickerprintDebugLog; // вывод лога в сериал-порт
Ticker tickerSendNextion;   // вывод данных на дисплей Nextion

// Ticker tickertest; //
// void test();

// ДУТ
ILEVEL_SENSOR *lls;

// ДУТ в емкости АТП

// насос
Out *pump;

// запас
Out *out_tmp;

// счетчик
COUNTER *countV;

// бак
TANK *tank;

// тарировка
TARRING *tar;

// дисплей
NEXTION *display;
