// From https://web.archive.org/web/20150627201205/http://diffusionht.blogspot.cz/2011/03/thermal-cam-arduino.html
// And https://github.com/xhresko/thermocam

/*
* All dates in this workbook are in the format yyyy-mm-dd, following the
* ISO 8601 standard.
*
* We must use the SD card (~2GB) as our RAM. Slow, but nothing compared to
* servos.
* -2013-06-10
*
* Currently, we have to wait about 32ms for the servo to move 1 pixel. Could be
* better with stepper motors (see stepper motors.txt, in ~/Dropbox/nvNotes
* -2013-06-12
*
*/

#include <SD.h>
#include <i2cmaster.h>
#include <Servo.h>

// Change the below values if you want to change anything (well, simple things).
// the width of the image (in vertical and horizontal directions)
// can not be greater than 170, the servo motion range.
#define ANGLE 45

// defining width and height.
#define WIDTH 25
#define HEIGHT 25

#define DEV 0x00
//(0x5A << 1)


// the constants that determines the width of blue-red
#define A 2.4e-1f
#define CENTER 30.0f
#define K 1.0f // since B taken in RGB

// * how many milliseconds to wait after each pixel
// * be warned the Serial.print takes a significant
// amount of time (on the order of milliseconds)
// * depends on settling time of the IR sensor (60ms is the fastest time)
#define WAIT_MS 37

#define DEBUG_PRINT 1
// e
#define E 2.718281828459045f



// must change from 4 to 8! (8 on sparkfun shield)
const int chipSelect = 8;

// pin that the SD is connected to (8 for SparkFun MicroSD shield, 4 for others )
const uint8_t cardPin = 8;

// our servos
Servo horizServo;
Servo vertServo;

// the number of digits in height and width
int NUMBER_OF_DIGITS = log10(WIDTH) + 1;

void I2CWrite(int adress, unsigned int LSB, unsigned int MSB, int PEC){
  i2c_start_wait(DEV+I2C_WRITE);
  i2c_write(adress);
  i2c_write(LSB);
  i2c_write(MSB);
  i2c_write(PEC);
  i2c_stop();
  delay(100);
}



// A method to read values from a EEPROM adress
unsigned int I2CRead(int adress){
  
  i2c_start_wait(DEV+I2C_WRITE);
  i2c_write(adress);
  i2c_rep_start(DEV+I2C_READ);
  unsigned int LSB = i2c_readAck();
  unsigned int MSB = i2c_readAck();
  unsigned int pec = i2c_readNak();
  
  // Serial.print("in I2CRead -- PEC: 0x");
  // Serial.println(pec, HEX);
  i2c_stop();
  
  unsigned int regValue = (((MSB) << 8) + LSB);
  
  // Serial.println();
  // Serial.print(MSB, HEX); Serial.println(LSB, HEX);
  // Serial.println(regValue, HEX);
  return regValue;
  delay(100);
  
}
void setup(){
    // setting up the serial port
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Start...");

    // setting up the output pins
    pinMode(10, OUTPUT);
    pinMode(9, OUTPUT);
    
    // if the SD card isn't there, return (don't do the rest of the function)
    if (!SD.begin(chipSelect)){
        Serial.println("SD?");
        return;
    }

    // setting up the IR
    setupIR();


    int width = WIDTH;
    int height = HEIGHT;
    
    // maximum temp settings
    I2CWrite(0x20, 0x00, 0x00, 0x43); delay(100);
    I2CWrite(0x20, 0x23, 0xFF, 0x21);
    delay(1000);
    unsigned int data = I2CRead(0x20);
    Serial.print("data (0xFF23?), in setup: 0x");
    Serial.println(data & 0xFFFF, HEX);
    // min temp settings
    I2CWrite(0x21, 0x00, 0x00, 0x28); delay(100);
    I2CWrite(0x21, 0x5B, 0x4F, 0x59);
    delay(1000);
    data = I2CRead(0x21);
    Serial.print("data (0x4F5B?), in setup: 0x");
    Serial.println(data & 0xFFFF, HEX);
    
    // writing the shortest possible delay time (at least, in the datasheet!)...
    I2CWrite(0x25, 0x00, 0x00, 0x83);
    delay(1000);
    data = I2CRead(0x25);
    Serial.print("data (all zeros?), in setup: 0x");
    Serial.println(data & 0xFFFF, HEX);
    
    I2CWrite(0x25, 0x74, 0xB4, 0x70);
    delay(1000);
    // according to cheap-thermocam, that must be working
    // now, let's read it
    data = I2CRead(0x25);
    delay(500);
    Serial.print("data (0xbx7x?), in setup: 0x");
    Serial.println(data & 0xFFFF, HEX);
//
// fromOnline();
    
    
    // printing the free RAM
    Serial.print("\n\nFree RAM:\n");
    Serial.println(freeRam());
    Serial.println("-----");
     
    Serial.println(NUMBER_OF_DIGITS);
    
    horizServo.attach(10);
    vertServo.attach(9);
    horizServo.write(ANGLE/2);
    vertServo.write(ANGLE/2);
    takePicture(width, height);
}

