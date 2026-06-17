#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <esp_system.h>

#include "ButtonDefinitions.h"

#include "startImage.h"
#include "byeImage.h"
#include "hitPaddle.h"
#include "pongImage.h"

#include "XImage.h"
#include "OImage.h"
#include "winScreen.h"
#include "loseScreen.h"
#include "tieScreen.h"

#include "TetrisGame.h"
#include "SnakeGame.h"
#include "PacmanGame.h"
#include "PongGame.h"
#include "TicTacToeGame.h"
#include "FlappyGame.h"
#include "InvadersGame.h"
#include "FroggerGame.h"
#include "AsteroidsGame.h"
#include "BlackjackGame.h"
#include "GalagaGame.h"
#include "Cube3D.h"
#include "MinesweeperGame.h"
#include "DinoGame.h"

TFT_eSPI menuTft = TFT_eSPI();

enum LauncherState {
  LauncherMenu,
  RunningTetris,
  RunningSnake,
  RunningPacman,
  RunningPong,
  RunningTicTacToe,
  RunningFlappy,
  RunningInvaders,
  RunningFrogger,
  RunningAsteroids,
  RunningBlackjack,
  RunningGalaga,
  RunningCube3D,
  RunningMinesweeper,
  RunningDino
};

LauncherState launcherState = LauncherMenu;
int selectedGame = 0;
int menuScrollOffset = 0;
const int menuVisibleItems = 10;
bool menuNeedsRedraw = true;
bool poweredOn = true;
bool powerLastPressed = false;
unsigned long lastMenuPressMs = 0;
unsigned long lastPowerPressMs = 0;
const unsigned long menuDebounceMs = 180;
const unsigned long powerDebounceMs = 120;
const unsigned long menuRepeatDelay = 300;  // Initial delay before repeat
const unsigned long menuRepeatRate = 100;   // Repeat rate while held
unsigned long lastMenuRepeatMs = 0;
bool menuRepeatActive = false;
int menuRepeatDirection = 0;
int prevSelectedGame = -1;
int prevMenuScrollOffset = -1;
bool prevScrollUp = false;
bool prevScrollDown = false;

const char* gameNames[] = {
  "TETRIS",
  "SNAKE",
  "PAC-MAN",
  "PONG",
  "TIC-TAC-TOE",
  "FLAPPY BIRD",
  "SPACE INVADERS",
  "FROGGER",
  "ASTEROIDS",
  "BLACKJACK",
  "GALAGA",
  "3D CUBE",
  "MINESWEEPER",
  "DINO RUN"
};
const int gameCount = 14;

void handlePowerButton();
void readLauncherButtons();
void updateLauncherButtons();
bool launcherButtonTapped(buttonName button);
void drawLauncherMenu();
void processLauncherMenu();
void startSelectedGame();
void runActiveGame();
void returnToLauncher();

void setup() {
  menuTft.init();
  menuTft.setRotation(1);
  menuTft.fillScreen(TFT_BLACK);

  Serial.begin(9600);
  randomSeed(esp_random());

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  powerLastPressed = digitalRead(buttons[Power].pin);
}

void loop() {
  handlePowerButton();
  if (!poweredOn) {
    delay(20);
    return;
  }

  if (launcherState == LauncherMenu) {
    readLauncherButtons();
    processLauncherMenu();
    updateLauncherButtons();
    delay(10);
    return;
  }

  runActiveGame();
}

void handlePowerButton() {
  bool powerPressed = digitalRead(buttons[Power].pin);
  if (powerPressed && !powerLastPressed && millis() - lastPowerPressMs > powerDebounceMs) {
    lastPowerPressMs = millis();
    poweredOn = !poweredOn;
    if (poweredOn) {
      menuTft.writecommand(0x29);
      if (launcherState == LauncherMenu) {
        menuNeedsRedraw = true;
      }
      for (int i = 0; i < buttonCount; i++) {
        buttons[i].pressed = false;
        buttons[i].lastPressed = false;
      }
      powerLastPressed = false;
    } else {
      menuTft.writecommand(0x28);
    }
  }
  powerLastPressed = powerPressed;
}

void readLauncherButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

void updateLauncherButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

bool launcherButtonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed && millis() - lastMenuPressMs > menuDebounceMs;
}

