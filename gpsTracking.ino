
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#include "ThingsBoard.h"
#include <TinyGPS.h>

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[] = "web";
const char user[] = "";
const char pass[] = "";

#define TOKEN "3uHiDbkkY91rtRxycwUJ"
//#define TOKEN "Q7S0Q1uGjf3kIlRpxgiF"
#define THINGSBOARD_SERVER "http://127.0.0.1/api/v1/gpsTracker"

#define THINGSBOARD_PORT 8080

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD 115200

// Serial port for GSM shield
SoftwareSerial serialGsm(11,10);  // RX, TX pins for communicating with modem


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(serialGsm, Serial);
TinyGsm modem(debugger);
#else
  // Initialize GSM modem
TinyGsm modem(serialGsm);
#endif

// Initialize GSM client
TinyGsmClient client(modem);

// Initialize ThingsBoard instance
ThingsBoardHttp tb(client, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT);

// Set to true, if modem is connected
bool modemConnected = true;


//GPS Part


TinyGPS gps;
SoftwareSerial ss(9,8);

static void smartdelay(unsigned long ms);

//GPS end

void setup() {
  // Set console baud rate

  Serial.begin(SERIAL_DEBUG_BAUD);
  Serial.println("-------SETUP------");

  // Set GSM module baud rate
  serialGsm.begin(115200);
  delay(3000);

  // Lower baud rate of the modem.
  // This is highly practical for Uno board, since SoftwareSerial there
  // works too slow to receive a modem data.

  serialGsm.write("AT+IPR=9600\r\n");
  serialGsm.end();
  serialGsm.begin(9600);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println(F("Initializing modem..."));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem: "));
  Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  //GPS Software Serial
  ss.begin(9600);
}

void loop() {
  delay(1000);

  if (!modemConnected) {
    Serial.print(F("Waiting for network..."));
    if (!modem.waitForNetwork()) {
      Serial.println(" fail");
      delay(10000);
      return;
    }
    Serial.println(" OK");

    Serial.print(F("Connecting to "));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, user, pass)) {
      Serial.println(" fail");
      delay(10000);
      return;
    }

    modemConnected = true;
    Serial.println(" OK");
  }




  //GPS part for lat and long

  float flat, flon;

  gps.f_get_position(&flat, &flon);

  Serial.print("Latitude : ");
  unsigned int latitude = Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  Serial.print(latitude);

  Serial.print(" ,Longitude : ");
  unsigned int longitude = Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
  Serial.println(longitude);

  tb.sendTelemetryFloat("latitude",10);
  tb.sendTelemetryFloat("longitude", 20);

  smartdelay(1000);  
}



static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
