#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace GalagaGame {

TFT_eSPI tft = TFT_eSPI();
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false;

bool shouldExitToLauncher() {
  return launcherExit;
}

// ===================== CONSTANTS =====================
const int shipY = 210;
const int shipW = 14;
const int shipH = 12;
const int enemyW = 14;
const int enemyH = 12;
const int bulletW = 3;
const int bulletH = 8;
const int enemyBulletW = 3;
const int enemyBulletH = 6;

const int formationCols = 5;
const int formationRows = 4;
const int totalEnemies = formationCols * formationRows;

// ===================== ENEMY TYPES =====================
// 0=basic (green), 1=medium (yellow), 2=advanced (red)
const int enemyTypes[formationRows][formationCols] = {
  {2, 1, 0, 1, 2},
  {1, 0, 0, 0, 1},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
};

// ===================== GAME STATE =====================
int shipX = 160;
int prevShipX = 160;
bool shipCaptured = false;
int captureTimer = 0;
bool dualShip = false;
int dualShipX = 160;

int score = 0;
int lives = 3;
int level = 1;
int bonusCount = 0;

bool onMenu = true;
bool gameOver = false;
bool gameWon = false;
bool quitConfirm = false;
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
int lastDrawnCountdownValue = -1;
bool needsDraw = true;
bool invincible = false;
unsigned long invincibleUntil = 0;
const unsigned long invincibleDuration = 2000;

unsigned long lastDebounce = 0;
const unsigned long debounceMs = 160;

// ===================== ENEMY FORMATION =====================
struct Enemy {
  int x, y;
  int homeX, homeY;
  int type;
  bool alive;
  bool inFormation;
  bool diving;
  int diveTimer;
  int diveDirX;
  int diveDirY;
  int prevX, prevY;
  bool prevAlive;
};

Enemy enemies[totalEnemies];

int formationCenterX = 160;
int formationY = 30;
int formationDir = 1;
unsigned long lastFormationMove = 0;
const int formationMoveInterval = 600;

// ===================== BULLETS =====================
int bulletX = -1, bulletY = -1;
int prevBulletX = -1, prevBulletY = -1;

const int maxEnemyBullets = 4;
int enemyBulletX[maxEnemyBullets];
int enemyBulletY[maxEnemyBullets];
bool enemyBulletActive[maxEnemyBullets];
int prevEnemyBulletX[maxEnemyBullets];
int prevEnemyBulletY[maxEnemyBullets];
bool prevEnemyBulletActive[maxEnemyBullets];

unsigned long lastEnemyShot = 0;
const int enemyShotInterval = 1200;

// ===================== STARS =====================
struct Star {
  int x, y;
  int speed;
  uint16_t color;
  int prevX, prevY;
};
Star stars[120];

void initStars() {
  for (int i = 0; i < 120; i++) {
    stars[i].x = random(0, 320);
    stars[i].y = random(0, 240);
    stars[i].speed = random(1, 4);
    stars[i].prevX = stars[i].x;
    stars[i].prevY = stars[i].y;
    // Random star colors: white, light blue, light yellow
    int c = random(0, 5);
    if (c == 0) stars[i].color = TFT_WHITE;
    else if (c == 1) stars[i].color = TFT_LIGHTGREY;
    else if (c == 2) stars[i].color = TFT_CYAN;
    else if (c == 3) stars[i].color = TFT_YELLOW;
    else stars[i].color = TFT_SKYBLUE;
  }
}

void updateStars() {
  for (int i = 0; i < 120; i++) {
    stars[i].prevX = stars[i].x;
    stars[i].prevY = stars[i].y;
    stars[i].y += stars[i].speed;
    if (stars[i].y > 239) {
      stars[i].y = 0;
      stars[i].x = random(0, 320);
      stars[i].speed = random(1, 4);
    }
  }
}

void drawStars() {
  for (int i = 0; i < 120; i++) {
    // Erase old position
    tft.drawPixel(stars[i].prevX, stars[i].prevY, TFT_BLACK);
    // Draw new position
    tft.drawPixel(stars[i].x, stars[i].y, stars[i].color);
  }
}

