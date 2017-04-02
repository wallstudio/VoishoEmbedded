#ifndef GAME_2D_H
#define GAME_2D_H

#ifndef LCD5110_Graph_h
#include <LCD5110_Graph.h>
#endif

typedef struct texture{ //8 Byte
    uint8_t *Data;
    uint8_t W;
    uint8_t H;
    uint8_t S;
} Texture;
enum Invert{
    None,
    Horizonal,
    Vertical,
    HorizonalAndVertical
};

class GameLCD : public LCD5110{
    public:
        int Width, Height, DataSize;
        GameLCD(int w, int h, int size, int SCK, int MOSI, int DC, int RST, int CS);
        void ClearInitLCD();
        uint8_t GetPixel(uint16_t x, uint16_t y);
        void SetPixel(uint16_t x, uint16_t y, uint8_t col);
        uint8_t GetScrBuf(int index);
        void SetScrBuf(int index, uint8_t data);
        void Clear(uint8_t pattern);
        void Roll180();
        void update();
};

class GameObject{
    public:
        bool Active;
        uint8_t Tx, Ty, Sx, Sy, Sw, Sh, Scl, TexNo;
        Invert Inv;
        uint8_t TexQuant;
        Texture *Tex;
        bool RamTex;
        uint8_t *Bitmap;
        uint8_t BitmapW, BitmapH;
    private:
        GameLCD *Screen;
    public:
        //texは一時変数でOK
        GameObject(GameLCD *screen, uint8_t **tex, int quantity);
        GameObject(GameLCD *screen, uint8_t **tex, int quantity, int texNo, int x, int y);
        GameObject(GameLCD *screen, uint8_t *bitmap, int w, int h);
        ~GameObject();
        void Rend();
        int GetWidth();
        int GetHeight();
    private:
        int GetWidthFromTex();
        int GetWidthFromTex(int no);
        int GetHeightFromTex();
        int GetHeightFromTex(int no);
};

enum ButtonDetectMode{
    Status,
    DonwTrig,
    UpTrig
};
bool ButtonDetect(int pin, bool *buffer, ButtonDetectMode mode);

void MemoryDebug(GameLCD *screen, uint8_t x, uint8_t y);
#endif