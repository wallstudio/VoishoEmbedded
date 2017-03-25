//............................................................
// Copyright (C) 2017 WallStudio
// Contact  Website: https://wallstudio.github.io/
//          Twitter: https://twitter.com/yukawallstudio
//............................................................
#include <LCD5110_Graph.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <avr/pgmspace.h>
#include "bitmap.h"
#include "Game2D.h"
#include "QREncode.h"
//............................................................
//............................................................
//..................... HARDWARE PREPARE .....................
//............................................................
//............................................................
// LCD .......................................................
#define SCREEN_BUF_SYSE 504
#define SCREEN_WIDTH    84
#define SCREEN_HEIGHT   48
extern uint8_t SmallFont[];
GameLCD screen(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BUF_SYSE, 8, 9, 10, 11, 12);
//............................................................
// Audio .....................................................
//............................................................
#define VOLUME      32
#define SE_BTN_OK   1
#define SE_BTN_NG   2
#define SE_GYN_0    3
#define SE_MUSIC_0  4
//............................................................
// I/O .......................................................
//............................................................
#define I0 14
#define I1 15
#define I2 16
#define O0 7
//............................................................
// GameDefine ................................................
//............................................................
#define MENU_SEL_CNT 7
uint8_t GameFps = 60;
bool BufL, BufC, BufR;
uint64_t Frame = 0;
// Status
uint8_t Life = 6;
uint8_t Love = 0;
uint8_t Hungery = 0;
uint8_t Sick = 0;
uint8_t Dirty = 0;
// Config
uint8_t Volume = VOLUME;
uint8_t Luminance = 4;
uint8_t SleepTimeOut = 100;
// Input
bool btnL,btnC, btnR, btnDownL, btnDownC, btnDownR ,btnUpL, btnUpC, btnUpR;
void BtnDetectAll(){
  btnL = ButtonDetect(I0, &BufL, Status);
  btnC = ButtonDetect(I1, &BufC, Status);
  btnR = ButtonDetect(I2, &BufR, Status);
  btnDownL = ButtonDetect(I0, &BufL, DonwTrig);
  btnDownC = ButtonDetect(I1, &BufC, DonwTrig);
  btnDownR = ButtonDetect(I2, &BufR, DonwTrig);
  btnUpL = ButtonDetect(I0, &BufL, UpTrig);
  btnUpC = ButtonDetect(I1, &BufC, UpTrig);
  btnUpR = ButtonDetect(I2, &BufR, UpTrig);
  BufL = btnL;
  BufC = btnC;
  BufR = btnR;
}
void SceneInit(){
  BtnDetectAll();
  Frame++;
  screen.Clear(0x00);
}
//............................................................
//............................................................
// Initialize ................................................
//............................................................
void InitAudio(){
  mp3_set_serial(Serial);
  delay(1);
  mp3_set_volume(Volume);
}
void InitIO(){
  Serial.begin(9600);
  pinMode(O0,OUTPUT);
  pinMode(I0,INPUT);
  pinMode(I1,INPUT);
  pinMode(I2,INPUT);
  if(Luminance > 0){
    digitalWrite(O0, HIGH);
  }else{
    digitalWrite(O0, LOW);
  }
}
void setup(){
  InitIO();
  screen.ClearInitLCD();
  screen.setFont(SmallFont);
  InitAudio();
  Start();
}
void loop(){
  screen.Clear(0x00);
  Update();
  screen.update();
  delay(1000/GameFps);
}
//............................................................
//............................................................
//......................... GAME MAIN ........................
//............................................................
//............................................................
// GameObjects ...............................................
//............................................................
// Interface
GameObject *LifeSign;
GameObject *LoveSign;
GameObject *HungrySign;
GameObject *SichSign;
GameObject *DirtySign;
// CHaractor
GameObject *Maki;
GameObject *Hand;
GameObject *Hurt;
// Menu
GameObject *selectionIcons;
GameObject *selectionsText;
//............................................................
//............................................................
void Start(){
  // Interface
  uint8_t *lifeTex[] = {Bmp_plus0, Bmp_plus1, Bmp_plus2, Bmp_plus3, Bmp_plus4, Bmp_plus5, Bmp_plus6};
  LifeSign = new GameObject(&screen, lifeTex, 7, 6, 2, 2);
  uint8_t *loveTex[] = {Bmp_hurt0, Bmp_hurt1, Bmp_hurt2, Bmp_hurt3, Bmp_hurt4, Bmp_hurt5, Bmp_hurt6};
  LoveSign = new GameObject(&screen, loveTex, 7, 3, 59, 2);
  uint8_t *hungryTex[] = {Bmp_meal};
  HungrySign = new GameObject(&screen, hungryTex, 1, 0, 7, 11);
  uint8_t *sickTex[] = {Bmp_poison};
  SichSign = new GameObject(&screen, sickTex, 1, 0, 58, 11);
  uint8_t *dirtyTex[] = {Bmp_dirt0, Bmp_dirt1, Bmp_dirt2};
  DirtySign = new GameObject(&screen, dirtyTex, 1, 0, 5, 30);
  // Charactor
  uint8_t *makiTex[] = {Bmp_gyun0, Bmp_gyun1};
  Maki = new GameObject(&screen, makiTex, 2);
  uint8_t *handTex[] = {Bmp_hand};
  Hand = new GameObject(&screen, handTex, 1);
  uint8_t *hurtTex[] = {Bmp_hurt};
  Hurt = new GameObject(&screen, hurtTex, 1);
  //Menu
  uint8_t *selectionsIconTex[] = {
    Bmp_menu_stroke,
    Bmp_menu_lasagna, 
    Bmp_menu_clean, 
    Bmp_menu_game, 
    Bmp_menu_garally, 
    Bmp_menu_setting, 
    Bmp_menu_return
  };
  selectionIcons = new GameObject(&screen, selectionsIconTex, MENU_SEL_CNT, 0, 6, 10);
  uint8_t *selectionsTextTex[] = {
    Bmp_txt_naderu,
    Bmp_txt_gohan,
    Bmp_txt_souji,
    Bmp_txt_asobu,
    Bmp_txt_gyarari,
    Bmp_txt_settei,
    Bmp_txt_modoru
  };
  selectionsText = new GameObject(&screen, selectionsTextTex, MENU_SEL_CNT, 0, 6, 30);
}
//............................................................
//............................................................
void Update(){
  SceneInit();
  // Interface
  LifeSign->Rend();
  LoveSign->Rend();
  screen.print(" -- ", 2, 40);
  screen.print("MENU", 30, 40);
  screen.print(" -- ", 58, 40);
  HungrySign->Rend();
  SichSign->Rend();
  DirtySign->Rend();
  // Maki
  Maki->Ty = 10;
  Maki->Tx = (int)(sin((float)Frame/8)*2+screen.Width/2-Maki->GetWidth()/2);
  Maki->TexNo = Frame/16%2;
  Maki->Rend();
  // Input
  if(btnDownL){
    mp3_play(SE_MUSIC_0);
  }
  if(btnDownC){
    mp3_play(SE_BTN_OK);
    MenuLauncher();
  }
  if(btnDownR){
    mp3_play(SE_GYN_0);
    //Mingame0Launcher();
  }
}

