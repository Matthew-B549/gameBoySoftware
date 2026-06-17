#pragma once
#include <Arduino.h>
#include <string.h>
#include "ButtonDefinitions.h"

namespace BlackjackGame {
TFT_eSPI tft = TFT_eSPI();
bool launcherExit = false;
bool onMenu = true;
bool roundOver = false;
bool quitConfirm = false;
bool needsDraw = true;

const int maxHand = 12;
int deck[52];
int deckPos = 0;
int playerHand[maxHand];
int dealerHand[maxHand];
int playerCount = 0;
int dealerCount = 0;
bool dealerHoleHidden = true;
char resultText[20] = "";
int resultColor = TFT_WHITE;
unsigned long lastDebounce = 0;
const unsigned long debounceMs = 160;

bool shouldExitToLauncher() { return launcherExit; }
void scanButtons(){buttons[0].pressed=digitalRead(buttons[0].pin);for(int i=1;i<buttonCount;i++)buttons[i].pressed=!digitalRead(buttons[i].pin);}
bool tapped(buttonName b){return buttons[b].pressed&&!buttons[b].lastPressed;}
void updateButtons(){for(int i=0;i<buttonCount;i++)buttons[i].lastPressed=buttons[i].pressed;}

int cardRank(int card){return card % 13;}
int cardValue(int card){int r=cardRank(card);if(r==0)return 11;if(r>=9)return 10;return r+1;}
const char* cardRankText(int card){
  static const char* names[]={"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
  return names[cardRank(card)];
}
char cardSuitChar(int card){
  static const char suits[]={'H','D','C','S'};
  return suits[card / 13];
}

void resetDeck() {
  for (int i = 0; i < 52; i++) deck[i] = i;
  for (int i = 51; i > 0; i--) {
    int j = random(i + 1);
    int tmp = deck[i];
    deck[i] = deck[j];
    deck[j] = tmp;
  }
  deckPos = 0;
}

int drawCard() {
  if (deckPos >= 52) resetDeck();
  return deck[deckPos++];
}

void dealCardTo(int* hand, int& count) {
  if (count < maxHand) hand[count++] = drawCard();
}

int handScore(const int* hand, int count) {
  int score = 0;
  int aces = 0;
  for (int i = 0; i < count; i++) {
    int value = cardValue(hand[i]);
    score += value;
    if (cardRank(hand[i]) == 0) aces++;
  }
  while (score > 21 && aces > 0) {
    score -= 10;
    aces--;
  }
  return score;
}

void startRound() {
  resetDeck();
  playerCount = 0;
  dealerCount = 0;
  dealerHoleHidden = true;
  resultText[0] = '\0';
  roundOver = false;
  needsDraw = true;
  dealCardTo(playerHand, playerCount);
  dealCardTo(dealerHand, dealerCount);
  dealCardTo(playerHand, playerCount);
  dealCardTo(dealerHand, dealerCount);
  if (handScore(playerHand, playerCount) == 21) {
    dealerHoleHidden = false;
    roundOver = true;
    strcpy(resultText, "BLACKJACK!");
    resultColor = TFT_YELLOW;
  }
}

void finishDealerTurn() {
  dealerHoleHidden = false;
  while (handScore(dealerHand, dealerCount) < 17) {
    dealCardTo(dealerHand, dealerCount);
  }

  int playerScore = handScore(playerHand, playerCount);
  int dealerScore = handScore(dealerHand, dealerCount);
  if (playerScore > 21) {
    strcpy(resultText, "BUST");
    resultColor = TFT_RED;
  } else if (dealerScore > 21 || playerScore > dealerScore) {
    strcpy(resultText, "YOU WIN");
    resultColor = TFT_GREEN;
  } else if (playerScore < dealerScore) {
    strcpy(resultText, "DEALER WINS");
    resultColor = TFT_RED;
  } else {
    strcpy(resultText, "PUSH");
    resultColor = TFT_CYAN;
  }
  roundOver = true;
  needsDraw = true;
}

void drawCardBox(int x, int y, int card, bool hidden) {
  tft.fillRoundRect(x, y, 48, 64, 4, TFT_WHITE);
  tft.drawRoundRect(x, y, 48, 64, 4, TFT_BLACK);
  if (hidden) {
    tft.fillRoundRect(x + 6, y + 6, 36, 52, 3, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(2);
    tft.setCursor(x + 14, y + 24);
    tft.print("??");
    return;
  }

  uint16_t color = (cardSuitChar(card) == 'H' || cardSuitChar(card) == 'D') ? TFT_RED : TFT_BLACK;
  tft.setTextColor(color, TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(x + 6, y + 6);
  tft.print(cardRankText(card));
  tft.setTextSize(1);
  tft.setCursor(x + 6, y + 28);
  tft.print(cardSuitChar(card));
  tft.setCursor(x + 6, y + 42);
  tft.print(cardSuitChar(card));
}

void drawTable() {
  tft.fillScreen(TFT_DARKGREEN);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
  tft.setTextSize(3);
  tft.setCursor(66, 18);
  tft.print("BLACKJACK");

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setCursor(16, 50);
  tft.print("Dealer");
  tft.setCursor(16, 140);
  tft.print("Player");

  for (int i = 0; i < dealerCount; i++) {
    bool hidden = dealerHoleHidden && i == 1 && !roundOver;
    drawCardBox(16 + i * 54, 62, dealerHand[i], hidden);
  }
  for (int i = 0; i < playerCount; i++) {
    drawCardBox(16 + i * 54, 152, playerHand[i], false);
  }

  int dealerScore = dealerHoleHidden && !roundOver ? cardValue(dealerHand[0]) : handScore(dealerHand, dealerCount);
  int playerScore = handScore(playerHand, playerCount);

  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setCursor(232, 50);
  tft.print("S:");
  tft.print(dealerScore);
  tft.setCursor(232, 140);
  tft.print("S:");
  tft.print(playerScore);

  tft.setTextColor(resultColor, TFT_DARKGREEN);
  tft.setTextSize(1);
  tft.setCursor(72, 220);
  if (resultText[0]) {
    tft.print(resultText);
  } else if (!roundOver) {
    tft.print("A hit   ENTER stand");
  } else {
    tft.print("START next round");
  }
}

void drawMenu() {
  tft.fillScreen(TFT_DARKGREEN);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
  tft.setTextSize(3);
  tft.setCursor(72, 34);
  tft.print("BLACKJACK");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setCursor(48, 104);
  tft.print("A hits");
  tft.setCursor(48, 122);
  tft.print("ENTER stands");
  tft.setCursor(48, 140);
  tft.print("B asks to quit");
  tft.setCursor(48, 158);
  tft.print("Beat the dealer without busting");
  tft.setTextSize(2);
  tft.setCursor(80, 196);
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

void returnToMenu() {
  launcherExit = true;
  onMenu = true;
  quitConfirm = false;
  roundOver = false;
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
      startRound();
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

  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuit();
    updateButtons();
    return;
  }

  if (!roundOver) {
    if (tapped(A) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      dealCardTo(playerHand, playerCount);
      if (handScore(playerHand, playerCount) > 21) finishDealerTurn();
      needsDraw = true;
    }
    if (tapped(Enter) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      finishDealerTurn();
    }
  } else {
    if ((tapped(Start) || tapped(Enter)) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      startRound();
    }
  }

  if (needsDraw) {
    drawTable();
    needsDraw = false;
  }

  updateButtons();
  delay(10);
}

} // namespace BlackjackGame