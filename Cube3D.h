#pragma once
#include <Arduino.h>
#include <math.h>
#include "ButtonDefinitions.h"

namespace Cube3D {

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
bool launcherExit = false;

bool shouldExitToLauncher() {
  return launcherExit;
}

// ===================== 3D CUBE ENGINE =====================
const float cubeVertices[8][3] = {
  {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
  {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
};

const int cubeEdges[12][2] = {
  {0,1}, {1,2}, {2,3}, {3,0},
  {4,5}, {5,6}, {6,7}, {7,4},
  {0,4}, {1,5}, {2,6}, {3,7}
};

const int cubeFaces[6][4] = {
  {0,1,2,3}, {4,5,6,7}, {0,1,5,4},
  {2,3,7,6}, {0,3,7,4}, {1,2,6,5}
};

const uint16_t faceColors[6] = {
  TFT_RED, TFT_BLUE, TFT_GREEN, TFT_YELLOW, TFT_MAGENTA, TFT_CYAN
};

const float faceNormals[6][3] = {
  { 0, 0, -1}, { 0, 0,  1}, { 0, -1,  0},
  { 0,  1,  0}, {-1,  0,  0}, { 1,  0,  0}
};

int projectedX[8];
int projectedY[8];

float angleX = 0, angleY = 0, angleZ = 0;
const float rotSpeedX = 0.02, rotSpeedY = 0.03, rotSpeedZ = 0.01;
const float scale = 50;
const int centerX = 160, centerY = 120;
int rotationStyle = 0;

// For screensaver mode
int ssDirX = 1, ssDirY = 1;
int ssOffsetX = 0, ssOffsetY = 0;
const int ssSpeed = 1;

int lastDrawnStyle = -1;

// For explode mode - vertex oscillation offsets
float vertexOffsets[8] = {0,0,0,0,0,0,0,0};
float explodePhase = 0;

// Light direction for shaded mode (normalized)
const float lightDir[3] = {0.577, 0.577, 0.577};

// Sprite dimensions for the cube area (covers max cube extent + margin)
const int sprW = 200;
const int sprH = 200;
const int sprOffsetX = (320 - sprW) / 2;  // 60
const int sprOffsetY = (240 - sprH) / 2;  // 20

void rotatePoint(float x, float y, float z, float &rx, float &ry, float &rz) {
  float y1 = y * cos(angleX) - z * sin(angleX);
  float z1 = y * sin(angleX) + z * cos(angleX);
  float x2 = x * cos(angleY) + z1 * sin(angleY);
  float z2 = -x * sin(angleY) + z1 * cos(angleY);
  rx = x2 * cos(angleZ) - y1 * sin(angleZ);
  ry = x2 * sin(angleZ) + y1 * cos(angleZ);
  rz = z2;
}

void project() {
  bool isExplode = (rotationStyle == 5);
  bool isScreensaver = (rotationStyle == 4);
  int ox = isScreensaver ? ssOffsetX : 0;
  int oy = isScreensaver ? ssOffsetY : 0;
  
  for (int i = 0; i < 8; i++) {
    float rx, ry, rz;
    float mult = isExplode ? (1.0 + vertexOffsets[i]) : 1.0;
    rotatePoint(cubeVertices[i][0] * mult, cubeVertices[i][1] * mult, cubeVertices[i][2] * mult, rx, ry, rz);
    float perspective = 3.0 / (3.0 + rz);
    projectedX[i] = (int)(centerX + ox + rx * scale * perspective);
    projectedY[i] = (int)(centerY + oy + ry * scale * perspective);
  }
}

float dotProduct(float ax, float ay, float az, float bx, float by, float bz) {
  return ax * bx + ay * by + az * bz;
}

bool isFaceVisible(int faceIdx) {
  bool isExplode = (rotationStyle == 5);
  float cx = 0, cy = 0, cz = 0;
  for (int v = 0; v < 4; v++) {
    int vi = cubeFaces[faceIdx][v];
    float rx, ry, rz;
    float mult = isExplode ? (1.0 + vertexOffsets[vi]) : 1.0;
    rotatePoint(cubeVertices[vi][0] * mult, cubeVertices[vi][1] * mult, cubeVertices[vi][2] * mult, rx, ry, rz);
    cx += rx; cy += ry; cz += rz;
  }
  cx /= 4; cy /= 4; cz /= 4;
  float nx, ny, nz;
  rotatePoint(faceNormals[faceIdx][0], faceNormals[faceIdx][1], faceNormals[faceIdx][2], nx, ny, nz);
  return dotProduct(nx, ny, nz, 0, 0, 1) > 0;
}

void sortFacesByDepth(int *faceOrder, int *faceCount) {
  bool isExplode = (rotationStyle == 5);
  float depths[6];
  *faceCount = 0;
  for (int f = 0; f < 6; f++) {
    if (isFaceVisible(f)) {
      float avgZ = 0;
      for (int v = 0; v < 4; v++) {
        int vi = cubeFaces[f][v];
        float rx, ry, rz;
        float mult = isExplode ? (1.0 + vertexOffsets[vi]) : 1.0;
        rotatePoint(cubeVertices[vi][0] * mult, cubeVertices[vi][1] * mult, cubeVertices[vi][2] * mult, rx, ry, rz);
        avgZ += rz;
      }
      avgZ /= 4;
      int pos = *faceCount;
      while (pos > 0 && depths[pos-1] < avgZ) {
        depths[pos] = depths[pos-1];
        faceOrder[pos] = faceOrder[pos-1];
        pos--;
      }
      depths[pos] = avgZ;
      faceOrder[pos] = f;
      (*faceCount)++;
    }
  }
}

void drawFilledFace(int faceIdx, uint16_t color) {
  int pts[8];
  for (int v = 0; v < 4; v++) {
    int vi = cubeFaces[faceIdx][v];
    pts[v*2] = projectedX[vi] - sprOffsetX;
    pts[v*2+1] = projectedY[vi] - sprOffsetY;
  }
  spr.fillTriangle(pts[0], pts[1], pts[2], pts[3], pts[4], pts[5], color);
  spr.fillTriangle(pts[0], pts[1], pts[4], pts[5], pts[6], pts[7], color);
}

void drawEdgeLines(uint16_t color) {
  for (int e = 0; e < 12; e++) {
    int v1 = cubeEdges[e][0], v2 = cubeEdges[e][1];
    spr.drawLine(projectedX[v1] - sprOffsetX, projectedY[v1] - sprOffsetY,
                 projectedX[v2] - sprOffsetX, projectedY[v2] - sprOffsetY, color);
  }
}

uint16_t getShadedColor(int faceIdx) {
  float nx, ny, nz;
  rotatePoint(faceNormals[faceIdx][0], faceNormals[faceIdx][1], faceNormals[faceIdx][2], nx, ny, nz);
  float len = sqrt(nx*nx + ny*ny + nz*nz);
  if (len > 0) { nx /= len; ny /= len; nz /= len; }
  float intensity = dotProduct(nx, ny, nz, lightDir[0], lightDir[1], lightDir[2]);
  intensity = max(0.15, min(1.0, intensity * 0.7 + 0.3));
  uint8_t b = (uint8_t)(intensity * 255);
  return tft.color565(b, b, b);
}

void drawCube() {
  int faceOrder[6], faceCount;
  sortFacesByDepth(faceOrder, &faceCount);
  
  if (rotationStyle == 3) {
    for (int f = 0; f < faceCount; f++) {
      drawFilledFace(faceOrder[f], getShadedColor(faceOrder[f]));
    }
    drawEdgeLines(tft.color565(80, 80, 80));
  } else {
    for (int f = 0; f < faceCount; f++) {
      drawFilledFace(faceOrder[f], faceColors[faceOrder[f]]);
    }
    drawEdgeLines(TFT_WHITE);
  }
}

void drawInfoOverlay() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(4, 3);
  tft.print("3D CUBE");
  
  tft.setCursor(180, 3);
  tft.print("A:pause B:quit");
  
  // Only redraw style text when the style actually changes
  if (rotationStyle != lastDrawnStyle) {
    // Erase the entire style text area first (longest style name is "SCREENSAVER" = 10 chars)
    tft.fillRect(4, 226, 160, 12, TFT_BLACK);
    tft.setCursor(4, 228);
    tft.print("STYLE: ");
    switch (rotationStyle) {
      case 0: tft.print("NORMAL"); break;
      case 1: tft.print("WOBBLE"); break;
      case 2: tft.print("FLIP"); break;
      case 3: tft.print("SHADED"); break;
      case 4: tft.print("SCREENSAVER"); break;
      case 5: tft.print("EXPLODE"); break;
    }
    lastDrawnStyle = rotationStyle;
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

// ===================== SETUP & LOOP =====================
bool onMenu = true;
bool needsDraw = true;
bool paused = false;
bool pauseScreenDrawn = false;
bool quitConfirm = false;
unsigned long lastDebounce = 0;
const unsigned long debounceMs = 160;

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(44, 30);
  tft.print("3D CUBE");
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(50, 90);
  tft.print("A to pause/unpause");
  tft.setCursor(50, 108);
  tft.print("B to quit");
  tft.setCursor(50, 126);
  tft.print("UP/DOWN change style");
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(64, 180);
  tft.print("START TO VIEW");
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

void drawQuitConfirm() {
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

void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(1);
  // Create a 200x200 sprite for the cube area (80KB - well within ESP32 free RAM)
  // This avoids the 153KB full-screen sprite that was failing
  if (!spr.createSprite(sprW, sprH)) {
    // If sprite creation fails, we'll fall back to direct drawing
    // (but 200x200 should always work on ESP32)
  }
  randomSeed(esp_random());
  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
  onMenu = true;
  needsDraw = true;
  angleX = 0; angleY = 0; angleZ = 0;
  rotationStyle = 0;
  paused = false;
  pauseScreenDrawn = false;
  quitConfirm = false;
  ssOffsetX = 0; ssOffsetY = 0;
  ssDirX = 1; ssDirY = 1;
  explodePhase = 0;
  for (int i = 0; i < 8; i++) vertexOffsets[i] = 0;
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
      needsDraw = true;
      tft.fillScreen(TFT_BLACK);
    }
    updateButtons();
    delay(10);
    return;
  }
  
  // Quit confirmation
  if (quitConfirm) {
    if (tapped(B)) {
      quitConfirm = false;
      needsDraw = true;
    }
    if (tapped(Enter)) {
      launcherExit = true;
    }
    updateButtons();
    delay(10);
    return;
  }
  
  // Quit - show confirmation
  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuitConfirm();
    updateButtons();
    return;
  }
  
  // Pause/unpause
  if (tapped(A) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    paused = !paused;
    if (paused) {
      drawPauseOverlay();
    } else {
      pauseScreenDrawn = false;
      needsDraw = true;
    }
  }
  
  if (paused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    updateButtons();
    delay(10);
    return;
  }
  
  // Change rotation style with UP/DOWN (6 styles: 0-5)
  if (tapped(Up) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    rotationStyle = (rotationStyle + 5) % 6;
    needsDraw = true;
    if (rotationStyle == 4) { ssOffsetX = 0; ssOffsetY = 0; ssDirX = 1; ssDirY = 1; }
    if (rotationStyle == 5) { explodePhase = 0; for (int i = 0; i < 8; i++) vertexOffsets[i] = 0; }
  }
  if (tapped(Down) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    rotationStyle = (rotationStyle + 1) % 6;
    needsDraw = true;
    if (rotationStyle == 4) { ssOffsetX = 0; ssOffsetY = 0; ssDirX = 1; ssDirY = 1; }
    if (rotationStyle == 5) { explodePhase = 0; for (int i = 0; i < 8; i++) vertexOffsets[i] = 0; }
  }
  
  // Update rotation based on style
  switch (rotationStyle) {
    case 0: // Normal
      angleX += rotSpeedX; angleY += rotSpeedY; angleZ += rotSpeedZ;
      break;
    case 1: // Wobble
      angleX = sin(millis() * 0.001) * 2;
      angleY = cos(millis() * 0.0013) * 2;
      angleZ = sin(millis() * 0.0007) * 1.5;
      break;
    case 2: // Flip
      {
        float t = millis() * 0.001;
        float phase = fmod(t, 4.0);
        if (phase < 2.0) { angleX += 0.08; angleY += 0.05; }
        angleZ += 0.01;
      }
      break;
    case 3: // Shaded
      angleX += rotSpeedX; angleY += rotSpeedY; angleZ += rotSpeedZ;
      break;
    case 4: // Screensaver
      angleX += rotSpeedX * 0.7; angleY += rotSpeedY * 0.7; angleZ += rotSpeedZ * 0.5;
      ssOffsetX += ssDirX * ssSpeed;
      ssOffsetY += ssDirY * ssSpeed;
      if (ssOffsetX > 110 || ssOffsetX < -110) ssDirX = -ssDirX;
      if (ssOffsetY > 70 || ssOffsetY < -70) ssDirY = -ssDirY;
      break;
    case 5: // Explode
      angleX += rotSpeedX * 0.5; angleY += rotSpeedY * 0.5; angleZ += rotSpeedZ * 0.3;
      explodePhase += 0.05;
      for (int i = 0; i < 8; i++) {
        vertexOffsets[i] = sin(explodePhase + i * 0.8) * 0.3;
      }
      break;
  }
  
  // Project
  project();
  
  // Draw cube to sprite (off-screen buffer), then push to display atomically
  // This eliminates flicker because the cube area is rendered off-screen first
  spr.fillSprite(TFT_BLACK);
  drawCube();
  spr.pushSprite(sprOffsetX, sprOffsetY);
  
  // Draw overlay text directly to TFT (these don't change every frame)
  drawInfoOverlay();
  
  updateButtons();
  delay(33);
}

}  // namespace Cube3D
