#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"
#include "XImage.h"
#include "OImage.h"

namespace TicTacToeGame {
bool launcherExit = false;
bool shouldExitToLauncher() {
  return launcherExit;
}

const int dacPin = 25;

// TFT LCD Setup
TFT_eSPI tft = TFT_eSPI();
const uint16_t backgroundColor = TFT_BLACK;
const uint16_t menuBackgroundColor = TFT_BLACK;
uint16_t colorToDraw = 0x0000;

bool drawTemporary = true;
const int screenCenterX = 160;
const int screenCenterY = 120;
const int maxX = 319;
const int maxY = 239;

// State Engine Variables
bool onSettingsMenu = true;
bool inGame = false;
bool gameOver = false;
bool gamePaused = false;
bool menuNeedsRedraw = true;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
int lastDrawnCountdownValue = -1;
bool quitConfirm = false;

// Grid Drawing Helpers
bool ticTacToeDrawn = false;
bool notYetDrawn = true;

// Grid: 3x3 cells, each 70x70 pixels
// Cell centers: col0=90, col1=160, col2=230; row0=50, row1=120, row2=190
const int leftX = 90;
const int upperY = 50;
const int middleX = 160;
const int middleY = 120;
const int rightX = 230;
const int bottomY = 190;

int currentX = middleX;
int currentY = middleY;
int oldX = currentX;
int oldY = currentY;

bool userMoved = false;

enum boardCellState {
  Empty,
  X,
  O
};

boardCellState board[3][3] = {
  { Empty, Empty, Empty },
  { Empty, Empty, Empty },
  { Empty, Empty, Empty },
};

int currentRow = -1;
int currentCol = -1;
int oldRow = -1;
int oldCol = -1;

int emptyRow = -1;
int emptyCol = -1;
int emptyX = -1;
int emptyY = -1;

bool humanWins = false;
bool computerWins = false;
bool p1Won = false;
bool p2Won = false;
bool tie = false;

enum computerStrategies {
  firstCell,
  randomMove,
  smartComputer,
};

computerStrategies computerStrategy = smartComputer;
bool endScreenShown = false;

int emptyRows[9];
int emptyCols[9];
int emptyCount = 0;

boardCellState humanSymbol = X;
boardCellState computerSymbol = O;
bool isTwoPlayerMode = false;
bool isPlayer1Turn = true;

// Menu Layout Options
const int menuItemCount = 3;
int selectedMenuItem = 0;
int settingMode = 0;    // 0: Vs CPU, 1: Local 2P
int settingSymbol = 0;  // 0: X, 1: O
int settingDiff = 1;    // 0: Easy, 1: Smart

const char* modeLabels[] = { "Vs CPU", "Local 2P" };
const char* symbolLabels[] = { "X", "O" };
const char* diffLabels[] = { "Easy", "Smart" };

// Time debouncer configurations
unsigned long globalDebounceTime = 0;
const unsigned long debounceDelayMs = 250;

// Function Declarations
void drawTicTacToeGrid();
void drawCellHighlight(int x, int y, uint16_t color);
void clearCellHighlight(int x, int y);
void drawXOrO(int shiftX, int shiftY, boardCellState symbolToDraw);
void clearXorO(int oldX, int oldY, uint16_t backgroundColor);
void getCurrentRowsAndColumns();
void getOldRowsAndColumns();
void checkForWins();
void processUserMovement();
void processComputerMove();
void displayWinner();
void getCoordsFromEmpty(int emptyRow, int emptyCol);
bool hasWin(boardCellState symbol);
bool findWinningMove(boardCellState symbol, int& outRow, int& outCol);
void placeComputerPiece(int row, int col);
void collectEmptyCells();
void resetGame();
void drawSettingsMenu();
void processSettingsMenu();
void drawToggleRow(int y, const char* label, const char* opt1, const char* opt2, int selection, bool isRowSelected);
void applySettingsFromMenu();
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void returnToMenu();
void redrawEntireBoardState();

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
  // Global input scan updates
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }

  // State: Settings Setup Menu
  if (onSettingsMenu) {
    processSettingsMenu();
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    return;
  }

  // State: Game Over Screen Screen Handler
  if (gameOver) {
    if (!endScreenShown) {
      displayWinner();
      endScreenShown = true;
    }
    if (((buttons[Start].pressed && !buttons[Start].lastPressed) || (buttons[Enter].pressed && !buttons[Enter].lastPressed)) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      onSettingsMenu = true;
      inGame = false;
      menuNeedsRedraw = true;
      delay(150);
    }
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  if (quitConfirm) {
    if (buttons[B].pressed && !buttons[B].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      cancelQuitOverlay();
    }
    if (buttons[Enter].pressed && !buttons[Enter].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  if (buttons[B].pressed && !buttons[B].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    quitConfirm = true;
    drawQuitOverlay();
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  // Game Pause Control Gate (Button A)
  if (buttons[A].pressed && !buttons[A].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      tft.fillScreen(backgroundColor);
      drawTicTacToeGrid();
      redrawEntireBoardState();
      notYetDrawn = true;
      startResumeCountdown();
    }
  }

  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      for (int i = 0; i < buttonCount; i++) {
        buttons[i].lastPressed = buttons[i].pressed;
      }
      delay(10);
      return;
    } else {
      // Countdown finished - redraw screen to remove the resume box
      tft.fillScreen(backgroundColor);
      drawTicTacToeGrid();
      redrawEntireBoardState();
      notYetDrawn = true;
    }
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) {
      drawPauseOverlay();
    }
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  // State: Core Match Engine Active Frame
  if (!ticTacToeDrawn) {
    drawTicTacToeGrid();
  }

  oldX = currentX;
  oldY = currentY;

  processUserMovement();

  if (oldX != currentX || oldY != currentY || notYetDrawn) {
    // Clear old red highlight box
    if (!notYetDrawn) {
      clearCellHighlight(oldX, oldY);
    }

    getOldRowsAndColumns();
    if (!notYetDrawn && board[oldRow][oldCol] == Empty) {
      clearXorO(oldX, oldY, backgroundColor);
    } else if (!notYetDrawn && board[oldRow][oldCol] != Empty) {
      drawTemporary = false;
      drawXOrO(oldX, oldY, board[oldRow][oldCol]);
    }
    getCurrentRowsAndColumns();
    if (board[currentRow][currentCol] == Empty) {
      drawTemporary = true;
      boardCellState turnSymbol = (isTwoPlayerMode) ? (isPlayer1Turn ? humanSymbol : computerSymbol) : humanSymbol;
      drawXOrO(currentX, currentY, turnSymbol);
    }
    // Draw red highlight box on current cell
    drawCellHighlight(currentX, currentY, TFT_RED);
    notYetDrawn = false;
  }

  checkForWins();

  if (userMoved && !gameOver) {
    userMoved = false;
    if (!isTwoPlayerMode) {
      delay(600);
      processComputerMove();
      checkForWins();
    } else {
      isPlayer1Turn = !isPlayer1Turn;
      notYetDrawn = true;
    }
  } else if (userMoved) {
    userMoved = false;
  }

  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
  delay(10);
}