//............................................................
//............................................................
//......................... MENU SCENE .......................
//............................................................
//............................................................
//............................................................
bool Menu(uint8_t *timer){
  SceneInit();
  // Interface
  LifeSign->Rend();
  screen.print("MENU", 30, 1);
  LoveSign->Rend();
  screen.print(" <  ", 2, 40);
  screen.print(" OK ", 30, 40);
  screen.print("  > ", 58, 40);
  // Selection
  selectionIcons->Tx = 6;
  selectionIcons->Ty = 10;
  selectionsText->Tx = 6;
  selectionsText->Ty = 30;
  selectionIcons->Rend();
  selectionsText->Rend();
  screen.printNumI(selectionIcons->TexQuant, 44, 20);
  screen.print("/", 39, 20);
  screen.printNumI(selectionIcons->TexNo+1, 34, 20);
  // Maki
  Maki->Ty = 10;
  Maki->Tx = (int)(sin((float)Frame/8)*2+screen.Width/2-Maki->GetWidth()/2)+24;
  Maki->TexNo = Frame/16%2;
  Maki->Rend();
  // Input
  if(btnDownL){
    mp3_play(SE_BTN_OK);
    *timer = 0;
    selectionIcons->TexNo += selectionIcons->TexQuant - 1;
    selectionsText->TexNo += selectionIcons->TexQuant - 1;
    selectionIcons->TexNo %= selectionIcons->TexQuant;
    selectionsText->TexNo %= selectionIcons->TexQuant;
  }
  if(btnDownC){
    mp3_play(SE_BTN_OK);
    switch(selectionIcons->TexNo){
      case 0: // Touch
        break;
      case 1: // Feed
        break;
      case 2: // Clean
        break;
      case 3: // Game
        break;
      case 4: // Garally
        break;
      case 5: // Config
        ConfigLauncher();
        break;
      case 6: // Return
        return false;
        break;
      default: break;
    }
  }
  if(btnDownR){
    mp3_play(SE_BTN_OK);
    *timer = 0;
    selectionIcons->TexNo++;
    selectionsText->TexNo++;
    selectionIcons->TexNo %= selectionIcons->TexQuant;
    selectionsText->TexNo %= selectionIcons->TexQuant;
  }
  return true;
}
void MenuLauncher(){
  uint8_t timeOut = 100;
  for(uint8_t i=0; i<timeOut; i++){
    if(!Menu(&i)) break;
    screen.update();
  }
}

