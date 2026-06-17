#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace InvadersGame {
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false, onMenu = true, gameOver = false, quitConfirm = false, won = false, needsDraw = true;
int shipX = 156, bulletX = -1, bulletY = -1, alienDir = 1, score = 0, lives = 3;
int alienX[18], alienY[18];
bool alienAlive[18];
int prevShipX = 156, prevBulletX = -1, prevBulletY = -1;
int prevAlienX[18], prevAlienY[18];
bool prevAlienAlive[18];
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
unsigned long lastFrame = 0, lastDebounce = 0, lastAlien = 0;
const unsigned long debounceMs = 160;
const unsigned long frameMs = 50;

bool shouldExitToLauncher() {
  return launcherExit;
}
void scanButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) buttons[i].pressed = !digitalRead(buttons[i].pin);
}
bool tapped(buttonName b) {
  return buttons[b].pressed && !buttons[b].lastPressed;
}
void updateButtons() {
  for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
}

void drawMenu() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.setTextSize(3);
  spr.setCursor(38, 32);
  spr.print("SPACE INVADERS");
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(64, 96);
  spr.print("LEFT/RIGHT move");
  spr.setCursor(64, 114);
  spr.print("A or ENTER shoots");
  spr.setCursor(64, 132);
  spr.print("B asks to quit");
  spr.setTextSize(2);
  spr.setCursor(76, 192);
  spr.print("START TO PLAY");
}

void resetGame() {
  gameOver = false;
  quitConfirm = false;
  won = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  shipX = 156;
  bulletX = -1;
  bulletY = -1;
  alienDir = 1;
  score = 0;
  lives = 3;
  for (int i = 0; i < 18; i++) {
    alienX[i] = 42 + (i % 6) * 40;
    alienY[i] = 42 + (i / 6) * 24;
    alienAlive[i] = true;
  }
  lastFrame = millis();
  lastAlien = millis();
}

void drawQuit() {
  spr.fillRect(54, 76, 212, 88, TFT_BLACK);
  spr.drawRect(52, 74, 216, 92, TFT_YELLOW);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(104, 96);
  spr.print("QUIT?");
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(88, 132);
  spr.print("ENTER quits   B cancels");
}

void drawPauseOverlay() {
  spr.fillRect(88, 98, 144, 44, TFT_BLACK);
  spr.drawRect(86, 96, 148, 48, TFT_WHITE);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(104, 112);
  spr.print("PAUSED");
  pauseScreenDrawn = true;
}
void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  if (!resumeCountdownBoxDrawn) {
    spr.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    spr.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    spr.setTextColor(TFT_YELLOW, TFT_BLACK);
    spr.setTextSize(2);
    spr.setCursor(boxX + 24, boxY + 12);
    spr.print("RESUME");
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextSize(1);
    spr.setCursor(boxX + 28, boxY + 74);
    spr.print("A to resume");
    resumeCountdownBoxDrawn = true;
  }

  spr.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(4);
  spr.setCursor(boxX + 50, boxY + 36);
  spr.print(resumeCountdownValue);
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false;
  resumeCountdownValue = 3;
  resumeCountdownLastMs = millis();
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
      return false;
    }
    drawResumeCountdown();
  }
  return true;
}

void fillClipped(int x, int y, int w, int h, uint16_t color) {
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > 320) w = 320 - x;
  if (y + h > 240) h = 240 - y;
  if (w > 0 && h > 0) spr.fillRect(x, y, w, h, color);
}
void drawHud() {
  spr.fillRect(0, 0, 320, 20, TFT_BLACK);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(8, 8);
  spr.print("SCORE ");
  spr.print(score);
  spr.setCursor(246, 8);
  spr.print("LIVES ");
  spr.print(lives);
}
void drawAlienAt(int x, int y) {
  spr.fillRoundRect(x - 10, y - 7, 20, 14, 3, TFT_GREEN);
  spr.fillCircle(x - 4, y - 1, 2, TFT_BLACK);
  spr.fillCircle(x + 4, y - 1, 2, TFT_BLACK);
}
void erasePreviousFrame() {
  fillClipped(prevShipX - 14, 212, 28, 22, TFT_BLACK);
  if (prevBulletY >= 0) fillClipped(prevBulletX - 3, prevBulletY - 2, 7, 12, TFT_BLACK);
  for (int i = 0; i < 18; i++)
    if (prevAlienAlive[i]) fillClipped(prevAlienX[i] - 12, prevAlienY[i] - 9, 24, 18, TFT_BLACK);
}
void rememberFrame() {
  prevShipX = shipX;
  prevBulletX = bulletX;
  prevBulletY = bulletY;
  for (int i = 0; i < 18; i++) {
    prevAlienX[i] = alienX[i];
    prevAlienY[i] = alienY[i];
    prevAlienAlive[i] = alienAlive[i];
  }
}

