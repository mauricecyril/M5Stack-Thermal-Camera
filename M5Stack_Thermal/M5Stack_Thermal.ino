/***************************************************************************
    M5Stack Thermal Camera
    Repo: https://github.com/m600x/M5Stack-Thermal-Camera
    Forked from: https://github.com/hkoffer/M5Stack-Thermal-Camera-

    Required hardware:
    - M5Stack
    - GridEye AMG88xx breakout board

    Required Library:
    - M5Stack (https://github.com/m5stack/M5Stack)
    - Adafruit AMG88xx (https://github.com/adafruit/Adafruit_AMG88xx)

    Feature:
        * Interpolation of the sensor grid from 8x8 to 24x24
        * Adjustable color scaling
        * Autoscaling (Double press B)
        * Pinpoint the minimal and maximal reading (see cold/hot spot)
        * Display spot, minimal and maximal reading
        * Display FPS (should be 13 btw)
        * Frozen state (Press B)

    For instructions, please go to the repo. That header is too long.

    Credit: Adafruit for the original Library
            Github user hkoffer for the base sketch
            Me and my dog
    * Original header moved to the file interpolation.cpp *
 ***************************************************************************/

#include <M5Stack.h>
#include <SPI.h>
// #include <SdFat.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AMG88xx.h>

/***************************************************************************
    USER SETTINGS

    ORIENTATION  : Change the orientation to 90degrees. Depend on M5Stack
                   batch, mine is 1 other have 0, so change it to 0 if the
                   orientation is wrong on yours (Default: 1)
    BRIGHTNESS   : Self explainatory, range is from 0 to 255 (Default: 255)
    SLEEP        : Time before going turning off - in minute (Default: 5)
    DEFAULT_MIN  : Default value of min scale (Default: 22)
    DEFAULT_MAX  : Default value of max scale (Default: 32)
 ***************************************************************************/

#define ORIENTATION         1
#define BRIGHTNESS          255
#define SLEEP               5
#define DEFAULT_MIN         22
#define DEFAULT_MAX         32

/***************************************************************************
    Below it's runtime. You shouldn't need to change anything for normal
    use. Of course if you want to improve the code, please go ahead. :)
 ***************************************************************************/

#define AMG_COLS            8                                   // Size of the sensor in X
#define AMG_ROWS            8                                   // Size of the sensor in Y
#define INT_COLS            24                                  // Interpolation factor in X
#define INT_ROWS            24                                  // Interpolation factor in Y
String M0[] = {"MODE", "SCALE", "PAUSE"};                       // Default menu
String M1[] = {"SMIN", "-", "+"};                               // Menu to set the scale min
String M2[] = {"SMAX", "-", "+"};                               // Menu to set the scale max
String M3[] = {"POINT", "MIN", "MAX"};                          // Menu to (de)activate pinpoint pixel
String MF[] = {"OFF", "SAVE", "START"};                         // Menu when frozen

struct                      sensorData
{
    float                   arrayRaw[AMG_COLS * AMG_ROWS];      // Sensor array
    float                   arrayInt[INT_ROWS * INT_COLS];      // Interpolated array
    int                     minScale = DEFAULT_MIN;             // Scale min
    int                     maxScale = DEFAULT_MAX;             // Scale max
    int                     valueMin = 0;                       // Reading min
    int                     valueMax = 0;                       // Reading max
    int                     minPixel[2] = {0, 0};               // Coordinate of min
    int                     maxPixel[2] = {0, 0};               // Coordinate of max
    boolean                 pinMin = false;                     // Display or not the min coordinate
    boolean                 pinMax = false;                     // Display or not the max coordinate
    boolean                 isRunning = true;                   // Frozen/Running state
    int                     menuState = 0;                      // State of the menu
    int                     sleepTime = 0;                      // Hold the last activity for autosleep
}                           sensor;

