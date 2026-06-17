#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace PacmanGame {

// Using inline allows a single-file header implementation without duplicate symbol linker errors
inline bool launcherExit = false;

inline bool shouldExitToLauncher() {
  return launcherExit;
}

inline const int dacPin = 25;

inline TFT_eSPI tft = TFT_eSPI();
inline const uint16_t backgroundColor = TFT_BLACK;
inline const int maxX = 319;
inline const int maxY = 239;

// Arcade-accurate grid: 28 cols x 31 rows
inline const int mazeCols = 28;
inline const int mazeRows = 31;
inline const int cellSize = 7;
inline const int mazeX = 50;
inline const int mazeY = 12;

// Ghost personalities
enum GhostName { BLINKY, PINKY, INKY, CLYDE };
enum GhostMode { SCATTER, CHASE, FRIGHTENED, EATEN, IN_HOUSE, LEAVING_HOUSE };

// ===================== MAZE LAYOUT =====================
// Legend: #=wall, .=pellet, o=power pellet, -=ghost house wall, G=ghost house gate, ' '=empty
// CRITICAL FIX: Every row is now EXACTLY 28 characters wide to prevent buffer overflow crashes!
inline const char levelMap[mazeRows][mazeCols + 1] = {
  "############################",
  "#............##............#",
  "#.####.#####.##.#####.####.#",
  "#o####.#####.##.#####.####o#",
  "#.####.#####.##.#####.####.#",
  "#..........................#",
  "#.####.##.########.##.####.#",
  "#.####.##.########.##.####.#",
  "#......##....##....##......#",
  "######.#####.##.#####.######",
  "######.#####.##.#####.######",
  "#......##..........##......#",
  "#.####.##.###  ###.##.####.#",
  "#.####.##.---GG---.##.####.#", 
  " o..##.##.|      |.##.##..o ", // Tunnel Row! Openings at columns 0 and 27
  "######.##.|      |.##.######", 
  "     #.##.--------.##.#     ", // Bottom of ghost house
  "     #.##.        .##.#     ",
  "     #.##.        .##.#     ",
  "######.##.        .##.######",
  "      .  .        .  .      ",
  "######.##.        .##.######",
  "     #.##.        .##.#     ",
  "     #.##.        .##.#     ",
  "     #.##.        .##.#     ",
  "######.##.########.##.######",
  "#............##............#",
  "#.####.#####.##.#####.####.#",
  "#.####.#####.##.#####.####.#",
  "#o..##.......  .......##..o#",
  "############################"
};

inline char maze[mazeRows][mazeCols];

// ===================== STRUCTS =====================
struct Actor {
  int col;
  int row;
  int dirX;
  int dirY;
  uint16_t color;
};

inline Actor pacman;
inline int nextDirX = -1;
inline int nextDirY = 0;

inline const int ghostCount = 4;
inline Actor ghosts[ghostCount];
inline GhostMode ghostModes[ghostCount];
inline GhostMode previousModes[ghostCount];
inline unsigned long ghostModeTimer[ghostCount];
inline unsigned long ghostReleaseTime[ghostCount];
inline int ghostFrightenedFlashCount = 0;
inline unsigned long frightenedFlashTimer = 0;
inline bool frightenedFlashState = false;

// Score popup tracking
struct ScorePopup {
  int col;
  int row;
  int points;
  unsigned long drawnAt;
  bool active;
};
inline ScorePopup scorePopups[ghostCount];
inline const unsigned long scorePopupDuration = 1500;

// Ghost scatter targets (corners)
inline const int scatterTargets[4][2] = {
  { mazeCols - 3, 0 },     // Blinky: top-right
  { 2, 0 },                // Pinky: top-left
  { mazeCols - 1, mazeRows - 1 }, // Inky: bottom-right
  { 0, mazeRows - 1 }      // Clyde: bottom-left
};

// Ghost house exit position adjusted to align with the new map geometry
inline const int ghostHouseExitCol = 13;
inline const int ghostHouseExitRow = 13; // Position of the 'G' gate row
inline const int ghostHouseCenterCol = 13;
inline const int ghostHouseCenterRow = 14;

