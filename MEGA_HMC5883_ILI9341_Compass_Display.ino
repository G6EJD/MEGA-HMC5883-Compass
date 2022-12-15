/*
  This software, the ideas and concepts is Copyright (c) David Bird 2021 and beyond.
  All rights to this software are reserved.
  It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
  5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies
  or substantial portions of the software and where the software use is visible to an end-user.
  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
  FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/   

(c) D BIRD 2013
An Arduino code example for interfacing with the HMC5883
Uses:
Analog input 4 I2C SDA or equivlanet for MEGA
Analog input 5 I2C SCL
*/
#include <SPI.h>
#include <Wire.h> //I2C Arduino Library

const int centreX  = 160;
const int centreY  = 120;
const int diameter = 100; 

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analogue 3
#define LCD_CD A2 // Command/Data goes to Analogue 2
#define LCD_WR A1 // LCD Write goes to Analogue 1
#define LCD_RD A0 // LCD Read goes to Analogue 0

#define LCD_RESET A4 // Can alternately just connect to Arduino reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
//   Assign human-readable names to some common 16-bit colour values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define address 0x1E //0011110b, I2C 7bit address of HMC5883
const int x_offset = 30;
const int y_offset = 128;
const int z_offset = 0;
int last_dx, last_dy, dx, dy;

// Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
Adafruit_TFTLCD tft;

void setup(){
  tft.reset();
  uint16_t identifier = tft.readID(); // Found ILI9325 LCD driver
  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  //Initialize I2C communications
  Wire.begin();
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();
  delay(300); // Slight delay for screen to start
  last_dx = centreX;
  last_dy = centreY;
}

void loop(){
  double angle;
  int x,y,z; //triple axis data
  //Tell the HMC5883 where to begin reading data
  Wire.beginTransmission(address);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
 
 //Read data from each axis, 2 registers per axis
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    x = Wire.read() << 8 | Wire.read();
    z = Wire.read() << 8 | Wire.read();
    y = Wire.read() << 8 | Wire.read();
  }
  Draw_Compass_Rose(); 
  angle= atan2((double)y + y_offset,(double)x + x_offset)* (180 / 3.141592654) + 180; 
  dx = (diameter * cos((angle-90)*3.14/180)) + centreX;    // calculate X position 
  dy = (diameter * sin((angle-90)*3.14/180)) + centreY;    // calculate Y position 
  arrow(last_dx,last_dy, centreX, centreY, 20, 20,BLACK);  // Erase last arrow      
  arrow(dx,dy, centreX, centreY, 20, 20,CYAN);             // Draw arrow in new position
  last_dx = dx; 
  last_dy = dy;
  delay(25);
}

void display_item(int x, int y, String token, int txt_colour, int txt_size) {
  tft.setCursor(x, y);
  tft.setTextColor(txt_colour);
  tft.setTextSize(txt_size);
  tft.print(token);
  tft.setTextSize(2); // Back to default text size
}

void arrow(int x2, int y2, int x1, int y1, int alength, int awidth, int colour) {
  float distance;
  int dx, dy, x2o,y2o,x3,y3,x4,y4,k;
  distance = sqrt(pow((x1 - x2),2) + pow((y1 - y2), 2));
  dx = x2 + (x1 - x2) * alength / distance;
  dy = y2 + (y1 - y2) * alength / distance;
  k = awidth / alength;
  x2o = x2 - dx;
  y2o = dy - y2;
  x3 = y2o * k + dx;
  y3 = x2o * k + dy;
  //
  x4 = dx - y2o * k;
  y4 = dy - x2o * k;
  tft.drawLine(x1, y1, x2, y2, colour);
  tft.drawLine(x1, y1, dx, dy, colour);
  tft.drawLine(x3, y3, x4, y4, colour);
  tft.drawLine(x3, y3, x2, y2, colour);
  tft.drawLine(x2, y2, x4, y4, colour);
} 

void Draw_Compass_Rose() {
  int dxo, dyo, dxi, dyi;
  tft.drawCircle(centreX,centreY,diameter,YELLOW);  // Draw compass circle
  for (float i = 0; i <360; i = i + 22.5) {
    dxo = diameter * cos((i-90)*3.14/180);
    dyo = diameter * sin((i-90)*3.14/180);
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    tft.drawLine(dxi+centreX,dyi+centreY,dxo+centreX,dyo+centreY,YELLOW);   
  }
  display_item((centreX-5),(centreY-85),"N",RED,2);
  display_item((centreX-5),(centreY+70),"S",RED,2);
  display_item((centreX+80),(centreY-5),"E",RED,2);
  display_item((centreX-85),(centreY-5),"W",RED,2);
}

