#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace PongGame {
bool launcherExit = false;
bool shouldExitToLauncher() {
  return launcherExit;
}
const int dacPin = 25;

TFT_eSPI tft = TFT_eSPI();

const uint16_t backgroundColor = TFT_GREEN;
const uint16_t menuBackgroundColor = TFT_BLACK;
const int screenCenterX = 160;
const int screenCenterY = 120;
const int maxX = 319;
const int maxY = 239;

bool startImageDrawn = false;
bool byeImageDrawn = false;
bool onStartScreen = true;
bool onSettingsMenu = false;
bool inPongGame = false;
bool gameOver = false;
bool gamePaused = false;
bool courtDrawn = false;
bool menuNeedsRedraw = true;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;

const int paddleWidth = 5;
const int paddleHeight = 40;
const int playerX = 10;
const int computerX = maxX - 10 - paddleWidth;
const int ballSize = 5;

const int playerSpeed = 3;
const int minBallSpeed = 2;

int computerSpeed = 2;
int computerAimError = 45;
int maxBallSpeed = 8;
int winScore = 5;

const int menuItemCount = 3;
int selectedMenuItem = 0;
int difficultySetting = 0;
int winScoreSetting = 5;
int ballSpeedSetting = 0;

const char* difficultyLabels[] = { "Easy", "Med", "Hard" };
const char* ballSpeedLabels[] = { "Slow", "Med", "Fast" };

int playerY = screenCenterY - paddleHeight / 2;
int computerY = screenCenterY - paddleHeight / 2;
int playerOldY = -1;
int computerOldY = -1;
int ballX = screenCenterX - ballSize / 2;
int ballY = screenCenterY - ballSize / 2;
int ballVelX = minBallSpeed;
int ballVelY = 2;

// FIX: Changed sentinel value from -1 to -999 so off-screen negative positions don't trip safety checks
int oldBallX = -999;
int oldBallY = -999;

int playerScore = 0;
int computerScore = 0;
int lastPlayerScore = -1;
int lastComputerScore = -1;

unsigned long lastFrameMs = 0;
const unsigned long frameIntervalMs = 16;
const int hitPaddleSampleCount = 1133;
const unsigned long soundSampleIntervalUs = 125;
const unsigned long serveDelayMs = 500;
bool soundPlaying = false;
int soundSampleIndex = 0;
unsigned long lastSoundSampleUs = 0;

bool servePending = false;
bool pendingServeTowardRight = true;
unsigned long serveDelayStartMs = 0;
unsigned int computerMoveFrame = 0;

const unsigned long menuCooldownMs = 400;
const unsigned long menuReleaseHoldMs = 250;
unsigned long menuLastActionMs = 0;
unsigned long menuReleaseStartMs = 0;
bool menuActionLocked = false;
bool playerWonGame = false;
bool computerWonGame = false;
bool gameOverOverlayDrawn = false;
bool quitConfirm = false;

// Forward Declarations to guarantee compilation
void readButtons();
void updateButtonStates();
bool buttonPressed(buttonName button);
void resetMenuInputState();
void updateMenuInputGate();
bool menuCanAcceptInput();
bool menuButtonPressed(buttonName button);
void applySettingsFromMenu();
void decreaseSelectedSetting();
void increaseSelectedSetting();
void drawSlider(int x, int y, int width, int value, int minValue, int maxValue, bool selected);
void drawSettingsMenu();
void processSettingsMenu();
void startNewMatch();
void resetToTitle();
void togglePause();
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void clearPauseOverlay();
void redrawActiveGame();
void drawCourt();
void redrawCenterLineInRect(int rx, int ry, int rw, int rh);
void clearBallArea(int x, int y);
void clearBallTrail(int fromX, int fromY, int toX, int toY);
void drawPaddle(int x, int y, uint16_t color);
void processPlayerMovement();
int predictBallYAtX(int targetX);
void processComputerMovement();
bool rectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
void handlePaddleHit(int paddleX, int paddleY, bool isLeftPaddle);
void processBallMovement();
void scheduleServe(bool towardRight);
void updateServeDelay();
void serveBall(bool towardRight);
void drawScore();
void endGame();
void displayStartImage();
void displayByeImage();
void drawGameOverOverlay();
void displayGameOverScreen();
void startCollisionSound();
void updateCollisionSound();

void setupGame() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(backgroundColor);
  randomSeed(esp_random());
  resetMenuInputState();

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  applySettingsFromMenu();
}

