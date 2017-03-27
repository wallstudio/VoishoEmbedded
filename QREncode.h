#ifndef QR_ENCODE_H
#define QR_ENCODE_H
#include "Game2D.h"

// Pinpointing version 1-L
// Under 17byte binary data
uint8_t* QREncode(GameLCD *screen, uint8_t* infoData, uint8_t infoLength, uint8_t* dest);
void WritePatternPixel(uint8_t* dest, uint16_t x, uint16_t y, uint8_t col);
void WritePatternPixel(uint8_t* dest, uint16_t x, uint16_t y, uint8_t col, uint8_t mask);
void DirectQREncode(GameLCD *screen, uint8_t* infoData, uint8_t infoLength, uint8_t tx, uint8_t ty);
void WritePatternPixel(GameLCD* dest, uint16_t x, uint16_t y, uint16_t tx, uint16_t ty, uint8_t col, uint8_t mask);

#endif