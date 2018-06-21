/***************************************************************************
    This is a library for the AMG88xx GridEYE 8x8 IR camera
    This sketch makes an inetrpolated pixel thermal camera with the
    GridEYE sensor and a 2.4" tft featherwing:
     https://www.adafruit.com/product/3315
    Designed specifically to work with the Adafruit AMG8833 Featherwing
                    https://www.adafruit.com/product/3622
    These sensors use I2C to communicate. The device's I2C address is 0x69
    Adafruit invests time and resources providing this open source code,
    please support Adafruit andopen-source hardware by purchasing products
    from Adafruit!
    Written by Dean Miller, James DeVito & ladyada for Adafruit Industries.
    BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <M5Stack.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

const uint16_t camColors[] = {0x480F,
0x400F, 0x400F, 0x400F, 0x4010, 0x3810, 0x3810, 0x3810, 0x3810, 0x3010, 0x3010,
0x3010, 0x2810, 0x2810, 0x2810, 0x2810, 0x2010, 0x2010, 0x2010, 0x1810, 0x1810,
0x1811, 0x1811, 0x1011, 0x1011, 0x1011, 0x0811, 0x0811, 0x0811, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0031, 0x0031, 0x0051, 0x0072, 0x0072, 0x0092, 0x00B2,
0x00B2, 0x00D2, 0x00F2, 0x00F2, 0x0112, 0x0132, 0x0152, 0x0152, 0x0172, 0x0192,
0x0192, 0x01B2, 0x01D2, 0x01F3, 0x01F3, 0x0213, 0x0233, 0x0253, 0x0253, 0x0273,
0x0293, 0x02B3, 0x02D3, 0x02D3, 0x02F3, 0x0313, 0x0333, 0x0333, 0x0353, 0x0373,
0x0394, 0x03B4, 0x03D4, 0x03D4, 0x03F4, 0x0414, 0x0434, 0x0454, 0x0474, 0x0474,
0x0494, 0x04B4, 0x04D4, 0x04F4, 0x0514, 0x0534, 0x0534, 0x0554, 0x0554, 0x0574,
0x0574, 0x0573, 0x0573, 0x0573, 0x0572, 0x0572, 0x0572, 0x0571, 0x0591, 0x0591,
0x0590, 0x0590, 0x058F, 0x058F, 0x058F, 0x058E, 0x05AE, 0x05AE, 0x05AD, 0x05AD,
0x05AD, 0x05AC, 0x05AC, 0x05AB, 0x05CB, 0x05CB, 0x05CA, 0x05CA, 0x05CA, 0x05C9,
0x05C9, 0x05C8, 0x05E8, 0x05E8, 0x05E7, 0x05E7, 0x05E6, 0x05E6, 0x05E6, 0x05E5,
0x05E5, 0x0604, 0x0604, 0x0604, 0x0603, 0x0603, 0x0602, 0x0602, 0x0601, 0x0621,
0x0621, 0x0620, 0x0620, 0x0620, 0x0620, 0x0E20, 0x0E20, 0x0E40, 0x1640, 0x1640,
0x1E40, 0x1E40, 0x2640, 0x2640, 0x2E40, 0x2E60, 0x3660, 0x3660, 0x3E60, 0x3E60,
0x3E60, 0x4660, 0x4660, 0x4E60, 0x4E80, 0x5680, 0x5680, 0x5E80, 0x5E80, 0x6680,
0x6680, 0x6E80, 0x6EA0, 0x76A0, 0x76A0, 0x7EA0, 0x7EA0, 0x86A0, 0x86A0, 0x8EA0,
0x8EC0, 0x96C0, 0x96C0, 0x9EC0, 0x9EC0, 0xA6C0, 0xAEC0, 0xAEC0, 0xB6E0, 0xB6E0,
0xBEE0, 0xBEE0, 0xC6E0, 0xC6E0, 0xCEE0, 0xCEE0, 0xD6E0, 0xD700, 0xDF00, 0xDEE0,
0xDEC0, 0xDEA0, 0xDE80, 0xDE80, 0xE660, 0xE640, 0xE620, 0xE600, 0xE5E0, 0xE5C0,
0xE5A0, 0xE580, 0xE560, 0xE540, 0xE520, 0xE500, 0xE4E0, 0xE4C0, 0xE4A0, 0xE480,
0xE460, 0xEC40, 0xEC20, 0xEC00, 0xEBE0, 0xEBC0, 0xEBA0, 0xEB80, 0xEB60, 0xEB40,
0xEB20, 0xEB00, 0xEAE0, 0xEAC0, 0xEAA0, 0xEA80, 0xEA60, 0xEA40, 0xF220, 0xF200,
0xF1E0, 0xF1C0, 0xF1A0, 0xF180, 0xF160, 0xF140, 0xF100, 0xF0E0, 0xF0C0, 0xF0A0,
0xF080, 0xF060, 0xF040, 0xF020, 0xF800,};

Adafruit_AMG88xx amg;
#define AMG_COLS 8
#define AMG_ROWS 8
#define INTERPOLATED_COLS 24
#define INTERPOLATED_ROWS 24

float arrayRaw[AMG_COLS * AMG_ROWS];
float arrayInterpolated[INTERPOLATED_ROWS * INTERPOLATED_COLS];
int minScale = 22;
int maxScale = 32;
int valueMax = 0;
int valueMin = 80;
int valueSpot = 0;
int minPixel[] = {0, 0};
int maxPixel[] = {0, 0};
boolean onHold = false;
boolean pinMin = false;
boolean pinMax = true;

uint16_t pixelSize = min(M5.Lcd.width() / INTERPOLATED_COLS, M5.Lcd.height() / INTERPOLATED_COLS);

float   get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void    set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void    get_adjacents_1d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void    get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
float   cubicInterpolate(float p[], float x);
float   bicubicInterpolate(float p[], float x, float y);
void    interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols);

void setup()
{
    M5.begin();
//    M5.setWakeupButton(BUTTON_B_PIN);
    M5.Lcd.begin();
    M5.Lcd.setRotation(1);
    M5.Lcd.setBrightness(255);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    while (!amg.begin())
        delay(10);
    drawScale();
    drawScaleValues();
}

void loop() {
    long start = millis();
    // 0. Handle buttons usage
    handleButtons();
    // If not in frozen state, get new data
    if (!onHold)
    {
        // 1. Read the sensor
        amg.readPixels(arrayRaw);
        // 2. Check for reading error
        errorCheck();
        // 3. Interpolate the image
        interpolate_image(arrayRaw, AMG_ROWS, AMG_COLS, arrayInterpolated, INTERPOLATED_ROWS, INTERPOLATED_COLS);
        // 4. Check the MIN/MAX values along with position in the interpolated image
        checkValues();
        // 5. Draw the image
        drawImage();
        // 6. Overlay the MIN/MAX pixel
        drawMinMax();
        // 7. Draw the reading and fps
        drawData(start);
    }
    M5.update();
}

void errorCheck(void) {
    for (int i = 0; i < (AMG_COLS * AMG_ROWS); i++) {
        if (arrayRaw[i] > 80 || arrayRaw[i] < 0)
        {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextDatum(MC_DATUM);
            M5.Lcd.setTextColor(RED);
            M5.Lcd.drawString("ERROR SENSOR READING", 160, 110);
            M5.Lcd.drawString("Auto reboot in 5 seconds", 160, 130);
            M5.Lcd.drawString("Array " + String(i) + ":" + String(arrayRaw[i]), 160, 150);
            delay(5000);
            esp_sleep_enable_timer_wakeup(1000);
            esp_deep_sleep_start();
        }
    }
}

void handleButtons(void) {
//    if (M5.BtnB.pressedFor(1000))
//    {
//        onHold = onHold ? false : true;
//        return ;
//    }
    if (M5.BtnB.wasPressed())
    {
        minScale = valueMin;
        maxScale = valueMax;
        drawScaleValues();
        return ;
    }
    // Active only if the screen is not in frozen state
    if (!onHold)
    {
        // Save the current value
        int prevMinScale = minScale;
        int prevMaxScale = maxScale;
        // Press A to increase MIN temp scale
        if (M5.BtnA.wasPressed())
            minScale = (minScale < maxScale - 1) ? (minScale + 1) : minScale;
        // Long press A to decrease MIN temp scale
        if (M5.BtnA.pressedFor(1000))
            minScale = (minScale > 0) ? (minScale - 1) : minScale;
        // Press B to increase MAX temp scale
        if (M5.BtnC.wasPressed())
            maxScale = (maxScale < 80) ? (maxScale + 1) : maxScale;
        // Long press B to decrease MAX temp scale
        if (M5.BtnC.pressedFor(1000))
            maxScale = (maxScale > minScale + 1) ? (maxScale - 1) : maxScale;
        // Check for modified values to redraw the scale
        if (prevMinScale != minScale || prevMaxScale != maxScale)
            drawScaleValues();
    }
    else
    {
        if (M5.BtnA.wasPressed())
        {
            pinMin = pinMin ? false : true;
            drawImage();
            drawMinMax();
        }
        if (M5.BtnC.wasPressed())
        {
            pinMax = pinMax ? false : true;
            drawImage();
            drawMinMax();
        }
    }

}

void drawScaleValues(void) {
    M5.Lcd.fillRect(0, 225, 36, 16, BLACK);
    M5.Lcd.fillRect(0, 0, 36, 16, BLACK);
    M5.Lcd.drawString(String(minScale) + "C", 0, 225);
    M5.Lcd.drawString(String(maxScale) + "C", 0, 1);
    M5.Lcd.setTextColor(DARKGREY);
    M5.Lcd.drawString("MAX", 284, 18);
    M5.Lcd.drawString("FPS", 284, 104);
    M5.Lcd.drawString("MIN", 284, 206);
    M5.Lcd.setTextColor(WHITE);
}

void drawScale() {
    int icolor = 255;
    for (int y = 16; y <= 223; y++)
        M5.Lcd.drawRect(0, 0, 35, y, camColors[icolor--]);
}

void drawData(long startTime) {
    // Min value
    M5.Lcd.fillRect(280, 225, 40, 15, BLACK);
    M5.Lcd.drawString(String(valueMin) + "C", 284, 225);
    // Max value
    M5.Lcd.fillRect(280, 0, 40, 15, BLACK);
    M5.Lcd.drawString(String(valueMax) + "C", 284, 0);
    // Spot value
    M5.Lcd.drawString(String(arrayRaw[28]), 130, 135);
    // Draw center spot over the image
    M5.Lcd.drawCircle(160, 120, 6, TFT_WHITE);
    M5.Lcd.drawLine(160, 110, 160, 130, TFT_WHITE);
    M5.Lcd.drawLine(150, 120, 170, 120, TFT_WHITE);
    // FPS value
    M5.Lcd.fillRect(280, 86, 40, 15, BLACK);
    M5.Lcd.drawString(String(1000 / (int)(millis() - startTime)), 288, 86);

}

void checkValues() {
    // Reset min and max value
    valueMax = INT_MIN;
    valueMin = INT_MAX;
    for (int y = 0; y < INTERPOLATED_ROWS; y++) {
        for (int x = 0; x < INTERPOLATED_COLS; x++) {
            int pixel = (int) get_point(arrayInterpolated, INTERPOLATED_ROWS, INTERPOLATED_COLS, x, y);
            // Save the max value and keep the coordinate of the pixel
            if (pixel > valueMax)
            {
                valueMax = pixel;
                maxPixel[0] = x;
                maxPixel[1] = y;
            }
            // Save the min value and keep the coordinate of the pixel
            if (pixel < valueMin)
            {
                valueMin = pixel;
                minPixel[0] = x;
                minPixel[1] = y;
            }
        }
    }
}

void drawImage(void) {
    // Draw the image from arrayInterpolated using color from camColors
    for (int y = 0; y < INTERPOLATED_ROWS; y++) {
        for (int x = 0; x < INTERPOLATED_COLS; x++) {
            float pixel = get_point(arrayInterpolated, INTERPOLATED_ROWS, INTERPOLATED_COLS, x, y);
            pixel = (pixel >= maxScale) ? maxScale : (pixel <= minScale) ? minScale : pixel;
            uint8_t colorIndex = constrain(map((int)pixel, minScale, maxScale, 0, 255), 0, 255);
            M5.Lcd.fillRect(40 + pixelSize * x, pixelSize * y, pixelSize, pixelSize, camColors[colorIndex]);
        }
    }
}

void drawMinMax(void) {
    // Draw the pixel that have the MIN value if pinMin is set to true
    if (pinMin)
    {
        int minX = 40 + pixelSize * minPixel[0];
        int minY = pixelSize * minPixel[1];
        M5.Lcd.fillRect(minX, minY, pixelSize, pixelSize, BLUE);
        M5.Lcd.drawLine(minX + (pixelSize / 2), minY + (pixelSize / 2), 279, 210, BLUE);
    }
    // Draw the pixel that have the MAX value if pinMax is set to true
    if (pinMax)
    {
        int maxX = 40 + pixelSize * maxPixel[0];
        int maxY = pixelSize * maxPixel[1];
        M5.Lcd.fillRect(maxX, maxY, pixelSize, pixelSize, WHITE);
        M5.Lcd.drawLine(maxX + (pixelSize / 2), maxY + (pixelSize / 2), 279, 5, WHITE);
    }
}