//............................................................
//............................................................
//......................... CONFIG SCENE .....................
//............................................................
//............................................................
//............................................................
bool Config(uint8_t *timer, uint8_t selectionCount, uint8_t *confNo, int8_t *mode){
  SceneInit();
  // Interface
  selectionsText->Tx = 30;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  if(*mode < 0){
    screen.print(" UP ", 2, 40);
    screen.print(" OK ", 30, 40);
    screen.print("DOWN", 58, 40);
  }else{
    screen.print(" -  ", 2, 40);
    screen.print(" OK ", 30, 40);
    screen.print("  + ", 58, 40);
  }
  // Paramater
  screen.print(" VOLUEM:", 1, 9);
  for(uint8_t i=0; i<Volume/8; i++) screen.print("*", 50+i*6, 9);
  screen.print(" LIGHT :", 1, 17);
  for(uint8_t i=0; i<Luminance; i++) screen.print("*", 50+i*6, 17);
  screen.print(" SLEEP :", 1, 25);
  for(uint8_t i=0; i<SleepTimeOut/25; i++) screen.print("*", 50+i*6, 25);
  screen.print(" [BACK]", 43, 32);
  screen.print(">", 1+(*confNo)/3*42, 9+8*(*confNo));
  // Input
  if(btnDownL){
    mp3_play(SE_BTN_OK);
    if(*mode < 0){
      *confNo += selectionCount - 1;
      *confNo %= selectionCount;
    }else{
      switch(*confNo){
        case 0: // Volume
          if(Volume == 0) break;
          Volume -= 8;
          break;
        case 1: // Luminuce
          if(Luminance == 0) break;
          Luminance -= 4;
          break;
        case 2: // SleepTimeOut
          if(SleepTimeOut == 0) break;
          SleepTimeOut -= 25;
          break;
        default: break;
      }
    }
  }
  if(btnDownC){
    mp3_play(SE_BTN_OK);
    *timer = 0;
    if(*mode < 0){
      switch(*confNo){
        case 0: // Volume
        case 1: // Luminuce
        case 2: // SleepTimeOut
          *mode = *confNo;
          break;
        case 3: // Return
          return false;
          break;
        default: break;
      }
    }else{
      InitAudio();
      mp3_play(SE_BTN_OK);
      if(Luminance > 0){
        digitalWrite(O0, HIGH);
      }else{
        digitalWrite(O0, LOW);
      }
      *mode = -1;
    }
  }
  if(btnDownR){
    mp3_play(SE_BTN_OK);
    *timer = 0;
    if(*mode < 0){
      *confNo += 1;
      *confNo %= selectionCount;
    }else{
        switch(*confNo){
        case 0: // Volume
          if(Volume == 32) break;
          Volume += 8;
          break;
        case 1: // Luminuce
          if(Luminance == 4) break;
          Luminance += 4;
          break;
        case 2: // SleepTimeOut
          if(SleepTimeOut == 100) break;
          SleepTimeOut += 25;
          break;
        default: break;
      }
    }
  }
  return true;
}
void ConfigLauncher(){
  uint8_t activSelection = 0;
  int8_t inSelection = -1;  // Common
  uint8_t selectionCount = 4;
  uint8_t timeOut = 255;
  for(uint8_t i=0; i<timeOut; i++){
    if(!Config(&i, selectionCount, &activSelection, &inSelection)) break;
    screen.update();
  }
}

