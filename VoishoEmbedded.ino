//............................................................
// Copyright (C) 2017 WallStudio
// Contact  Website: https://wallstudio.github.io/
//          Twitter: https://twitter.com/yukawallstudio
//............................................................
#include <LCD5110_Graph.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <new.h>
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
#define SE_DEAD     5
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
#define MENU_SEL_CNT  8
#define LIFE_INTERVAL 2000
uint8_t GameFps = 60;
bool BufL, BufC, BufR;
uint64_t Frame = 0;
uint16_t PreFrame = 0;
// Status
uint8_t Life = 6;
uint8_t Love = 0;
uint8_t Hungery = 0;
uint8_t Sick = 0;
uint8_t Dirty = 0;
uint8_t Rank = 0;
// Config
uint8_t Volume = VOLUME;
uint8_t Luminance = 4;
uint8_t SleepTimeOut = 100;
void Save(){
  // Signiture
  uint8_t i = 0;
  EEPROM.write(i++, 'Y');
  EEPROM.write(i++, 'u');
  EEPROM.write(i++, 'k');
  EEPROM.write(i++, 'a');
  EEPROM.write(i++, 'M');
  EEPROM.write(i++, 'a');
  EEPROM.write(i++, 'k');
  EEPROM.write(i++, 'i');
  // Data
  for(uint8_t j=0; j<8; j++){
    EEPROM.write(i, Frame >> (8*j) & 0x000000000000FF);
    i++;
  }
  EEPROM.write(i++, Life);
  EEPROM.write(i++, Love);
  EEPROM.write(i++, Hungery);
  EEPROM.write(i++, Sick);
  EEPROM.write(i++, Dirty);
  EEPROM.write(i++, Volume);
  EEPROM.write(i++, Luminance);
  EEPROM.write(i++, SleepTimeOut);
  EEPROM.write(i++, PreFrame>>8);
  EEPROM.write(i++, PreFrame&0x00FF);
  screen.Clear(0x00);
  EEPROM.write(i++, Rank);
  screen.print("SAVEDATA", 18, 16);
  screen.print(" SAVING ", 18, 24);
  screen.update();
  //delay(500);
}
void Load(){  
  // Signiture
  uint8_t i = 0;
  bool sig = true;
  sig &= EEPROM.read(i++) == 'Y';
  sig &= EEPROM.read(i++) == 'u';
  sig &= EEPROM.read(i++) == 'k';
  sig &= EEPROM.read(i++) == 'a';
  sig &= EEPROM.read(i++) == 'M';
  sig &= EEPROM.read(i++) == 'a';
  sig &= EEPROM.read(i++) == 'k';
  sig &= EEPROM.read(i++) == 'i';
  if(!sig) return;
  // Data
  Frame = 0;
  for(uint8_t j=0; j<8; j++){
    Frame += EEPROM.read(i) << (8*j);
    i++;
  }
  Life = EEPROM.read(i++);
  Love = EEPROM.read(i++);
  Hungery = EEPROM.read(i++);
  Sick = EEPROM.read(i++);
  Dirty = EEPROM.read(i++);
  Volume = EEPROM.read(i++);
  Luminance = EEPROM.read(i++);
  SleepTimeOut = EEPROM.read(i++);
  PreFrame = EEPROM.read(i++) * 0x100;
  PreFrame += EEPROM.read(i++);  
  Rank = EEPROM.read(i++);
  screen.Clear(0x00);
  screen.print("SAVEDATA", 18, 16);
  screen.print("LOADING", 18, 24);
  screen.update();
  //delay(1000);
}
void Clear(){
  for(uint8_t i=0; i<128; i++){
    EEPROM.write(i, 0);
  }
  screen.Clear(0x00);
  screen.print("SAVEDATA", 18, 16);
  screen.print("REMOVEING", 18, 24);
  screen.update();
  delay(3000);
  asm volatile ("  jmp 0");  
}
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
// Sleep
void Sleep(){
  digitalWrite(O0, LOW);
  pinMode(O0, INPUT);
  Save();
  wdt_reset();
  wdt_enable(WDTO_8S);  // After 1s Restart.
  set_sleep_mode(SLEEP_MODE_STANDBY); 
  sleep_mode(); // System go down.
}
void DebugLEDFlash(uint8_t count, uint8_t length){
    pinMode(2,OUTPUT);
    for(uint8_t i=0; i<count; i++){
      digitalWrite(2, HIGH);
      delay(length);
      digitalWrite(2, LOW);
      delay(length);
    }
}
void SleepCatch(){
  //DebugLEDFlash(1, 100);
  Load();
  Frame += 160;
  if(Life>0){
    if((uint16_t)Frame-PreFrame > LIFE_INTERVAL) { // Interval is about 10s.
      //DebugLEDFlash(8, 50);
      if(Life == 0){
        // Dead
      }else{
        // Good Metabolic 
        if(Love<6 && Sick==0 && Hungery<10 && Dirty==0 && Life==6){
          Love++;
          //DebugLEDFlash(4, 150);
        };
        if(Life<6 && Sick==0 && Hungery<10 && Dirty==0) Life++;
        // Bad Metabolic 
        randomSeed(Frame%2000 + analogRead(5)%164);
        if(random(0, 3)==0){
          if(Hungery < 100)Hungery++;
          if(Dirty<2) Dirty++;
          //DebugLEDFlash(4, 150);
        }
        // Event
        randomSeed(Frame + analogRead(5)%81);
        if(Hungery>10 && Dirty>1 && random(0, 3)==0){
          Sick = 1; 
          //DebugLEDFlash(4, 150);
        }
        // Damage
        if(Hungery==100 || Sick==1){
          if(Life>0)Life--;
          if(Love>0)Love--;
          //DebugLEDFlash(4, 150);
        }
      }
      PreFrame = (uint16_t)Frame;
      Save();
    }
  }
  Save();
  BtnDetectAll();
  if(btnL || btnC || btnR){
    // Wake up
    return;
  }else{
    Sleep();  // System go down.
  }
}
// Common
void SceneInit(){
  BtnDetectAll();
  Frame++;
  screen.Clear(0x00);
}
bool Dead = false;
bool LifeCycle(){
  if(!Dead && Life==0) {
    mp3_play(SE_DEAD);
    Dead = true;
  }
  if((uint16_t)Frame-PreFrame > LIFE_INTERVAL) { // Interval is about 10s.
    if(Life == 0){
      // Dead
    }else{
      // Advance
      randomSeed(Frame%5 + analogRead(5)%211);
      if(Rank<2 &&Love==6 && Sick==0 && Hungery<10 && Dirty==0 && Life>3 && random(0, 3)==0){
        Rank++;
        Love = 0;
      }
      // Good Metabolic 
      if(Love<6 && Sick==0 && Hungery<10 && Dirty==0 && Life==6){
        Love++;
        mp3_play(SE_GYN_0);
      };
      if(Life<6 && Sick==0 && Hungery<10 && Dirty==0) Life++;
      // Bad Metabolic 
      randomSeed(Frame%11 + analogRead(5)%74);
      if(random(0, 3)==0){
        if(Hungery < 100)Hungery++;
        if(Dirty<2) Dirty++;
        // AutoSave
        //Save();
      }
      // Event
      randomSeed(Frame%9 + analogRead(5)%69);
      if(Hungery>10 && Dirty>1 && random(0, 3)==0) Sick = 1;
      // Damage
      if(Hungery==100 || Sick==1){
        if(Life>0)Life--;
        if(Love>0)Love--;
      }
      // AutoSleep
      //randomSeed(Frame%1000 + analogRead(5)%94);
      //if(random(0, 20)==0) Sleep();
      Save();
    }
    PreFrame = (uint16_t)Frame;
    return true;
  }
  return false;
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
  wdt_reset();
  SleepCatch();
  Load();
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
GameObject *SickSign;
GameObject *DirtySign;
// CHaractor
GameObject *Maki;
GameObject *Hand;
GameObject *Hurt;
// Menu
GameObject *selectionIcons;
GameObject *selectionsText;
// Feed
GameObject *Lasagna;
// Communication
GameObject *ComInstruntion;
//............................................................
//............................................................
void Start(){
  // Interface
  uint8_t *lifeTex[] = {Bmp_plus0, Bmp_plus1, Bmp_plus2, Bmp_plus3, Bmp_plus4, Bmp_plus5, Bmp_plus6};
  LifeSign = new GameObject(&screen, lifeTex, 7, 6, 2, 2);
  uint8_t *loveTex[] = {Bmp_hurt0, Bmp_hurt1, Bmp_hurt2, Bmp_hurt3, Bmp_hurt4, Bmp_hurt5, Bmp_hurt6};
  LoveSign = new GameObject(&screen, loveTex, 7, 6, 59, 2);
  uint8_t *hungryTex[] = {Bmp_meal};
  HungrySign = new GameObject(&screen, hungryTex, 1, 0, 7, 11);
  uint8_t *sickTex[] = {Bmp_poison};
  SickSign = new GameObject(&screen, sickTex, 1, 0, 58, 11);
  uint8_t *dirtyTex[] = {Bmp_dirt0, Bmp_dirt1, Bmp_dirt2};
  DirtySign = new GameObject(&screen, dirtyTex, 3, 2, 5, 30);
  // Charactor
  uint8_t *makiTex[] = {
    // Egg
    Bmp_gyun0_nml_egg, 
    Bmp_gyun1_nml_egg,
    Bmp_gyun0_egg, 
    Bmp_gyun1_egg,
    Bmp_gyun0_bad_egg, 
    Bmp_gyun1_bad_egg,
    // Baby
    Bmp_gyun0_nml_baby, 
    Bmp_gyun1_nml_baby,
    Bmp_gyun0_baby, 
    Bmp_gyun1_baby,
    Bmp_gyun0_bad_baby, 
    Bmp_gyun1_bad_baby,
    // Adult
    Bmp_gyun0_nml, 
    Bmp_gyun1_nml,
    Bmp_gyun0, 
    Bmp_gyun1,
    Bmp_gyun0_bad, 
    Bmp_gyun1_bad,
    // Dead
    Bmp_gyun_tumb
  };
  Maki = new GameObject(&screen, makiTex, 19);
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
    Bmp_menu_communication,
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
    Bmp_txt_tushin,
    Bmp_txt_settei,
    Bmp_txt_modoru
  };
  selectionsText = new GameObject(&screen, selectionsTextTex, MENU_SEL_CNT, 0, 6, 30);
  // FEED
  uint8_t *lasagnaTex[] = {Bmp_menu_lasagna, Bmp_menu_lasagna1, Bmp_menu_lasagna2, Bmp_menu_lasagna3};
  Lasagna = new GameObject(&screen, lasagnaTex, 4);
  // Communication
  uint8_t *comInsTex[] = {Bmp_qr_inst, Bmp_g_inst};
  ComInstruntion = new GameObject(&screen, comInsTex, 2, 0, 0, 10);
}
//............................................................
//............................................................
void Update(){
  //Logic
  LifeCycle();
  SceneInit();
  // Interface
  LifeSign->TexNo = Life;
  LifeSign->Rend();
  LoveSign->TexNo = Love;
  LoveSign->Rend();
  if(Life>0){
    screen.print("SAVE", 0, 40);
    screen.print("MENU", 27, 40);
    screen.print("SLEEP", 54, 40);
  }else{
    screen.print("-- ", 2, 40);
    screen.print("RESET", 22, 40);
    screen.print("SLEEP", 54, 40);
  }
  if(Hungery > 10) HungrySign->Rend();
  if(Sick > 0) SickSign->Rend();
  DirtySign->TexNo = Dirty;
  DirtySign->Rend();
  // Maki
  Maki->Ty = 10;
  Maki->Tx = (int)(sin((float)Frame/8)*2+screen.Width/2-Maki->GetWidth()/2);
  Maki->TexNo = Frame/16%2 + Rank*6;
  if(Dirty>0 || Hungery>10 || Sick>0) Maki->TexNo += 4;
  if(Life<=0){
    Maki->Tx = screen.Width/2-Maki->GetWidth()/2;
    Maki->TexNo = 18;
  };
  Maki->Rend();
  // Input
  if(btnDownL){
    if(Life>0){
      mp3_play(SE_BTN_OK);
      Save();
      screen.Clear(0x00);
      screen.print("*SAVING*", 18, 19);
      screen.update();
      delay(500);
    }
  }
  if(btnL){
    screen.printNumI(Frame >> 48 & 0x000000000000FFFF, 1, 8);
    screen.printNumI(Frame >> 32 & 0x000000000000FFFF, 1, 16);
    screen.printNumI(Frame >> 16 & 0x000000000000FFFF, 1, 24);
    screen.printNumI(Frame >> 0  & 0x000000000000FFFF, 1, 32);
    screen.printNumI(Hungery, 27, 8);
    screen.printNumI(PreFrame, 27, 16);
    screen.printNumI(analogRead(5), 27, 24);
    MemoryDebug(&screen, 50, 8);
  }
  if(btnDownC){
    mp3_play(SE_BTN_OK);
    if(Life>0){
      MenuLauncher();
    }else{
      Clear();
    }
  }
  if(btnDownR){
    mp3_play(SE_BTN_OK);
    Sleep();
  }
  if(btnR){
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
  Maki->TexNo = Frame/16%2 +Rank*6;
  if(Dirty>0 || Hungery>10 || Sick>0) Maki->TexNo += 4;
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
        TouchLauncher();
        break;
      case 1: // Feed
        FeedLauncher();
        break;
      case 2: // Clean
        CleanLauncher();
        break;
      case 3: // Game
        GameLauncher();
        break;
      case 4: // Garally
        GarallyLauncher();
        break;
      case 5: // Communication
        CommunicationLauncher();
        break;
      case 6: // Config
        ConfigLauncher();
        break;
      case 7: // Return
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
//......................... TOUCH SCENE ......................
//............................................................
//............................................................
//............................................................
bool Touch(uint8_t *timer){
  SceneInit();
  // Interface
  selectionsText->Tx = 30;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  // Maki
  Maki->Tx = 28;
  Maki->Ty = 10;
  Maki->TexNo = *timer/16%2 + 6*Rank + 2;
  Maki->Rend();
  //Hand
  Hand->Tx = 30;
  Hand->Ty = 13 + (int)(sin((float)*timer/2)*2);
  Hand->Rend();
  return true;
}
void TouchLauncher(){
  uint8_t timeOut = 70;
  delay(1000);
  mp3_play(SE_GYN_0);
  for(uint8_t i=0; i<timeOut; i++){
    if(!Touch(&i)) break;
    screen.update();
  }
  Sick = 0;
}

//............................................................
//............................................................
//......................... FEED SCENE .......................
//............................................................
//............................................................
//............................................................
bool Feed(uint8_t *timer, uint8_t timeLength){
  SceneInit();
  // Interface
  selectionsText->Tx = 30;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  // Maki
  Maki->Tx = 34;
  Maki->Ty = 10;
  Maki->TexNo = Frame/16%2 + 6*Rank + 2;
  Maki->Rend();
  // Food
  Lasagna->TexNo = *timer/(timeLength/(Lasagna->TexQuant-1));
  Lasagna->Tx = 20;
  Lasagna->Ty = 20;
  Lasagna->Rend();
  return true;
}
void FeedLauncher(){
  uint8_t timeOut = 70;
  delay(1000);
  mp3_play(SE_GYN_0);
  for(uint8_t i=0; i<timeOut; i++){
    if(!Feed(&i, timeOut)) break;
    screen.update();
  }
  Hungery = 0;
}

//............................................................
//............................................................
//......................... CLEAN SCENE ......................
//............................................................
//............................................................
//............................................................
bool Clean(uint8_t *timer, uint8_t timeLength){
  SceneInit();
  // Interface
  selectionsText->Tx = 30;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  // Tool
  selectionIcons->Tx = *timer*84/timeLength;
  selectionIcons->Ty = 20;
  selectionIcons->Rend();
  // Dirt
  DirtySign->TexNo = 2-(*timer+15)/(timeLength/(3-1));
  DirtySign->Rend();
  return true;
}
void CleanLauncher(){
  uint8_t timeOut = 70;
  delay(1000);
  for(uint8_t i=0; i<timeOut; i++){
    if(!Clean(&i, timeOut)) break;
    screen.update();
  }
  Dirty = 0;
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
  screen.print(" [CLEAR]",37, 25);
  screen.print(" [BACK ]", 37, 32);
  screen.print(">", 1+(*confNo)/2*36, 9+8*(*confNo));
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
          *mode = *confNo;
          break;
        case 2: // Clear
          Clear();
          return false;
          break;
        case 3: // Return
          Save();
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
  delay(1000);
}

//............................................................
//............................................................
//......................... GARALLY SCENE ....................
//............................................................
//............................................................
//............................................................
bool Garally(uint8_t *timer , uint8_t *imageNo){
  SceneInit();
  // Interface
  screen.print("/", 66, 9);
  screen.printNumI(Maki->TexQuant, 71, 9);
  screen.printNumI(*imageNo+1, 53, 9);
  selectionsText->Tx = 30;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  screen.print(" <  ", 2, 40);
  screen.print("BACK", 30, 40);
  screen.print("  > ", 58, 40);
  // Image
  Maki->TexNo = *imageNo;
  Maki->Tx = 30;
  Maki->Ty = 11;
  Maki->Rend();
  // Input
  if(btnDownL){
    mp3_play(SE_BTN_OK);
    *imageNo += Maki->TexQuant - 1;
    *imageNo %= Maki->TexQuant;
  }
  if(btnDownC){
    mp3_play(SE_BTN_OK);
    return false;
  }
  if(btnDownR){
    mp3_play(SE_BTN_OK);
    *timer = 0;
    *imageNo += 1;
    *imageNo %= Maki->TexQuant;
  }
  return true;
}
void GarallyLauncher(){
  int8_t imageNo = 0;
  uint8_t timeOut = 255;
  for(uint8_t i=0; i<timeOut; i++){
    if(!Garally(&i, &imageNo)) break;
    screen.update();
  }
}

//............................................................
//............................................................
//...................... COMMUNICATION SCENE .................
//............................................................
//............................................................
//............................................................
void CommunicationLauncher(){
  Save();
  SceneInit();
  // QR
  uint8_t infoData[17] = "YM-";
  {
    uint8_t i = 3;
    for(; i<8; i++){
      infoData[i] = Frame >> (8*i) & 0x000000000000FF;
    }
    infoData[i++] = Life;
    infoData[i++] = Love;
    infoData[i++] = Hungery;
    infoData[i++] = Sick;
    infoData[i++] = Dirty;
    infoData[i++] = Rank;
  }
  DirectQREncode(&screen, infoData, sizeof(infoData), 62, 13);
  // Interface
  selectionsText->Tx = 30;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  screen.print(" -- ", 2, 40);
  screen.print("BACK", 30, 40);
  screen.print(" -- ", 58, 40);
  ComInstruntion->TexNo = 0;
  ComInstruntion->Rend();
  screen.update();
  delay(2000);
  // Input
  uint16_t timeOut = 1000;
  for(uint8_t i=0; i<timeOut; i++){
    BtnDetectAll();
    if(btnDownL){
      mp3_play(SE_BTN_OK);
      i = 0;
    }
    if(btnDownC){
      mp3_play(SE_BTN_OK);
      break;
    }
    if(btnDownR){
      mp3_play(SE_BTN_OK);
      i = 0;
    }
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
    for(uint8_t i=255; i>0; i--) Mingame0(i, &loveCnt);
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
    uint8_t qrTex[21*3*2];  //126
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
//........................ GAME SCENE ........................
//............................................................
//............................................................
//............................................................
bool Game(int countDown, int maxTime, uint8_t* loveCnt){
  SceneInit();
  // Interface
  screen.print("TIME", 1, 1);
  screen.printNumI(maxTime-countDown, 1, 9);
  selectionsText->Tx = 29;
  selectionsText->Ty = 1;
  selectionsText->Rend();
  screen.print("SCORE", 53, 1);
  screen.printNumI(*loveCnt, 60, 9);
  screen.print(" ^  ", 2, 40);
  screen.print(" ^ ", 33, 40);
  screen.print("  ^ ", 58, 40);
  //Maki
    // <-----------84-------------->
    // <--------59---------><--27-->
    // <-20--><--20-><--19->
  randomSeed(countDown/20*99&111);
  uint8_t pos = random(0,3)%3;
  Maki->Ty = 10;
  Maki->Tx = 4 + pos*26;
  Maki->TexNo = Frame/16%2 + Rank*6;
  Hurt->Tx = Maki->Tx-3;
  Hurt->Ty = Maki->Ty+3;
  //Hand
  bool hitting = false;
  if(btnL){
    if(pos==0){
      Maki->TexNo += 2;
      hitting = true;
    }
  }else if(btnC){
    if(pos==1){
      Maki->TexNo += 2;
      hitting = true;
    }
  }else if(btnR){
    if(pos==2){
      Maki->TexNo += 2;
      hitting = true;
    }
  }
  Maki->Rend();
  if(hitting) Hurt->Rend();
  //HitCheck
  if(btnDownL){
    if(pos==0){
      mp3_play(SE_GYN_0);
      (*loveCnt)++;
    }
  }else if(btnDownC){
    if(pos==1){
      mp3_play(SE_GYN_0);
      (*loveCnt)++;
    }
  }else if(btnDownR){
    if(pos==2){
      mp3_play(SE_GYN_0);
      (*loveCnt)++;
    }
  }
  return true;
}
void GameLauncher(){
  {
    SceneInit();
    // Interface
    selectionsText->Tx = 30;
    selectionsText->Ty = 1;
    selectionsText->Rend();
    screen.print("BACK", 2, 40);
    screen.print(" OK ", 30, 40);
    screen.print(" -- ", 58, 40);
    ComInstruntion->TexNo = 1;
    ComInstruntion->Rend();
    screen.update();
    delay(2000);
    // Input
    uint16_t timeOut = 1000;
    for(uint8_t i=0; i<timeOut; i++){
      BtnDetectAll();
      if(btnDownL){
        mp3_play(SE_BTN_OK);
        return;
      }
      if(btnDownC){
        mp3_play(SE_BTN_OK);
        break;
      }
      if(btnDownR){
        mp3_play(SE_BTN_OK);
        i = 0;
      }
    }
  }
  //Main
  int timeOut = 600;
  uint8_t loveCnt = 0;
  for(int i=0; i<timeOut; i++){
     if(!Game(i, timeOut, &loveCnt)) break;
     screen.update();
  }
  {
    SceneInit();
    // Interface
    selectionsText->Tx = 30;
    selectionsText->Ty = 1;
    selectionsText->Rend();
    screen.print(" -- ", 2, 40);
    screen.print(" OK ", 30, 40);
    screen.print(" -- ", 58, 40);
    //Result
    screen.print("YOUR SCORE!", 0, 16);
    screen.printNumI(loveCnt, 35, 26);
    screen.update();
    delay(2000);
    // Input
    uint16_t timeOut = 1000;
    for(uint8_t i=0; i<timeOut; i++){
      BtnDetectAll();
      if(btnDownL){
        mp3_play(SE_BTN_OK);
        i = 0;
      }
      if(btnDownC){
        mp3_play(SE_BTN_OK);
        break;
      }
      if(btnDownR){
        mp3_play(SE_BTN_OK);
        i = 0;
      }
    }
  }
}
//............................................................
//............................................................
//.......................... END .............................
//............................................................