void loopGame() {
  readButtons();
  updateCollisionSound();

  if (onStartScreen) {
    displayStartImage();
    updateMenuInputGate();
    if (menuButtonPressed(Start)) {
      onStartScreen = false;
      onSettingsMenu = true;
      menuNeedsRedraw = true;
      resetMenuInputState();
      tft.fillScreen(menuBackgroundColor);
    }
    updateButtonStates();
    return;
  }

  if (onSettingsMenu) {
    processSettingsMenu();
    updateButtonStates();
    return;
  }

  if (gameOver) {
    displayGameOverScreen();
    updateMenuInputGate();
    if (menuButtonPressed(Start)) {
      resetToTitle();
    }
    updateButtonStates();
    return;
  }

  if (inPongGame) {
    if (quitConfirm) {
      if (buttonPressed(B)) {
        cancelQuitOverlay();
      }
      if (buttonPressed(Enter)) {
        resetToTitle();
      }
      updateButtonStates();
      return;
    }

    if (buttonPressed(B)) {
      quitConfirm = true;
      drawQuitOverlay();
      updateButtonStates();
      return;
    }

    if (buttonPressed(A)) {
      togglePause();
    }

    if (resumeCountdownActive) {
      if (updateResumeCountdown()) {
        updateButtonStates();
        return;
      }
    }

    if (!gamePaused) {
      updateServeDelay();
      unsigned long now = millis();
      if (now - lastFrameMs >= frameIntervalMs) {
        lastFrameMs = now;
        processPlayerMovement();
        processComputerMovement();
        if (!servePending) {
          processBallMovement();
        }
        drawScore();
      }
    }
  }

  updateButtonStates();
}

void readButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

void updateButtonStates() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

bool buttonPressed(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed;
}

void resetMenuInputState() {
  menuLastActionMs = 0;
  menuReleaseStartMs = 0;
  menuActionLocked = false;
}

void updateMenuInputGate() {
  bool anyDown = false;
  for (int i = 1; i < buttonCount; i++) {
    if (buttons[i].pressed) {
      anyDown = true;
      break;
    }
  }

  if (!anyDown) {
    if (menuActionLocked) {
      if (menuReleaseStartMs == 0) {
        menuReleaseStartMs = millis();
      } else if (millis() - menuReleaseStartMs >= menuReleaseHoldMs) {
        menuActionLocked = false;
        menuReleaseStartMs = 0;
      }
    } else {
      menuReleaseStartMs = 0;
    }
  } else {
    menuReleaseStartMs = 0;
  }
}

bool menuCanAcceptInput() {
  if (menuActionLocked) {
    return false;
  }
  if (millis() - menuLastActionMs < menuCooldownMs) {
    return false;
  }
  return true;
}

bool menuButtonPressed(buttonName button) {
  if (!(buttons[button].pressed && !buttons[button].lastPressed)) {
    return false;
  }

  updateMenuInputGate();
  if (!menuCanAcceptInput()) {
    return false;
  }

  menuLastActionMs = millis();
  menuActionLocked = true;
  return true;
}

void applySettingsFromMenu() {
  winScore = winScoreSetting;

  switch (difficultySetting) {
    case 0:
      computerSpeed = 1;
      computerAimError = 50;
      break;
    case 1:
      computerSpeed = 2;
      computerAimError = 28;
      break;
    default:
      computerSpeed = 2;
      computerAimError = 12;
      break;
  }

  switch (ballSpeedSetting) {
    case 0:
      maxBallSpeed = 5;
      break;
    case 1:
      maxBallSpeed = 7;
      break;
    default:
      maxBallSpeed = 9;
      break;
  }
}

void decreaseSelectedSetting() {
  switch (selectedMenuItem) {
    case 0:
      difficultySetting = max(0, difficultySetting - 1);
      break;
    case 1:
      winScoreSetting = max(3, winScoreSetting - 1);
      break;
    case 2:
      ballSpeedSetting = max(0, ballSpeedSetting - 1);
      break;
  }
}

void increaseSelectedSetting() {
  switch (selectedMenuItem) {
    case 0:
      difficultySetting = min(2, difficultySetting + 1);
      break;
    case 1:
      winScoreSetting = min(10, winScoreSetting + 1);
      break;
    case 2:
      ballSpeedSetting = min(2, ballSpeedSetting + 1);
      break;
  }
}