// ===================== BUTTONS =====================
void scanButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

bool tapped(buttonName b) {
  return buttons[b].pressed && !buttons[b].lastPressed;
}

void updateButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

// ===================== DRAWING =====================
void drawShip(int x, bool captured) {
  if (captured) {
    // Draw captured ship (upside down, attached to enemy)
    tft.fillTriangle(x, shipY, x - 6, shipY + 10, x + 6, shipY + 10, TFT_RED);
    tft.fillRect(x - 2, shipY - 4, 4, 4, TFT_RED);
    return;
  }
  // Original Galaga-style ship (Fighter)
  // Main body - wider at bottom, pointed at top
  tft.fillTriangle(x, shipY - 10, x - 8, shipY + 2, x + 8, shipY + 2, TFT_GREEN);
  // Wings
  tft.fillTriangle(x - 8, shipY, x - 14, shipY + 4, x - 8, shipY + 4, TFT_GREEN);
  tft.fillTriangle(x + 8, shipY, x + 14, shipY + 4, x + 8, shipY + 4, TFT_GREEN);
  // Wing tips
  tft.fillRect(x - 15, shipY + 4, 3, 2, TFT_RED);
  tft.fillRect(x + 12, shipY + 4, 3, 2, TFT_RED);
  // Cockpit
  tft.fillCircle(x, shipY - 4, 3, TFT_CYAN);
  // Cockpit highlight
  tft.drawPixel(x - 1, shipY - 5, TFT_WHITE);
  // Engine glow
  tft.fillRect(x - 3, shipY + 2, 6, 3, TFT_YELLOW);
  tft.fillRect(x - 1, shipY + 5, 2, 2, TFT_RED);
}

void drawEnemy(int x, int y, int type) {
  uint16_t color;
  if (type == 0) color = TFT_GREEN;
  else if (type == 1) color = TFT_YELLOW;
  else color = TFT_RED;

  // Body
  tft.fillRoundRect(x - 7, y - 6, 14, 12, 3, color);
  // Wings
  tft.fillTriangle(x - 7, y, x - 12, y + 4, x - 7, y + 4, color);
  tft.fillTriangle(x + 7, y, x + 12, y + 4, x + 7, y + 4, color);
  // Eyes
  tft.fillCircle(x - 3, y - 2, 2, TFT_WHITE);
  tft.fillCircle(x + 3, y - 2, 2, TFT_WHITE);
  tft.fillCircle(x - 3, y - 2, 1, TFT_BLACK);
  tft.fillCircle(x + 3, y - 2, 1, TFT_BLACK);
  // Antenna
  tft.drawPixel(x - 2, y - 7, color);
  tft.drawPixel(x + 2, y - 7, color);
}

void drawBullet(int x, int y) {
  tft.fillRect(x - 1, y, 3, 8, TFT_YELLOW);
}

void drawEnemyBullet(int x, int y) {
  tft.fillRect(x - 1, y, 3, 6, TFT_RED);
}

void drawHud() {
  tft.fillRect(0, 0, 320, 12, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(4, 3);
  tft.print("SCORE ");
  tft.print(score);
  tft.setCursor(140, 3);
  tft.print("LVL ");
  tft.print(level);
  tft.setCursor(240, 3);
  tft.print("LIVES ");
  tft.print(lives);
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(60, 30);
  tft.print("GALAGA");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(50, 90);
  tft.print("LEFT/RIGHT to move");
  tft.setCursor(50, 108);
  tft.print("A or ENTER to shoot");
  tft.setCursor(50, 126);
  tft.print("A to pause, B to quit");

  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(64, 180);
  tft.print("START TO PLAY");
}

void drawQuit() {
  tft.fillRect(54, 76, 212, 88, TFT_BLACK);
  tft.drawRect(52, 74, 216, 92, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(104, 96);
  tft.print("QUIT?");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(88, 132);
  tft.print("ENTER quits   B cancels");
}

void drawPauseOverlay() {
  tft.fillRect(88, 98, 144, 44, TFT_BLACK);
  tft.drawRect(86, 96, 148, 48, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(104, 112);
  tft.print("PAUSED");
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
    lastDrawnCountdownValue = -1;
  }

  // Only redraw the number if it changed
  if (resumeCountdownValue != lastDrawnCountdownValue) {
    tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(boxX + 50, boxY + 36);
    tft.print(resumeCountdownValue);
    lastDrawnCountdownValue = resumeCountdownValue;
  }
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false;
  resumeCountdownValue = 3;
  lastDrawnCountdownValue = -1;
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
      lastDrawnCountdownValue = -1;
      return false;
    }
  }

  drawResumeCountdown();
  return true;
}