//............................................................
//............................................................
//......................... SUB SCENE ........................
//............................................................
//............................................................
//............................................................
void Mingame0(uint8_t countDown, uint8_t* loveCnt){
  SceneInit();
  //Information
  screen.print("TIME", 1, 1);
  screen.printNumI(countDown, 50, 1);
  screen.print("SCORE", 1, 40);
  screen.printNumI(*loveCnt, 50, 40);
  //Interface
  //Maki
    // <-----------84-------------->
    // <--------59---------><--27-->
    // <-20--><--20-><--19->
  Maki->Ty = 10;
  Maki->Tx = (int)(sin((float)(300-countDown+(random(countDown/40)*3))/8)*16+screen.Width/2-Maki->GetWidth()/2);
  Maki->TexNo = Frame/16%2;
  Maki->Rend();
  Hurt->Tx = Maki->Tx -5;
  Hurt->Ty = Maki->Ty;
  //Hand
  if(btnL){
    Hand->Tx = 10;
    Hand->Ty = 13 + (int)(sin((float)Frame/2)*2);
    Hand->Rend();
    if(Maki->Tx<20) Hurt->Rend();
  }else if(btnC){
    Hand->Tx = 30;
    Hand->Ty = 13 + (int)(sin((float)Frame/2)*2);
    Hand->Rend();
    if(Maki->Tx>=20 && Maki->Tx<40) Hurt->Rend();
  }else if(btnR){
    Hand->Tx = 50;
    Hand->Ty = 13 + (int)(sin((float)Frame/2)*2);
    Hand->Rend();
    if(Maki->Tx>=40) Hurt->Rend();
  }
  //HitCheck
  if((btnDownL && Maki->Tx<20) || (btnDownC && Maki->Tx>=20 && Maki->Tx<40) || (btnDownR && Maki->Tx>=40)) {
      (*loveCnt)++;
      mp3_play(1);
  } else if(btnDownL || btnDownC || btnDownR){
      // fail effect
  }
  screen.update();
}
void Mingame0Launcher(){
    //Manual
    screen.Clear(0x00);
    screen.print("MINIGAME START", 0, 10);
    screen.print("POSITION KMAKI", 0, 23);
    screen.print("PLZ PUSH BUTON", 0, 31);
    screen.update();
    delay(10000);
    //Main
    uint8_t loveCnt = 0;
    for(uint8_t i=300; i>=0; i--) Mingame0(i, &loveCnt);
    //Result
    screen.Clear(0x00);
    screen.print("YOUR SCORE!", 0, 10);
    screen.print(" LOVE: ", 0, 20);
    screen.printNumI(loveCnt, 40, 20);
    screen.update();
    delay(10000);
    //Future QR
    screen.Clear(0x00);
    screen.print("FUTURE", 0, 2);
    screen.print("  ->  ", 0, 10);
    screen.print("PLZ COME", 0, 32);
    screen.print("KOETSUKI", 0, 40);
    uint8_t infoData[] = "goo.gl/eWF1m2";
    uint8_t qrTex[21*3*2];
    GameObject* qr = new GameObject(&screen, QREncode(&screen, infoData, sizeof(infoData), qrTex), 21, 21);
    qr->Scl = 1;
    qr->Tx = 55;
    qr->Ty = 10;
    qr->Rend();
    screen.update();
    delay(30000);
    delete qr;
}
//............................................................
//............................................................
//.......................... END .............................
//............................................................