// ===================== GAME STATE =====================
inline int score = 0;
inline int lives = 3;
inline int pelletsLeft = 0;
inline int pelletsEaten = 0;
inline int ghostCombo = 0;
inline bool gameOver = false;
inline bool gameWon = false;
inline bool gameOverScreenDrawn = false;
inline bool gameWonScreenDrawn = false;
inline bool onMenu = true;
inline bool quitConfirm = false;
inline bool gamePaused = false;
inline bool pauseScreenDrawn = false;
inline bool resumeCountdownActive = false;
inline int resumeCountdownValue = 0;
inline unsigned long resumeCountdownLastMs = 0;
inline bool resumeCountdownBoxDrawn = false;
inline int lastDrawnCountdownValue = -1;

inline bool mouthOpen = true;
inline int mouthFrame = 0;
inline unsigned long mouthTimer = 0;

inline unsigned long powerUntilMs = 0;
inline unsigned long lastDebounce = 0;
inline const unsigned long debounceMs = 160;

inline unsigned long nextPacmanMoveMs = 0;
inline unsigned long nextGhostMoveMs = 0;
inline const unsigned long pacmanIntervalMs = 180;
inline const unsigned long ghostIntervalMs = 200;

// Ghost mode timing (milliseconds)
inline const unsigned long scatterDuration = 7000;
inline const unsigned long chaseDuration = 20000;
inline const unsigned long frightenedDuration = 6000;
inline const unsigned long frightenedFlashDuration = 4000;

// Fix Global AI Progression across multiple playthroughs
inline int ghostPhase = 0;
inline unsigned long ghostPhaseStart = 0;

// ===================== SOUND =====================
inline bool soundPlaying = false;
inline int soundSampleIndex = 0;
inline unsigned long lastSoundSampleUs = 0;
inline const unsigned long soundSampleIntervalUs = 125;
inline int currentSoundSampleCount = 0;
inline const uint8_t* currentSoundData = nullptr;

inline const int chompSampleCount = 1000;
inline uint8_t chompSound[chompSampleCount];

inline void generateChompSound() {
  for (int i = 0; i < chompSampleCount; i++) {
    chompSound[i] = (i % 30 < 15) ? 40 : 0;
  }
}

inline void startSound(const uint8_t* data, int count) {
  currentSoundData = data;
  currentSoundSampleCount = count;
  soundPlaying = true;
  soundSampleIndex = 0;
  lastSoundSampleUs = micros();
}

inline void updateSound() {
  if (!soundPlaying) return;
  unsigned long now = micros();
  while (soundPlaying && now - lastSoundSampleUs >= soundSampleIntervalUs) {
    if (soundSampleIndex >= currentSoundSampleCount) {
      dacWrite(dacPin, 0);
      soundPlaying = false;
    } else {
      dacWrite(dacPin, currentSoundData[soundSampleIndex++]);
      lastSoundSampleUs += soundSampleIntervalUs;
    }
  }
}

// ===================== BUTTONS =====================
inline void scanButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

inline bool tapped(buttonName b) {
  return buttons[b].pressed && !buttons[b].lastPressed;
}

inline void updateButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

// ===================== MAZE NAVIGATION =====================
inline bool isWalkable(int col, int row, bool isGhost) {
  // Tunnel wrap - row 14 is the open tunnel row
  if (row == 14 && (col < 0 || col >= mazeCols)) return true;
  if (col < 0 || col >= mazeCols || row < 0 || row >= mazeRows) return false;

  char cell = maze[row][col];
  if (row == 14 && (col == 0 || col == mazeCols - 1)) return true;
  if (cell == '#') return false;
  if (cell == 'G' && !isGhost) return false;
  if (cell == '-' && !isGhost) return false;
  if (cell == '|' && !isGhost) return false;

  return true;
}

// ===================== DRAWING =====================
inline void drawCell(int col, int row) {
  if (col < 0 || col >= mazeCols || row < 0 || row >= mazeRows) return;
  
  int x = mazeX + col * cellSize;
  int y = mazeY + row * cellSize;
  char cell = maze[row][col];

  if (cell == '#') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x + 1, y + 1, cellSize - 2, cellSize - 2, TFT_BLUE);
  } else if (cell == '-') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x, y + cellSize / 2 - 1, cellSize, 2, TFT_WHITE);
  } else if (cell == '|') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x + cellSize / 2 - 1, y, 2, cellSize, TFT_WHITE);
  } else if (cell == 'G') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x, y + cellSize / 2 - 1, cellSize, 2, 0xF81F);
  } else {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    if (cell == '.') {
      tft.fillRect(x + cellSize / 2 - 1, y + cellSize / 2 - 1, 2, 2, 0xFDCB);
    } else if (cell == 'o') {
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 3, 0xFDCB);
    }
  }
}

