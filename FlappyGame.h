#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace FlappyGame {
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
const uint16_t skyColor = 0x867D;
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false;
bool onMenu = true;
bool gameOver = false;
bool quitConfirm = false;
bool needsDraw = true;
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
int birdX = 52;
int birdY = 112;
int birdV = 0;
int pipeX = 320;
int gapY = 96;
int score = 0;
int prevBirdX = 52;
int prevBirdY = 112;
int prevPipeX = 320;
int prevGapY = 96;
unsigned long lastFrame = 0;
unsigned long lastDebounce = 0;
const unsigned long frameMs = 50;
const unsigned long debounceMs = 160;

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

void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();

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

void drawMenu() {
  spr.fillSprite(skyColor);
  spr.setTextColor(TFT_YELLOW, skyColor);
  spr.setTextSize(3);
  spr.setCursor(52, 36);
  spr.print("FLAPPY BIRD");
  spr.setTextColor(TFT_BLACK, skyColor);
  spr.setTextSize(1);
  spr.setCursor(70, 98);
  spr.print("A / UP / ENTER flaps");
  spr.setCursor(70, 116);
  spr.print("Fly through the pipes");
  spr.setCursor(70, 134);
  spr.print("B asks to quit");
  spr.setTextSize(2);
  spr.setCursor(70, 190);
  spr.print("START TO PLAY");
}

void resetGame() {
  gameOver = false;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  birdY = 112;
  birdV = 0;
  pipeX = 320;
  gapY = random(58, 160);
  score = 0;
  lastFrame = millis();
  prevBirdX = birdX;
  prevBirdY = birdY;
  prevPipeX = pipeX;
  prevGapY = gapY;
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

void returnToMenu() {
  launcherExit = true;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
}

void drawFrame() {
  if (needsDraw) {
    spr.fillSprite(skyColor);
    needsDraw = false;
  } else {
    fillClipped(0, 0, 320, 20, skyColor);
    fillClipped(prevBirdX - 10, prevBirdY - 10, 28, 20, skyColor);
    fillClipped(prevPipeX, 0, 32, prevGapY - 38, skyColor);
    fillClipped(prevPipeX, prevGapY + 38, 32, 216 - prevGapY - 38, skyColor);
  }
  spr.fillRect(0, 216, 320, 24, TFT_GREEN);
  spr.fillRect(pipeX, 0, 32, gapY - 38, TFT_GREEN);
  spr.fillRect(pipeX, gapY + 38, 32, 216 - gapY - 38, TFT_GREEN);

  // --- Upgraded Bird Character Rendering ---
  // 1. Main Yellow Body
  spr.fillCircle(birdX, birdY, 8, TFT_YELLOW);

  // 2. Cartoon Eye (White base + Black Pupil)
  spr.fillCircle(birdX + 3, birdY - 3, 2, TFT_WHITE);
  spr.drawPixel(birdX + 4, birdY - 3, TFT_BLACK);

  // 3. Orange Beak
  spr.fillTriangle(birdX + 7, birdY - 2, birdX + 15, birdY, birdX + 7, birdY + 3, TFT_ORANGE);

  // 4. Flapping Wing (Alternates position dynamically every 140ms)
  bool wingUp = (millis() / 140) % 2 == 0;
  if (wingUp) {
    // Wing Angled Upwards (using a slightly darker gold/orange 0xFD60)
    spr.fillTriangle(birdX - 5, birdY, birdX - 1, birdY - 7, birdX + 2, birdY, 0xFD60);
  } else {
    // Wing Angled Downwards
    spr.fillTriangle(birdX - 5, birdY, birdX - 1, birdY + 6, birdX + 2, birdY, 0xFD60);
  }
  // -----------------------------------------

  spr.setTextColor(TFT_BLACK, skyColor);
  spr.setTextSize(2);
  spr.setCursor(8, 8);
  spr.print(score);
  prevBirdX = birdX;
  prevBirdY = birdY;
  prevPipeX = pipeX;
  prevGapY = gapY;
}

void drawGameOver() {
  spr.fillRect(70, 78, 180, 82, TFT_BLACK);
  spr.drawRect(68, 76, 184, 86, TFT_RED);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(100, 96);
  spr.print("GAME OVER");
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(100, 118);
  spr.print("SCORE: ");
  spr.print(score);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(100, 138);
  spr.print("START again");
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
    if (tapped(Enter)) returnToMenu();
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
    } else {
      drawPauseOverlay();
    }
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
  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuit();
    spr.pushSprite(0, 0);
    updateButtons();
    return;
  }
  if (gameOver) {
    if (tapped(Start) || tapped(Enter)) resetGame();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }
  if ((tapped(A) || tapped(Up) || tapped(Enter)) && millis() - lastDebounce > 80) {
    lastDebounce = millis();
    birdV = -6;
  }
  if (millis() - lastFrame >= frameMs) {
    lastFrame = millis();
    birdV++;
    birdY += birdV;
    pipeX -= 4;
    if (pipeX < -32) {
      pipeX = 320;
      gapY = random(58, 160);
      score++;
    }
    if (birdY < 8 || birdY > 208) gameOver = true;
    if (birdX + 8 > pipeX && birdX - 8 < pipeX + 32 && (birdY - 8 < gapY - 38 || birdY + 8 > gapY + 38)) gameOver = true;
    drawFrame();
    if (gameOver) drawGameOver();
    spr.pushSprite(0, 0);
  }
  updateButtons();
  delay(8);
}

}  // namespace FlappyGame