const uint16_t camColors[] = {
    0x480F, 0x400F, 0x400F, 0x400F, 0x4010, 0x3810, 0x3810, 0x3810, 0x3810, 0x3010, 0x3010,
    0x3010, 0x2810, 0x2810, 0x2810, 0x2810, 0x2010, 0x2010, 0x2010, 0x1810, 0x1810, 0x1811,
    0x1811, 0x1011, 0x1011, 0x1011, 0x0811, 0x0811, 0x0811, 0x0011, 0x0011, 0x0011, 0x0011,
    0x0011, 0x0031, 0x0031, 0x0051, 0x0072, 0x0072, 0x0092, 0x00B2, 0x00B2, 0x00D2, 0x00F2,
    0x00F2, 0x0112, 0x0132, 0x0152, 0x0152, 0x0172, 0x0192, 0x0192, 0x01B2, 0x01D2, 0x01F3,
    0x01F3, 0x0213, 0x0233, 0x0253, 0x0253, 0x0273, 0x0293, 0x02B3, 0x02D3, 0x02D3, 0x02F3,
    0x0313, 0x0333, 0x0333, 0x0353, 0x0373, 0x0394, 0x03B4, 0x03D4, 0x03D4, 0x03F4, 0x0414,
    0x0434, 0x0454, 0x0474, 0x0474, 0x0494, 0x04B4, 0x04D4, 0x04F4, 0x0514, 0x0534, 0x0534,
    0x0554, 0x0554, 0x0574, 0x0574, 0x0573, 0x0573, 0x0573, 0x0572, 0x0572, 0x0572, 0x0571,
    0x0591, 0x0591, 0x0590, 0x0590, 0x058F, 0x058F, 0x058F, 0x058E, 0x05AE, 0x05AE, 0x05AD,
    0x05AD, 0x05AD, 0x05AC, 0x05AC, 0x05AB, 0x05CB, 0x05CB, 0x05CA, 0x05CA, 0x05CA, 0x05C9,
    0x05C9, 0x05C8, 0x05E8, 0x05E8, 0x05E7, 0x05E7, 0x05E6, 0x05E6, 0x05E6, 0x05E5, 0x05E5,
    0x0604, 0x0604, 0x0604, 0x0603, 0x0603, 0x0602, 0x0602, 0x0601, 0x0621, 0x0621, 0x0620,
    0x0620, 0x0620, 0x0620, 0x0E20, 0x0E20, 0x0E40, 0x1640, 0x1640, 0x1E40, 0x1E40, 0x2640,
    0x2640, 0x2E40, 0x2E60, 0x3660, 0x3660, 0x3E60, 0x3E60, 0x3E60, 0x4660, 0x4660, 0x4E60,
    0x4E80, 0x5680, 0x5680, 0x5E80, 0x5E80, 0x6680, 0x6680, 0x6E80, 0x6EA0, 0x76A0, 0x76A0,
    0x7EA0, 0x7EA0, 0x86A0, 0x86A0, 0x8EA0, 0x8EC0, 0x96C0, 0x96C0, 0x9EC0, 0x9EC0, 0xA6C0,
    0xAEC0, 0xAEC0, 0xB6E0, 0xB6E0, 0xBEE0, 0xBEE0, 0xC6E0, 0xC6E0, 0xCEE0, 0xCEE0, 0xD6E0,
    0xD700, 0xDF00, 0xDEE0, 0xDEC0, 0xDEA0, 0xDE80, 0xDE80, 0xE660, 0xE640, 0xE620, 0xE600,
    0xE5E0, 0xE5C0, 0xE5A0, 0xE580, 0xE560, 0xE540, 0xE520, 0xE500, 0xE4E0, 0xE4C0, 0xE4A0,
    0xE480, 0xE460, 0xEC40, 0xEC20, 0xEC00, 0xEBE0, 0xEBC0, 0xEBA0, 0xEB80, 0xEB60, 0xEB40,
    0xEB20, 0xEB00, 0xEAE0, 0xEAC0, 0xEAA0, 0xEA80, 0xEA60, 0xEA40, 0xF220, 0xF200, 0xF1E0,
    0xF1C0, 0xF1A0, 0xF180, 0xF160, 0xF140, 0xF100, 0xF0E0, 0xF0C0, 0xF0A0, 0xF080, 0xF060,
    0xF040, 0xF020, 0xF800};                                    // Definition of the color used