void drawSlider(int x, int y, int width, int value, int minValue, int maxValue, bool selected) {
  uint16_t trackColor = selected ? TFT_DARKGREY : TFT_NAVY;
  uint16_t fillColor = selected ? TFT_YELLOW : TFT_WHITE;

  tft.drawRect(x, y, width, 14, TFT_WHITE);
  tft.fillRect(x + 1, y + 1, width - 2, 12, trackColor);
  int fillWidth = map(value - minValue, 0, maxValue - minValue, 0, width - 4);
  if (fillWidth > 0) {
    tft.fillRect(x + 2, y + 3, fillWidth, 8, fillColor);
  }
}

void drawSettingsMenu() {
  tft.fillScreen(menuBackgroundColor);

  tft.setTextColor(TFT_WHITE, menuBackgroundColor);
  tft.setTextSize(2);
  tft.setCursor(72, 16);
  tft.print("PONG SETUP");

  const int labelX = 24;
  const int sliderX = 24;
  const int sliderWidth = 180;
  const int valueX = 230;
  int rowY = 58;
  const int rowStep = 52;

  for (int i = 0; i < menuItemCount; i++) {
    bool selected = i == selectedMenuItem;
    uint16_t labelColor = selected ? TFT_YELLOW : TFT_WHITE;

    tft.setTextColor(labelColor, menuBackgroundColor);
    tft.setTextSize(1);
    tft.setCursor(labelX, rowY);
    switch (i) {
      case 0:
        tft.print("Difficulty");
        drawSlider(sliderX, rowY + 14, sliderWidth, difficultySetting, 0, 2, selected);
        tft.setCursor(valueX, rowY + 16);
        tft.print(difficultyLabels[difficultySetting]);
        break;
      case 1:
        tft.print("Win Score");
        drawSlider(sliderX, rowY + 14, sliderWidth, winScoreSetting, 3, 10, selected);
        tft.setCursor(valueX, rowY + 16);
        tft.print(winScoreSetting);
        break;
      case 2:
        tft.print("Ball Speed");
        drawSlider(sliderX, rowY + 14, sliderWidth, ballSpeedSetting, 0, 2, selected);
        tft.setCursor(valueX, rowY + 16);
        tft.print(ballSpeedLabels[ballSpeedSetting]);
        break;
    }

    rowY += rowStep;
  }

  tft.setTextColor(TFT_DARKGREY, menuBackgroundColor);
  tft.setTextSize(1);
  tft.setCursor(24, 210);
  tft.print("UP/DOWN select LEFT/RIGHT adjust");
  tft.setCursor(24, 222);
  tft.print("Release button between each press");
  tft.setCursor(24, 234);
  tft.print("START to play");
}

void processSettingsMenu() {
  bool changed = false;
  if (menuButtonPressed(Up)) {
    selectedMenuItem = (selectedMenuItem + menuItemCount - 1) % menuItemCount;
    changed = true;
  }
  if (menuButtonPressed(Down)) {
    selectedMenuItem = (selectedMenuItem + 1) % menuItemCount;
    changed = true;
  }
  if (menuButtonPressed(Left)) {
    decreaseSelectedSetting();
    changed = true;
  }
  if (menuButtonPressed(Right)) {
    increaseSelectedSetting();
    changed = true;
  }
  if (menuButtonPressed(Start) || menuButtonPressed(Enter)) {
    applySettingsFromMenu();
    onSettingsMenu = false;
    inPongGame = true;
    startNewMatch();
    return;
  }

  updateMenuInputGate();
  if (changed || menuNeedsRedraw) {
    drawSettingsMenu();
    menuNeedsRedraw = false;
  }
}

void startNewMatch() {
  tft.fillScreen(backgroundColor);
  courtDrawn = false;
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  byeImageDrawn = false;
  playerScore = 0;
  computerScore = 0;
  lastPlayerScore = -1;
  lastComputerScore = -1;
  servePending = false;
  soundPlaying = false;
  playerY = screenCenterY - paddleHeight / 2;
  computerY = screenCenterY - paddleHeight / 2;
  playerOldY = -1;
  computerOldY = -1;
  oldBallX = -999;
  oldBallY = -999;
  computerMoveFrame = 0;
  drawCourt();
  drawPaddle(playerX, playerY, TFT_WHITE);
  drawPaddle(computerX, computerY, TFT_WHITE);
  serveBall(true);
}

void resetToTitle() {
  tft.fillScreen(backgroundColor);
  startImageDrawn = false;
  onStartScreen = true;
  inPongGame = false;
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  launcherExit = true;
}

