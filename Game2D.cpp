#include <avr/pgmspace.h>
#include <new.h>
#include "Game2D.h"

#define SCREEN_BUF_SYSE 504
#define SCREEN_WIDTH 84
#define SCREEN_HEIGHT 48

GameLCD::GameLCD(int w, int h, int size, int SCK, int MOSI, int DC, int RST, int CS) : LCD5110(SCK, MOSI, DC, RST, CS){
    Width = w;
    Height = h;
    DataSize = size;
}
void GameLCD::ClearInitLCD(){
    InitLCD();
    Clear(0x00);
}
uint8_t GameLCD::GetPixel(uint16_t x, uint16_t y){
    int by, bi;
	if (x >= 0 && x < Width && y >= 0 && y < Height){
		by=((y/8)*Width)+x;
		bi=y % 8;
		return (scrbuf[by] >> bi) & 0x01;
	}
};
void GameLCD::SetPixel(uint16_t x, uint16_t y, uint8_t col){
	int by, bi;

	if ((x>=0) and (x<84) and (y>=0) and (y<48))
	{
		by=((y/8)*84)+x;
		bi=y % 8;
        if(col){
            //black
            scrbuf[by] |= (1<<bi);
        }else{
            //white
            scrbuf[by] &= ~(1<<bi);
        }
	}
}
uint8_t GameLCD::GetScrBuf(int index){
    return scrbuf[index];
}
void GameLCD::SetScrBuf(int index, uint8_t data){
    scrbuf[index] = data;
}
void GameLCD::Clear(uint8_t pattern){
    for (int i = 0; i < SCREEN_BUF_SYSE; i++) {
        SetScrBuf(i, pattern);
    }
}
void GameLCD::Roll180(){
    for(int i=0; i<84*48/2; i++){
            uint8_t tmp = GetPixel(i%84, i/84);
            SetPixel(i%84, i/84, GetPixel(83-i%84, 47-i/84));
            SetPixel(83-i%84, 47-i/84, tmp);
    }
}
void GameLCD::update(){
    if (_sleep==false){
		_LCD_Write(PCD8544_SETYADDR, LCD_COMMAND);
		_LCD_Write(PCD8544_SETXADDR, LCD_COMMAND);
		for (int b=503; b>=0; b--){
            uint8_t byteBuf  = scrbuf[b] << 7;
            byteBuf |= (scrbuf[b] & 0b00000010) << 5;
            byteBuf |= (scrbuf[b] & 0b00000100) << 3;
            byteBuf |= (scrbuf[b] & 0b00001000) << 1;
            byteBuf |= (scrbuf[b] & 0b00010000) >> 1;
            byteBuf |= (scrbuf[b] & 0b00100000) >> 3;
            byteBuf |= (scrbuf[b] & 0b01000000) >> 5;
            byteBuf |= (scrbuf[b] & 0b10000000) >> 7;
            _LCD_Write(byteBuf, LCD_DATA);
        }
	}
}
GameObject::GameObject(GameLCD *screen, uint8_t **tex, int quantity){
    RamTex = false;
    Screen = screen;
    Tex = new Texture[quantity];   //quantity*8 Byte
    int i = 0;
    for(i = 0; i < quantity; i++){
        //ROMからロード
        Tex[i].W = pgm_read_word(tex[i] + 0);
        Tex[i].H = pgm_read_word(tex[i] + 2);
        Tex[i].S = pgm_read_word(tex[i] + 4);
        Tex[i].Data = tex[i] + 8;
    }
    TexQuant = quantity;
    Active = true;
    TexNo = 0;
    Tx = 0;
    Ty = 0;
    Sx = 0;
    Sy = 0;
    Sw = GetWidthFromTex();
    Sh = GetHeightFromTex();
    Scl = 1;
}
GameObject::GameObject(GameLCD *screen, uint8_t **tex, int quantity, int texNo, int x, int y){
    RamTex = false;
    Screen = screen;
    Tex = new Texture[quantity];   //quantity*8 Byte
    int i = 0;
    for(i = 0; i < quantity; i++){
        //ROMからロード
        Tex[i].W = pgm_read_word(tex[i] + 0);
        Tex[i].H = pgm_read_word(tex[i] + 2);
        Tex[i].S = pgm_read_word(tex[i] + 4);
        Tex[i].Data = tex[i] + 8;
    }
    TexQuant = quantity;
    Active = true;
    TexNo = texNo;
    Tx = x;
    Ty = y;
    Sx = 0;
    Sy = 0;
    Sw = GetWidthFromTex();
    Sh = GetHeightFromTex();
    Scl = 1;
}
GameObject::GameObject(GameLCD *screen, uint8_t *bitmap, int w, int h){
    RamTex = true;
    Screen = screen;
    Tex = new Texture[1];   //8 Byte
    //RAMからロード
    Tex[0].W = w;
    Tex[0].H = h;
    Tex[0].S = w * ceil(((float)h)/8);
    Bitmap = new uint8_t[Tex[0].S*2];
    for(int i=0; i<Tex[0].S*2; i++){
        Bitmap[i] = bitmap[i];
    }
    Tex[0].Data = Bitmap;
    TexQuant = 1;
    Active = true;
    TexNo = 0;
    Tx = 0;
    Ty = 0;
    Sx = 0;
    Sy = 0;
    Sw = GetWidthFromTex();
    Sh = GetHeightFromTex();
    Scl = 1;
}
GameObject::~GameObject() {
    delete Bitmap;
    delete Tex;
}
void GameObject::Rend(){
    Texture tex = Tex[TexNo];
    for (int i = 0; i < tex.H; i = i + 8){
        for (int j = 0; j < tex.W; j++){
            uint8_t byteData, byteDataAlpha;
            if(!RamTex){
                byteData = pgm_read_byte(tex.Data + i / 8 * tex.W + j);
                byteDataAlpha = pgm_read_byte(tex.S + tex.Data + i / 8 * tex.W + j);
            }else{
                byteData = Bitmap[i / 8 * tex.W + j];
                byteDataAlpha = Bitmap[21*3 + i / 8 * tex.W + j];
            }
            for(int k = 0; k < 8; k++){
                int x = j;
                int y = i + k;
                int tx = Tx + x - Sx;
                int ty = Ty + y - Sy; 
                //選択範囲外
                if(x < Sx || x >= Sx+Sw || y < Sy || y >= Sy+Sh) continue;
                //画面外
                if(tx < 0 || tx >= Screen->Width || ty < 0 || ty >= Screen->Height) continue;
                //描画
                if(byteDataAlpha >> k & 0x01){
                    for(int l=0; l<Scl; l++){
                        for(int m=0; m<Scl; m++){
                            Screen->SetPixel(Scl*tx+l, Scl*ty+m, byteData >> k & 0x01);
                        }
                    }
                }
            }
        }
    }
}
int GameObject::GetWidth(){
    return min(Tex[TexNo].W, Sw);
}
int GameObject::GetHeight(){
    return min(Tex[TexNo].H, Sh);
}
int GameObject::GetWidthFromTex(){
    return Tex[TexNo].W;
}
int GameObject::GetWidthFromTex(int no){
    return Tex[no].W;
}
int GameObject::GetHeightFromTex(){
    return Tex[TexNo].H;
}
int GameObject::GetHeightFromTex(int no){
    return Tex[no].H;
}

bool ButtonDetect(int pin, bool *buffer, ButtonDetectMode mode){
    bool buf = *buffer;
    bool status = digitalRead(pin)==LOW;
    bool rtn;
    switch(mode){
        case Status: 
            rtn = status;
            break;
        case DonwTrig:
            rtn = status && (!buf);
            break;
        case UpTrig:
            rtn = (!status) && buf;
            break;
        default:
            break;
    }
    return rtn;
}

void MemoryDebug(GameLCD *screen, uint8_t x, uint8_t y){
    uint8_t *h;
    h = new uint8_t[1];
    screen->print("HEAP", x, y);
    screen->printNumI(h, x+3, y+8);
    uint8_t s;
    screen->print("STACK", x, y+16);
    screen->printNumI(&s, x+3, y+24);
    delete h;
}