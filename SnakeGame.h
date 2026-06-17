#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace SnakeGame {
bool launcherExit = false;

bool shouldExitToLauncher() {
  return launcherExit;
}

const int dacPin = 25;

// TFT LCD Setup
TFT_eSPI tft = TFT_eSPI();
const uint16_t backgroundColor = TFT_BLACK;
const int maxX = 319;
const int maxY = 239;

// Snake Board
const int gridCols = 30;
const int gridRows = 18;
const int maxSnakeLength = gridCols * gridRows;
const int cellSize = 10;
const int boardX = 10;
const int boardY = 50;

struct Point {
  int col;
  int row;
};

Point snake[maxSnakeLength];
int snakeLength = 4;
int directionX = 1;
int directionY = 0;
int nextDirectionX = 1;
int nextDirectionY = 0;
int pendingGrowth = 0;

enum FoodType {
  Apple,
  Berry,
  Pepper,
  Gold
};

struct Food {
  Point position;
  FoodType type;
};

Food food;

const char* foodNames[] = { "APPLE", "BERRY", "PEPPER", "GOLD" };
const uint16_t foodColors[] = { TFT_RED, TFT_MAGENTA, TFT_ORANGE, TFT_YELLOW };
const int foodGrowth[] = { 1, 2, 1, 3 };
const int foodPoints[] = { 10, 25, 15, 50 };

// State Engine Variables
bool onMenu = true;
bool inGame = false;
bool gameOver = false;
bool gamePaused = false;
bool menuNeedsRedraw = true;
bool boardNeedsRedraw = true;
bool pauseScreenDrawn = false;
bool endScreenShown = false;
bool quitConfirm = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;

// Menu slider variables
int selectedMenuItem = 0;
const int menuItemCount = 2;
int snakeColorIndex = 0;
const int snakeColorCount = 8;
const uint16_t snakeColors[snakeColorCount] = {
  TFT_GREEN,
  TFT_CYAN,
  TFT_BLUE,
  TFT_MAGENTA,
  TFT_RED,
  TFT_ORANGE,
  TFT_YELLOW,
  TFT_WHITE
};
const char* snakeColorNames[snakeColorCount] = {
  "GREEN",
  "CYAN",
  "BLUE",
  "PURPLE",
  "RED",
  "ORANGE",
  "YELLOW",
  "WHITE"
};
FoodType selectedFoodType = Apple;

unsigned long globalDebounceTime = 0;
unsigned long lastMoveTime = 0;
const unsigned long debounceDelayMs = 180;
const unsigned long startMoveDelayMs = 210;
const unsigned long minMoveDelayMs = 75;
unsigned long moveDelayMs = startMoveDelayMs;

int score = 0;
int foodsEaten = 0;

// Function Declarations
void scanButtons();
bool buttonTapped(buttonName button);
void drawMenu();
void drawColorSlider();
void drawFoodSlider();
void processMenu();
void resetGame();
void drawBoardFrame();
void drawScoreBar();
void drawCell(int col, int row, uint16_t color);
void drawSnake();
void drawFood();
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void displayGameOver();
void returnToMenu();
void processGameInput();
void moveSnake();
bool pointEquals(Point a, Point b);
bool snakeContains(Point point, bool includeTail);
void spawnFood();
FoodType randomFoodType();
void eatFood();
void updateSpeed();