void togglePause() {
  gamePaused = !gamePaused;
  if (gamePaused) {
    drawPauseOverlay();
  } else {
    clearPauseOverlay();
    startResumeCountdown();
  }
}

void drawPauseOverlay() {
  tft.fillRect(80, 80, 160, 60, TFT_BLACK);
  tft.drawRect(80, 80, 160, 60, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(120, 95);
  tft.print("PAUSED");
  tft.setTextSize(1);
  tft.setCursor(95, 120);
  tft.print("Press A to Resume");
}

void clearPauseOverlay() {
  redrawActiveGame();
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownValue = 3;
  resumeCountdownLastMs = millis();
  resumeCountdownBoxDrawn = false;
  drawResumeCountdown();
}

bool updateResumeCountdown() {
  if (!resumeCountdownActive) return false;

  unsigned long now = millis();
  if (now - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs = now;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      redrawActiveGame();
      return false;
    }
    drawResumeCountdown();
  }
  return true;
}

void drawResumeCountdown() {
  if (!resumeCountdownBoxDrawn) {
    tft.fillRect(110, 80, 100, 60, TFT_BLACK);
    tft.drawRect(110, 80, 100, 60, TFT_WHITE);
    resumeCountdownBoxDrawn = true;
  } else {
    tft.fillRect(111, 81, 98, 58, TFT_BLACK);
  }
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(150, 98);
  tft.print(resumeCountdownValue);
}

void drawQuitOverlay() {
  tft.fillRect(60, 70, 200, 80, TFT_BLACK);
  tft.drawRect(60, 70, 200, 80, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(85, 85);
  tft.print("QUIT TO LAUNCHER?");
  tft.setCursor(80, 110);
  tft.setTextColor(TFT_GREEN);
  tft.print("ENTER -> Confirm");
  tft.setCursor(80, 125);
  tft.setTextColor(TFT_RED);
  tft.print("B     -> Cancel");
}

void cancelQuitOverlay() {
  quitConfirm = false;
  redrawActiveGame();
}

void redrawActiveGame() {
  tft.fillScreen(backgroundColor);
  courtDrawn = false;
  drawCourt();
  playerOldY = -1;
  computerOldY = -1;
  drawPaddle(playerX, playerY, TFT_WHITE);
  drawPaddle(computerX, computerY, TFT_WHITE);
  oldBallX = -999;
  oldBallY = -999;
  if (!servePending) {
    tft.fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);
  }
  lastPlayerScore = -1;
  lastComputerScore = -1;
  drawScore();
}

void drawCourt() {
  if (courtDrawn) return;
  tft.drawRect(0, 0, maxX + 1, maxY + 1, TFT_WHITE);
  for (int y = 0; y < maxY; y += 12) {
    tft.fillRect(screenCenterX - 1, y + 3, 2, 6, TFT_WHITE);
  }
  courtDrawn = true;
}

void redrawCenterLineInRect(int rx, int ry, int rw, int rh) {
  int cx = screenCenterX - 1;
  if (cx + 2 < rx || cx > rx + rw) return;
  int startY = max(ry, 0);
  int endY = min(ry + rh, maxY);
  for (int y = startY; y < endY; y++) {
    int rem = y % 12;
    if (rem >= 3 && rem < 9) {
      tft.drawFastHLine(cx, y, 2, TFT_WHITE);
    }
  }
}

void clearBallArea(int x, int y) {
  tft.fillRect(x, y, ballSize, ballSize, backgroundColor);
  redrawCenterLineInRect(x, y, ballSize, ballSize);
}

void clearBallTrail(int fromX, int fromY, int toX, int toY) {
  if (fromX == toX && fromY == toY) return;
  clearBallArea(fromX, fromY);
}

void drawPaddle(int x, int y, uint16_t color) {
  int oldY = (x == playerX) ? playerOldY : computerOldY;
  if (oldY == y) return;

  if (oldY != -1) {
    if (y > oldY) {
      tft.fillRect(x, oldY, paddleWidth, y - oldY, backgroundColor);
    } else {
      tft.fillRect(x, y + paddleHeight, paddleWidth, oldY - y, backgroundColor);
    }
  }
  tft.fillRect(x, y, paddleWidth, paddleHeight, color);

  if (x == playerX) playerOldY = y;
  else computerOldY = y;
}