// Function to redraw locked elements post-unpause frame
void redrawEntireBoardState() {
  drawTemporary = false;
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board[row][col] != Empty) {
        getCoordsFromEmpty(row, col);
        drawXOrO(emptyX, emptyY, board[row][col]);
      }
    }
  }
}

// Visual Pause Overlay System Function
void drawPauseOverlay() {
  int boxW = 200;
  int boxH = 80;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(boxX + 44, boxY + 18);
  tft.print("PAUSED");

  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(boxX + 38, boxY + 50);
  tft.print("Press A to Resume");
  pauseScreenDrawn = true;
}

void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  if (!resumeCountdownBoxDrawn) {
    tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(boxX + 24, boxY + 12);
    tft.print("RESUME");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(boxX + 28, boxY + 74);
    tft.print("A to resume");
    resumeCountdownBoxDrawn = true;
  }

  // Smooth countdown: only clear the number area, no flicker
  tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(boxX + 50, boxY + 36);
  tft.print(resumeCountdownValue);
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false;
  resumeCountdownValue = 3;
  resumeCountdownLastMs = millis();
  lastDrawnCountdownValue = -1;
  drawResumeCountdown();
}

bool updateResumeCountdown() {
  unsigned long now = millis();
  if (now - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs += 1000;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      resumeCountdownBoxDrawn = false;
      lastDrawnCountdownValue = -1;
      return false;
    }
  }

  // Only redraw if the value changed (prevents flicker)
  if (resumeCountdownValue != lastDrawnCountdownValue) {
    int boxW = 132;
    int boxH = 96;
    int boxX = (maxX + 1 - boxW) / 2;
    int boxY = (maxY + 1 - boxH) / 2;
    tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(boxX + 50, boxY + 36);
    tft.print(resumeCountdownValue);
    lastDrawnCountdownValue = resumeCountdownValue;
  }
  return true;
}