void setupGame() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(backgroundColor);
  randomSeed(esp_random());

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void loopGame() {
  scanButtons();
  if (onMenu) {
    processMenu();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }
  if (gameOver) {
    if (!endScreenShown) {
      displayGameOver();
      endScreenShown = true;
    }
    if ((buttonTapped(Start) || buttonTapped(Enter)) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      onMenu = true;
      inGame = false;
      menuNeedsRedraw = true;
    }
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }
  if (quitConfirm) {
    if (buttonTapped(B) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      cancelQuitOverlay();
    }
    if (buttonTapped(Enter) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  // FIXED: Stop button scanning/taps AND freeze gameplay frames entirely during active countdown
  if (resumeCountdownActive) {
    updateResumeCountdown();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (buttonTapped(B) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    quitConfirm = true;
    drawQuitOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }
  if (buttonTapped(A) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      startResumeCountdown();
    }
  }
  if (gamePaused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (boardNeedsRedraw) {
    tft.fillScreen(backgroundColor);
    drawScoreBar();
    drawBoardFrame();
    drawSnake();
    drawFood();
    boardNeedsRedraw = false;
  }

  processGameInput();
  moveSnake();
  for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
  delay(10);
}

void scanButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

bool buttonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed;
}

void drawMenu() {
  tft.fillScreen(backgroundColor);

  tft.setTextColor(TFT_CYAN, backgroundColor);
  tft.setTextSize(3);
  tft.setCursor(112, 20);
  tft.print("SNAKE");
  tft.drawFastHLine(32, 58, 256, TFT_WHITE);

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(28, 78);
  tft.print("UP/DOWN selects menu row");
  tft.setCursor(28, 94);
  tft.print("LEFT/RIGHT changes value");
  tft.setCursor(28, 110);
  tft.print("B asks to quit in-game");

  drawColorSlider();
  drawFoodSlider();

  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.setTextSize(2);
  tft.setCursor(84, 210);
  tft.print("START TO PLAY");
}

void drawColorSlider() {
  int sliderX = 28;
  int sliderY = 144;
  int sliderW = 250;
  int knobW = 16;
  int stepW = (sliderW - knobW) / (snakeColorCount - 1);
  int knobX = sliderX + snakeColorIndex * stepW;

  tft.fillRect(0, 126, maxX + 1, 54, backgroundColor);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(28, 126);
  tft.print(selectedMenuItem == 0 ? "> SNAKE COLOR: " : "  SNAKE COLOR: ");
  tft.setTextColor(snakeColors[snakeColorIndex], backgroundColor);
  tft.print(snakeColorNames[snakeColorIndex]);

  for (int i = 0; i < snakeColorCount; i++) {
    int markerX = sliderX + i * stepW + knobW / 2;
    tft.fillCircle(markerX, sliderY + 7, 4, snakeColors[i]);
  }

  tft.drawRoundRect(sliderX, sliderY, sliderW, 16, 4, TFT_WHITE);
  tft.fillRoundRect(knobX, sliderY - 4, knobW, 24, 4, snakeColors[snakeColorIndex]);
  tft.drawRoundRect(knobX, sliderY - 4, knobW, 24, 4, TFT_WHITE);

  tft.fillRect(286, 134, 20, 20, snakeColors[snakeColorIndex]);
}

void drawFoodSlider() {
  int y = 178;
  tft.fillRect(0, y - 8, maxX + 1, 30, backgroundColor);
  tft.setTextColor(selectedMenuItem == 1 ? TFT_YELLOW : TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(28, y);
  tft.print(selectedMenuItem == 1 ? "> FOOD: " : "  FOOD: ");
  tft.setTextColor(foodColors[selectedFoodType], backgroundColor);
  tft.print(foodNames[selectedFoodType]);

  for (int i = 0; i < 4; i++) {
    int x = 110 + i * 34;
    tft.drawRect(x, y + 15, 18, 10, TFT_DARKGREY);
    tft.fillRect(x + 2, y + 17, 14, 6, foodColors[i]);
    if (i == selectedFoodType) {
      tft.drawRect(x - 2, y + 13, 22, 14, TFT_WHITE);
    }
  }
}

void processMenu() {
  if (menuNeedsRedraw) {
    drawMenu();
    menuNeedsRedraw = false;
  }

  if (buttonTapped(B) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    returnToMenu();
    return;
  }
  if (buttonTapped(Up) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    selectedMenuItem = (selectedMenuItem + menuItemCount - 1) % menuItemCount;
    drawColorSlider();
    drawFoodSlider();
  }
  if (buttonTapped(Down) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    selectedMenuItem = (selectedMenuItem + 1) % menuItemCount;
    drawColorSlider();
    drawFoodSlider();
  }
  if (buttonTapped(Left) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    if (selectedMenuItem == 0) {
      snakeColorIndex = (snakeColorIndex + snakeColorCount - 1) % snakeColorCount;
      drawColorSlider();
    } else {
      selectedFoodType = (FoodType)((selectedFoodType + 3) % 4);
      drawFoodSlider();
    }
  }
  if (buttonTapped(Right) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    if (selectedMenuItem == 0) {
      snakeColorIndex = (snakeColorIndex + 1) % snakeColorCount;
      drawColorSlider();
    } else {
      selectedFoodType = (FoodType)((selectedFoodType + 1) % 4);
      drawFoodSlider();
    }
  }
  if (buttonTapped(Start) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    onMenu = false;
    inGame = true;
    resetGame();
  }
}

void resetGame() {
  gameOver = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownBoxDrawn = false;
  endScreenShown = false;
  boardNeedsRedraw = true;
  score = 0;
  foodsEaten = 0;
  moveDelayMs = startMoveDelayMs;
  directionX = 1;
  directionY = 0;
  nextDirectionX = 1;
  nextDirectionY = 0;
  pendingGrowth = 0;
  snakeLength = 4;

  int startCol = gridCols / 2;
  int startRow = gridRows / 2;
  for (int i = 0; i < snakeLength; i++) {
    snake[i].col = startCol - i;
    snake[i].row = startRow;
  }

  spawnFood();
  lastMoveTime = millis();
}

void drawBoardFrame() {
  tft.drawRect(boardX - 2, boardY - 2, gridCols * cellSize + 4, gridRows * cellSize + 4, TFT_WHITE);
}

void drawScoreBar() {
  tft.fillRect(0, 0, maxX + 1, 44, backgroundColor);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("SCORE ");
  tft.print(score);
  tft.setCursor(10, 28);
  tft.print("ATE ");
  tft.print(foodsEaten);

  tft.setCursor(180, 10);
  tft.print("FOOD ");
  tft.setTextColor(foodColors[food.type], backgroundColor);
  tft.print(foodNames[food.type]);
}

void drawCell(int col, int row, uint16_t color) {
  int x = boardX + col * cellSize;
  int y = boardY + row * cellSize;
  tft.fillRect(x, y, cellSize, cellSize, color);
}

void drawSnake() {
  for (int i = snakeLength - 1; i >= 0; i--) {
    uint16_t color = (i == 0) ? TFT_WHITE : snakeColors[snakeColorIndex];
    drawCell(snake[i].col, snake[i].row, color);
  }
}

void drawFood() {
  int x = boardX + food.position.col * cellSize;
  int y = boardY + food.position.row * cellSize;
  uint16_t color = foodColors[food.type];

  if (food.type == Apple) {
    tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 4, color);
  } else if (food.type == Berry) {
    tft.fillCircle(x + 4, y + 4, 3, color);
    tft.fillCircle(x + 7, y + 7, 3, color);
  } else if (food.type == Pepper) {
    tft.fillTriangle(x + 2, y + 9, x + 9, y + 5, x + 3, y + 2, color);
  } else {
    tft.fillRect(x + 3, y + 3, 6, 6, color);
    tft.drawRect(x + 2, y + 2, 8, 8, TFT_WHITE);
  }
}

void drawPauseOverlay() {
  int boxW = 118;
  int boxH = 92;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(boxX + 24, boxY + 18);
  tft.print("PAUSED");

  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(boxX + 12, boxY + 54);
  tft.print("Press A to Resume");
  pauseScreenDrawn = true;
}

void drawQuitOverlay() {
  int boxW = 196;
  int boxH = 88;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_YELLOW);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(boxX + 42, boxY + 18);
  tft.print("QUIT GAME?");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 28, boxY + 54);
  tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  quitConfirm = false;
  boardNeedsRedraw = true;
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false; 
  resumeCountdownValue = 3;
  resumeCountdownLastMs = millis();
  drawResumeCountdown();
}