// ===================== CONFETTI =====================
struct Confetti {
  int x, y;
  int vx, vy;
  uint16_t color;
  int life;
  bool active;
};
Confetti confetti[40];
unsigned long lastConfettiSpawn = 0;

void initConfetti() {
  for (int i = 0; i < 40; i++) {
    confetti[i].active = false;
  }
}

void spawnConfetti() {
  for (int i = 0; i < 40; i++) {
    if (!confetti[i].active) {
      confetti[i].x = random(0, 320);
      confetti[i].y = random(0, 80);
      confetti[i].vx = random(-2, 3);
      confetti[i].vy = random(1, 4);
      int c = random(0, 7);
      uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_ORANGE};
      confetti[i].color = colors[c];
      confetti[i].life = random(30, 80);
      confetti[i].active = true;
      break;
    }
  }
}

void updateConfetti() {
  unsigned long now = millis();
  if (now - lastConfettiSpawn > 100) {
    lastConfettiSpawn = now;
    spawnConfetti();
  }
  for (int i = 0; i < 40; i++) {
    if (confetti[i].active) {
      // Erase old
      tft.drawPixel(confetti[i].x, confetti[i].y, TFT_BLACK);
      // Update
      confetti[i].x += confetti[i].vx;
      confetti[i].y += confetti[i].vy;
      confetti[i].life--;
      if (confetti[i].life <= 0 || confetti[i].y > 240 || confetti[i].x < 0 || confetti[i].x > 319) {
        confetti[i].active = false;
      } else {
        // Draw new
        tft.drawPixel(confetti[i].x, confetti[i].y, confetti[i].color);
      }
    }
  }
}

void drawEnd() {
  if (gameWon) {
    // Victory celebration - flashy border
    static unsigned long lastFlash = 0;
    static bool flashState = false;
    unsigned long now = millis();
    if (now - lastFlash > 200) {
      lastFlash = now;
      flashState = !flashState;
    }
    uint16_t borderColor = flashState ? TFT_GREEN : TFT_YELLOW;
    tft.fillRect(66, 78, 188, 86, TFT_BLACK);
    tft.drawRect(64, 76, 192, 90, borderColor);
    tft.drawRect(63, 75, 194, 92, borderColor);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(112, 96);
    tft.print("YOU WIN");
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(80, 130);
    tft.print("SCORE: ");
    tft.print(score);
    tft.setCursor(102, 146);
    tft.print("START again");
    updateConfetti();
  } else {
    tft.fillRect(66, 78, 188, 86, TFT_BLACK);
    tft.drawRect(64, 76, 192, 90, TFT_RED);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(94, 96);
    tft.print("GAME OVER");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(102, 130);
    tft.print("START again");
  }
}

// ===================== GAME LOGIC =====================
void initEnemies() {
  for (int r = 0; r < formationRows; r++) {
    for (int c = 0; c < formationCols; c++) {
      int idx = r * formationCols + c;
      enemies[idx].homeX = formationCenterX + (c - 2) * 30;
      enemies[idx].homeY = formationY + r * 22;
      enemies[idx].x = enemies[idx].homeX;
      enemies[idx].y = enemies[idx].homeY;
      enemies[idx].type = enemyTypes[r][c];
      enemies[idx].alive = true;
      enemies[idx].inFormation = true;
      enemies[idx].diving = false;
      enemies[idx].diveTimer = 0;
      enemies[idx].diveDirX = 0;
      enemies[idx].diveDirY = 0;
      enemies[idx].prevX = enemies[idx].x;
      enemies[idx].prevY = enemies[idx].y;
      enemies[idx].prevAlive = true;
    }
  }
}