void loop(){}




unsigned char readPixel(long int i, char * filename){
    // TODO: change to work with NUMBER_OF_DIGITS
    // TODO: get rid of SD.open()
    File dataFile = SD.open(filename);
    dataFile.seek(11 + 3*i);
    
    unsigned char R = dataFile.read(); // R
    unsigned char G = dataFile.read(); // G
    unsigned char B = dataFile.read(); // B
    
    if(B == 255){
        return G/3;
    } else if (G == 255){
        return 255 - B/3;
    } else if (R == 255){
        return 255 - G/3;
    }

    return -1;
    // for error...
  
}
void writePixel2(long int i, File dataFile, unsigned char x){
    // assumes file.open() has been called earlier
    // assumes file.close() will be called later
        unsigned char R, G, B;
    if(x <= 85){
        R = 0;
        G = 3*x;
        B = 255;
      
    } else if (x <= 170){
        R = 3 * (x - 85);
        G = 255;
        B = 255 - 3*(x - 85);
      
    } else if (x <= 255){
        R = 255;
        G = 255 - 3*(x - 170);
        B = 0;
    } else Serial.println("writePixel: error!");
    
    boolean as = dataFile.seek(9 + 3*i + 2*NUMBER_OF_DIGITS);;
    dataFile.write(R);
    dataFile.write(G);
    dataFile.write(B);
}


float readTemp(int address, int O_PRINT){
    // for this specific IR sensor. change if using a different one.
    int dev = 0x5A << 1;
    int data_low;
    int data_high;
    int pec;
    
    // from the comment by Sensorjunkie at http://forum.arduino.cc/index.php/topic,21317.0.html
    i2c_start_wait(dev + I2C_WRITE);
    i2c_write(address);
    i2c_rep_start(dev+I2C_READ);
    data_low = i2c_readAck();
    data_high = i2c_readAck();
    pec = i2c_readNak();
    i2c_stop();
    
    double tempFactor = 0.02;
    float temp=0;
    
    // from the datasheet
    unsigned int data = (data_high << 8) + data_low;
    data = data & 0xFFFF;
    
    //data = 0x3AF7;
    if (DEBUG_PRINT && O_PRINT == 1){
      Serial.print(" data == ");
      Serial.print(data, HEX);
    }
    temp = data * 0.02 - 273.15;
    return temp;
}

