#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace FroggerGame {
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false, onMenu = true, gameOver = false, quitConfirm = false, won = false, needsDraw = true;
int frogX = 152, frogY = 216, score = 0, lives = 3;
int carX[8], carY[8], carSpeed[8];
int prevFrogX = 152, prevFrogY = 216, prevCarX[8], prevCarY[8];
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
unsigned long lastFrame = 0, lastDebounce = 0;
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

void drawMenu() {
  tft.fillScreen(TFT_DARKGREEN);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
  tft.setTextSize(3);
  tft.setCursor(92, 32);
  tft.print("FROGGER");
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setTextSize(1);
  tft.setCursor(64, 98);
  tft.print("Cross the road and river");
  tft.setCursor(64, 116);
  tft.print("Arrows hop, B asks to quit");
  tft.setTextSize(2);
  tft.setCursor(76, 190);
  tft.print("START TO PLAY");
}
void resetLevel() {
  frogX = 152;
  frogY = 216;
}
void resetGame() {
  gameOver = false;
  quitConfirm = false;
  won = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  score = 0;
  lives = 3;
  resetLevel();
  for (int i = 0; i < 8; i++) {
    carX[i] = random(0, 300);
    carY[i] = 68 + (i % 4) * 32;
    carSpeed[i] = (i % 2 == 0 ? 3 : -4);
  }
  lastFrame = millis();
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
void drawBackground() {
  spr.fillSprite(TFT_BLACK);
  spr.fillRect(0, 0, 320, 38, TFT_BLUE);
  spr.fillRect(0, 40, 320, 28, TFT_DARKGREEN);
  spr.fillRect(0, 68, 320, 132, TFT_DARKGREY);
  spr.fillRect(0, 204, 320, 36, TFT_DARKGREEN);
  for (int y = 84; y < 196; y += 32)
    for (int x = 0; x < 320; x += 28) spr.drawFastHLine(x, y, 14, TFT_WHITE);
}
void drawHud() {
  spr.fillRect(0, 0, 320, 20, TFT_BLUE);
  spr.setTextColor(TFT_WHITE, TFT_BLUE);
  spr.setTextSize(1);
  spr.setCursor(8, 8);
  spr.print("SCORE ");
  spr.print(score);
  spr.setCursor(246, 8);
  spr.print("LIVES ");
  spr.print(lives);
}
void drawFrame() {
  if (needsDraw) {
    drawBackground();
    needsDraw = false;
  } else {
    // Erase previous positions by redrawing background over them
    int ex = prevFrogX - 10, ey = prevFrogY - 12, ew = 20, eh = 22;
    if (ex < 0) { ew += ex; ex = 0; }
    if (ey < 0) { eh += ey; ey = 0; }
    if (ex + ew > 320) ew = 320 - ex;
    if (ey + eh > 240) eh = 240 - ey;
    if (ew > 0 && eh > 0) {
      if (ey < 38) spr.fillRect(ex, ey, ew, min(eh, 38 - ey), TFT_BLUE);
      if (ey < 68 && ey + eh > 40) {
        int top = max(ey, 40), bot = min(ey + eh, 68);
        spr.fillRect(ex, top, ew, bot - top, TFT_DARKGREEN);
      }
      if (ey < 200 && ey + eh > 68) {
        int top = max(ey, 68), bot = min(ey + eh, 200);
        spr.fillRect(ex, top, ew, bot - top, TFT_DARKGREY);
      }
      if (ey < 204 && ey + eh > 200) {
        int top = max(ey, 200), bot = min(ey + eh, 204);
        spr.fillRect(ex, top, ew, bot - top, TFT_BLACK);
      }
      if (ey + eh > 204) {
        int top = max(ey, 204), bot = min(ey + eh, 240);
        spr.fillRect(ex, top, ew, bot - top, TFT_DARKGREEN);
      }
      // Redraw lane dividers in erased area
      for (int y = 84; y < 196; y += 32)
        if (y >= ey && y < ey + eh)
          for (int x = 0; x < 320; x += 28) spr.drawFastHLine(x, y, 14, TFT_WHITE);
    }
    for (int i = 0; i < 8; i++) {
      int cx = prevCarX[i], cy = prevCarY[i];
      spr.fillRect(cx, cy, 28, 16, TFT_DARKGREY);
      // Redraw lane divider if car was over it
      for (int y = 84; y < 196; y += 32)
        if (y >= cy && y < cy + 16)
          for (int x = 0; x < 320; x += 28) spr.drawFastHLine(x, y, 14, TFT_WHITE);
    }
  }
  for (int i = 0; i < 8; i++) spr.fillRoundRect(carX[i], carY[i], 28, 16, 3, i % 2 ? TFT_RED : TFT_ORANGE);
  spr.fillCircle(frogX, frogY, 8, TFT_GREEN);
  spr.fillCircle(frogX - 4, frogY - 5, 2, TFT_WHITE);
  spr.fillCircle(frogX + 4, frogY - 5, 2, TFT_WHITE);
  drawHud();
  prevFrogX = frogX;
  prevFrogY = frogY;
  for (int i = 0; i < 8; i++) { prevCarX[i] = carX[i]; prevCarY[i] = carY[i]; }
}
void drawEnd() {
  spr.fillRect(68, 78, 184, 86, TFT_BLACK);
  spr.drawRect(66, 76, 188, 90, won ? TFT_GREEN : TFT_RED);
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
  // Use full-screen sprite with 8-bit color to save RAM
  // 320x240 at 8-bit = 76,800 bytes (75KB) vs 153,600 bytes (150KB) at 16-bit
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
  if (millis() - lastDebounce > debounceMs) {
    if (tapped(Up)) {
      frogY -= 18;
      lastDebounce = millis();
      score++;
    }
    if (tapped(Down)) {
      frogY += 18;
      lastDebounce = millis();
    }
    if (tapped(Left)) {
      frogX -= 18;
      lastDebounce = millis();
    }
    if (tapped(Right)) {
      frogX += 18;
      lastDebounce = millis();
    }
  }
  frogX = constrain(frogX, 8, 312);
  frogY = constrain(frogY, 20, 224);
  if (frogY < 36) {
    won = true;
    score += 100;
  }
  if (millis() - lastFrame > 32) {
    lastFrame = millis();
    for (int i = 0; i < 8; i++) {
      carX[i] += carSpeed[i];
      if (carX[i] > 330) carX[i] = -30;
      if (carX[i] < -30) carX[i] = 330;
      if (abs(frogX - (carX[i] + 14)) < 18 && abs(frogY - (carY[i] + 8)) < 14) {
        lives--;
        if (lives <= 0) gameOver = true;
        else resetLevel();
      }
    }
    drawFrame();
    if (gameOver || won) drawEnd();
  }
  spr.pushSprite(0, 0);
  updateButtons();
  delay(8);
}

}  // namespace FroggerGame
