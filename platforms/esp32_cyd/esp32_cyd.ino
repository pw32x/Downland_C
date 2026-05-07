// Downland for the ESP32 Cheap Yellow Display.

// standard includes
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// libraries
#include <XPT2046_Touchscreen.h> // https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <TFT_eSPI.h> // https://github.com/Bodmer/TFT_eSPI

// touchscreen pins stuff
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// device pins stuff
#define SD_CS        5
#define TFT_CS       15
#define TOUCH_CS     33

// devices
SPIClass theSpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Downland stuff
extern "C"
{
#include "game.h"
#include "game_types.h"
#include "dl_sound.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"
}

GameData gameData;
Resources resources;

#define CRT_FRAMEBUFFER_SIZE (256*192*2)
dl_u16* crtFramebuffer = NULL;


static dl_u8 memory[18288];
static dl_u8* memoryEnd = NULL;

const int fileBufferSize = 8192;
dl_u8 fileBuffer[fileBufferSize];

const char* romFileNames[] = 
{
    "/downland.bin",
    "/downland.rom",
    "/Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};
int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);


extern "C"
{
/*
void log_(const char* text)
{
  Serial.println(text);	
}

void logn_(dl_u32 number)
{
  Serial.println(number);	
}
*/

void* dl_alloc(dl_u32 size)
{
    if (memoryEnd == NULL)
    {
	    memoryEnd = memory;
    }

    dl_u8* memoryAddress = memoryEnd;

    memoryEnd += size;
    return (void*)memoryAddress;
}

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{
    memset(source, value, count);
}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
    memcpy(destination, source, count);
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}
}

bool loadFile(const char* romPath, dl_u8* fileBuffer, dl_u32 fileBufferSize)
{
    Serial.println(romPath);

    File f = SD.open(romPath);

    if (!f)
    {
        Serial.println("ROM not found");
        return false;
    }

    int bytesRead = f.read(fileBuffer, fileBufferSize);
    Serial.println(bytesRead);

    if (bytesRead != fileBufferSize)
    {
        f.close();

        Serial.println("bytesRead != fileBufferSize");
        
        Serial.println(fileBufferSize);
        return false;
    }

    f.close();
    Serial.println("ROM found");

	return true;
}

void deselectAll()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
}


bool initSDCard()
{
    theSpi.begin(18, 19, 23);

    if (!SD.begin(SD_CS, theSpi, 80000000))
    {
        Serial.println("FAIL");
        SD.end();
        theSpi.end();
        return false;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) 
    {
        Serial.println("No SD card attached");
        SD.end();
        theSpi.end();
        return false;
    }

    bool romFoundAndLoaded = false;

    for (int loop = 0; loop < romFileNamesCount; loop++)
    {
        if (loadFile(romFileNames[loop], fileBuffer, fileBufferSize) &&
            checksumCheckLitteEndian(fileBuffer, fileBufferSize) &&
            ResourceLoaderBuffer_Init(fileBuffer, fileBufferSize, &resources))
        {
            romFoundAndLoaded = true;
            break;
        }
    }

    SD.end();
    theSpi.end();
    return romFoundAndLoaded;
}

void setup()
{
    Serial.begin(115200);
    delay(300);

    deselectAll();

    if (!initSDCard())
        return;

    // Start the SPI for the touch screen and init the TS library
    theSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI);
    delay(100);

    ts.begin(theSpi);
    ts.setRotation(1);

    // Start the tft display and set it to black
    tft.init();
    tft.initDMA();
    tft.setRotation(1); //This is the display in landscape
    tft.setSwapBytes(true);

    // Clear the screen before writing to it
    tft.fillScreen(TFT_BLACK);

    // Init the game
    Serial.println("Game_Init");
    Game_Init(&gameData, &resources);

    Serial.println("Init CRT Framebuffer");
    //crtFramebuffer = (dl_u16*)malloc(CRT_FRAMEBUFFER_SIZE);
    crtFramebuffer =(dl_u16*)heap_caps_malloc(256 * 192 * 2, MALLOC_CAP_DMA);
    memset(crtFramebuffer, 0, CRT_FRAMEBUFFER_SIZE);  
    Serial.println("DONE Init CRT Framebuffer");

    dl_u16* crtFramebuffer16bitRunner = (dl_u16*)crtFramebuffer;
    for (int loop = 0; loop < (256*192); loop++)
    {
        *crtFramebuffer16bitRunner = 0x001f;
        crtFramebuffer16bitRunner++;
    }
}

// touch handling stuff
bool g_oldTouched = false;
TS_Point tapPoint(-100, -100, 0);

unsigned short screenPointX = -100;
unsigned short screenPointY = -100;

#define TOUCH_X_MIN 250
#define TOUCH_X_MAX 3720
#define TOUCH_X_RANGE (TOUCH_X_MAX - TOUCH_X_MIN)
#define TOUCH_Y_MIN 350
#define TOUCH_Y_MAX 3775
#define TOUCH_Y_RANGE (TOUCH_Y_MAX - TOUCH_Y_MIN)

void mapTouchPointToScreenPoint(const TS_Point& tapPoint, unsigned short& screenPointX, unsigned short& screenPointY)
{
    // touchscreen value ranges when I touch the corners
    // x: 250 to 3720
    // y: 350 to 3775
    screenPointX = ((tapPoint.x - TOUCH_X_MIN) / (float)TOUCH_X_RANGE) * 320.0f;
    screenPointY = ((tapPoint.y - TOUCH_Y_MIN) / (float)TOUCH_Y_RANGE) * 240.0f;
}

