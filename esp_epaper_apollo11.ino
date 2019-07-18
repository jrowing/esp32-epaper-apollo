
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>

WiFiMulti wifiMulti;
WiFiUDP ntpUDP;

#include "SPIFFS.h"
File file;
#include <NTPClient.h>



// include library, include base class, make path known
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

#define DEFALUT_FONT  FreeMono9pt7b
// #define DEFALUT_FONT  FreeMonoBoldOblique9pt7b
// #define DEFALUT_FONT FreeMonoBold9pt7b
// #define DEFALUT_FONT FreeMonoOblique9pt7b
//#define DEFALUT_FONT FreeSans9pt7b
// #define DEFALUT_FONT FreeSansBold9pt7b
// #define DEFALUT_FONT FreeSansBoldOblique9pt7b
// #define DEFALUT_FONT FreeSansOblique9pt7b
// #define DEFALUT_FONT FreeSerif9pt7b
// #define DEFALUT_FONT FreeSerifBold9pt7b
// #define DEFALUT_FONT FreeSerifBoldItalic9pt7b
// #define DEFALUT_FONT FreeSerifItalic9pt7b

const GFXfont *fonts[] = {
  &FreeMono9pt7b,
  &FreeMonoBoldOblique9pt7b,
  &FreeMonoBold9pt7b,
  &FreeMonoOblique9pt7b,
  &FreeSans9pt7b,
  &FreeSansBold9pt7b,
  &FreeSansBoldOblique9pt7b,
  &FreeSansOblique9pt7b,
  &FreeSerif9pt7b,
  &FreeSerifBold9pt7b,
  &FreeSerifBoldItalic9pt7b,
  &FreeSerifItalic9pt7b
};

const char *fontnames[] = {
  "FreeMono9pt7b",
  "FreeMonoBoldOblique9pt7b",
  "FreeMonoBold9pt7b",
  "FreeMonoOblique9pt7b",
  "FreeSans9pt7b",
  "FreeSansBold9pt7b",
  "FreeSansBoldOblique9pt7b",
  "FreeSansOblique9pt7b",
  "FreeSerif9pt7b",
  "FreeSerifBold9pt7b",
  "FreeSerifBoldItalic9pt7b",
  "FreeSerifItalic9pt7b"
};

#include "SPI.h"
#include "Esp.h"
#include <GxGDE0213B72/board_def.h>

typedef enum {
  RIGHT_ALIGNMENT = 0,
  LEFT_ALIGNMENT,
  CENTER_ALIGNMENT,
} Text_alignment;

GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

int fontnum = 0;
unsigned int apollo_location = 0;  //line number of text file we are up to
unsigned long actual_time = 0;
unsigned long offset_time = 1563283920; // epoch time for desired apollo 11 launch, eg July 16 2019 13:32  UTC = 1563283920 https://www.epochconverter.com/  1563201384
unsigned long update_time = 1000;
unsigned long current_event_time = 0;
unsigned long next_event_time = 0;

// holding the next communication to be displayed here
char id[6];
char mission_time[8];
char comms[2220];  //longest expected line is 2212 chars
char speaker[9];
unsigned int dim_id = 0;
unsigned int dim_mission_time = 0;
unsigned int dim_comms = 0;
unsigned int dim_speaker = 0;
//

//NTP server config
NTPClient timeClient(ntpUDP, "0.au.pool.ntp.org", 3600 * 0, 30000); //UTC offset (0 hrs), refresh cycle (30 mins)

void displayText(const String &str, int16_t y, uint8_t alignment)
{
  int16_t x = 0;
  int16_t x1, y1;
  uint16_t w, h;
  display.setCursor(x, y);
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h);

  switch (alignment) {
    case RIGHT_ALIGNMENT:
      display.setCursor(display.width() - w - x1, y);
      break;
    case LEFT_ALIGNMENT:
      display.setCursor(0, y);
      break;
    case CENTER_ALIGNMENT:
      display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
      break;
    default:
      break;
  }
  display.println(str);
}