void processPlayerMovement() {
  int newY = playerY;
  if (buttons[Up].pressed) newY -= playerSpeed;
  if (buttons[Down].pressed) newY += playerSpeed;
  newY = constrain(newY, 1, maxY - paddleHeight - 1);
  if (newY != playerY) {
    playerY = newY;
    drawPaddle(playerX, playerY, TFT_WHITE);
  }
}

int predictBallYAtX(int targetX) {
  if (ballVelX <= 0) return screenCenterY;
  int tempX = ballX;
  int tempY = ballY;
  int tempVelY = ballVelY;
  while (tempX < targetX) {
    tempX += ballVelX;
    tempY += tempVelY;
    if (tempY <= 1) {
      tempY = 1;
      tempVelY = -tempVelY;
    } else if (tempY >= maxY - ballSize - 1) {
      tempY = maxY - ballSize - 1;
      tempVelY = -tempVelY;
    }
  }
  return tempY;
}

void processComputerMovement() {
  computerMoveFrame++;
  static int targetY = screenCenterY;
  if (computerMoveFrame % 10 == 0) {
    if (ballVelX > 0 && ballX > screenCenterX) {
      int predictedY = predictBallYAtX(computerX);
      int error = random(-computerAimError, computerAimError + 1);
      targetY = predictedY + error - paddleHeight / 2;
    } else {
      targetY = ballY - paddleHeight / 2 + random(-15, 16);
    }
  }
  int currentCenter = computerY + paddleHeight / 2;
  int desiredCenter = targetY + paddleHeight / 2;
  int newY = computerY;
  if (abs(currentCenter - desiredCenter) > 4) {
    if (currentCenter < desiredCenter) newY += computerSpeed;
    else newY -= computerSpeed;
  }
  newY = constrain(newY, 1, maxY - paddleHeight - 1);
  if (newY != computerY) {
    computerY = newY;
    drawPaddle(computerX, computerY, TFT_WHITE);
  }
}

bool rectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
  return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by);
}

void handlePaddleHit(int paddleX, int paddleY, bool isLeftPaddle) {
  startCollisionSound();
  ballVelX = -ballVelX;
  if (isLeftPaddle) {
    ballX = paddleX + paddleWidth + 1;
  } else {
    ballX = paddleX - ballSize - 1;
  }
  int paddleCenter = paddleY + paddleHeight / 2;
  int ballCenter = ballY + ballSize / 2;
  int hitOffset = ballCenter - paddleCenter;
  ballVelY = hitOffset / 5;
  if (ballVelY == 0) ballVelY = (random(0, 2) == 0) ? 1 : -1;
  if (abs(ballVelX) < maxBallSpeed) {
    ballVelX += (ballVelX > 0) ? 1 : -1;
  }
}

void processBallMovement() {
  int newX = ballX + ballVelX;
  int newY = ballY + ballVelY;

  if (newY <= 1) {
    newY = 1;
    ballVelY = -ballVelY;
    startCollisionSound();
  } else if (newY >= maxY - ballSize - 1) {
    newY = maxY - ballSize - 1;
    ballVelY = -ballVelY;
    startCollisionSound();
  }

  if (rectsOverlap(newX, newY, ballSize, ballSize, playerX, playerY, paddleWidth, paddleHeight)) {
    handlePaddleHit(playerX, playerY, true);
    newX = ballX;
    newY = ballY;
  } else if (rectsOverlap(newX, newY, ballSize, ballSize, computerX, computerY, paddleWidth, paddleHeight)) {
    handlePaddleHit(computerX, computerY, false);
    newX = ballX;
    newY = ballY;
  }

  if (newX < 0) {
    computerScore++;
    if (computerScore >= winScore) endGame();
    else scheduleServe(true);
    return;
  } else if (newX > maxX - ballSize) {
    playerScore++;
    if (playerScore >= winScore) endGame();
    else scheduleServe(false);
    return;
  }

  if (oldBallX != -999 && oldBallY != -999) {
    clearBallTrail(oldBallX, oldBallY, newX, newY);
  }

  ballX = newX;
  ballY = newY;
  tft.fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);
  oldBallX = ballX;
  oldBallY = ballY;
}

void scheduleServe(bool towardRight) {
  if (oldBallX != -999 && oldBallY != -999) {
    clearBallArea(oldBallX, oldBallY);
  }
  servePending = true;
  pendingServeTowardRight = towardRight;
  serveDelayStartMs = millis();
}