void drawResumeCountdown() {
  int boxW = 118;
  int boxH = 92;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  if (!resumeCountdownBoxDrawn) {
    tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(boxX + 24, boxY + 14);
    tft.print("RESUME");
    resumeCountdownBoxDrawn = true;
  }

  tft.fillRect(boxX + 39, boxY + 44, 40, 34, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(boxX + 46, boxY + 44);
  tft.print(resumeCountdownValue);
}

bool updateResumeCountdown() {
  unsigned long now = millis();
  if (now - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs += 1000;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      resumeCountdownBoxDrawn = false;
      
      // Clear the popup area by running a full board redraw right here
      boardNeedsRedraw = true; 
      
      lastMoveTime = millis();
      return false;
    }
    drawResumeCountdown(); 
  }
  return true;
}

void displayGameOver() {
  int boxW = 196;
  int boxH = 94;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_RED);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(boxX + 38, boxY + 18);
  tft.print("GAME OVER");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 42, boxY + 52);
  tft.print("Score: ");
  tft.print(score);
  tft.setCursor(boxX + 32, boxY + 70);
  tft.print("START or ENTER");
}

void returnToMenu() {
  launcherExit = true;
  onMenu = true;
  inGame = false;
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  menuNeedsRedraw = true;
  boardNeedsRedraw = true;
  pauseScreenDrawn = false;
  endScreenShown = false;
  tft.fillScreen(backgroundColor);
}