void showMainPage(void)
{
  displayInit();
  display.fillScreen(GxEPD_WHITE);
  //    drawBitmap(DEFALUT_AVATAR_BMP, 10, 10, true);
  /*    displayText(String(info.name), 30, RIGHT_ALIGNMENT);
      displayText(String(info.company), 50, RIGHT_ALIGNMENT);
      displayText(String(info.email), 70, RIGHT_ALIGNMENT);
      displayText(String(info.link), 90, RIGHT_ALIGNMENT);*/

  displayText("The quick brown fox", 50, CENTER_ALIGNMENT);
  displayText("jumps over the lazy dog.", 70, CENTER_ALIGNMENT);
  //    displayText(String(fonts[0]), 110, RIGHT_ALIGNMENT);
  //    displayText("Mk1", 110, RIGHT_ALIGNMENT);
  display.update();
}

void showStartupPage(void) {
  //displayInit();
  display.fillScreen(GxEPD_WHITE);
  //    drawBitmap(DEFALUT_AVATAR_BMP, 10, 10, true);
  /*    displayText(String(info.name), 30, RIGHT_ALIGNMENT);
      displayText(String(info.company), 50, RIGHT_ALIGNMENT);
      displayText(String(info.email), 70, RIGHT_ALIGNMENT);
      displayText(String(info.link), 90, RIGHT_ALIGNMENT);*/
  if (WiFi.status() == WL_CONNECTED) {
    displayText("Connected!", 30, LEFT_ALIGNMENT);
    String textSSID = "";
    String textIP = "";
    textSSID = "SSID:\t" + WiFi.SSID();
    displayText(textSSID, 50, LEFT_ALIGNMENT);
    textIP = "IP:\t" + WiFi.localIP().toString();
    displayText(textIP, 70, LEFT_ALIGNMENT);
  } else {
    displayText("Wifi FAILED", 30, LEFT_ALIGNMENT);
  }

  //    displayText("Mk1", 110, RIGHT_ALIGNMENT);
  display.update();
  delay(3000);
}

void showBootError(String err_string) {
  display.fillScreen(GxEPD_WHITE);
  String error = "Error: " + err_string;
  if (WiFi.status() == WL_CONNECTED) {
    displayText("Connected!", 12, LEFT_ALIGNMENT);
    String textSSID = "";
    String textIP = "";
    textSSID = "SSID:\t" + WiFi.SSID();
    displayText(textSSID, 30, LEFT_ALIGNMENT);
    textIP = "IP:\t" + WiFi.localIP().toString();
    displayText(textIP, 50, LEFT_ALIGNMENT);
  } else {
    displayText("Wifi FAILED", 12, LEFT_ALIGNMENT);
  }
  displayText(error, 70, LEFT_ALIGNMENT);
  display.update();    
}

String padder(unsigned long in) {
  if (in < 1) {
    return "00";
  }
  if (in < 10) {
    return "0" + String(in);
  }
  if (in >= 10) {
    return String(in);
  }
}

String missionClock() {
  #define c_DAYS 86400
  #define c_HOURS 3600
  #define c_MINS 60
  unsigned long mtime = atoi(mission_time);
  unsigned long mclock = 0;
  String mout = "";
  //days
  mclock = mtime / c_DAYS;
  mout = padder(mclock)+ " ";
  mtime = mtime - (mclock*c_DAYS);
  //hours
  mclock = mtime / c_HOURS;
  mout = mout + padder(mclock) + " ";
  mtime = mtime - (mclock*c_HOURS);
  //minutes
  mclock = mtime / c_MINS;
  mout = mout + padder(mclock) + " ";
  mtime = mtime - (mclock*c_MINS);
  //seconds
  mout = mout + padder(mtime);
  return mout;
}

void showApollo() {
  String textMissionClock = "";
  String textNextEvent = "";
  String textEvent = "";
 // textMissionClock = "Mission Clock-T+" + String(mission_time);
  textMissionClock = "T+" + missionClock();
  textNextEvent = "Next Event T+" + String(next_event_time);
  textEvent = String(speaker) + ": " + String(comms);
  display.setTextColor(GxEPD_WHITE);
  display.fillScreen(GxEPD_BLACK);
//  displayText(strcat(String("Mission Clock T+"),String(mission_time)) , 30, LEFT_ALIGNMENT);
//  displayText(strcat(String("Next Event T+"), String(next_event_time)), 30, RIGHT_ALIGNMENT);
//  displayText(strcat((strcat(String(speaker), String(": "))), String(comms)), 50, LEFT_ALIGNMENT);
  display.setFont(&FreeMonoBold9pt7b);
  displayText(textMissionClock , 12, LEFT_ALIGNMENT);
//  displayText(textNextEvent, 30, LEFT_ALIGNMENT);
  display.setFont(&FreeMono9pt7b);
  displayText(textEvent, 30, LEFT_ALIGNMENT);
  display.update();
//  delay(1000);
}

