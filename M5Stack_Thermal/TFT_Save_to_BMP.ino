// From DaveEvans https://forum.arduino.cc/index.php?topic=406416.0
// Creates a bitmap (BMP) snapshot of the Adafruit 3.5" screen with HX8357D controller
// The BMP file has 2 bytes per pixel, RGB555 format
//
// The readPixel function was written by David Evans, June 2016
//
// The BMP writing function is a mashup of code from here:
// http://forum.arduino.cc/index.php?topic=177361.0
// http://forum.arduino.cc/index.php?topic=112733.0


/*  Used for testing:
 *   
 *  Magenta: 11111 000000 11111
 *  DkGreen: 00011 011111 00000
 *  Yellow:  11111 111111 00000
 *  Grey:    11011 111000 11100
 *  Red:     11111 000000 00000
 *  LtRed:   11111 110011 11001
 *  Black:   00000 000000 00000
 *  White:   11111 111111 11111
 *  
 *  const unsigned int GREY = 0xDF1C;
 *  const unsigned int LTRED = 0xFE79;
 *  const unsigned int DKGREEN = 0x1BE0;
 */

#include <SPI.h>
#include <SdFat.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

const byte TFT_DC = 9;
const byte TFT_CS = 10;
const byte TFT_CLK = 13;

const byte SD_CS = 4;

const int w = 480;     // image width in pixels
const int h = 320;     // height

char str[] = "TEST11.BMP";

const unsigned int GREY = 0xDF1C;
const unsigned int LTRED = 0xFE79;
const unsigned int DKGREEN = 0x1BE0;

SdFat SD;
File outFile;

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC);

void setup()
{
  tft.begin(HX8357D);

  // print some objects on TFT to be captured in BMP
  tft.setRotation(3);
  tft.setTextSize(3);
  
  tft.fillScreen(HX8357_BLUE);
  
  tft.drawPixel(13,3,HX8357_WHITE);
  tft.drawPixel(14,3,HX8357_WHITE);
  tft.drawPixel(15,3,HX8357_WHITE);
  
  tft.drawLine(0,1,100,200,HX8357_YELLOW);
  tft.drawLine(0,2,100,202,HX8357_YELLOW);
  tft.drawLine(0,3,100,203,HX8357_YELLOW);
  
  tft.fillRect(150, 150, 30, 30, GREY);
  tft.fillRect(30, 150, 30, 30, LTRED);
  tft.fillRect(150, 30, 30, 30, DKGREEN);
  
  tft.setCursor(50, 50);
  tft.print("hello world");
  // end test print to TFT
  
  Serial.begin(115200);
  Serial.println("starting");
  
  //init SD Card
  if (!SD.begin(SD_CS))
  {
    Serial.println("err strtng SD");
    while (1);    //If failed, stop here
  }

  Serial.println("working");
  GrabImage(str);
  Serial.println("done");
  
  tft.setCursor(100, 100);
  tft.print("Done");
}

void GrabImage(char* str)
{
  byte VH, VL;
  int i, j = 0;

  //Create the File
  outFile = SD.open(str, FILE_WRITE);
  if (! outFile) {
    Serial.println("err opng file");
    return;
  };

  unsigned char bmFlHdr[14] = {
    'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0
  };
  // 54 = std total "old" Windows BMP file header size = 14 + 40
  
  unsigned char bmInHdr[40] = {
    40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 0
  };   
  // 40 = info header size
  //  1 = num of color planes
  // 16 = bits per pixel
  // all other header info = 0, including RI_RGB (no compr), DPI resolution

  unsigned long fileSize = 2ul * h * w + 54; // pix data + 54 byte hdr
  
  bmFlHdr[ 2] = (unsigned char)(fileSize      ); // all ints stored little-endian
  bmFlHdr[ 3] = (unsigned char)(fileSize >>  8); // i.e., LSB first
  bmFlHdr[ 4] = (unsigned char)(fileSize >> 16);
  bmFlHdr[ 5] = (unsigned char)(fileSize >> 24);

  bmInHdr[ 4] = (unsigned char)(       w      );
  bmInHdr[ 5] = (unsigned char)(       w >>  8);
  bmInHdr[ 6] = (unsigned char)(       w >> 16);
  bmInHdr[ 7] = (unsigned char)(       w >> 24);
  bmInHdr[ 8] = (unsigned char)(       h      );
  bmInHdr[ 9] = (unsigned char)(       h >>  8);
  bmInHdr[10] = (unsigned char)(       h >> 16);
  bmInHdr[11] = (unsigned char)(       h >> 24);

  outFile.write(bmFlHdr, sizeof(bmFlHdr));
  outFile.write(bmInHdr, sizeof(bmInHdr));

  for (i = h; i > 0; i--) {
    for (j = 0; j < w; j++) {

      uint16_t rgb = readPixA(j,i); // get pix color in rgb565 format
      
      VH = (rgb & 0xFF00) >> 8; // High Byte
      VL = rgb & 0x00FF;        // Low Byte
      
      //RGB565 to RGB555 conversion... 555 is default for uncompressed BMP
      //this conversion is from ...topic=177361.0 and has not been verified
      VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
      VH = VH >> 1;
      
      //Write image data to file, low byte first
      outFile.write(VL);
      outFile.write(VH);
    }
  }
  //Close the file
  outFile.close();
}

void loop()
{
}

uint16_t readPixA(int x, int y) { // get pixel color code in rgb565 format

  tft.setAddrWindow(x,y,x,y);

  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_CLK, LOW);
  digitalWrite(TFT_CS, LOW);
  tft.spiwrite(0x2E); // memory read command

  digitalWrite(TFT_DC, HIGH);

  uint16_t r = 0;
  r = tft.spiread(); // discard dummy read
  r = tft.spiread() >> 3; // red: use 5 highest bits (discard three LSB)
  r = (r << 6) | tft.spiread() >> 2; // green: use 6 highest bits (discard two LSB)
  r = (r << 5) | tft.spiread() >> 3; // blue: use 5 highest bits (discard three LSB)

  digitalWrite(TFT_CS, HIGH);

  return r;
}