inline void drawMaze() {
  tft.fillScreen(backgroundColor);
  for (int r = 0; r < mazeRows; r++) {
    for (int c = 0; c < mazeCols; c++) {
      drawCell(c, r);
    }
  }
}

inline void drawPacman() {
  if (pacman.col < 0 || pacman.col >= mazeCols) {
    int x, y;
    if (pacman.col < 0) {
      x = mazeX - cellSize / 2;
    } else {
      x = mazeX + mazeCols * cellSize + cellSize / 2;
    }
    y = mazeY + pacman.row * cellSize + cellSize / 2;
    tft.fillCircle(x, y, 3, TFT_YELLOW);
    return;
  }
  int x = mazeX + pacman.col * cellSize + cellSize / 2;
  int y = mazeY + pacman.row * cellSize + cellSize / 2;

  tft.fillRect(x - 4, y - 4, 9, 9, backgroundColor);

  char cell = maze[pacman.row][pacman.col];
  if (cell == '.') {
    tft.fillRect(x - 1, y - 1, 2, 2, 0xFDCB);
  } else if (cell == 'o') {
    tft.fillCircle(x, y, 3, 0xFDCB);
  }

  tft.fillCircle(x, y, 3, TFT_YELLOW);

  if (mouthOpen) {
    int mouthSize = 2;
    int px1, py1, px2, py2;

    if (pacman.dirX == 1) { // right
      px1 = x; py1 = y - mouthSize;
      px2 = x; py2 = y + mouthSize;
    } else if (pacman.dirX == -1) { // left
      px1 = x; py1 = y - mouthSize;
      px2 = x; py2 = y + mouthSize;
    } else if (pacman.dirY == -1) { // up
      px1 = x - mouthSize; py1 = y;
      px2 = x + mouthSize; py2 = y;
    } else { // down
      px1 = x - mouthSize; py1 = y;
      px2 = x + mouthSize; py2 = y;
    }
    tft.fillTriangle(x, y, px1, py1, px2, py2, backgroundColor);
  }
}

inline void drawGhost(int index) {
  Actor& g = ghosts[index];
  if (g.col < 0 || g.col >= mazeCols) return;

  int x = mazeX + g.col * cellSize + cellSize / 2;
  int y = mazeY + g.row * cellSize + cellSize / 2;
  unsigned long now = millis();

  GhostMode mode = ghostModes[index];

  drawCell(g.col, g.row);

  if (mode == EATEN) {
    tft.fillRect(x - 2, y - 1, 2, 2, TFT_WHITE);
    tft.fillRect(x + 1, y - 1, 2, 2, TFT_WHITE);
    int eyeDirX = (ghostHouseCenterCol > g.col) ? 1 : (ghostHouseCenterCol < g.col) ? -1 : 0;
    int eyeDirY = (ghostHouseCenterRow > g.row) ? 1 : (ghostHouseCenterRow < g.row) ? -1 : 0;
    tft.fillRect(x - 2 + (eyeDirX > 0 ? 1 : 0), y - 1 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLACK);
    tft.fillRect(x + 1 + (eyeDirX > 0 ? 1 : 0), y - 1 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLACK);
    return;
  }

  uint16_t bodyColor;
  if (mode == FRIGHTENED) {
    if (now >= frightenedFlashTimer) {
      bodyColor = frightenedFlashState ? TFT_WHITE : TFT_BLUE;
    } else {
      bodyColor = TFT_BLUE;
    }
  } else {
    bodyColor = g.color;
  }

  tft.fillCircle(x, y, 2, bodyColor);
  tft.fillRect(x - 2, y + 2, 5, 2, bodyColor);

  tft.drawPixel(x - 1, y + 3, bodyColor);
  tft.drawPixel(x + 1, y + 3, bodyColor);

  if (mode == FRIGHTENED) {
    tft.fillRect(x - 2, y - 2, 2, 2, TFT_WHITE);
    tft.fillRect(x + 1, y - 2, 2, 2, TFT_WHITE);
  } else {
    tft.fillRect(x - 2, y - 2, 2, 2, TFT_WHITE);
    tft.fillRect(x + 1, y - 2, 2, 2, TFT_WHITE);
    int eyeDirX = g.dirX;
    int eyeDirY = g.dirY;
    tft.fillRect(x - 2 + (eyeDirX > 0 ? 1 : 0), y - 2 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLUE);
    tft.fillRect(x + 1 + (eyeDirX > 0 ? 1 : 0), y - 2 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLUE);
  }
}