void drawQuitOverlay() {
  int boxW = 210;
  int boxH = 84;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_YELLOW);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(boxX + 50, boxY + 18);
  tft.print("QUIT GAME?");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 34, boxY + 54);
  tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  quitConfirm = false;
  tft.fillScreen(backgroundColor);
  drawTicTacToeGrid();
  redrawEntireBoardState();
  notYetDrawn = true;
  pauseScreenDrawn = false;
  if (gamePaused) {
    drawPauseOverlay();
  }
}

void returnToMenu() {
  launcherExit = true;
  quitConfirm = false;
  onSettingsMenu = true;
  inGame = false;
  gameOver = false;
  gamePaused = false;
  resumeCountdownActive = false;
  menuNeedsRedraw = true;
  pauseScreenDrawn = false;
  endScreenShown = false;
  tft.fillScreen(menuBackgroundColor);
}

// 2-Option Text Toggle Layout Component
void drawToggleRow(int y, const char* label, const char* opt1, const char* opt2, int selection, bool isRowSelected) {
  uint16_t titleColor = isRowSelected ? TFT_YELLOW : TFT_WHITE;
  tft.setTextColor(titleColor, menuBackgroundColor);
  tft.setTextSize(2);
  tft.setCursor(15, y);
  tft.print(label);

  int opt1X = 120;
  int opt2X = 220;

  if (selection == 0) {
    tft.setTextColor(TFT_GREEN, menuBackgroundColor);
    tft.setCursor(opt1X, y);
    tft.print(opt1);
    tft.setTextColor(TFT_DARKGREY, menuBackgroundColor);
    tft.setCursor(opt2X, y);
    tft.print(opt2);
  } else {
    tft.setTextColor(TFT_DARKGREY, menuBackgroundColor);
    tft.setCursor(opt1X, y);
    tft.print(opt1);
    tft.setTextColor(TFT_GREEN, menuBackgroundColor);
    tft.setCursor(opt2X, y);
    tft.print(opt2);
  }
}

void drawSettingsMenu() {
  tft.fillScreen(menuBackgroundColor);
  tft.setTextColor(TFT_CYAN, menuBackgroundColor);
  tft.setTextSize(2);
  tft.setCursor(55, 15);
  tft.print("TIC-TAC-TOE SETUP");
  tft.drawFastHLine(20, 38, 280, TFT_WHITE);

  drawToggleRow(60, "Mode:", "Vs CPU", "Local 2P", settingMode, (selectedMenuItem == 0));
  drawToggleRow(110, "Symbol:", "X", "O", settingSymbol, (selectedMenuItem == 1));
  drawToggleRow(160, "Config:", "Easy", "Smart", settingDiff, (selectedMenuItem == 2));

  tft.drawFastHLine(20, 202, 280, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, menuBackgroundColor);
  tft.setTextSize(1);
  tft.setCursor(35, 212);
  tft.print("UP/DOWN: Select Row | LEFT/RIGHT: Toggle");
  tft.setCursor(32, 225);
  tft.print("Press START or ENTER to save and begin match");
}

