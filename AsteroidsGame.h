#pragma once
#include <Arduino.h>
#include <math.h>
#include "ButtonDefinitions.h"

namespace AsteroidsGame {
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false, onMenu = true, gameOver = false, quitConfirm = false, needsDraw = true;
float shipX = 160, shipY = 120, shipVX = 0, shipVY = 0;
int dir = 0, score = 0, lives = 3, bulletX = -1, bulletY = -1, bulletVX = 0, bulletVY = 0;
float rockX[6], rockY[6], rockVX[6], rockVY[6];
bool rockAlive[6];
int prevShipX = 160, prevShipY = 120, prevDir = 0, prevBulletX = -1, prevBulletY = -1, prevScore = -1, prevLives = -1;
float prevRockX[6], prevRockY[6];
bool prevRockAlive[6];
// Explosion particles
struct Particle { int x, y, vx, vy; uint16_t color; int life; };
Particle explosions[30];
int explosionCount = 0;
bool prevExplosionActive = false;
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
unsigned long lastFrame = 0, lastDebounce = 0, lastBulletMs = 0;
unsigned long lastTurnMs = 0;
const unsigned long turnRepeatDelay = 150;  // ms before auto-repeat starts
const unsigned long turnRepeatRate = 100;   // ms between auto-repeat turns
const unsigned long debounceMs = 100;       // lower debounce for snappier response (was 150)
const unsigned long frameMs = 33;           // ~30fps for smoother movement
const unsigned long bulletCooldownMs = 300; // ~33% faster than before

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
int dxForDir(int d) {
  int v[8] = { 0, 2, 3, 2, 0, -2, -3, -2 };
  return v[d];
}
int dyForDir(int d) {
  int v[8] = { -3, -2, 0, 2, 3, 2, 0, -2 };
  return v[d];
}
int dxForDir() {
  return dxForDir(dir);
}
int dyForDir() {
  return dyForDir(dir);
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

// --- Galaga-inspired Ship: clean fighter shape with wings ---
void drawShipAt(int x, int y, int d, uint16_t bodyColor) {
  // Direction vectors
  int fdx = dxForDir(d);  // forward
  int fdy = dyForDir(d);
  int sdx = -dyForDir(d); // sideways (perpendicular)
  int sdy = dxForDir(d);

  // Nose tip
  int noseX = x + fdx * 6;
  int noseY = y + fdy * 6;

  // Body mid points (where wings attach)
  int bodyMidLx = x + fdx * 1 + sdx * 4;
  int bodyMidLy = y + fdy * 1 + sdy * 4;
  int bodyMidRx = x + fdx * 1 - sdx * 4;
  int bodyMidRy = y + fdy * 1 - sdy * 4;

  // Rear of body
  int rearX = x - fdx * 3;
  int rearY = y - fdy * 3;

  // Wing tips (swept back)
  int wingLx = x - fdx * 2 + sdx * 7;
  int wingLy = y - fdy * 2 + sdy * 7;
  int wingRx = x - fdx * 2 - sdx * 7;
  int wingRy = y - fdy * 2 - sdy * 7;

  // Engine glow (flickering orange/yellow) - draw BEFORE ship body
  bool engineOn = buttons[Up].pressed;
  if (engineOn) {
    // Consistent flame length with slight flicker
    int flameLen = 4 + (random(0, 3));
    int fx = x - fdx * (3 + flameLen);
    int fy = y - fdy * (3 + flameLen);
    int flx = rearX - sdx * 2;
    int fly = rearY - sdy * 2;
    int frx = rearX + sdx * 2;
    int fry = rearY + sdy * 2;
    spr.fillTriangle(flx, fly, frx, fry, fx, fy, TFT_ORANGE);
    spr.fillTriangle(flx, fly, frx, fry, fx, fy, TFT_YELLOW);
  }

  // Main body - pointed nose to wide mid to flat rear
  spr.fillTriangle(noseX, noseY, bodyMidLx, bodyMidLy, bodyMidRx, bodyMidRy, bodyColor);
  spr.fillTriangle(bodyMidLx, bodyMidLy, bodyMidRx, bodyMidRy, rearX, rearY, bodyColor);

  // Wings (swept back from mid-body)
  spr.fillTriangle(bodyMidLx, bodyMidLy, wingLx, wingLy, rearX, rearY, TFT_DARKGREY);
  spr.fillTriangle(bodyMidRx, bodyMidRy, wingRx, wingRy, rearX, rearY, TFT_DARKGREY);

  // Wing tip accents
  spr.fillCircle(wingLx, wingLy, 1, TFT_RED);
  spr.fillCircle(wingRx, wingRy, 1, TFT_RED);

  // Ship outline
  spr.drawLine(noseX, noseY, bodyMidLx, bodyMidLy, TFT_WHITE);
  spr.drawLine(noseX, noseY, bodyMidRx, bodyMidRy, TFT_WHITE);
  spr.drawLine(bodyMidLx, bodyMidLy, wingLx, wingLy, TFT_WHITE);
  spr.drawLine(bodyMidRx, bodyMidRy, wingRx, wingRy, TFT_WHITE);
  spr.drawLine(wingLx, wingLy, rearX, rearY, TFT_WHITE);
  spr.drawLine(wingRx, wingRy, rearX, rearY, TFT_WHITE);
  spr.drawLine(bodyMidLx, bodyMidLy, rearX, rearY, TFT_WHITE);
  spr.drawLine(bodyMidRx, bodyMidRy, rearX, rearY, TFT_WHITE);

  // Cockpit window
  int cx = x + fdx * 2;
  int cy = y + fdy * 2;
  spr.fillCircle(cx, cy, 2, TFT_CYAN);
  spr.drawPixel(cx - 1, cy - 1, TFT_WHITE);
}

void erasePreviousFrame() {
  // Ship with wings (span ~14 each side) + flame (up to ~9 behind) = need big erase area
  fillClipped(prevShipX - 28, prevShipY - 28, 56, 56, TFT_BLACK);
  if (prevBulletX >= 0) fillClipped(prevBulletX - 4, prevBulletY - 4, 8, 8, TFT_BLACK);
  for (int i = 0; i < 6; i++)
    if (prevRockAlive[i]) fillClipped((int)prevRockX[i] - 14, (int)prevRockY[i] - 14, 28, 28, TFT_BLACK);
  // Clear explosion particles from previous frame by erasing a large area around them
  if (explosionCount > 0) {
    for (int i = 0; i < explosionCount; i++) {
      fillClipped(explosions[i].x - 1, explosions[i].y - 1, 3, 3, TFT_BLACK);
    }
  }
}
void rememberFrame() {
  prevShipX = (int)shipX;
  prevShipY = (int)shipY;
  prevDir = dir;
  prevBulletX = bulletX;
  prevBulletY = bulletY;
  prevScore = score;
  prevLives = lives;
  for (int i = 0; i < 6; i++) {
    prevRockX[i] = rockX[i];
    prevRockY[i] = rockY[i];
    prevRockAlive[i] = rockAlive[i];
  }
}

void drawMenu() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(3);
  spr.setCursor(74, 32);
  spr.print("ASTEROIDS");
  spr.setTextSize(1);
  spr.setCursor(58, 96);
  spr.print("LEFT/RIGHT rotate, UP thrust");
  spr.setCursor(58, 114);
  spr.print("A or ENTER shoots");
  spr.setCursor(58, 132);
  spr.print("B asks to quit");
  spr.setTextSize(2);
  spr.setCursor(76, 192);
  spr.print("START TO PLAY");
}
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void resetRocks() {
  for (int i = 0; i < 6; i++) {
    rockX[i] = random(20, 300);
    rockY[i] = random(30, 210);
    rockVX[i] = random(-25, 26) / 10.0;
    rockVY[i] = random(-25, 26) / 10.0;
    if (abs(rockVX[i]) < 0.5) rockVX[i] = 1.0;
    if (abs(rockVY[i]) < 0.5) rockVY[i] = -1.0;
    rockAlive[i] = true;
  }
}
void resetGame() {
  gameOver = false;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  shipX = 160;
  shipY = 120;
  shipVX = 0;
  shipVY = 0;
  dir = 0;
  score = 0;
  lives = 3;
  bulletX = -1;
  lastBulletMs = 0;
  resetRocks();
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

// --- Upgraded Asteroids: more detailed rocky look ---
void drawRockAt(int x, int y, int size, uint16_t color) {
  // Draw a rough jagged asteroid
  int r = size;
  // Fill with dark color first
  spr.fillCircle(x, y, r, color);
  // Draw jagged outline using a rough polygon approach
  int px[8], py[8];
  for (int i = 0; i < 8; i++) {
    float angle = i * 45 * 3.14159 / 180;
    int rr = r - 2 + random(0, 5);
    px[i] = x + rr * cos(angle);
    py[i] = y + rr * sin(angle);
  }
  for (int i = 0; i < 8; i++) {
    int next = (i + 1) % 8;
    spr.drawLine(px[i], py[i], px[next], py[next], TFT_WHITE);
  }
  // Add some crater details
  spr.fillCircle(x - r/3, y - r/3, r/5, TFT_BLACK);
  spr.fillCircle(x + r/4, y + r/4, r/4, TFT_BLACK);
}

// --- Explosion system ---
void spawnExplosion(int x, int y, uint16_t color1, uint16_t color2) {
  explosionCount = min(explosionCount + 15, 30);
  int startIdx = 30 - explosionCount;
  for (int i = startIdx; i < 30; i++) {
    explosions[i].x = x + random(-3, 4);
    explosions[i].y = y + random(-3, 4);
    float angle = random(0, 360) * 3.14159 / 180;
    float speed = random(10, 40) / 10.0;
    explosions[i].vx = cos(angle) * speed;
    explosions[i].vy = sin(angle) * speed;
    explosions[i].color = random(0, 2) ? color1 : color2;
    explosions[i].life = random(8, 20);
  }
}

void updateExplosions() {
  for (int i = 0; i < explosionCount; i++) {
    explosions[i].x += explosions[i].vx;
    explosions[i].y += explosions[i].vy;
    explosions[i].life--;
  }
  // Remove dead particles from front
  int writeIdx = 0;
  for (int readIdx = 0; readIdx < explosionCount; readIdx++) {
    if (explosions[readIdx].life > 0) {
      explosions[writeIdx] = explosions[readIdx];
      writeIdx++;
    }
  }
  explosionCount = writeIdx;
}

void drawExplosions() {
  for (int i = 0; i < explosionCount; i++) {
    spr.drawPixel(explosions[i].x, explosions[i].y, explosions[i].color);
  }
}

void drawFrame() {
  if (needsDraw) {
    spr.fillSprite(TFT_BLACK);
    needsDraw = false;
  } else erasePreviousFrame();
  drawHud();
  drawShipAt((int)shipX, (int)shipY, dir, TFT_CYAN);
  if (bulletX >= 0) {
    // Red bullet - simple bright circle
    spr.fillCircle(bulletX, bulletY, 2, TFT_RED);
    spr.drawPixel(bulletX, bulletY, TFT_WHITE);
  }
  for (int i = 0; i < 6; i++)
    if (rockAlive[i]) drawRockAt((int)rockX[i], (int)rockY[i], 10, TFT_DARKGREY);
  drawExplosions();
  rememberFrame();
}
void drawEnd() {
  spr.fillRect(66, 78, 188, 96, TFT_BLACK);
  spr.drawRect(64, 76, 192, 100, TFT_RED);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(94, 96);
  spr.print("GAME OVER");
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(100, 120);
  spr.print("SCORE: ");
  spr.print(score);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(102, 142);
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
  if (gameOver) {
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
  // Hold-to-turn with auto-repeat: initial delay then repeat at moderate rate
  if (buttons[Left].pressed) {
    if (tapped(Left)) {
      dir = (dir + 7) % 8;
      lastTurnMs = millis();
    } else if (millis() - lastTurnMs > turnRepeatDelay) {
      unsigned long elapsed = millis() - lastTurnMs - turnRepeatDelay;
      if (elapsed / turnRepeatRate > (elapsed - 20) / turnRepeatRate) {
        dir = (dir + 7) % 8;
      }
    }
  }
  if (buttons[Right].pressed) {
    if (tapped(Right)) {
      dir = (dir + 1) % 8;
      lastTurnMs = millis();
    } else if (millis() - lastTurnMs > turnRepeatDelay) {
      unsigned long elapsed = millis() - lastTurnMs - turnRepeatDelay;
      if (elapsed / turnRepeatRate > (elapsed - 20) / turnRepeatRate) {
        dir = (dir + 1) % 8;
      }
    }
  }
  // Reset turn timer when neither is held
  if (!buttons[Left].pressed && !buttons[Right].pressed) lastTurnMs = millis();
  if (buttons[Up].pressed) {
    shipVX += dxForDir() * 0.06;  // Slightly stronger thrust
    shipVY += dyForDir() * 0.06;
  }
  // Faster shooting with cooldown timer
  if ((tapped(A) || tapped(Enter)) && bulletX < 0 && millis() - lastBulletMs > bulletCooldownMs) {
    bulletX = (int)shipX;
    bulletY = (int)shipY;
    bulletVX = dxForDir() * 3;  // Faster bullet
    bulletVY = dyForDir() * 3;
    lastBulletMs = millis();
  }
  if (millis() - lastFrame > frameMs) {
    lastFrame = millis();
    shipX += shipVX;
    shipY += shipVY;
    shipVX *= 0.98;  // Less friction = more floaty space feel
    shipVY *= 0.98;
    if (shipX < 0) shipX = 319;
    if (shipX > 319) shipX = 0;
    if (shipY < 20) shipY = 239;
    if (shipY > 239) shipY = 20;
    if (bulletX >= 0) {
      bulletX += bulletVX;
      bulletY += bulletVY;
      if (bulletX < 0 || bulletX > 319 || bulletY < 0 || bulletY > 239) bulletX = -1;
    }
    bool any = false;
    for (int i = 0; i < 6; i++)
      if (rockAlive[i]) {
        any = true;
        rockX[i] += rockVX[i];
        rockY[i] += rockVY[i];
        if (rockX[i] < 0) rockX[i] = 319;
        if (rockX[i] > 319) rockX[i] = 0;
        if (rockY[i] < 20) rockY[i] = 239;
        if (rockY[i] > 239) rockY[i] = 20;
        if (bulletX >= 0 && abs(bulletX - rockX[i]) < 12 && abs(bulletY - rockY[i]) < 12) {
          rockAlive[i] = false;
          bulletX = -1;
          score += 20;
          spawnExplosion((int)rockX[i], (int)rockY[i], TFT_ORANGE, TFT_YELLOW);
        }
        if (abs(shipX - rockX[i]) < 13 && abs(shipY - rockY[i]) < 13) {
          spawnExplosion((int)shipX, (int)shipY, TFT_RED, TFT_ORANGE);
          rockAlive[i] = false;  // Destroy the rock so it can't kill you again on respawn
          lives--;
          shipX = 160;
          shipY = 120;
          shipVX = 0;
          shipVY = 0;
          if (lives <= 0) gameOver = true;
        }
      }
    if (!any) resetRocks();
    updateExplosions();
    drawFrame();
    if (gameOver) drawEnd();
    spr.pushSprite(0, 0);
  }
  updateButtons();
  delay(8);
}

}  // namespace AsteroidsGame
