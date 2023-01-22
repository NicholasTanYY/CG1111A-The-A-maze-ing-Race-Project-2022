#include "MeMCore.h"

#define RGBWait 200 //in milliseconds: LDR response time
#define LDRWait 10 //in milliseconds: time btwn each LDR reading

int LDRpin = A0;
int sigA = A2;
int sigB = A3;

// Floats to hold colour arrays
float colourArray[] = {0, 0, 0};            // for individual colours
float whiteArray[] = {0, 0, 0};
float blackArray[] = {0, 0, 0};
float greyDiff[] = {0, 0, 0};
char colourStr[3][5] = {"R = ", "G = ", "B = "};

void colour(int i)
{
  // Y3 LOW - Turning the R LED on
  if (i == 0)
  {
    digitalWrite(A2, HIGH);
    digitalWrite(A3, HIGH);
  }

  // Y1 LOW - Turning the G LED on
  if (i == 1)
  {
    digitalWrite(A2, HIGH);
    digitalWrite(A3, LOW);
  }

  // Y2 LOW - Turning the B LED on
  if (i == 2)
  {
    digitalWrite(A2, LOW);
    digitalWrite(A3, HIGH);
  }

  // Y0 LOW - Turning the IR emitter on / switches off all LED
  if (i == 3)
  {
    digitalWrite(A2, LOW);
    digitalWrite(A3, LOW);
  }
}

// white and black callibration
void calibrate()
{
  // Gives user 5 seconds to put White paper into position
  Serial.println("Put White Sample For Calibration ...");
  delay(5000);

  // Turn on R, G and B one at a time and store values for white colour (get average of 5 readings)
  for (int i = 0; i <= 2; i++)
  {
    colour(i);
    delay(RGBWait);
    whiteArray[i] = getAvgReading(5);
    colour(3);                              // switches off all LED
    delay(RGBWait);
  }

  // Gives user 5 seconds to put Black paper into position
  Serial.println("Put Black Sample For Calibration ...");
  delay(5000);

  // Turn on R, G and B one at a time and store values for black colour (get average of 5 readings)
  for (int i = 0; i <= 2; i++)
  {
    colour(i);
    delay(RGBWait);
    blackArray[i] = getAvgReading(5);
    colour(3);                              // switches off all LED
    delay(RGBWait);

    // Calculate range for greyDiff
    greyDiff[i] = whiteArray[i] - blackArray[i];
  }

  // Prints to serial monitor the resultant white and black array
  Serial.println("White: ");
  for (int i = 0; i <= 2; i++)
  {
    Serial.println(whiteArray[i]);
  }
  Serial.println("Black: ");
  for (int i = 0; i <= 2; i++)
  {
    Serial.println(blackArray[i]);
  }
}

// returns average value from analogRead(LDRpin) from white and black callibration
int getAvgReading(int times)
{
  int reading;
  int total = 0;

  for (int i = 0; i < times; i++)
  {
    reading = analogRead(LDRpin);           // returns value from 0 (0V) to 1023 (5V)
    total = reading + total;
    delay(LDRWait);
  }

  return total / times;
}

void setup()
{
  //begin serial communication
  Serial.begin(9600);

  //setup the outputs for the colour sensor
  pinMode(sigA, OUTPUT);
  pinMode(sigB, OUTPUT);

  calibrate();                              // white and black calibration before loop()
}

void loop()
{
  // individual colour calibration
  for (int c = 0; c <= 2 ; c++)
  {
    Serial.print(colourStr[c]);
    colour(c);                              // switches on 1 LED in each loop
    delay(RGBWait);
    // get the average of 5 consecutive readings for the current colour and return an average
    colourArray[c] = getAvgReading(5);
    // the average reading returned minus the lowest value divided by the maximum possible range, multiplied by 255 will give a value between 0-255, representing the value for the current reflectivity (i.e. the colour LDR is exposed to)
    colourArray[c] = (colourArray[c] - blackArray[c]) / (greyDiff[c]) * 255;
    colour(3);                              // switches off LED
    delay(RGBWait);

    Serial.println(int(colourArray[c]));    // show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
  }
}