inline void eraseActor(Actor a) {
  if (a.col < 0 || a.col >= mazeCols) return;
  drawCell(a.col, a.row);
}

inline void drawHud() {
  tft.fillRect(0, 0, mazeX, maxY + 1, backgroundColor);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(4, 20);
  tft.print("SCORE");
  tft.setCursor(4, 36);
  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.print(score);

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setCursor(4, 70);
  tft.print("LIVES");
  tft.setCursor(4, 86);
  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.print(lives < 0 ? 0 : lives);
}

// Forward declarations
inline void resetActors();
inline void initMaze();

// ===================== GHOST AI =====================
inline int manhattanDist(int c1, int r1, int c2, int r2) {
  return abs(c1 - c2) + abs(r1 - r2);
}

inline void chooseGhostDirection(int index) {
  Actor& g = ghosts[index];
  GhostMode mode = ghostModes[index];

  if (mode == EATEN) {
    int options[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int bestDir = -1;
    int bestDist = 9999;

    for (int i = 0; i < 4; i++) {
      int dc = options[i][0];
      int dr = options[i][1];
      if (dc == -g.dirX && dr == -g.dirY) continue;

      int nc = g.col + dc;
      int nr = g.row + dr;

      if (!isWalkable(nc, nr, true)) continue;

      int d = manhattanDist(nc, nr, ghostHouseCenterCol, ghostHouseCenterRow);
      if (d < bestDist) {
        bestDist = d;
        bestDir = i;
      }
    }

    if (bestDir != -1) {
      g.dirX = options[bestDir][0];
      g.dirY = options[bestDir][1];
    }
    return;
  }

  if (mode == FRIGHTENED) {
    int options[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int validDirs[4];
    int validCount = 0;

    for (int i = 0; i < 4; i++) {
      int dc = options[i][0];
      int dr = options[i][1];
      if (dc == -g.dirX && dr == -g.dirY) continue;
      int nc = g.col + dc;
      int nr = g.row + dr;
      if (isWalkable(nc, nr, true)) {
        validDirs[validCount++] = i;
      }
    }

    if (validCount > 0) {
      int chosen = validDirs[random(0, validCount)];
      g.dirX = options[chosen][0];
      g.dirY = options[chosen][1];
    } else {
      g.dirX = -g.dirX;
      g.dirY = -g.dirY;
    }
    return;
  }

  int targetCol, targetRow;

  if (mode == SCATTER) {
    targetCol = scatterTargets[index][0];
    targetRow = scatterTargets[index][1];
  } else {
    switch (index) {
      case BLINKY:
        targetCol = pacman.col;
        targetRow = pacman.row;
        break;

      case PINKY:
        targetCol = pacman.col + pacman.dirX * 4;
        targetRow = pacman.row + pacman.dirY * 4;
        break;

      case INKY: {
        int pivotCol = pacman.col + pacman.dirX * 2;
        int pivotRow = pacman.row + pacman.dirY * 2;
        int blinkyCol = ghosts[BLINKY].col;
        int blinkyRow = ghosts[BLINKY].row;
        targetCol = pivotCol + (pivotCol - blinkyCol);
        targetRow = pivotRow + (pivotRow - blinkyRow);
        break;
      }

      case CLYDE: {
        int dist = manhattanDist(g.col, g.row, pacman.col, pacman.row);
        if (dist > 8) {
          targetCol = pacman.col;
          targetRow = pacman.row;
        } else {
          targetCol = scatterTargets[CLYDE][0];
          targetRow = scatterTargets[CLYDE][1];
        }
        break;
      }
    }
  }

  int options[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
  int bestDir = -1;
  int bestDist = 9999;

  for (int i = 0; i < 4; i++) {
    int dc = options[i][0];
    int dr = options[i][1];
    if (dc == -g.dirX && dr == -g.dirY) continue;

    int nc = g.col + dc;
    int nr = g.row + dr;

    if (nr == 14 && (nc == -1 || nc == mazeCols)) {
      bestDir = i;
      break;
    }

    if (!isWalkable(nc, nr, true)) continue;

    int d = manhattanDist(nc, nr, targetCol, targetRow);
    if (d < bestDist) {
      bestDist = d;
      bestDir = i;
    }
  }

  if (bestDir != -1) {
    g.dirX = options[bestDir][0];
    g.dirY = options[bestDir][1];
  } else {
    g.dirX = -g.dirX;
    g.dirY = -g.dirY;
  }
}

// ===================== GHOST MODE MANAGEMENT =====================
inline void setGhostMode(GhostMode mode) {
  unsigned long now = millis();
  for (int i = 0; i < ghostCount; i++) {
    if (ghostModes[i] == IN_HOUSE || ghostModes[i] == LEAVING_HOUSE || ghostModes[i] == EATEN) continue;
    if (mode == FRIGHTENED && ghostModes[i] != FRIGHTENED) {
      previousModes[i] = ghostModes[i];
      ghostModes[i] = FRIGHTENED;
      ghosts[i].dirX = -ghosts[i].dirX;
      ghosts[i].dirY = -ghosts[i].dirY;
    } else if (mode != FRIGHTENED && ghostModes[i] == FRIGHTENED) {
      ghostModes[i] = previousModes[i];
    } else if (mode != FRIGHTENED) {
      ghostModes[i] = mode;
    }
  }
  ghostModeTimer[0] = now;
}

inline void updateGhostModes() {
  unsigned long now = millis();
  unsigned long elapsed = now - ghostModeTimer[0];

  if (ghostModes[0] == FRIGHTENED) {
    if (elapsed >= frightenedDuration) {
      setGhostMode(previousModes[0]);
      ghostModeTimer[0] = now;
    } else if (elapsed >= frightenedFlashDuration && !frightenedFlashState) {
      frightenedFlashTimer = now + 200;
      frightenedFlashState = true;
    }
    return;
  }

  unsigned long phaseElapsed = now - ghostPhaseStart;
  const unsigned long phaseDurations[] = { 7000, 20000, 7000, 20000, 5000, 20000, 5000 };
  const int phaseCount = 7;

  if (ghostPhase < phaseCount && phaseElapsed >= phaseDurations[ghostPhase]) {
    ghostPhase++;
    ghostPhaseStart = now;
    GhostMode newMode = (ghostPhase % 2 == 0) ? SCATTER : CHASE;
    setGhostMode(newMode);
  }
}

// ===================== GHOST MOVEMENT =====================
inline void moveGhosts() {
  unsigned long now = millis();
  for (int i = 0; i < ghostCount; i++) {
    if (now < ghostReleaseTime[i]) {
      if (ghostModes[i] == IN_HOUSE) {
        eraseActor(ghosts[i]);
        drawGhost(i);
      }
      continue;
    }

    if (ghostModes[i] == IN_HOUSE) {
      ghostModes[i] = LEAVING_HOUSE;
    }

    if (ghostModes[i] == LEAVING_HOUSE) {
      eraseActor(ghosts[i]);
      if (ghosts[i].row > ghostHouseExitRow) {
        ghosts[i].row--;
      } else if (ghosts[i].row == ghostHouseExitRow) {
        if (ghosts[i].col < ghostHouseExitCol) ghosts[i].col++;
        else if (ghosts[i].col > ghostHouseExitCol) ghosts[i].col--;
        else {
          ghosts[i].row = ghostHouseExitRow - 1; // Step up through the gate cleanly
          ghostModes[i] = (ghostPhase % 2 == 0) ? SCATTER : CHASE;
          ghosts[i].dirX = -1;
          ghosts[i].dirY = 0;
        }
      }
      drawGhost(i);
      continue;
    }

    chooseGhostDirection(i);
    eraseActor(ghosts[i]);
    ghosts[i].col += ghosts[i].dirX;
    ghosts[i].row += ghosts[i].dirY;

    // Tunnel wrap
    if (ghosts[i].row == 14) {
      if (ghosts[i].col < 0) ghosts[i].col = mazeCols - 1;
      else if (ghosts[i].col >= mazeCols) ghosts[i].col = 0;
    }

    if (ghostModes[i] == EATEN && ghosts[i].col == ghostHouseCenterCol && ghosts[i].row == ghostHouseCenterRow) {
      ghostModes[i] = LEAVING_HOUSE;
      ghostReleaseTime[i] = now;
    }

    drawGhost(i);
  }
}

// ===================== PACMAN MOVEMENT =====================
inline void movePacman() {
  if (isWalkable(pacman.col + nextDirX, pacman.row + nextDirY, false)) {
    pacman.dirX = nextDirX;
    pacman.dirY = nextDirY;
  }

  int nextX = pacman.col + pacman.dirX;
  int nextY = pacman.row + pacman.dirY;

  if (pacman.row == 14) {
    if (nextX < 0 || nextX >= mazeCols) {
      eraseActor(pacman);
      if (nextX < 0) {
        pacman.col = mazeCols - 1;
      } else {
        pacman.col = 0;
      }
      pacman.row = nextY;
      drawPacman();
      return;
    }
  }

  if (!isWalkable(nextX, nextY, false)) return;

  eraseActor(pacman);
  pacman.col = nextX;
  pacman.row = nextY;

  char cell = maze[pacman.row][pacman.col];
  if (cell == '.' || cell == 'o') {
    score += (cell == '.') ? 10 : 50;
    pelletsLeft--;
    pelletsEaten++;
    maze[pacman.row][pacman.col] = ' ';
    drawHud();
    startSound(chompSound, chompSampleCount);

    if (cell == 'o') {
      powerUntilMs = millis() + frightenedDuration;
      ghostCombo = 0;
      setGhostMode(FRIGHTENED);
      ghostModeTimer[0] = millis();
      frightenedFlashState = false;
    }

    if (pelletsLeft <= 0) {
      gameWon = true;
    }
  }

  drawPacman();
}

// ===================== SCORE POPUP MANAGEMENT =====================
inline void clearExpiredScorePopups() {
  unsigned long now = millis();
  for (int i = 0; i < ghostCount; i++) {
    if (!scorePopups[i].active) continue;
    if (now - scorePopups[i].drawnAt >= scorePopupDuration) {
      scorePopups[i].active = false;
      drawCell(scorePopups[i].col, scorePopups[i].row);
    }
  }
}

// ===================== COLLISIONS =====================
inline void checkCollisions() {
  for (int i = 0; i < ghostCount; i++) {
    if (ghostModes[i] == EATEN || ghostModes[i] == IN_HOUSE || ghostModes[i] == LEAVING_HOUSE) continue;
    if (ghosts[i].col != pacman.col || ghosts[i].row != pacman.row) continue;

    if (ghostModes[i] == FRIGHTENED) {
      ghostCombo++;
      int points = 200 * ghostCombo;
      score += points;
      drawHud();

      scorePopups[i].col = ghosts[i].col;
      scorePopups[i].row = ghosts[i].row;
      scorePopups[i].points = points;
      scorePopups[i].drawnAt = millis();
      scorePopups[i].active = true;

      int x = mazeX + ghosts[i].col * cellSize;
      int y = mazeY + ghosts[i].row * cellSize;
      tft.fillRect(x, y, cellSize * 3, cellSize, backgroundColor);
      tft.setTextColor(TFT_CYAN, backgroundColor);
      tft.setTextSize(1);
      tft.setCursor(x, y);
      tft.print(points);

      ghostModes[i] = EATEN;
      eraseActor(ghosts[i]);
      ghosts[i].dirX = -ghosts[i].dirX;
      ghosts[i].dirY = -ghosts[i].dirY;
    } else {
      lives--;
      drawHud();
      if (lives <= 0) {
        lives = 0;
        gameOver = true;
      } else {
        delay(300);
        drawMaze();
        resetActors();
        drawPacman();
        for (int g = 0; g < ghostCount; g++) drawGhost(g);
        nextPacmanMoveMs = millis() + pacmanIntervalMs;
        nextGhostMoveMs = millis() + ghostIntervalMs;
      }
      return;
    }
  }
}

// ===================== RESET =====================
inline void resetActors() {
  pacman.col = 14;
  pacman.row = 23;
  pacman.dirX = -1;
  pacman.dirY = 0;
  pacman.color = TFT_YELLOW;
  nextDirX = -1;
  nextDirY = 0;
  mouthOpen = true;
  mouthFrame = 0;

  uint16_t colors[ghostCount] = { TFT_RED, 0xFC10, TFT_CYAN, 0xFDB8 };
  unsigned long now = millis();
  unsigned long delays[ghostCount] = { 0, 5000, 10000, 15000 };

  // Adjusting start rows to match the fixed center house layout
  ghosts[BLINKY] = { 13, 12, -1, 0, colors[BLINKY] }; // Outside above gate
  ghostModes[BLINKY] = CHASE;
  ghostReleaseTime[BLINKY] = now;

  ghosts[PINKY] = { 13, 14, 0, -1, colors[PINKY] }; // Center inside
  ghostModes[PINKY] = IN_HOUSE;
  ghostReleaseTime[PINKY] = now + delays[PINKY];

  ghosts[INKY] = { 12, 14, 0, -1, colors[INKY] }; // Left inside
  ghostModes[INKY] = IN_HOUSE;
  ghostReleaseTime[INKY] = now + delays[INKY];

  ghosts[CLYDE] = { 14, 14, 0, -1, colors[CLYDE] }; // Right inside
  ghostModes[CLYDE] = IN_HOUSE;
  ghostReleaseTime[CLYDE] = now + delays[CLYDE];

  ghostModeTimer[0] = now;
  powerUntilMs = 0;
  ghostCombo = 0;
  frightenedFlashState = false;
  
  // Cleanly wipe AI progression phase variables
  ghostPhase = 0;
  ghostPhaseStart = now;

  for (int i = 0; i < ghostCount; i++) {
    scorePopups[i].active = false;
  }
}

inline void initMaze() {
  pelletsLeft = 0;
  pelletsEaten = 0;
  for (int r = 0; r < mazeRows; r++) {
    for (int c = 0; c < mazeCols; c++) {
      maze[r][c] = levelMap[r][c];
      if (maze[r][c] == '.' || maze[r][c] == 'o') {
        pelletsLeft++;
      }
    }
  }
}

// ===================== MENU / OVERLAYS =====================
inline void drawMenu() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.setTextSize(4);
  tft.setCursor(80, 30);
  tft.print("PAC-MAN");

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(60, 110);
  tft.print("Use Arrow buttons to change direction.");
  tft.setCursor(60, 130);
  tft.print("Press A to Pause. B to Exit.");

  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, backgroundColor);
  tft.setCursor(64, 180);
  tft.print("START TO PLAY");
}

inline void drawQuit() {
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

inline void drawPauseOverlay() {
  int boxW = 170;
  int boxH = 78;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(boxX + 48, boxY + 18);
  tft.print("PAUSED");

  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(boxX + 35, boxY + 52);
  tft.print("Press A to Resume");
  pauseScreenDrawn = true;
}

inline void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextSize(3);
  tft.setCursor(boxX + 56, boxY + 24);
  tft.print(resumeCountdownValue);
  resumeCountdownBoxDrawn = true;
  lastDrawnCountdownValue = resumeCountdownValue;
}

inline void removePauseOverlay() {
  for (int r = 0; r < mazeRows; r++) {
    for (int c = 0; c < mazeCols; c++) {
      drawCell(c, r);
    }
  }
  drawPacman();
  for (int g = 0; g < ghostCount; g++) drawGhost(g);
  drawHud();
  pauseScreenDrawn = false;
}

inline void removeResumeCountdown() {
  removePauseOverlay();
  resumeCountdownBoxDrawn = false;
  lastDrawnCountdownValue = -1;
}

// ===================== GAME OVER / WIN =====================
inline void drawGameOver() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_RED, backgroundColor);
  tft.setTextSize(3);
  tft.setCursor(60, 60);
  tft.print("GAME OVER");
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(60, 120);
  tft.print("Score: ");
  tft.print(score);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, backgroundColor);
  tft.setCursor(64, 180);
  tft.print("START TO PLAY");
}