Adafruit_AMG88xx amg;
uint16_t pixelSize = min(M5.Lcd.width() / INT_COLS, M5.Lcd.height() / INT_COLS);
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

float   get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void    set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void    get_adjacents_1d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void    get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
float   cubicInterpolate(float p[], float x);
float   bicubicInterpolate(float p[], float x, float y);
void    interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols);

/***************************************************************************
    Setup variables for counter
 ***************************************************************************/
unsigned long duration;
unsigned long starttime;

/***************************************************************************
    Start Setup Loop
 ***************************************************************************/

/*
 * Boot up the device. Init the M5Stack, wait for the sensor to answer and draw
 * the scale plus his values on the left side.
*/
void setup()
{
    M5.begin();
    M5.Lcd.begin();
    M5.Lcd.setRotation(ORIENTATION);
    M5.Lcd.setBrightness(BRIGHTNESS);
    M5.setWakeupButton(BUTTON_A_PIN);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    while (!amg.begin())
        delay(10);
    drawScale();
    drawScaleValues();
    
    // Get Start Time
    starttime = millis();//get the current time;
    
    
  /* Setup Serial */
  Serial.begin(9600);
  Serial.println("Starting Serial 9600...");
  
  writeFile(SD, "/hello.txt", "Hello world from M5Stack !!");
  readFile(SD, "/hello.txt");
    
}

/*
 * Main workflow:
 * 
 * 0. Check the sleep timer.
 * 1. Handle the buttons using menu()
 * 2. Check the frozen state
 *   2.1. Read the sensor value
 *   2.2. Check for error
 *   2.3. Interpolate the reading
 *   2.4. Retrieve the min/max values
 *   2.5. Draw the image
 *   2.6. Draw the pinpoint of the pixel location
 *   2.7. Draw the values and FPS on the right side
 *   2.8. Draw the menu bar on the bottom
 * 3. Trigger an M5Stack update.
*/
void loop() {
    long start = millis();
    if (millis() / 60000 > sensor.sleepTime + ((SLEEP > 0) ? SLEEP : 5))
        M5.powerOFF();
    menu();
    if (sensor.isRunning)
    {
        amg.readPixels(sensor.arrayRaw);
        serialprintArray()
        errorCheck();
        interpolate_image(sensor.arrayRaw, AMG_ROWS, AMG_COLS, sensor.arrayInt, INT_ROWS, INT_COLS);
        checkValues();
        drawImage();
        drawMinMax();
        drawData(start);
        drawMenu();
    }
    M5.update();
}