unsigned char * takePicture(int width, int height){
    // assumes width, height of 11, 11
    //
    int xx, yy;
    int i;
    float ta, t1, t2;
    int raw1, raw2;
    
    int HORIZPIN = 9;
    int VERTPIN = 10;
    char * name = initPPM(width, height);
    File file = SD.open(name, FILE_WRITE);
    Serial.print("in takePicture, printing \n");
    Serial.println(name);
    Serial.println();
    
    gotoPixel(0, 0, horizServo, vertServo, width, height);
    for (i=0; i<100; i++){
        raw1 = readTemp(0x05, 0);
        raw2 = readTemp(0x06, 0);
        ta = readTemp(0x06, 0);
        t1 = readTemp(0x07, 0);
        t2 = readTemp(0x08, 0);
 
        if(DEBUG_PRINT){
            Serial.print(" ta: ");
            Serial.print(ta);
            Serial.print(" tobj1: ");
            Serial.print(t1);
            Serial.print(" tobj2: ");
            Serial.print(t2);
            Serial.print(" raw1: 0x");
            Serial.print(raw1 & 0xFFFF, HEX);
            Serial.print(" raw2: 0x");
            Serial.print(raw2 & 0xFFFF, HEX);
            Serial.println();
        }
            
        delay(60);
    }

    for (yy=0; yy<width; yy++){
        
        Serial.println(yy);
        for (xx=0; xx<height; xx++){
              int horizPixel;
              
              if (yy%2 == 0) horizPixel = xx;
              if (yy%2 == 1) horizPixel = xx;//width - xx - 1;

              
              gotoPixel(horizPixel, yy, horizServo, vertServo, width, height);
              if (xx == 0) delay(500);
              
              delay(WAIT_MS);

              float temp = readTemp(0x07, 1);
              
              if (DEBUG_PRINT){
                  Serial.print(" row: ");
                  Serial.print(yy);
                  Serial.print(" temp: ");
                  Serial.print(temp);
                  Serial.print(" ");
              }

              // we're taking temperature values between -20 and 108
              temp = 255 * 1 / (1 + K*pow(E, -(temp-CENTER) * A));
            if(DEBUG_PRINT){
                Serial.print("color: ");
                Serial.print((unsigned char)temp);
                Serial.print("\n");
            }
            writePixel2(horizPixel + yy*width, file, (unsigned char)temp);
        }
        
    }
    file.close();
    delay(1000);
    horizServo.write(ANGLE/2);
    vertServo.write(ANGLE/2);
    Serial.println("Done");
}



void gotoPixel(int x, int y, Servo horizServo, Servo vertServo, int height, int width){
    // assumes horizServo and vertServo have called .attach(pin) before
    // assumes angle of 45 degrees
    int angle = 45;
    unsigned char horizAngle = 1.0 * x * angle / width;
    unsigned char vertAngle = 1.0 * y * angle / height;
   
    // assumes that the library handles the wait (or, don't handle it in this function)
    horizServo.write(horizAngle);
    vertServo.write(vertAngle);
    if(DEBUG_PRINT){
      Serial.print(" horizAngle: ");
      Serial.print(horizAngle);
    }

}


void setupIR(){
    i2c_init();
    
    // enabling pullups
    PORTC = (1 << PORTC4) | (1 << PORTC5);
}