void drawMenuItem(int index, bool selected) {
  int startY = 50;
  int itemH = 14;
  int gap = 2;
  int totalItemH = itemH + gap;
  int y = startY + index * totalItemH;
  uint16_t boxColor = selected ? TFT_YELLOW : TFT_DARKGREY;
  uint16_t textColor = selected ? TFT_BLACK : TFT_WHITE;
  int idx = menuScrollOffset + index;

  menuTft.fillRect(46, y, 228, itemH, selected ? TFT_YELLOW : TFT_BLACK);
  menuTft.drawRect(46, y, 228, itemH, boxColor);
  menuTft.setTextColor(textColor, selected ? TFT_YELLOW : TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(70, y + 4);
  menuTft.print(gameNames[idx]);
}

void drawScrollIndicators() {
  int startY = 50;
  int itemH = 14;
  int gap = 2;
  int totalItemH = itemH + gap;
  bool showUp = menuScrollOffset > 0;
  bool showDown = menuScrollOffset + menuVisibleItems < gameCount;

  // Clear old indicator area on right side
  menuTft.fillRect(276, 48, 20, 170, TFT_BLACK);

  if (showUp) {
    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(280, 60);
    menuTft.print("^");
  }
  if (showDown) {
    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(280, 190);
    menuTft.print("v");
  }
}

void drawLauncherMenu() {
  menuTft.setRotation(1);
  menuTft.fillScreen(TFT_BLACK);

  menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
  menuTft.setTextSize(2);
  menuTft.setCursor(76, 18);
  menuTft.print("GAME LAUNCHER");
  menuTft.drawFastHLine(24, 44, 272, TFT_WHITE);

  int startY = 50;
  int itemH = 14;
  int gap = 2;
  int totalItemH = itemH + gap;

  for (int i = 0; i < menuVisibleItems && (menuScrollOffset + i) < gameCount; i++) {
    drawMenuItem(i, (menuScrollOffset + i) == selectedGame);
  }

  drawScrollIndicators();

  menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(34, 220);
  menuTft.print("UP/DOWN select   START/ENTER runs   POWER off");

  prevSelectedGame = selectedGame;
  prevMenuScrollOffset = menuScrollOffset;
}

void moveSelection(int direction) {
  int oldSelected = selectedGame;
  int oldScrollOffset = menuScrollOffset;

  selectedGame = (selectedGame + direction + gameCount) % gameCount;

  // Adjust scroll offset to keep selected visible
  if (selectedGame < menuScrollOffset) {
    menuScrollOffset = selectedGame;
  } else if (selectedGame >= menuScrollOffset + menuVisibleItems) {
    menuScrollOffset = selectedGame - menuVisibleItems + 1;
  }

  // If scroll offset changed, redraw all visible items
  if (menuScrollOffset != oldScrollOffset) {
    for (int i = 0; i < menuVisibleItems && (menuScrollOffset + i) < gameCount; i++) {
      drawMenuItem(i, (menuScrollOffset + i) == selectedGame);
    }
    drawScrollIndicators();
  } else {
    // Only redraw the old and new selected items
    int startY = 50;
    int itemH = 14;
    int gap = 2;
    int totalItemH = itemH + gap;

    // Deselect old item
    int oldIndex = oldSelected - menuScrollOffset;
    if (oldIndex >= 0 && oldIndex < menuVisibleItems) {
      drawMenuItem(oldIndex, false);
    }

    // Select new item
    int newIndex = selectedGame - menuScrollOffset;
    if (newIndex >= 0 && newIndex < menuVisibleItems) {
      drawMenuItem(newIndex, true);
    }
  }
}

void processLauncherMenu() {
  if (menuNeedsRedraw) {
    drawLauncherMenu();
    menuNeedsRedraw = false;
  }

  unsigned long now = millis();

  // Handle hold-to-scroll with auto-repeat
  bool upHeld = buttons[Up].pressed;
  bool downHeld = buttons[Down].pressed;

  if (upHeld || downHeld) {
    int dir = upHeld ? -1 : 1;
    bool justTapped = (upHeld && !buttons[Up].lastPressed) || (downHeld && !buttons[Down].lastPressed);

    if (justTapped) {
      // First press - move immediately
      lastMenuPressMs = now;
      lastMenuRepeatMs = now;
      menuRepeatActive = false;
      menuRepeatDirection = dir;
      moveSelection(dir);
    } else if (dir == menuRepeatDirection) {
      // Same direction held - check repeat timing
      if (!menuRepeatActive) {
        // Waiting for initial delay
        if (now - lastMenuPressMs >= menuRepeatDelay) {
          menuRepeatActive = true;
          lastMenuRepeatMs = now;
          moveSelection(dir);
        }
      } else {
        // Auto-repeating
        if (now - lastMenuRepeatMs >= menuRepeatRate) {
          lastMenuRepeatMs = now;
          moveSelection(dir);
        }
      }
    } else {
      // Direction changed - reset
      menuRepeatActive = false;
      menuRepeatDirection = dir;
      lastMenuPressMs = now;
      lastMenuRepeatMs = now;
    }
  } else {
    menuRepeatActive = false;
    menuRepeatDirection = 0;
  }

  if (launcherButtonTapped(Start) || launcherButtonTapped(Enter)) {
    lastMenuPressMs = millis();
    startSelectedGame();
  }
}

void startSelectedGame() {
  switch (selectedGame) {
    case 0:
      TetrisGame::launcherExit = false;
      launcherState = RunningTetris;
      TetrisGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 1:
      SnakeGame::launcherExit = false;
      launcherState = RunningSnake;
      SnakeGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 2:
      PacmanGame::launcherExit = false;
      launcherState = RunningPacman;
      PacmanGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 3:
      PongGame::launcherExit = false;
      launcherState = RunningPong;
      PongGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 4:
      TicTacToeGame::launcherExit = false;
      launcherState = RunningTicTacToe;
      TicTacToeGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 5:
      FlappyGame::launcherExit = false;
      launcherState = RunningFlappy;
      FlappyGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 6:
      InvadersGame::launcherExit = false;
      launcherState = RunningInvaders;
      InvadersGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 7:
      FroggerGame::launcherExit = false;
      launcherState = RunningFrogger;
      FroggerGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 8:
      AsteroidsGame::launcherExit = false;
      launcherState = RunningAsteroids;
      AsteroidsGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 9:
      BlackjackGame::launcherExit = false;
      launcherState = RunningBlackjack;
      BlackjackGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 10:
      GalagaGame::launcherExit = false;
      launcherState = RunningGalaga;
      GalagaGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 11:
      Cube3D::launcherExit = false;
      launcherState = RunningCube3D;
      Cube3D::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 12:
      MinesweeperGame::launcherExit = false;
      launcherState = RunningMinesweeper;
      MinesweeperGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 13:
      DinoGame::launcherExit = false;
      launcherState = RunningDino;
      DinoGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    default:
      break;
  }
}

void runActiveGame() {
  switch (launcherState) {
    case RunningTetris:
      TetrisGame::loopGame();
      if (TetrisGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningSnake:
      SnakeGame::loopGame();
      if (SnakeGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningPacman:
      PacmanGame::loopGame();
      if (PacmanGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningPong:
      PongGame::loopGame();
      if (PongGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningTicTacToe:
      TicTacToeGame::loopGame();
      if (TicTacToeGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningFlappy:
      FlappyGame::loopGame();
      if (FlappyGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningInvaders:
      InvadersGame::loopGame();
      if (InvadersGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningFrogger:
      FroggerGame::loopGame();
      if (FroggerGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningAsteroids:
      AsteroidsGame::loopGame();
      if (AsteroidsGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningBlackjack:
      BlackjackGame::loopGame();
      if (BlackjackGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningGalaga:
      GalagaGame::loopGame();
      if (GalagaGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningCube3D:
      Cube3D::loopGame();
      if (Cube3D::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningMinesweeper:
      MinesweeperGame::loopGame();
      if (MinesweeperGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningDino:
      DinoGame::loopGame();
      if (DinoGame::shouldExitToLauncher()) returnToLauncher();
      break;
    default:
      returnToLauncher();
      break;
  }
}

void returnToLauncher() {
  launcherState = LauncherMenu;
  menuNeedsRedraw = true;
  lastMenuPressMs = millis();
  menuTft.init();
  menuTft.setRotation(1);
  menuTft.fillScreen(TFT_BLACK);

  for (int i = 0; i < buttonCount; i++) {
    buttons[i].pressed = false;
    buttons[i].lastPressed = false;
  }
}