/*
 * Buttons function
*/
void menu(void)
{
    if ((M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()))
        sensor.sleepTime = millis() / 60000;
    if (sensor.isRunning && (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()))
    {
        if (M5.BtnA.wasPressed())
            sensor.menuState++;
        if (sensor.menuState > 3)
            sensor.menuState = 0;
        switch (sensor.menuState) {
            case 1:
                if (M5.BtnB.wasPressed())
                    sensor.minScale = (sensor.minScale > 0) ? (sensor.minScale - 1) : sensor.minScale;
                if (M5.BtnC.wasPressed())
                    sensor.minScale = (sensor.minScale < sensor.maxScale - 1) ? (sensor.minScale + 1) : sensor.minScale;
                break;
            case 2:
                if (M5.BtnB.wasPressed())
                    sensor.maxScale = (sensor.maxScale > sensor.minScale + 1) ? (sensor.maxScale - 1) : sensor.maxScale;
                if (M5.BtnC.wasPressed())
                    sensor.maxScale = (sensor.maxScale < 80) ? (sensor.maxScale + 1) : sensor.maxScale;
                break;
            case 3:
                if (M5.BtnB.wasPressed())
                    sensor.pinMin = sensor.pinMin ? false : true;
                if (M5.BtnC.wasPressed())
                    sensor.pinMax = sensor.pinMax ? false : true;
                break;
            default:
                if (M5.BtnB.wasPressed())
                {
                    sensor.minScale = sensor.valueMin;
                    sensor.maxScale = sensor.valueMax;
                }
                if (M5.BtnC.wasPressed())
                {
                    sensor.isRunning = false;
                    M5.Lcd.fillRect(40, 220, 240, 240, BLACK);
                    drawMenu();
                    return ;
                }
                break;
        }
        drawScaleValues();
    }
    else if (!sensor.isRunning)
    {
        if (M5.BtnC.wasPressed())
            sensor.isRunning = true;
        
        /*
         * Placeholder for "Save" function
         */
        if (M5.BtnB.wasPressed())
            sensor.isRunning = true;
            savetime = millis()
            
            amg.readPixels(sensor.arrayRaw);
            interpolate_image(sensor.arrayRaw, AMG_ROWS, AMG_COLS, sensor.arrayInt, INT_ROWS, INT_COLS);
            

            Serial.println((String)"Time: "+savetime+" R: "+ratio+" ");
        
        if (M5.BtnA.wasPressed())
            M5.powerOFF();
    }
}

/*
 * Draw the menu
*/
void drawMenu() {
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(DARKGREY);
    M5.Lcd.fillRect(40, 220, 240, 20, BLACK);
    String *menu = MF;
    if (sensor.isRunning)
    {
        switch (sensor.menuState) {
            case 1:
                menu = M1;
                break;
            case 2:
                menu = M2;
                break;
            case 3:
                menu = M3;
                break;
            default:
                menu = M0;
                break;
        }
    }
    M5.Lcd.drawString(menu[0], 75, 232);
    M5.Lcd.drawString(menu[1], 160, 232);
    M5.Lcd.drawString(menu[2], 245, 232);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(TL_DATUM);
}

/*
 * Check for error
*/
void errorCheck(void) {
    for (int i = 0; i < (AMG_COLS * AMG_ROWS); i++) {
        if (sensor.arrayRaw[i] > 80 || sensor.arrayRaw[i] < 0)
        {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextDatum(MC_DATUM);
            M5.Lcd.setTextColor(RED);
            M5.Lcd.drawString("ERROR SENSOR READING", 160, 20);
            M5.Lcd.drawString("PRESS TO REBOOT", 160, 232);

            M5.Lcd.setTextSize(1);
            String dump = "";
            for (int y = 0; y < 8; y++) {
                dump = "";
                for (int x = 0; x < 8; x++) {
                    dump = dump + String((int)sensor.arrayRaw[x + y]) + " ";
                }
                M5.Lcd.drawString(dump, 150, 80 + (y * 15));
            }
            while (1)
            {
                if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed())
                {
                    esp_sleep_enable_timer_wakeup(1);
                    esp_deep_sleep_start();
                }
                M5.update();
                delay (10);
            }
        }
    }
}

/*
 * Draw the scale values on the left side
*/
void drawScaleValues(void) {
    M5.Lcd.fillRect(0, 225, 36, 16, BLACK);
    M5.Lcd.fillRect(0, 0, 36, 16, BLACK);
    M5.Lcd.drawString(String(sensor.minScale) + "C", 0, 225);
    M5.Lcd.drawString(String(sensor.maxScale) + "C", 0, 1);
    M5.Lcd.setTextColor(DARKGREY);
    M5.Lcd.drawString("MAX", 284, 18);
    M5.Lcd.drawString("FPS", 284, 104);
    M5.Lcd.drawString("MIN", 284, 206);
    M5.Lcd.setTextColor(WHITE);
}

/*
 * Draw the color scale on the left side
*/
void drawScale() {
    int icolor = 255;
    for (int y = 16; y <= 223; y++)
        M5.Lcd.drawRect(0, 0, 35, y, camColors[icolor--]);
}