char * initPPM(int width, int height){
    Serial.println("\nIn initPPM");
    long long int i;
    //char filename[] = "ir_00000.ppm";
    // malloc, since we want to return the address (and not a local)
    char * filename = (char *)malloc(sizeof(char)*(8+1+3));
    filename = "ir_00000.ppm";
    // auto increment name if file already exists

      // if name exists, create new filename
  for (int i=0; i<10000; i++) {
    filename[4] = (i/1000)%10 + '0'; // thousands place
    filename[5] = (i/100)%10 + '0'; // hundreds
    filename[6] = (i/10)%10 + '0'; // tens
    filename[7] = i%10 + '0'; // ones
    if (!SD.exists(filename)) {
      if (DEBUG_PRINT) Serial.println("Did the if");
      break;
    }
  }
    
    // opening the file, deleting if there's already one
    File dataFile;
    if(SD.exists(filename)) SD.remove(filename);
    dataFile = SD.open(filename, FILE_WRITE);
    
    dataFile.print("P6");
    // specifing a PPM binary RGB file

    //dataFile.write("\n");
    dataFile.print(" ");
    dataFile.print(width);
    dataFile.print(" ");
    dataFile.print(height);
    dataFile.print(" ");
    //dataFile.print(0xFF);
    dataFile.print(255);
    dataFile.write(0x0A);

    if(DEBUG_PRINT){
        Serial.println("\nPosition()");
        Serial.println(dataFile.position());
        Serial.println("end position()\n");
    }
    int32_t length = 1;
    // since we're printing R, G, and B values in one loop
    length = length * width;
    length = length * width;
    Serial.println(length);
   
    
    for (i=0; i<length; i++){
        //Serial.write(i);
        unsigned char colorVal = 115;
        dataFile.write(colorVal);
        dataFile.write(colorVal);
        dataFile.write(colorVal);
    }
    dataFile.close();
    
    Serial.println("initPPM, done");
    // return the filename written?
    return filename;
}



























// -----------------------------------------------------------------------------
// ------------------- below or "writeImage" functions of various complexity ---
// -----------------------------------------------------------------------------

void writePPMImage(unsigned char * data, int w, int h, char filename[]){
    // assumes that 0 < data[i] < 255
  
    // the file where we're going to write to
    File dataFile;
    char name[] = "test.ppm";
    int height = h;
    int width = w;
    // we can't open a file that already exists...
    if(SD.exists(name)) SD.remove(name);
    dataFile = SD.open(name, FILE_WRITE);
    
    // P3 means RGB color in the PPM format
    dataFile.println("P3");
    
    // width and height
    dataFile.print(w); dataFile.print(" ");
    dataFile.println(h);
    
    // setting the max color
    dataFile.println(255);
    
    int xx, yy;
    for (yy=0; yy<height; yy++){
        for (xx=0; xx<width; xx++){
            int R, G, B, C;
            C = data[yy*width + xx];
            Serial.print(C);
            Serial.write(C);
            Serial.println(" ");
            R = C;
            G = C;
            B = C; // change R,G,B to a red-blue colormap later
            dataFile.print(R); // R
            dataFile.print(" ");
            
            dataFile.print(G); // G
            dataFile.print(" ");
            
            dataFile.print(B); // B
            dataFile.print(" ");
        }
        dataFile.print("\n");
    }
    dataFile.close();
  
}

