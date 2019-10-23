#include <SPI.h>
#include <SandTimer.h>
#include <millisDelay.h>
#include <Adafruit_GPS.h>   //Libraries
#define GPSSerial Serial
Adafruit_GPS GPS(&GPSSerial);
SandTimer timer1;
#define GPSECHO false
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8X8_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8X8(U8X8_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
uint32_t timer = millis();
const int chipSelect = 4;
float ms = 0; //Meters/Second variable
float avems = 0;  //Average M/S variable
float mscounter = 0;  //Speed total (for average)
int integcounter = 0; //Counts half-seconds
float meterstraveled = 0; //Total meters
int splittotal = 0;   //Split total (in seconds)  (Essential to be an integer)
int splitseconds = 0; //Seconds on split display  (Essential to be an integer)
int splitminutes = 0; //Minutes on split display  (Essential to be an integer)
int seconds = 0;
int minutes = 0;
int hours = 0;    //Self-explanatory (for timers)
int splitaveragetotal = 0; //All split seconds added together (for average split)
int splitaverageseconds = 0; //Split average seconds
int splitaverageminutes = 0; //Split average minutes
int splitaverage = 0; //Split average
int splitaveragesecondsdisp = 0; //Display variables, fixes a bug
int splitaverageminutesdisp = 0;
float splitmdisp = 0;
float splitsdisp = 0;  //Display Variables
int tc = 0; //Counter variable
int dp = 0; //Data-point variable
int dpave = 0; //Average of the data-points
int dprec = 0; //Counter variable
float knot = 0;//Speed in knots recorded from GPS
float aveknots = 0;  //Speed in knots averaged over 1/2 second (equal to 1 data point and 5 knot readings)
int knotcounter = 0; //Counter variable
int sensorValue = analogRead(A3); //For the data write switch
float msfm = 0; //M/S for the meters calculation

millisDelay secdelay;   //Timing delay (for timer)



void setup() {
  float voltage = sensorValue * (5.0 / 1023.0);   //Converts A3 reading into a 5v equivalent
  Wire.begin();
  Wire.setClock(400000L);
  Serial.begin(115200);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);    //GPS initialization
 U8X8.begin();
  GPSSerial.println(PMTK_Q_RELEASE);
  int realhours = GPS.hour - 8;
  if (realhours < 0) {
    realhours = realhours + 24; //For SD timestamp (24 hour format)
  }
}

const char s[] PROGMEM = "Progmem";
const char s1[] PROGMEM  = "ABC" ;
const char s2[] PROGMEM  = "DEF" ;
const char s3[] PROGMEM  = "GHI" ;
const char s4[] PROGMEM  = "SMT" ;

const char *a[] = { s1, s2, s3, s4 };
void loop() {

  sensorValue = analogRead(A3);
  float voltage = sensorValue * (5.0 / 1023.0);

  if (secdelay.justFinished()) {
    seconds++;  //Checks if 1 second has passed, updates seconds variable
    secdelay.repeat();
  }
  if (seconds == 60) {
    seconds = 0;  //Checks if 60 seconds have passed, resets seconds, increases minutes
    minutes++;
  }
  if (minutes == 60) {
    minutes = 0;  //Checks if 60 minutes have passed, resets minutes, increases hours
    hours++;
  }
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);
  if (GPS.newNMEAreceived()) {
    Serial.println(GPS.lastNMEA());
    if (!GPS.parse(GPS.lastNMEA()));  //Serial GPS info for debugging
  }

  if (timer > millis()) timer = millis();   //Restarts main timer


  if (timer1.finished()) {
    knot = knot + GPS.speed; knotcounter++;
    timer1.startOver();
  }

  if (millis() - timer > 500) {   //Runs every half-second (responsible for screen-flicker)
    timer = millis(); // Resets timer
    if (GPS.fix) {

      aveknots = knot / knotcounter;
      Serial.println(aveknots);
      ms = (aveknots * 0.5144444); //Converts knots to meters/second
      Serial.println(ms);
      knot = 0;
      tc = 0;
      knotcounter = 0;
      aveknots = 0;
      if (ms <= 0.75) {
        U8X8.clearBuffer();         // clear the internal memory
        U8X8.setFont(U8X8_font_courR08_tr); // choose a suitable font
        U8X8.drawStr(0, 10, "Error: IDLE"); // write something to the internal memory
        .drawStr(1, 10, "Battery: ");
        U8X8.setFont(U8X8_font_courR08_tr);
        U8X8.setCursor(1, 80);
        U8X8.drawStr(2, 10, "Meters:");
        U8X8.setFont(U8X8_font_courR08_tr);
        U8X8.setCursor(2, 80);
        U8X8.print(meterstraveled);


        U8X8.sendBuffer();          // transfer internal memory to the display






        if (ms > 0.75)
        {

          msfm = ms;
          mscounter = (mscounter + msfm); //Continually adds speed to total (for averages)
          integcounter = (integcounter + 1); //Adds +1 to variable every half second (for averages)
          avems = mscounter / integcounter; //Generates average meters/second (for total)
          meterstraveled = avems * (integcounter / 2); //Calculates total (meters/second divided by seconds)
          meterstraveled = round(meterstraveled);
          splittotal = 500 / (ms); //Generates seconds per 500m
          dp = dp + splittotal;  //Adds data-points for dpave variable
          dprec++;
          if (dprec >= 6) {
            dpave = dp / 6; //Calculates average
            dprec = 0;
            dp = 0; //Resets data-points and counter variable


            splitminutes = dpave / 60; //Generates split minutes from split seconds
            splitseconds = dpave - (splitminutes * 60); //Displays remaining seconds
            splitaveragetotal = splitaveragetotal + splittotal; //All split times (in seconds) added together
            splitaverage = splitaveragetotal / (integcounter); //Averages out split times w/ seconds
            splitaverageminutes = splitaverage / 60; //Calculates split minutes
            splitaverageseconds = splitaverage - splitaverageminutes * 60; //Calculates split seconds
            splitaverageminutesdisp = splitaverageminutes;
            splitaveragesecondsdisp = splitaverageseconds;
            splitmdisp = splitminutes;
            splitsdisp = splitseconds;      //Display variables
            U8X8.clearBuffer();          // clear the internal memory
            U8X8.setFont(U8X8_font_courR08_tr); // choose a suitable font --------------------CHANGE THIS FONT TO SOMETHING LARGER
            U8X8.drawStr(0, 10, "M:");
            U8X8.setFont(U8X8_font_courR08_tr);

            U8X8.setCursor(0, 25);
            U8X8.setFont(U8X8_font_courR08_tr);
            U8X8.print(meterstraveled);
            U8X8.drawStr(1, 10, "S:");

            U8X8.setFont(U8X8_font_courR08_tr);
            U8X8.setCursor(1, 25);
            U8X8.print(splitmdisp);
            U8X8.drawStr(1, 50, ":");

            U8X8.setFont(U8X8_font_courR08_tr);
            U8X8.setCursor(1, 75);
            U8X8.print(splitsdisp); //adjust pixels to get a good display
            U8X8.sendBuffer();          // transfer internal memory to the display

          }
        }
      }
    }
  }
}