void processGameInput() {
  if (buttonTapped(Up) && directionY != 1) {
    nextDirectionX = 0;
    nextDirectionY = -1;
  }
  if (buttonTapped(Down) && directionY != -1) {
    nextDirectionX = 0;
    nextDirectionY = 1;
  }
  if (buttonTapped(Left) && directionX != 1) {
    nextDirectionX = -1;
    nextDirectionY = 0;
  }
  if (buttonTapped(Right) && directionX != -1) {
    nextDirectionX = 1;
    nextDirectionY = 0;
  }
}

void moveSnake() {
  // FIXED: If the countdown popup is active, instantly bounce out. 
  // This completely stops the snake from rendering or stepping forward during the 3 seconds.
  if (resumeCountdownActive) return;

  if (millis() - lastMoveTime < moveDelayMs) return;
  lastMoveTime = millis();

  directionX = nextDirectionX;
  directionY = nextDirectionY;

  Point oldTail = snake[snakeLength - 1];
  Point newHead;
  newHead.col = snake[0].col + directionX;
  newHead.row = snake[0].row + directionY;

  if (newHead.col < 0 || newHead.col >= gridCols || newHead.row < 0 || newHead.row >= gridRows || snakeContains(newHead, false)) {
    gameOver = true;
    return;
  }

  bool eating = pointEquals(newHead, food.position);
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0] = newHead;

  if (eating) {
    eatFood();
    drawScoreBar();
    drawFood();
  }

  if (pendingGrowth > 0 && snakeLength < maxSnakeLength) {
    snake[snakeLength] = oldTail;
    snakeLength++;
    pendingGrowth--;
  } else {
    drawCell(oldTail.col, oldTail.row, backgroundColor);
  }

  drawSnake();
}

bool pointEquals(Point a, Point b) {
  return a.col == b.col && a.row == b.row;
}

bool snakeContains(Point point, bool includeTail) {
  int limit = includeTail ? snakeLength : snakeLength - 1;
  for (int i = 0; i < limit; i++) {
    if (pointEquals(point, snake[i])) return true;
  }
  return false;
}

void spawnFood() {
  do {
    food.position.col = random(0, gridCols);
    food.position.row = random(0, gridRows);
  } while (snakeContains(food.position, true));

  food.type = selectedFoodType;
}

FoodType randomFoodType() {
  int roll = random(0, 100);
  if (roll < 55) return Apple;
  if (roll < 80) return Berry;
  if (roll < 94) return Pepper;
  return Gold;
}

void eatFood() {
  FoodType eatenType = food.type;
  pendingGrowth += foodGrowth[eatenType];
  score += foodPoints[eatenType];
  foodsEaten++;
  if (eatenType == Pepper && moveDelayMs > minMoveDelayMs + 20) {
    moveDelayMs -= 20;
  }
  updateSpeed();
  drawCell(food.position.col, food.position.row, backgroundColor);
  spawnFood();
}

void updateSpeed() {
  unsigned long earnedSpeed = foodsEaten * 4;
  unsigned long targetDelay = (startMoveDelayMs > earnedSpeed + minMoveDelayMs) ? startMoveDelayMs - earnedSpeed : minMoveDelayMs;
  if (targetDelay < moveDelayMs) moveDelayMs = targetDelay;
}

}  // namespace SnakeGame