void writeBMPImage(unsigned char * input, char fileName[], int w, int h){
    // Works, but only for small (about 10x10) images. I'm saying "screw it" and
    // encoding the image in the PPM format -- see writePPMImage instead.
    
    //Serial.write("width == "); Serial.println(w);
    //Serial.write("height == "); Serial.println(h);
    // SD setup
    // as of now, this function does NOT write to the filename. Instead, it writes to
    // "toDebug.bmp"
    
    //Serial.begin(9600);
    //if (!sd.init(SPI_FULL_SPEED, cardPin)) {
    // sd.initErrorHalt();
    // Serial.println("---");
    //}
    char name[] = "bur.bmp";
    Serial.print("In the writeBMPImage, writing ");
    Serial.write(name);
    Serial.write("\n\n");
   
    

    // DEBUG: make it so you can pass a name in...
    
    File dataFile;
    // we can't open a file that already exists...
    if(SD.exists(name)) SD.remove(name);
    dataFile = SD.open(name, FILE_WRITE);
    
    //while(!dataFile);
    Serial.write("dataFile == ");Serial.println(dataFile);
    // set fileSize (used in bmp header)
    int rowSize = 4 * ((3*w + 3)/4); // how many bytes in the row (used to create padding)
    int fileSize = 54 + h*rowSize; // headers (54 bytes) + pixel data
    Serial.write("fileSize == "); Serial.println(fileSize);
    Serial.write("rowSize == "); Serial.println(rowSize);
    // create image data; heavily modified version via:
    // http://stackoverflow.com/a/2654860
    unsigned char *img = NULL; // image data
    if (img) {
      // if there's already data in the array, clear it
        free(img);
    }
    img = (unsigned char *)malloc(3*w*h);

    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            int colorVal = input[y*w + x]; // classic formula for px listed in line
            //Serial.println(colorVal);
            img[(y*w + x)*3+0] = (unsigned char)(colorVal); // R
            img[(y*w + x)*3+1] = (unsigned char)((colorVal)); // G
            img[(y*w + x)*3+2] = (unsigned char)(colorVal); // B
            // padding (the 4th byte) will be added later as needed...
        }
    }

    // print px and img data for debugging
    if (0) {
        Serial.print("\nWriting \"");
        Serial.print("\"");Serial.print(name); Serial.print("\" ");
        Serial.print("\" to file...\n");
        for (int i=0; i<w*h; i++) {
            Serial.print(input[i]);
            Serial.print(" ");
        }
    }

    // create padding (based on the number of pixels in a row
    //unsigned char bmpPad[rowSize - 3*w];
    unsigned char * bmpPad = (unsigned char *)malloc(sizeof(unsigned char) * (rowSize - 3*w));
    for (int i=0; i<rowSize-3*w; i++) {
        bmpPad[i] = 0;
    }

    // create file headers (also taken from StackOverflow example)
    unsigned char bmpFileHeader[14] = { // file header (always starts with BM!)
        'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
    unsigned char bmpInfoHeader[40] = { // info about the file (size, etc)
        40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };

    bmpFileHeader[ 2] = (unsigned char)(fileSize );
    bmpFileHeader[ 3] = (unsigned char)(fileSize >> 8);
    bmpFileHeader[ 4] = (unsigned char)(fileSize >> 16);
    bmpFileHeader[ 5] = (unsigned char)(fileSize >> 24);

    bmpInfoHeader[ 4] = (unsigned char)( w );
    bmpInfoHeader[ 5] = (unsigned char)( w >> 8);
    bmpInfoHeader[ 6] = (unsigned char)( w >> 16);
    bmpInfoHeader[ 7] = (unsigned char)( w >> 24);
    bmpInfoHeader[ 8] = (unsigned char)( h );
    bmpInfoHeader[ 9] = (unsigned char)( h >> 8);
    bmpInfoHeader[10] = (unsigned char)( h >> 16);
    bmpInfoHeader[11] = (unsigned char)( h >> 24);
    
    //Serial.write(bmpFileHeader, sizeof(bmpFileHeader));
    //Serial.println(*bmpInfoHeader);
    // write the file (thanks forum!)
    dataFile.write(bmpFileHeader, sizeof(bmpFileHeader));
    delay(100);
    dataFile.write(bmpInfoHeader, sizeof(bmpInfoHeader));
    delay(100);
    //file.write(bmpFileHeader, sizeof(bmpFileHeader)); // write file header
    //file.write(bmpInfoHeader, sizeof(bmpInfoHeader)); // " info header

    for (int i=0; i<h; i++) { // iterate image array
        dataFile.write(img+(w*(h-i-1)*3), 3*w);
        delay(200);
        //file.write(img+(w*(h-i-1)*3), 3*w);
        // write px data
        dataFile.write(bmpPad, (4-(w*3)%4)%4);
        // and padding as needed
        delay(200);
    }
    dataFile.close(); // close file when done writing

    if (0) {
        Serial.print("\n\n---\n");
    }
}



void writePlainTextImage(int * pix, int w, int h){
    int x;
    int y;
    
    File file = SD.open("wxn.txt");
    
    for(y=0; y<h; y++){
      for (x=0; x<w; x++){
        file.print(x); file.write(",");
      }
    }
    file.close();
    
  
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