void processSettingsMenu() {
  bool changed = false;
  if ((buttons[Up].pressed && !buttons[Up].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    selectedMenuItem = (selectedMenuItem + menuItemCount - 1) % menuItemCount;
    changed = true;
  }
  if ((buttons[Down].pressed && !buttons[Down].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    selectedMenuItem = (selectedMenuItem + 1) % menuItemCount;
    changed = true;
  }

  if ((buttons[Left].pressed && !buttons[Left].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (selectedMenuItem == 0) settingMode = (settingMode == 0) ? 1 : 0;
    if (selectedMenuItem == 1) settingSymbol = (settingSymbol == 0) ? 1 : 0;
    if (selectedMenuItem == 2) settingDiff = (settingDiff == 0) ? 1 : 0;
    changed = true;
  }
  if ((buttons[Right].pressed && !buttons[Right].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (selectedMenuItem == 0) settingMode = (settingMode == 0) ? 1 : 0;
    if (selectedMenuItem == 1) settingSymbol = (settingSymbol == 0) ? 1 : 0;
    if (selectedMenuItem == 2) settingDiff = (settingDiff == 0) ? 1 : 0;
    changed = true;
  }

  if (menuNeedsRedraw || changed) {
    drawSettingsMenu();
    menuNeedsRedraw = false;
  }

  if (((buttons[Start].pressed && !buttons[Start].lastPressed) || (buttons[Enter].pressed && !buttons[Enter].lastPressed)) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    applySettingsFromMenu();
    onSettingsMenu = false;
    inGame = true;
    resetGame();
  }
}

void applySettingsFromMenu() {
  isTwoPlayerMode = (settingMode == 1);
  if (settingSymbol == 0) {
    humanSymbol = X;
    computerSymbol = O;
  } else {
    humanSymbol = O;
    computerSymbol = X;
  }
  computerStrategy = (settingDiff == 0) ? randomMove : smartComputer;
}

void drawTicTacToeGrid() {
  tft.fillScreen(backgroundColor);
  // Grid lines at x=125 and x=195 (between cell centers 90/160 and 160/230)
  tft.drawFastVLine(125, 15, 210, TFT_WHITE);
  tft.drawFastVLine(195, 15, 210, TFT_WHITE);
  // Grid lines at y=85 and y=155 (between cell centers 50/120 and 120/190)
  tft.drawFastHLine(55, 85, 210, TFT_WHITE);
  tft.drawFastHLine(55, 155, 210, TFT_WHITE);
  ticTacToeDrawn = true;
}

void drawCellHighlight(int x, int y, uint16_t color) {
  // Draw a red box around the cell (additionally to any preview)
  int left = x - 24;
  int top = y - 24;
  int w = 48;
  int h = 48;
  tft.drawRect(left, top, w, h, TFT_RED);
}

void clearCellHighlight(int x, int y) {
  int left = x - 24;
  int top = y - 24;
  int w = 48;
  int h = 48;
  tft.drawRect(left, top, w, h, backgroundColor);
}

void drawXOrO(int shiftX, int shiftY, boardCellState symbolToDraw) {
  if (symbolToDraw == X) {
    if (drawTemporary) {
      // Draw X in yellow for cursor preview
      for (int row = 0; row < 45; row++) {
        for (int col = 0; col < 45; col++) {
          if (XImage[row][col] != 0x0000) {
            tft.drawPixel(shiftX - 22 + col, shiftY - 22 + row, TFT_YELLOW);
          }
        }
      }
    } else {
      // Draw the full X image in blue (placed piece)
      for (int row = 0; row < 45; row++) {
        for (int col = 0; col < 45; col++) {
          if (XImage[row][col] != 0x0000) {
            tft.drawPixel(shiftX - 22 + col, shiftY - 22 + row, TFT_BLUE);
          }
        }
      }
    }
  } else if (symbolToDraw == O) {
    if (drawTemporary) {
      // Draw O in yellow for cursor preview
      for (int row = 0; row < 45; row++) {
        for (int col = 0; col < 45; col++) {
          if (OImage[row][col] != 0x0000) {
            tft.drawPixel(shiftX - 22 + col, shiftY - 22 + row, TFT_YELLOW);
          }
        }
      }
    } else {
      // Draw the full O image in blue (placed piece)
      for (int row = 0; row < 45; row++) {
        for (int col = 0; col < 45; col++) {
          if (OImage[row][col] != 0x0000) {
            tft.drawPixel(shiftX - 22 + col, shiftY - 22 + row, TFT_BLUE);
          }
        }
      }
    }
  }
}

void clearXorO(int oldX, int oldY, uint16_t bckColor) {
  tft.fillRect(oldX - 23, oldY - 23, 46, 46, bckColor);
}

void getCurrentRowsAndColumns() {
  if (currentX == leftX) currentCol = 0;
  if (currentX == middleX) currentCol = 1;
  if (currentX == rightX) currentCol = 2;

  if (currentY == upperY) currentRow = 0;
  if (currentY == middleY) currentRow = 1;
  if (currentY == bottomY) currentRow = 2;
}

void getOldRowsAndColumns() {
  if (oldX == leftX) oldCol = 0;
  if (oldX == middleX) oldCol = 1;
  if (oldX == rightX) oldCol = 2;

  if (oldY == upperY) oldRow = 0;
  if (oldY == middleY) oldRow = 1;
  if (oldY == bottomY) oldRow = 2;
}

void processUserMovement() {
  getCurrentRowsAndColumns();

  if ((buttons[Left].pressed && !buttons[Left].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (currentX == middleX) currentX = leftX;
    else if (currentX == rightX) currentX = middleX;
  }
  if ((buttons[Right].pressed && !buttons[Right].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (currentX == leftX) currentX = middleX;
    else if (currentX == middleX) currentX = rightX;
  }
  if ((buttons[Up].pressed && !buttons[Up].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (currentY == middleY) currentY = upperY;
    else if (currentY == bottomY) currentY = middleY;
  }
  if ((buttons[Down].pressed && !buttons[Down].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (currentY == upperY) currentY = middleY;
    else if (currentY == middleY) currentY = bottomY;
  }

  if ((buttons[Enter].pressed && !buttons[Enter].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    if (board[currentRow][currentCol] == Empty) {
      boardCellState placedSymbol = (isTwoPlayerMode) ? (isPlayer1Turn ? humanSymbol : computerSymbol) : humanSymbol;
      board[currentRow][currentCol] = placedSymbol;
      drawTemporary = false;
      drawXOrO(currentX, currentY, placedSymbol);
      userMoved = true;
    }
  }
}

void collectEmptyCells() {
  emptyCount = 0;
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board[row][col] == Empty) {
        emptyRows[emptyCount] = row;
        emptyCols[emptyCount] = col;
        emptyCount++;
      }
    }
  }
}

void getCoordsFromEmpty(int empRow, int empCol) {
  if (empCol == 0) emptyX = leftX;
  if (empCol == 1) emptyX = middleX;
  if (empCol == 2) emptyX = rightX;

  if (empRow == 0) emptyY = upperY;
  if (empRow == 1) emptyY = middleY;
  if (empRow == 2) emptyY = bottomY;
}

void placeComputerPiece(int row, int col) {
  board[row][col] = computerSymbol;
  getCoordsFromEmpty(row, col);
  drawTemporary = false;
  drawXOrO(emptyX, emptyY, computerSymbol);
  notYetDrawn = true;
}

void processComputerMove() {
  collectEmptyCells();
  if (emptyCount == 0) return;

  if (computerStrategy == randomMove) {
    int choice = random(0, emptyCount);
    placeComputerPiece(emptyRows[choice], emptyCols[choice]);
    return;
  }

  if (computerStrategy == smartComputer) {
    int targetRow, targetCol;
    if (findWinningMove(computerSymbol, targetRow, targetCol)) {
      placeComputerPiece(targetRow, targetCol);
      return;
    }
    if (findWinningMove(humanSymbol, targetRow, targetCol)) {
      placeComputerPiece(targetRow, targetCol);
      return;
    }
    if (board[1][1] == Empty) {
      placeComputerPiece(1, 1);
      return;
    }
    int choice = random(0, emptyCount);
    placeComputerPiece(emptyRows[choice], emptyCols[choice]);
  }
}

bool hasWin(boardCellState symbol) {
  if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) return true;
  if (board[2][0] == symbol && board[1][1] == symbol && board[0][2] == symbol) return true;

  for (int i = 0; i < 3; i++) {
    if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) return true;
    if (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol) return true;
  }
  return false;
}

bool findWinningMove(boardCellState symbol, int& outRow, int& outCol) {
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board[row][col] == Empty) {
        board[row][col] = symbol;
        bool wins = hasWin(symbol);
        board[row][col] = Empty;
        if (wins) {
          outRow = row;
          outCol = col;
          return true;
        }
      }
    }
  }
  return false;
}

void checkForWins() {
  if (hasWin(humanSymbol)) {
    gameOver = true;
    if (isTwoPlayerMode) p1Won = true;
    else humanWins = true;
    return;
  }
  if (hasWin(computerSymbol)) {
    gameOver = true;
    if (isTwoPlayerMode) p2Won = true;
    else computerWins = true;
    return;
  }

  bool hasEmpty = false;
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board[row][col] == Empty) hasEmpty = true;
    }
  }

  if (!hasEmpty) {
    gameOver = true;
    tie = true;
  }
}