void drawFrame() {
  if (needsDraw) {
    spr.fillSprite(TFT_BLACK);
    needsDraw = false;
  } else erasePreviousFrame();
  drawHud();
  spr.fillTriangle(shipX, 214, shipX - 12, 232, shipX + 12, 232, TFT_CYAN);
  if (bulletY >= 0) spr.fillRect(bulletX - 1, bulletY, 3, 8, TFT_YELLOW);
  for (int i = 0; i < 18; i++)
    if (alienAlive[i]) drawAlienAt(alienX[i], alienY[i]);
  rememberFrame();
}

void drawEnd() {
  spr.fillRect(66, 78, 188, 86, TFT_BLACK);
  spr.drawRect(64, 76, 192, 90, won ? TFT_GREEN : TFT_RED);
  spr.setTextColor(won ? TFT_GREEN : TFT_RED, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(won ? 112 : 94, 96);
  spr.print(won ? "YOU WIN" : "GAME OVER");
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(102, 130);
  spr.print("START again");
}

void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(1);
  spr.setColorDepth(8);
  spr.createSprite(320, 240);
  randomSeed(esp_random());
  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
  onMenu = true;
  needsDraw = true;
}

void loopGame() {
  scanButtons();
  if (onMenu) {
    if (needsDraw) {
      drawMenu();
      spr.pushSprite(0, 0);
      needsDraw = false;
    }
    if ((tapped(Start) || tapped(Enter)) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      onMenu = false;
      resetGame();
    }
    updateButtons();
    delay(10);
    return;
  }
  if (quitConfirm) {
    if (tapped(B)) {
      quitConfirm = false;
      needsDraw = true;
    }
    if (tapped(Enter)) launcherExit = true;
    updateButtons();
    delay(10);
    return;
  }
  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuit();
    spr.pushSprite(0, 0);
    updateButtons();
    return;
  }
  if (gameOver || won) {
    if (tapped(Start) || tapped(Enter)) resetGame();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }
  if (tapped(A) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      needsDraw = true;
      startResumeCountdown();
    } else drawPauseOverlay();
  }
  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      spr.pushSprite(0, 0);
      updateButtons();
      delay(10);
      return;
    }
  }
  if (gamePaused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }
  if (buttons[Left].pressed) shipX = max(16, shipX - 5);
  if (buttons[Right].pressed) shipX = min(304, shipX + 5);
  if ((tapped(A) || tapped(Enter)) && bulletY < 0) {
    bulletX = shipX;
    bulletY = 206;
  }
  if (millis() - lastFrame > frameMs) {
    lastFrame = millis();
    if (bulletY >= 0) bulletY -= 7;
    if (bulletY < 24) bulletY = -1;
    for (int i = 0; i < 18; i++)
      if (alienAlive[i] && bulletY >= 0 && abs(bulletX - alienX[i]) < 13 && abs(bulletY - alienY[i]) < 12) {
        alienAlive[i] = false;
        bulletY = -1;
        score += 10;
      }
    bool any = false;
    for (int i = 0; i < 18; i++)
      if (alienAlive[i]) any = true;
    if (!any) won = true;
    if (millis() - lastAlien > 420) {
      lastAlien = millis();
      bool edge = false;
      for (int i = 0; i < 18; i++)
        if (alienAlive[i] && (alienX[i] < 20 || alienX[i] > 300)) edge = true;
      if (edge) alienDir = -alienDir;
      for (int i = 0; i < 18; i++)
        if (alienAlive[i]) {
          alienX[i] += alienDir * 10;
          if (edge) alienY[i] += 8;
          if (alienY[i] > 196) gameOver = true;
        }
    }
    drawFrame();
    if (gameOver || won) drawEnd();
    spr.pushSprite(0, 0);
  }
  updateButtons();
  delay(8);
}
}  // namespace InvadersGame