void resetGame() {
  gameOver = false;
  gameWon = false;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  invincible = false;
  shipX = 160;
  prevShipX = 160;
  shipCaptured = false;
  captureTimer = 0;
  dualShip = false;
  bulletX = -1;
  bulletY = -1;
  prevBulletX = -1;
  prevBulletY = -1;
  score = 0;
  lives = 3;
  level = 1;
  bonusCount = 0;
  formationCenterX = 160;
  formationDir = 1;
  lastFormationMove = millis();
  lastEnemyShot = millis();

  for (int i = 0; i < maxEnemyBullets; i++) {
    enemyBulletActive[i] = false;
    enemyBulletX[i] = -1;
    enemyBulletY[i] = -1;
    prevEnemyBulletActive[i] = false;
  }

  initEnemies();
  initStars();
  initConfetti();
}

void startDive(int idx) {
  if (!enemies[idx].alive || !enemies[idx].inFormation) return;
  enemies[idx].inFormation = false;
  enemies[idx].diving = true;
  enemies[idx].diveTimer = 0;
  // Dive towards player's position
  int dx = shipX - enemies[idx].x;
  int dy = 220 - enemies[idx].y;
  // Normalize direction
  float len = sqrt(dx * dx + dy * dy);
  if (len > 0) {
    enemies[idx].diveDirX = (int)(dx / len * 3);
    enemies[idx].diveDirY = (int)(dy / len * 3);
    if (enemies[idx].diveDirX == 0) enemies[idx].diveDirX = (dx > 0) ? 1 : -1;
    if (enemies[idx].diveDirY == 0) enemies[idx].diveDirY = 1;
  } else {
    enemies[idx].diveDirX = 0;
    enemies[idx].diveDirY = 3;
  }
}

void updateEnemies() {
  unsigned long now = millis();

  // Move formation
  if (now - lastFormationMove > formationMoveInterval) {
    lastFormationMove = now;
    bool edge = false;
    for (int i = 0; i < totalEnemies; i++) {
      if (enemies[i].alive && enemies[i].inFormation) {
        if (enemies[i].x < 20 || enemies[i].x > 300) edge = true;
      }
    }
    if (edge) formationDir = -formationDir;
    for (int i = 0; i < totalEnemies; i++) {
      if (enemies[i].alive && enemies[i].inFormation) {
        enemies[i].x += formationDir * 8;
        enemies[i].homeX = enemies[i].x;
      }
    }
  }

  // Random enemies start diving
  if (random(0, 100) < 3) {
    int idx = random(0, totalEnemies);
    if (enemies[idx].alive && enemies[idx].inFormation) {
      startDive(idx);
    }
  }

  // Update diving enemies
  for (int i = 0; i < totalEnemies; i++) {
    if (!enemies[i].alive || !enemies[i].diving) continue;

    enemies[i].x += enemies[i].diveDirX;
    enemies[i].y += enemies[i].diveDirY;
    enemies[i].diveTimer++;

    // Check if enemy reached bottom - return to formation
    if (enemies[i].y > 220) {
      enemies[i].diving = false;
      enemies[i].inFormation = true;
      enemies[i].x = enemies[i].homeX;
      enemies[i].y = enemies[i].homeY;
    }

    // Check if enemy can capture ship (generous hitbox)
    if (!shipCaptured && abs(enemies[i].x - shipX) < 18 && abs(enemies[i].y - shipY) < 16) {
      shipCaptured = true;
      captureTimer = 0;
    }
  }

  // Enemy shooting
  if (now - lastEnemyShot > enemyShotInterval) {
    lastEnemyShot = now;
    // Find a random alive enemy in formation to shoot
    int shooters[totalEnemies];
    int shooterCount = 0;
    for (int i = 0; i < totalEnemies; i++) {
      if (enemies[i].alive && enemies[i].inFormation) {
        shooters[shooterCount++] = i;
      }
    }
    if (shooterCount > 0) {
      int shooter = shooters[random(0, shooterCount)];
      for (int b = 0; b < maxEnemyBullets; b++) {
        if (!enemyBulletActive[b]) {
          enemyBulletActive[b] = true;
          enemyBulletX[b] = enemies[shooter].x;
          enemyBulletY[b] = enemies[shooter].y + 8;
          break;
        }
      }
    }
  }

  // Move enemy bullets
  for (int b = 0; b < maxEnemyBullets; b++) {
    if (enemyBulletActive[b]) {
      enemyBulletY[b] += 4;
      if (enemyBulletY[b] > 240) {
        enemyBulletActive[b] = false;
      }
    }
  }

  // Check win condition
  bool anyAlive = false;
  for (int i = 0; i < totalEnemies; i++) {
    if (enemies[i].alive) anyAlive = true;
  }
  if (!anyAlive) {
    gameWon = true;
  }
}