void updateServeDelay() {
  if (!servePending) return;
  if (millis() - serveDelayStartMs >= serveDelayMs) {
    serveBall(pendingServeTowardRight);
  }
}

void serveBall(bool towardRight) {
  servePending = false;
  ballX = screenCenterX - ballSize / 2;
  ballY = screenCenterY - ballSize / 2;
  oldBallX = -999;
  oldBallY = -999;
  ballVelX = towardRight ? minBallSpeed : -minBallSpeed;
  ballVelY = (random(0, 2) == 0) ? 2 : -2;
  tft.fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);
}

void drawScore() {
  if (playerScore == lastPlayerScore && computerScore == lastComputerScore) return;

  tft.setTextSize(2);
  if (playerScore != lastPlayerScore) {
    tft.setTextColor(backgroundColor);
    tft.setCursor(screenCenterX - 40, 15);
    if (lastPlayerScore != -1) tft.print(lastPlayerScore);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(screenCenterX - 40, 15);
    tft.print(playerScore);
    lastPlayerScore = playerScore;
  }
  if (computerScore != lastComputerScore) {
    tft.setTextColor(backgroundColor);
    tft.setCursor(screenCenterX + 25, 15);
    if (lastComputerScore != -1) tft.print(lastComputerScore);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(screenCenterX + 25, 15);
    tft.print(computerScore);
    lastComputerScore = computerScore;
  }
}

void endGame() {
  if (oldBallX != -999 && oldBallY != -999) {
    clearBallArea(oldBallX, oldBallY);
  }
  gameOver = true;
  inPongGame = false;
  playerWonGame = (playerScore >= winScore);
  computerWonGame = (computerScore >= winScore);
  gameOverOverlayDrawn = false;
}

void displayStartImage() {
  if (startImageDrawn) return;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.setCursor(110, 60);
  tft.print("PONG");
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(95, 130);
  tft.print("Press START Button");
  startImageDrawn = true;
}

void displayByeImage() {
  if (byeImageDrawn) return;
  tft.fillScreen(TFT_BLACK);
  byeImageDrawn = true;
}

void drawGameOverOverlay() {
  if (gameOverOverlayDrawn) return;

  tft.fillRect(30, 40, 260, 160, TFT_BLACK);
  tft.drawRect(30, 40, 260, 160, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(92, 55);
  tft.print("MATCH OVER");

  tft.setTextSize(1);
  tft.setCursor(100, 100);
  tft.print("Final Score:");
  tft.setCursor(110, 120);
  tft.print("Player:   ");
  tft.print(playerScore);
  tft.setCursor(110, 135);
  tft.print("Computer: ");
  tft.print(computerScore);

  tft.setTextSize(2);
  if (playerWonGame) {
    tft.setCursor(118, 170);
    tft.setTextColor(TFT_GREEN);
    tft.print("YOU WIN!");
  } else if (computerWonGame) {
    tft.setCursor(110, 170);
    tft.setTextColor(TFT_RED);
    tft.print("YOU LOSE");
  } else {
    tft.setCursor(128, 170);
    tft.print("TIE GAME");
  }

  tft.setTextColor(TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(75, 212);
  tft.print("Press START to return");

  gameOverOverlayDrawn = true;
}

void displayGameOverScreen() {
  displayByeImage();
  drawGameOverOverlay();
}

void startCollisionSound() {
  soundPlaying = true;
  soundSampleIndex = 0;
  lastSoundSampleUs = micros();
}

void updateCollisionSound() {
  if (!soundPlaying) {
    return;
  }

  unsigned long now = micros();
  while (soundPlaying && now - lastSoundSampleUs >= soundSampleIntervalUs) {
    if (soundSampleIndex >= hitPaddleSampleCount) {
      dacWrite(dacPin, 0);
      soundPlaying = false;
      break;
    }

    // Simple synthesized square wave clip sequence logic
    uint8_t sampleValue = 0;
    if (soundSampleIndex < 400) {
      sampleValue = ((soundSampleIndex / 16) % 2 == 0) ? 90 : 20;
    } else if (soundSampleIndex < 800) {
      sampleValue = ((soundSampleIndex / 22) % 2 == 0) ? 70 : 30;
    } else {
      sampleValue = ((soundSampleIndex / 30) % 2 == 0) ? 50 : 35;
    }

    dacWrite(dacPin, sampleValue);
    soundSampleIndex++;
    lastSoundSampleUs += soundSampleIntervalUs;
  }
}
}