/*
 * Draw the reading values on the right side
*/
void drawData(long startTime) {
    M5.Lcd.fillRect(280, 225, 40, 15, BLACK);
    M5.Lcd.drawString(String(sensor.valueMin) + "C", 284, 225);
    M5.Lcd.fillRect(280, 0, 40, 15, BLACK);
    M5.Lcd.drawString(String(sensor.valueMax) + "C", 284, 0);
    M5.Lcd.drawString(String(sensor.arrayRaw[28]), 130, 135);
    M5.Lcd.drawCircle(160, 120, 6, TFT_WHITE);
    M5.Lcd.drawLine(160, 110, 160, 130, TFT_WHITE);
    M5.Lcd.drawLine(150, 120, 170, 120, TFT_WHITE);
    M5.Lcd.fillRect(280, 86, 40, 15, BLACK);
    M5.Lcd.drawString(String(1000 / (int)(millis() - startTime)), 288, 86);
    
}

/*
 * Check the values in the array interpolated
*/
void checkValues() {
    sensor.valueMax = INT_MIN;
    sensor.valueMin = INT_MAX;
    for (int y = 0; y < INT_ROWS; y++) {
        for (int x = 0; x < INT_COLS; x++) {
            int pixel = (int) get_point(sensor.arrayInt, INT_ROWS, INT_COLS, x, y);
            if (pixel > sensor.valueMax)
            {
                sensor.valueMax = pixel;
                sensor.maxPixel[0] = x;
                sensor.maxPixel[1] = y;
            }
            if (pixel < sensor.valueMin)
            {
                sensor.valueMin = pixel;
                sensor.minPixel[0] = x;
                sensor.minPixel[1] = y;
            }
        }
    }
}

/*
 * Draw the interpolated array
 */
void drawImage(void) {
    for (int y = 0; y < INT_ROWS; y++) {
        for (int x = 0; x < INT_COLS; x++) {
            float pixel = get_point(sensor.arrayInt, INT_ROWS, INT_COLS, x, y);
            pixel = (pixel >= sensor.maxScale) ? sensor.maxScale : (pixel <= sensor.minScale) ? sensor.minScale : pixel;
            uint8_t colorIndex = constrain(map((int)pixel, sensor.minScale, sensor.maxScale, 0, 255), 0, 255);
            if ((pixelSize * y) < 220)
                M5.Lcd.fillRect(40 + pixelSize * x, pixelSize * y, pixelSize, pixelSize, camColors[colorIndex]);
        }
    }
}


/*
 * Draw the pinpoint of min/max reading (if activated)
*/
void drawMinMax(void) {
    if (sensor.pinMin)
    {
        int minX = 40 + pixelSize * sensor.minPixel[0];
        int minY = pixelSize * sensor.minPixel[1];
        if (minY < 220)
            minY -= pixelSize;
        M5.Lcd.fillRect(minX, minY, pixelSize, pixelSize, BLUE);
        M5.Lcd.drawLine(minX + (pixelSize / 2), minY + (pixelSize / 2), 279, 210, BLUE);
    }
    if (sensor.pinMax)
    {
        int maxX = 40 + pixelSize * sensor.maxPixel[0];
        int maxY = pixelSize * sensor.maxPixel[1];
        if (maxY < 220)
            maxY -= pixelSize;
        M5.Lcd.fillRect(maxX, maxY, pixelSize, pixelSize, WHITE);
        M5.Lcd.drawLine(maxX + (pixelSize / 2), maxY + (pixelSize / 2), 279, 5, WHITE);
    }
}

void serialprintArray() {
    //read all the pixels
    amg.readPixels(pixels);

    Serial.println("-- Array --");
    Serial.println();
    
    Serial.print("[");
    for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
        Serial.print(pixels[i-1]);
        Serial.print(", ");
        if( i%8 == 0 ) Serial.println();
    }

    Serial.println("]");
    Serial.println();

    //delay a second
    delay(1000);
}

void readFile(fs::FS &fs, const char * path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        int ch = file.read();
        Serial.write(ch);
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}