inline void drawWin() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_GREEN, backgroundColor);
  tft.setTextSize(3);
  tft.setCursor(60, 60);
  tft.print("YOU WIN!");
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(60, 120);
  tft.print("Score: ");
  tft.print(score);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, backgroundColor);
  tft.setCursor(64, 180);
  tft.print("START TO PLAY");
}

// ===================== MAIN GAME LOOP =====================
inline void setupGame() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(backgroundColor);

  generateChompSound();

  score = 0;
  lives = 3;
  gameOver = false;
  gameWon = false;
  onMenu = true;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  resumeCountdownBoxDrawn = false;
  lastDrawnCountdownValue = -1;
  nextPacmanMoveMs = 0;
  nextGhostMoveMs = 0;
  ghostModeTimer[0] = 0;

  initMaze();
  resetActors();

  drawMenu();
}

inline void loopGame() {
  scanButtons();
  updateSound();

  unsigned long now = millis();

  if (onMenu) {
    if (tapped(Start) || tapped(Enter)) {
      onMenu = false;
      gameOver = false;
      gameWon = false;
      score = 0;
      lives = 3;
      initMaze();
      resetActors();
      drawMaze();
      drawPacman();
      for (int g = 0; g < ghostCount; g++) drawGhost(g);
      drawHud();
      nextPacmanMoveMs = now + pacmanIntervalMs;
      nextGhostMoveMs = now + ghostIntervalMs;
      ghostModeTimer[0] = now;
    }
    updateButtons();
    return;
  }

  if (gameOver) {
    if (!gameOverScreenDrawn) {
      drawGameOver();
      gameOverScreenDrawn = true;
    }
    if (tapped(Start) || tapped(Enter)) {
      onMenu = true;
      gameOverScreenDrawn = false;
    }
    updateButtons();
    return;
  }

  if (gameWon) {
    if (!gameWonScreenDrawn) {
      drawWin();
      gameWonScreenDrawn = true;
    }
    if (tapped(Start) || tapped(Enter)) {
      onMenu = true;
      gameWonScreenDrawn = false;
    }
    updateButtons();
    return;
  }

  // Handle pause toggle
  if (tapped(A)) {
    if (!gamePaused) {
      gamePaused = true;
      pauseScreenDrawn = false;
      resumeCountdownActive = false;
    } else if (!resumeCountdownActive) {
      resumeCountdownActive = true;
      resumeCountdownValue = 3;
      resumeCountdownLastMs = now;
      resumeCountdownBoxDrawn = false;
      lastDrawnCountdownValue = -1;
    }
  }

  // Handle quit logic
  // CRITICAL FIX: Restructured with "else if" so button toggles do not instantly cancel themselves out on the same frame execution
  if (tapped(B)) {
    if (!quitConfirm) {
      quitConfirm = true;
      drawQuit();
    } else {
      quitConfirm = false;
      removePauseOverlay();
    }
  } else if (quitConfirm) {
    if (tapped(Enter) || tapped(Start)) {
      launcherExit = true;
    }
    if (tapped(B)) {
      quitConfirm = false;
      removePauseOverlay();
    }
    updateButtons();
    return;
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) {
      drawPauseOverlay();
    }
    if (resumeCountdownActive) {
      if (now - resumeCountdownLastMs >= 1000) {
        resumeCountdownLastMs = now;
        resumeCountdownValue--;
        if (resumeCountdownValue <= 0) {
          resumeCountdownActive = false;
          gamePaused = false;
          removeResumeCountdown();
        } else {
          drawResumeCountdown();
        }
      } else if (!resumeCountdownBoxDrawn) {
        drawResumeCountdown();
      }
    }
    updateButtons();
    return;
  }

  // Read direction input
  if (tapped(Up)) { nextDirX = 0; nextDirY = -1; }
  if (tapped(Down)) { nextDirX = 0; nextDirY = 1; }
  if (tapped(Left)) { nextDirX = -1; nextDirY = 0; }
  if (tapped(Right)) { nextDirX = 1; nextDirY = 0; }

  // Clear expired score popups
  clearExpiredScorePopups();

  // Independent Mouth Animation Hook: Animates continuously and smoothly while running
  if (now - mouthTimer > 100) {
    mouthTimer = now;
    mouthFrame = (mouthFrame + 1) % 3;
    mouthOpen = (mouthFrame < 2);
    drawPacman(); 
  }

  // Move pacman at interval
  if (now >= nextPacmanMoveMs) {
    nextPacmanMoveMs = now + pacmanIntervalMs;
    movePacman();
    checkCollisions();
  }

  // Move ghosts at interval
  if (now >= nextGhostMoveMs) {
    nextGhostMoveMs = now + ghostIntervalMs;
    updateGhostModes();
    moveGhosts();
    checkCollisions();
  }

  updateButtons();
}

} // namespace PacmanGame