void displayWinner() {
  int boxW = 200;
  int boxH = 90;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);

  tft.setTextSize(2);
  if (tie) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(boxX + 54, boxY + 18);
    tft.print("TIE MATCH");
  } else if (humanWins) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(boxX + 48, boxY + 18);
    tft.print("YOU WIN!");
  } else if (computerWins) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(boxX + 44, boxY + 18);
    tft.print("YOU LOSE");
  } else if (p1Won) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(boxX + 36, boxY + 18);
    tft.print("PLAYER 1 WINS");
  } else if (p2Won) {
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setCursor(boxX + 36, boxY + 18);
    tft.print("PLAYER 2 WINS");
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 34, boxY + 56);
  tft.print("Press START for Options");
}

void resetGame() {
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  humanWins = false;
  computerWins = false;
  p1Won = false;
  p2Won = false;
  tie = false;
  userMoved = false;
  notYetDrawn = true;
  endScreenShown = false;
  pauseScreenDrawn = false;
  isPlayer1Turn = true;
  emptyCount = 0;
  currentX = middleX;
  currentY = middleY;
  currentRow = -1;
  currentCol = -1;
  oldRow = -1;
  oldCol = -1;

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      board[row][col] = Empty;
    }
  }

  if (settingSymbol == 0) {
    colorToDraw = TFT_CYAN;
  } else {
    colorToDraw = TFT_RED;
  }
  ticTacToeDrawn = false;
}

}  // namespace TicTacToeGame