void respawnPlayer() {
  shipX = 160;
  prevShipX = 160;
  bulletX = -1;
  bulletY = -1;
  prevBulletX = -1;
  prevBulletY = -1;
  needsDraw = true;
  invincible = true;
  invincibleUntil = millis() + invincibleDuration;
  shipCaptured = false;
  captureTimer = 0;
}

void updatePlayer() {
  // Update invincibility
  if (invincible && millis() > invincibleUntil) {
    invincible = false;
  }

  // Move ship
  if (buttons[Left].pressed) shipX = max(16, shipX - 4);
  if (buttons[Right].pressed) shipX = min(304, shipX + 4);

  // Shoot
  if ((tapped(A) || tapped(Enter)) && bulletY < 0) {
    bulletX = shipX;
    bulletY = shipY - 10;
  }

  // Move bullet
  if (bulletY >= 0) {
    bulletY -= 6;
    if (bulletY < 0) bulletY = -1;
  }

  // Check bullet hits - use full bounding box overlap with generous margins
  if (bulletY >= 0) {
    for (int i = 0; i < totalEnemies; i++) {
      if (enemies[i].alive) {
        // Bullet is 3x8, enemy is 14x12 - check overlap with generous margins
        // Bullet left edge < enemy right edge AND bullet right edge > enemy left edge
        // AND bullet top < enemy bottom AND bullet bottom > enemy top
        int bulletLeft = bulletX - 1;
        int bulletRight = bulletX + 1;
        int bulletTop = bulletY;
        int bulletBottom = bulletY + 8;
        int enemyLeft = enemies[i].x - 7;
        int enemyRight = enemies[i].x + 7;
        int enemyTop = enemies[i].y - 6;
        int enemyBottom = enemies[i].y + 6;
        
        if (bulletRight >= enemyLeft && bulletLeft <= enemyRight &&
            bulletBottom >= enemyTop && bulletTop <= enemyBottom) {
          enemies[i].alive = false;
          enemies[i].inFormation = false;
          enemies[i].diving = false;
          bulletY = -1;
          score += (enemies[i].type == 0) ? 50 : (enemies[i].type == 1) ? 100 : 150;
          break;
        }
      }
    }
  }

  // Skip collision checks if invincible
  if (invincible) return;

  // Check enemy bullet hits player (generous hitbox - ship is 30px wide)
  for (int b = 0; b < maxEnemyBullets; b++) {
    if (enemyBulletActive[b] && abs(enemyBulletX[b] - shipX) < 18 && abs(enemyBulletY[b] - shipY) < 14) {
      enemyBulletActive[b] = false;
      lives--;
      if (lives <= 0) {
        gameOver = true;
        needsDraw = true;
      } else {
        respawnPlayer();
      }
    }
  }

  // Check diving enemy hits player - generous hitbox, enemy also dies on collision
  for (int i = 0; i < totalEnemies; i++) {
    if (enemies[i].alive && enemies[i].diving && abs(enemies[i].x - shipX) < 18 && abs(enemies[i].y - shipY) < 16) {
      // Enemy dies too when crashing into player
      enemies[i].alive = false;
      enemies[i].diving = false;
      enemies[i].inFormation = false;
      lives--;
      if (lives <= 0) {
        gameOver = true;
        needsDraw = true;
      } else {
        respawnPlayer();
      }
    }
  }
}