void printTouchToSerial(TS_Point p) 
{
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    Serial.println();
}

uint32_t frameCount = 0;
uint32_t lastFPSUpdate = 0;
float fps = 0;

void updateFPS()
{
    frameCount++;

    uint32_t now = millis();

    if (now - lastFPSUpdate >= 1000)
    {
        fps = frameCount * 1000.0f / (now - lastFPSUpdate);

        Serial.print("FPS: ");
        Serial.println(fps);

        frameCount = 0;
        lastFPSUpdate = now;
    }
}

void updateTouchControls(dl_u8 playerIndex, JoystickState* joystickState)
{
    bool touched = /*ts.tirqTouched() &&*/ ts.touched();
    //String displayText = "";
    bool tapped = false;

    if (!g_oldTouched && touched)
    {
        tapped = true;
        //printTouchToSerial(tapPoint);
    }

    tapPoint = ts.getPoint();
    mapTouchPointToScreenPoint(tapPoint, screenPointX, screenPointY);

    bool leftDown = false;
    bool rightDown = false;
    bool upDown = false;
    bool downDown = false;
    bool jumpDown = false;

    if (touched)
    {
        if (tapped && screenPointX < 60)
        {
            jumpDown = true;
            leftDown = true;
        }
        else if (tapped && screenPointX > 260)
        {
            jumpDown = true;
            rightDown = true;
        }
        else if (screenPointY < 60)
        {
            upDown = true;
        }
        else if (screenPointY > 180)
        {
            downDown = true;
        }
        else if (screenPointX < 160)
        {
            leftDown = true;
        }
        else
        {
            rightDown = true;
        }
    }

    joystickState->leftPressed = !joystickState->leftDown & leftDown;
    joystickState->rightPressed = !joystickState->rightDown & rightDown;
    joystickState->upPressed = !joystickState->upDown & upDown;
    joystickState->downPressed =  !joystickState->downDown & downDown;
    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;

    joystickState->leftReleased = joystickState->leftDown & !leftDown;
    joystickState->rightReleased = joystickState->rightDown & !rightDown;
    joystickState->upReleased = joystickState->upDown & !upDown;
    joystickState->downReleased =  joystickState->downDown & !downDown;
    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;

    /*
    tft.drawCircle(screenPointX, screenPointY, 10, TFT_WHITE);

    if (touched)
    {
        displayText = "touching";
    }
    else
    {
        displayText = "not touching";
    }

    int x = 320 / 2; // center of display
    int y = 100;
    int fontSize = 2;

    tft.drawCentreString(displayText, x, y, fontSize);
    */

    g_oldTouched = touched;
}


void convert1bppImageTo16bppCrtEffectImage3(const dl_u8* originalImage, dl_u16* crtImage) 
{
    const dl_u16 BLACK  = 0x0000;
    const dl_u16 BLUE   = 0x001F;
    const dl_u16 ORANGE = 0xFC80; 
    const dl_u16 WHITE  = 0xFFFF;

    for (int y = 0; y < 192; ++y) {
        dl_u16* rowDest = &crtImage[y * 256];
        const dl_u8* rowSrc = &originalImage[y * 32];

        // First pass for this row: Basic color expansion (2 pixels per pair of bits)
        for (int xByte = 0; xByte < 32; ++xByte) {
            dl_u8 b = rowSrc[xByte];
            for (int shift = 6; shift >= 0; shift -= 2) {
                dl_u8 bits = (b >> shift) & 0x03;
                dl_u16 color = BLACK;
                if (bits == 1) color = BLUE;
                else if (bits == 2) color = ORANGE;
                else if (bits == 3) color = WHITE;
                
                *rowDest++ = color;
                *rowDest++ = color;
            }
        }

        // Second pass for this row: Artifact correction
        // We do this row-by-row to keep data in the CPU Cache (L1/L2)
        // This is much faster than processing the whole screen then re-processing
        dl_u16* p = &crtImage[y * 256];
        for (int x = 0; x < 256; x += 2) {
            dl_u16 left = p[x];
            dl_u16 right = p[x + 1];

            if (right == BLUE && x < 254) {
                dl_u16 next = p[x + 2];
                if (next == ORANGE || next == WHITE) {
                    right = WHITE;
                    left = BLACK;
                }
            } 
            else if (left == ORANGE && x >= 2) {
                dl_u16 prev = p[x - 1];
                if (prev == BLUE || prev == WHITE) {
                    left = WHITE;
                    right = BLACK;
                }
            }
            p[x] = left;
            p[x + 1] = right;
        }
    }
}



void loop() 
{
    int controllerIndex = 0;
    if (gameData.currentPlayerData != NULL)
    {
        controllerIndex = gameData.currentPlayerData->playerNumber;
    }

    updateTouchControls(controllerIndex, &gameData.joystickState);

    if (!gameData.paused)
    {
        Game_Update(&gameData, &resources);
        //Game_Update(&gameData, &resources);
        //Serial.println("game_update");
    }

    convert1bppImageTo16bppCrtEffectImage3(gameData.framebuffer, crtFramebuffer);

    //tft.pushImage(32, 24, 256, 192, crtFramebuffer);
    //tft.pushImageDMA(32, 24, 256, 192, crtFramebuffer);
    tft.startWrite();
    tft.pushImageDMA(32, 24, 256, 192, crtFramebuffer);
    tft.dmaWait();
    tft.endWrite(); 

    //tft.drawLine(160, 0, 160, 240, TFT_WHITE);

    //updateFPS();
}