void displayInit(void)
{
  static bool isInit = false;
  if (isInit) {
    return;
  }
  isInit = true;
  display.init();
  display.setRotation(1);
  display.eraseDisplay();
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&DEFALUT_FONT);
  //    display.setFont(&fonts[fontnum]);
  display.setTextSize(0);
  display.update();
 // delay(2000);
}

void showFont(const char name[], const GFXfont* f)
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  display.println(name);
  display.println(" !\"#$%&'()*+,-./");
  display.println("0123456789:;<=>?");
  display.println("@ABCDEFGHIJKLMNO");
  display.println("PQRSTUVWXYZ[\\]^_");
#if defined(HAS_RED_COLOR)
  display.setTextColor(GxEPD_RED);
#endif
  display.println("`abcdefghijklmno");
  display.println("pqrstuvwxyz{|}~ ");
  display.update();
  delay(5000);
}

void printChar(char inarray[], unsigned int dim) {
  for (int k = 0; k < dim; k++) {
    Serial.print(inarray[k]);
  }
}

void clearChar(char inarray[], unsigned int dim) {
  for (int k = 0; k < dim; k++) {
    inarray[k] = '\0';
  }
}

void lineDisplay() {

  //   printChar(id, dim_id);
  //   Serial.print(",\t");
  printChar(mission_time, dim_mission_time);
  Serial.print("\t");
  //   Serial.print(next_event_time);
  //   Serial.print(",\t");
  printChar(speaker, dim_speaker);
  Serial.print("\t");
  printChar(comms, dim_comms);
  Serial.println("");

  showApollo();
  
}

void lineRead() {
  char inchar;
//  unsigned int line = 0;
  unsigned int i = 0; // which variable we are looking at (0 = id, 1 = mission time, 2 = comms, 3 = speaker)
  unsigned int j = 0; // which character we are up to for that variable

  dim_id = 0;
  dim_mission_time = 0;
  dim_comms = 0;
  dim_speaker = 0;
  clearChar(mission_time, 8);
  clearChar(id, 6);
  clearChar(comms, 2220);
  clearChar(speaker, 9);
  
  // File file = SPIFFS.open("/apollo.csv");
  /* if(!file){
     Serial.println("Failed to open file for reading");
     return;
    }*/

  while (file.available()) {

    inchar = file.read();

    /*    if (line > apollo_location) {
          apollo_location++;
          break;
        }
    */
    //    if (line == apollo_location) {
    if (inchar == '\n') {
      //new line
      //       Serial.println("-----");
//      line++;
      i = 0;
      break;
    } else if (inchar == '\t') {
      //new variable
      //       Serial.println("");
      i++;
      j = 0;
    } else {
      //data!
      //       Serial.print(inchar);
      switch (i) {
        case 0:
          id[j] = inchar;
          dim_id++;
          break;
        case 1:
          mission_time[j] = inchar;
          dim_mission_time++;
          break;
        case 2:
          comms[j] = inchar;
          dim_comms++;
          break;
        case 3:
          speaker[j] = inchar;
          dim_speaker++;
          break;
        default:
          Serial.println("case error");
          break;
      }
      j++;
    }
    //    }

    /*   if (line < apollo_location) {
         if (inchar == '\n') {
         //new line
         //Serial.println("-----");
         line++;
         //i = 0;
         }
       }
    */
  }
  //  file.close();
  next_event_time = atoi(mission_time);
}