// ===================== RENDERING =====================
void erasePreviousFrame() {
  // Erase ship
  tft.fillRect(prevShipX - 16, shipY - 12, 33, 20, TFT_BLACK);

  // Erase bullet
  if (prevBulletY >= 0) {
    tft.fillRect(prevBulletX - 2, prevBulletY - 1, 5, 10, TFT_BLACK);
  }

  // Erase enemies
  for (int i = 0; i < totalEnemies; i++) {
    if (enemies[i].prevAlive) {
      tft.fillRect(enemies[i].prevX - 13, enemies[i].prevY - 8, 27, 18, TFT_BLACK);
    }
  }

  // Erase enemy bullets
  for (int b = 0; b < maxEnemyBullets; b++) {
    if (prevEnemyBulletActive[b]) {
      tft.fillRect(prevEnemyBulletX[b] - 2, prevEnemyBulletY[b] - 1, 5, 8, TFT_BLACK);
    }
  }
}

void rememberFrame() {
  prevShipX = shipX;
  prevBulletX = bulletX;
  prevBulletY = bulletY;

  for (int i = 0; i < totalEnemies; i++) {
    enemies[i].prevX = enemies[i].x;
    enemies[i].prevY = enemies[i].y;
    enemies[i].prevAlive = enemies[i].alive;
  }

  for (int b = 0; b < maxEnemyBullets; b++) {
    prevEnemyBulletX[b] = enemyBulletX[b];
    prevEnemyBulletY[b] = enemyBulletY[b];
    prevEnemyBulletActive[b] = enemyBulletActive[b];
  }
}

void drawFrame() {
  if (needsDraw) {
    tft.fillScreen(TFT_BLACK);
    needsDraw = false;
    // Redraw stars after full screen clear
    for (int i = 0; i < 120; i++) {
      tft.drawPixel(stars[i].x, stars[i].y, stars[i].color);
    }
  } else {
    erasePreviousFrame();
  }

  drawStars();
  drawHud();

  // Draw ship (blink during invincibility)
  if (!shipCaptured) {
    if (!invincible || (millis() / 100) % 2 == 0) {
      drawShip(shipX, false);
    }
  }

  // Draw bullet
  if (bulletY >= 0) {
    drawBullet(bulletX, bulletY);
  }

  // Draw enemies
  for (int i = 0; i < totalEnemies; i++) {
    if (enemies[i].alive) {
      drawEnemy(enemies[i].x, enemies[i].y, enemies[i].type);
    }
  }

  // Draw enemy bullets
  for (int b = 0; b < maxEnemyBullets; b++) {
    if (enemyBulletActive[b]) {
      drawEnemyBullet(enemyBulletX[b], enemyBulletY[b]);
    }
  }

  rememberFrame();
}

// ===================== SETUP & LOOP =====================
void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(1);
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
    updateButtons();
    return;
  }

  if (gameOver || gameWon) {
    // Always draw the end screen when game is over/won
    // Use a static flag to avoid redrawing every frame
    static bool endScreenDrawn = false;
    if (!endScreenDrawn) {
      tft.fillScreen(TFT_BLACK);
      drawHud();
      drawEnd();
      endScreenDrawn = true;
    } else if (gameWon) {
      // Update confetti every frame during win screen for continuous animation
      updateConfetti();
    }
    if (tapped(Start) || tapped(Enter)) {
      onMenu = true;
      needsDraw = true;
      endScreenDrawn = false;
    }
    updateButtons();
    delay(10);
    return;
  }

  if (tapped(A) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      // Redraw the full game screen when unpausing (to clear pause overlay)
      needsDraw = true;
      startResumeCountdown();
    } else {
      drawPauseOverlay();
    }
  }

  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      updateButtons();
      delay(10);
      return;
    }
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    updateButtons();
    delay(10);
    return;
  }

  // Update game logic
  updateStars();
  updateEnemies();
  updatePlayer();

  drawFrame();

  updateButtons();
  delay(16);
}

}  // namespace GalagaGame
