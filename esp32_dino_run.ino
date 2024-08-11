// esp32_dino_run.ino
// A port of the popular browser game to the ESP32 and a 16x2 LCD!
// Author: Evan Tye - https://github.com/ev-tye

// Copyright (c) Evan Tye 2024

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished 
// to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
// THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <LiquidCrystal_I2C.h>

/////// DEFINES ////////////////////////////////////

#define BASE_SPEED          70
#define INPUT_BUTTON_PIN    12
#define LCD_ADDR            0x27
#define LCD_COLS            16
#define LCD_ROWS            2
#define LCD_TOP_ROW         0,0
#define LCD_BOTTOM_ROW      0,1
#define LCD_SCORE_POS       6,0
#define LCD_SDSCORE_OFFSET  14,0
#define START_STATE         0
#define GAME_STATE          1
#define GAME_OVER_STATE     2
#define DINO_FRAME_A_LCD    0
#define DINO_FRAME_B_LCD    1
#define CACTUS_LCD          2
#define DINO_AIR_TIME       20
#define INPUT_HOLD_TIME     25
#define SPEED_INCREASE_VAL  10

////////////////////////////////////////////////////

////// GLOBAL //////////////////////////////////////

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
int currState = START_STATE;
int gameSpeed = BASE_SPEED;
int inputState = 0;
int inputHoldTimer = 0;
int score = 0;
int dinoY = 1;
int dinoCurrAirTime = 0;
char dinoCurrFrame = 'A';
int currCactusX = 16;
const char* startScreenTopStr = "ESP32 Dino Run";
const char* startScreenBottomStr = "Press To Start!";
const char* gameOverTopStr = "Game Over!";
const char* scoreText = "Score:";

// Graphics

byte dinoGfxFrameA[8] =
{
  0b00000,0b01110,0b10101,0b10001,
  0b10010,0b11110,0b10100,0b01100
};

byte dinoGfxFrameB[8] =
{
  0b00000,0b01110,0b10101,0b10001,
  0b10010,0b11110,0b10100,0b10010
};

byte cactusGfx[8] = 
{
  0b00100,0b10101,0b10101,0b10101,
  0b01110,0b01110,0b01110,0b01110
};

////////////////////////////////////////////////////

///// FUNCTIONS ////////////////////////////////////

void _initHardware()
{
  pinMode(INPUT_BUTTON_PIN, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void _initGraphics()
{
  lcd.createChar(DINO_FRAME_A_LCD, dinoGfxFrameA);
  lcd.createChar(DINO_FRAME_B_LCD, dinoGfxFrameB);
  lcd.createChar(CACTUS_LCD, cactusGfx);
}

void _startStateProcess()
{
  inputState = digitalRead(INPUT_BUTTON_PIN);
  if(inputState == 1)
  {
    currState = GAME_STATE;
  }
}

void _gameStateProcess()
{
  // Check if input has been pressed, if so stick dino in the air
  // and set the air time
  inputState = digitalRead(INPUT_BUTTON_PIN);
  if(inputState == 1 && dinoCurrAirTime == 0 && inputHoldTimer <= 0)
  {
    dinoY = 0;
    dinoCurrAirTime = DINO_AIR_TIME;
    inputHoldTimer = INPUT_HOLD_TIME;
  }
  
  // Shift the cactus along
  if(currCactusX > 1)
  {
    currCactusX--;
  }
  else
  {
    if(dinoY == 0)
    {
      score++;
      if(score % 10 == 0)
      {
        // We increase the game speed by decreasing the delay between "frames"
        gameSpeed -= SPEED_INCREASE_VAL;
      }
      currCactusX = 16;
    }
    else
    {
      // Dino has collided with a cactus
      gameSpeed = BASE_SPEED;
      currState = GAME_OVER_STATE;
      return;
    }
  }

  if(dinoCurrAirTime == 1)
  {
    // Bring dino back down to row 1 of the LCD
    dinoY = 1;
  }

  if(dinoCurrAirTime <= 0)
  {
    dinoCurrAirTime = 0;
  }
  else
  {
    dinoCurrAirTime--;
  }

  if(inputHoldTimer > 0)
  {
    inputHoldTimer--;
  }
}

void _gameOverStateProcess()
{
  inputState = digitalRead(INPUT_BUTTON_PIN);
  if(inputState == 1)
  {
    delay(250);
    _resetGame();
  }
}

void _resetGame()
{
  score = 0;
  dinoY = 1;
  dinoCurrAirTime = 0;
  dinoCurrFrame = 'A';
  currCactusX = 16;
  currState = START_STATE;
}

// Graphics

void _drawStartScreen()
{
  lcd.clear();
  lcd.setCursor(LCD_TOP_ROW);
  lcd.print(startScreenTopStr);
  lcd.setCursor(LCD_BOTTOM_ROW);
  lcd.print(startScreenBottomStr);
}

void _drawGameOverScreen()
{
  lcd.clear();
  lcd.setCursor(LCD_TOP_ROW);
  lcd.print(gameOverTopStr);
  lcd.setCursor(LCD_BOTTOM_ROW);
  lcd.print(scoreText);
  lcd.print(score);
}

void _drawGameGraphics()
{
  lcd.clear();

  // Draw dino in it's current Y position
  lcd.setCursor(0, dinoY);
  if(dinoCurrFrame == 'A')
  {
    lcd.write(DINO_FRAME_A_LCD);
    dinoCurrFrame = 'B';
  }
  else
  {
    lcd.write(DINO_FRAME_B_LCD);
    dinoCurrFrame = 'A';
  }

  // Draw cactus at current X position
  lcd.setCursor(currCactusX, 1);
  lcd.write(CACTUS_LCD);

  // Draw current score
  lcd.setCursor(LCD_SCORE_POS);
  lcd.print(scoreText);
  if(score > 9)
  {
    lcd.print(score);
  }
  else
  {
    lcd.setCursor(LCD_SDSCORE_OFFSET);
    lcd.print(score);
  }
}

////////////////////////////////////////////////////

/////// MAIN CODE //////////////////////////////////

void setup() 
{
  _initHardware();
  _initGraphics();
}

void loop() 
{
  switch(currState)
  {
    case START_STATE:
      _startStateProcess();
      _drawStartScreen();
      break;
    case GAME_STATE:
      _gameStateProcess();
      _drawGameGraphics();
      break;
    case GAME_OVER_STATE:
      _gameOverStateProcess();
      _drawGameOverScreen();
      break;
    default:
      break;
  }
  delay(gameSpeed);
}

////////////////////////////////////////////////////