void set_apollo_location() {
  char tmp_id[6];
  char tmp_mission_time[8];
  char tmp_comms[2220];  //longest expected line is 2212 chars
  char tmp_speaker[9];
  unsigned int tmp_dim_id = 0;
  unsigned int tmp_dim_mission_time = 0;
  unsigned int tmp_dim_comms = 0;
  unsigned int tmp_dim_speaker = 0;
  unsigned int tmp_next_event_time = 0;
  //line read until next_event_time > timeClient.getEpochTime() - offset_time
  Serial.print("Current clock time:\t");
  Serial.println(timeClient.getEpochTime());
  Serial.print("Clock offset time:\t");
  Serial.println(offset_time);
  Serial.print("Simulated mission time:\t     ");
  Serial.println(timeClient.getEpochTime() - offset_time);
  char inchar;
  unsigned int line = 0;
  unsigned int i = 0; // which variable we are looking at (0 = id, 1 = mission time, 2 = comms, 3 = speaker)
  unsigned int j = 0; // which character we are up to for that variable
  bool flag = 0;  // exit while loop once we're caught up
  bool flag2 = 0; // ignore first line
//  while (file.available() && next_event_time < timeClient.getEpochTime() - offset_time) {
  while (flag == 0) {
    inchar = file.read();
    if (inchar == '\n') {
      //new line
      line++;
      i = 0;
      tmp_next_event_time = atoi(tmp_mission_time);
      if (tmp_next_event_time > timeClient.getEpochTime() - offset_time) {
        if (flag2 == 0) {
          flag2 = 1;
        } else {
        Serial.print("tmp:\t");
        Serial.print(tmp_next_event_time);
        Serial.print("\ttarget: ");
        Serial.println(timeClient.getEpochTime() - offset_time);
        next_event_time = tmp_next_event_time;
        lineDisplay();
        
        flag = 1;
        }
      }
      clearChar(mission_time, 8);
      clearChar(id, 6);
      clearChar(comms, 2220);
      clearChar(speaker, 9);
      strncpy(id, tmp_id, tmp_dim_id);
      strncpy(mission_time, tmp_mission_time, tmp_dim_mission_time);
      strncpy(comms, tmp_comms, tmp_dim_comms);  //longest expected line is 2212 chars
      strncpy(speaker, tmp_speaker, tmp_dim_speaker);
      dim_id = tmp_dim_id;
      dim_mission_time = tmp_dim_mission_time;
      dim_comms = tmp_dim_comms;
      dim_speaker = tmp_dim_speaker;
      tmp_dim_id = 0;
      tmp_dim_mission_time = 0;
      tmp_dim_comms = 0;
      tmp_dim_speaker = 0;
      clearChar(tmp_mission_time, 8);
      clearChar(tmp_id, 6);
      clearChar(tmp_comms, 2220);
      clearChar(tmp_speaker, 9);

    } else if (inchar == '\t') {
      //new variable
      i++;
      j = 0;
    } else {
      //data!
      //       Serial.print(inchar);
      switch (i) {
        case 0:
          tmp_id[j] = inchar;
          tmp_dim_id++;
          break;
        case 1:
          tmp_mission_time[j] = inchar;
          tmp_dim_mission_time++;
          break;
        case 2:
          tmp_comms[j] = inchar;
          tmp_dim_comms++;
          break;
        case 3:
          tmp_speaker[j] = inchar;
          tmp_dim_speaker++;
          break;
        default:
          Serial.println("case error");
          break;
      }
      j++;
    }
  }
  Serial.println("******** exiting set_apollo_location");

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, -1);

  displayInit();

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  file = SPIFFS.open("/apollo.csv");

  wifiMulti.addAP("SSID_1", "PASS_1");
  //  wifiMulti.addAP("SSID_2", "PASS_2");
  //  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }


  timeClient.begin();
  timeClient.update();
  if (offset_time > timeClient.getEpochTime()) {
    showBootError("Offset time not valid");
    delay(2000);
    offset_time = timeClient.getEpochTime();
  } else {
    showStartupPage();
  }
  
  Serial.println("setup() complete");


 // Serial.println("clock\tspeaker\tcomms");
  set_apollo_location();
}



void loop() {
  // put your main code here, to run repeatedly:
  timeClient.update();

  if (next_event_time < timeClient.getEpochTime() - offset_time) {
    lineDisplay();
    lineRead();
  }

  delay(100);


  if (update_time + actual_time < millis()) {
    //   Serial.println(timeClient.getFormattedTime());
    Serial.print("Simulated Clock: ");
    Serial.print(timeClient.getEpochTime() - offset_time);
    Serial.print("\tNext Event: ");
    Serial.println(mission_time);
    actual_time = millis();
  }



}
