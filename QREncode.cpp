#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "QREncode.h"
#include "Game2D.h"
#define QR_BMP_SIZE 63
#define QR_SIZE     21
#define QR_DATA     19
#define QR_EC       7
#define QR_HEAD_CON 8
#define QR_HEAD     15
#define MAX_INFO    17
#define MODE_SING   0b0100
#define PADDING_1   0b11101100
#define PADDING_2   0b00010001
#define DIV_0 96
#define DIV_1 176
#define DIV_2 208
#define MASK_MODE   0b100

extern const uint8_t QRTemplate[] PROGMEM = { //63*2 + 6 Byte
    0x15, 0x00, //Width 21
    0x15, 0x00, //Height 21
    0x3F, 0x00, //Size 63 x2
    0x00, 0x00, //Reserve 0

    //Color
    0x7F, 0x41, 0x5D, 0x5D, 0x5D, 0x41, 0x7F, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x7F, 0x41, 0x5D, 0x5D, 0x5D, 0x41, 0x7F, 0xC0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xD5, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x17, 0x17, 0x17, 0x10, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //Alpha
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x40, 0x40, 0x40, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xE0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
extern const uint8_t Galois28Table[] PROGMEM = { //256 byte
    1, 2, 4, 8, 16, 32, 64, 128, 29, 58, 116, 232, 205, 135, 19, 38, 76, 152, 45, 90, 180, 117, 234, 201, 143, 3, 6, 12, 24, 48, 96, 192, 157, 39, 78, 156, 37, 74, 148, 53, 106, 212, 181, 119, 238, 193, 159, 35, 70, 140, 5, 10, 20, 40, 80, 160, 93, 186, 105, 210, 185, 111, 222, 161, 95, 190, 97, 194, 153, 47, 94, 188, 101, 202, 137, 15, 30, 60, 120, 240, 253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163, 91, 182, 113, 226, 217, 175, 67, 134, 17, 34, 68, 136, 13, 26, 52, 104, 208, 189, 103, 206, 129, 31, 62, 124, 248, 237, 199, 147, 59, 118, 236, 197, 151, 51, 102, 204, 133, 23, 46, 92, 184, 109, 218, 169, 79, 158, 33, 66, 132, 21, 42, 84, 168, 77, 154, 41, 82, 164, 85, 170, 73, 146, 57, 114, 228, 213, 183, 115, 230, 209, 191, 99, 198, 145, 63, 126, 252, 229, 215, 179, 123, 246, 241, 255, 227, 219, 171, 75, 150, 49, 98, 196, 149, 55, 110, 220, 165, 87, 174, 65, 130, 25, 50, 100, 200, 141, 7, 14, 28, 56, 112, 224, 221, 167, 83, 166, 81, 162, 89, 178, 121, 242, 249, 239, 195, 155, 43, 86, 172, 69, 138, 9, 18, 36, 72, 144, 61, 122, 244, 245, 247, 243, 251, 235, 203, 139, 11, 22, 44, 88, 176, 125, 250, 233, 207, 131, 27, 54, 108, 216, 173, 71, 142, 1 
};
extern const uint8_t InvGalois28Table[] PROGMEM = { //256 byte
    0, 0, 1, 25, 2, 50, 26, 198, 3, 223, 51, 238, 27, 104, 199, 75, 4, 100, 224, 14, 52, 141, 239, 129, 28, 193, 105, 248, 200, 8, 76, 113, 5, 138, 101, 47, 225, 36, 15, 33, 53, 147, 142, 218, 240, 18, 130, 69, 29, 181, 194, 125, 106, 39, 249, 185, 201, 154, 9, 120, 77, 228, 114, 166, 6, 191, 139, 98, 102, 221, 48, 253, 226, 152, 37, 179, 16, 145, 34, 136, 54, 208, 148, 206, 143, 150, 219, 189, 241, 210, 19, 92, 131, 56, 70, 64, 30, 66, 182, 163, 195, 72, 126, 110, 107, 58, 40, 84, 250, 133, 186, 61, 202, 94, 155, 159, 10, 21, 121, 43, 78, 212, 229, 172, 115, 243, 167, 87, 7, 112, 192, 247, 140, 128, 99, 13, 103, 74, 222, 237, 49, 197, 254, 24, 227, 165, 153, 119, 38, 184, 180, 124, 17, 68, 146, 217, 35, 32, 137, 46, 55, 63, 209, 91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190, 97, 242, 86, 211, 171, 20, 42, 93, 158, 132, 60, 57, 83, 71, 109, 65, 162, 31, 45, 67, 216, 183, 123, 164, 118, 196, 23, 73, 236, 127, 12, 111, 246, 108, 161, 59, 82, 41, 157, 85, 170, 251, 96, 134, 177, 187, 204, 62, 90, 203, 89, 95, 176, 156, 169, 160, 81, 11, 245, 22, 235, 122, 117, 44, 215, 79, 174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168, 80, 88, 175
};

uint8_t* QREncode(GameLCD *screen, uint8_t* infoData, uint8_t infoLength, uint8_t* dest){

    if(infoLength > MAX_INFO) return; // Exception

    for(int i=QR_BMP_SIZE; i<QR_BMP_SIZE*2; i++) dest[i] = 0xFF;
    uint8_t buf[QR_DATA + QR_EC];   // + 26 Byte : QR_DATA + QR_EC
    for(int i=0; i<QR_DATA + QR_EC; i++){
        buf[i] = 0x00;
    }
    //............................................
    // Step 0
    // Mode config
    //............................................
    buf[0] |= MODE_SING << 4;      // Binary Mode
    buf[0] |= infoLength >> 4;  // Count before
    buf[1] |= infoLength << 4;  // Count after

    //............................................
    // Step 1
    // Plain encode
    //............................................
    {
        int i;
        for(i=2; i<infoLength + 2; i++){
            buf[i-1] |= infoData[i-2] >> 4;
            buf[i] |= infoData[i-2] << 4;
        }
        while(i < QR_DATA){
            buf[i++] = PADDING_1;
            if(i >= QR_DATA) break;
            buf[i++] = PADDING_2;
        }
    }

    //............................................
    // Step 2
    // RS encode
    //............................................
    uint8_t gp[] = {0, 87, 229, 146, 149, 238, 102, 21};    // + 8 Byte : QR_EC+1
    uint8_t ip[QR_DATA+QR_EC];                                    // + 26 Byte : QR_EC+QR_DATA
    //Copy
    for(int i=0; i<QR_DATA; i++){
        ip[i] = buf[i];
    }
    for(int i=QR_DATA; i<QR_DATA+QR_EC; i++){
        ip[i] = 0;
    }
    // Divid
    for(int i=0; i<QR_DATA; i++){
        // Tune and convert genePoly
        if(ip[i]==0) continue;
        uint8_t scale = pgm_read_byte(InvGalois28Table + ip[i]);

        uint8_t gpcp[QR_EC+1];
        for(int j=0; j<QR_EC+1; j++){
            gpcp[j] = (gp[j]+scale)%255;
            gpcp[j] = pgm_read_byte(Galois28Table + gpcp[j]);
        }
        // infoPoly - scale * genePoly * x^(N-M)
        for(int j=0; j<QR_EC+1; j++){
            ip[i+j] = ip[i+j]^gpcp[j];
        }
    }
    //Concat
    for(int i=0; i<QR_EC; i++){
        buf[QR_DATA+i] = ip[QR_DATA+i];
    }

    //............................................
    // Step 3
    // Map on 2D
    //............................................
    {
        int i;
        uint8_t x, y;
        bool right, up;
        //Division 0
        i = 0;
        x = QR_SIZE-1;
        y = QR_SIZE-1;
        up = true;
        right = true;
        for(; i<DIV_0; i++){ //12 Byte
            uint8_t address = i/8;
            uint8_t bit = 7-i%8;
            uint8_t col = (buf[address] & (1<<bit)) >> bit;
            WritePatternPixel(dest, x, y, col, MASK_MODE);
            //Move
            if(right){
                right = false;
                x--;
            }else{
                right = true;
                if(up){
                    if(y==9){
                        up = false;
                        x--;
                    }else{
                        x++;
                        y--;
                    }
                }else{
                    if(y==20){
                        up = true;
                        x--;
                    }else{
                        x++;
                        y++;
                    }
                }
            }
        }
        //Division 1
        for(; i<DIV_1; i++){  //10 Byte
            uint8_t address = i/8;
            uint8_t bit = 7-i%8;
            uint8_t col = (buf[address] & (1<<bit)) >> bit;
            WritePatternPixel(dest, x, y, col, MASK_MODE);
            //Move
            if(right){
                right = false;
                x--;
            }else{
                right = true;
                if(up){
                    if(y==0){
                        up = false;
                        x--;
                    }else{
                        x++;
                        y--;
                        if(y==6)y--;
                    }
                }else{
                    if(y==20){
                        up = true;
                        x--;
                    }else{
                        x++;
                        y++;
                        if(y==6)y++;
                    }
                }
            }
        }
        //Division 2
        x = 8;
        y = 12;
        for(; i<DIV_2; i++){  //4 Byte
            uint8_t address = i/8;
            uint8_t bit = 7-i%8;
            uint8_t col = (buf[address] & (1<<bit)) >> bit;
            WritePatternPixel(dest, x, y, col, MASK_MODE);
            //Move
            if(right){
                right = false;
                x--;
            }else{
                right = true;
                if(up){
                    if(y==9){
                        up = false;
                        x--;
                        if(x==6) x--;
                    }else{
                        x++;
                        y--;
                    }
                }else{
                    if(y==12){
                        up = true;
                        x--;
                        if(x==6) x--;
                    }else{
                        x++;
                        y++;
                    }
                }
            }
        }
    }

    //............................................
    // Step 4
    // Header
    //............................................
    uint16_t header = 0;
    header |= 0b01 << 13;     // Recovery mode L (01)
    header |= MASK_MODE << 10;    // Mask mode (001)
    // BCH Encode
    uint16_t genePoly = 0b0000010100110111; // 0000 0101 0011 0111 
    uint8_t geneDeg = 10;
    uint16_t infoPoly = header;             // 0010 0100 0000 0000
    uint8_t infoDeg = 5;
    for(int i=4; i>=0; i--){
        if((infoPoly >> (10+i)) & 1){
            infoPoly ^= genePoly << i;
        }
    }
    header |= infoPoly;
    header ^= 0b101010000010010;
    {
        //for(int i=0; i<QR_BMP_SIZE; i++) dest[i] = 0x00;
        uint8_t i = 0;
        int x0 = 8;     // upper-left
        int y0 = 0;
        int x1 = 20;    // right-under
        int y1 = 8;
        for(; i<QR_HEAD_CON; i++){
            uint8_t col;
            if(header& (1<<i)) col = 0xFF;
            else col = 0x00;
            WritePatternPixel(dest, x0, y0, col);
            WritePatternPixel(dest, x1, y1, col);
            y0++;
            if(y0==6) y0++;
            x1--;
        }
        y0--;
        x0--;
        x1 = 8;
        y1 = 14;
        for(; i<QR_HEAD; i++){
            uint8_t col;
            if(header& (1<<i)) col = 0xFF;
            else col = 0x00;
            WritePatternPixel(dest, x0, y0, col);
            WritePatternPixel(dest, x1, y1, col);
            x0--;
            if(x0==6) x0--;
            y1++;
        }
    }

    //............................................
    // Step 5
    // Add template
    //............................................
    for(int i=0; i<QR_BMP_SIZE; i++){
        dest[i] = dest[i] & (~pgm_read_byte(QRTemplate+8+QR_BMP_SIZE+i));
        dest[i] |= pgm_read_byte(QRTemplate+8+i);
    }

    return dest;
}

void WritePatternPixel(uint8_t* dest, uint16_t x, uint16_t y, uint8_t col){
	int by, bi;
    by=((y/8)*QR_SIZE)+x;
    bi=y % 8;
    if(col){
        //black
        dest[by] |= (1<<bi);
    }else{
        //white
        dest[by] &= ~(1<<bi);
    }
}
void WritePatternPixel(uint8_t* dest, uint16_t x, uint16_t y, uint8_t col, uint8_t mask){
	int by, bi;
    by=((y/8)*QR_SIZE)+x;
    bi=y % 8;
    
    switch(mask){
        case 0b000 :
            if((x+y)%2 == 0) col = !col;
            break;
        case 0b001 :
            if(y%2 == 0) col = !col;
            break;
        case 0b010 :
            if(x%3 == 0) col = !col;
            break;
        case 0b011 :
            if((x+y)%3 == 0) col = !col;
            break;
        case 0b100 :
            if(((y/2)+(x/3))%2 == 0) col = !col;
            break;
        default:
            col = 0x00;
            break;
    }
    //*/
    if(col){
        //black
        dest[by] |= (1<<bi);
    }else{
        //white
        dest[by] &= ~(1<<bi);
    }
}

//........................................................................
//........................................................................
//........................................................................
//................................USE EEPROM .............................
//........................................................................
//........................................................................

void DirectQREncode(GameLCD *screen, uint8_t* infoData, uint8_t infoLength, uint8_t tx, uint8_t ty){
    if(infoLength > MAX_INFO) return; // Exception

    uint8_t buf[QR_DATA + QR_EC];   // + 26 Byte : QR_DATA + QR_EC
    for(int i=0; i<QR_DATA + QR_EC; i++){
        buf[i] = 0x00;
    }
    //............................................
    // Step 0
    // Mode config
    //............................................
    buf[0] |= MODE_SING << 4;      // Binary Mode
    buf[0] |= infoLength >> 4;  // Count before
    buf[1] |= infoLength << 4;  // Count after

    //............................................
    // Step 1
    // Plain encode
    //............................................
    {
        int i;
        for(i=2; i<infoLength + 2; i++){
            buf[i-1] |= infoData[i-2] >> 4;
            buf[i] |= infoData[i-2] << 4;
        }
        while(i < QR_DATA){
            buf[i++] = PADDING_1;
            if(i >= QR_DATA) break;
            buf[i++] = PADDING_2;
        }
    }

    //............................................
    // Step 2
    // RS encode
    //............................................
    uint8_t gp[] = {0, 87, 229, 146, 149, 238, 102, 21};    // + 8 Byte : QR_EC+1
    uint8_t ip[QR_DATA+QR_EC];                                    // + 26 Byte : QR_EC+QR_DATA
    //Copy
    for(int i=0; i<QR_DATA; i++){
        ip[i] = buf[i];
    }
    for(int i=QR_DATA; i<QR_DATA+QR_EC; i++){
        ip[i] = 0;
    }
    // Divid
    for(int i=0; i<QR_DATA; i++){
        // Tune and convert genePoly
        if(ip[i]==0) continue;
        uint8_t scale = pgm_read_byte(InvGalois28Table + ip[i]);

        uint8_t gpcp[QR_EC+1];
        for(int j=0; j<QR_EC+1; j++){
            gpcp[j] = (gp[j]+scale)%255;
            gpcp[j] = pgm_read_byte(Galois28Table + gpcp[j]);
        }
        // infoPoly - scale * genePoly * x^(N-M)
        for(int j=0; j<QR_EC+1; j++){
            ip[i+j] = ip[i+j]^gpcp[j];
        }
    }
    //Concat
    for(int i=0; i<QR_EC; i++){
        buf[QR_DATA+i] = ip[QR_DATA+i];
    }
    
    //............................................
    // Step 5
    // Add template
    //............................................
    for(int i=0; i<QR_BMP_SIZE; i++){
        uint8_t col = pgm_read_byte(QRTemplate+8+i);
        for(int k=7; k>=0; k--){
            screen->SetPixel(i%21 + tx, i/21*8+k + ty, col>>k&0x01);
        }
    }

    //............................................
    // Step 3
    // Map on 2D
    //............................................
    {
        int i;
        int x, y;
        bool right, up;
        //Division 0
        i = 0;
        x = QR_SIZE-1;
        y = QR_SIZE-1;
        up = true;
        right = true;
        for(; i<DIV_0; i++){ //12 Byte
            uint8_t address = i/8;
            uint8_t bit = 7-i%8;
            uint8_t col = (buf[address] & (1<<bit)) >> bit;
            WritePatternPixel(screen, x, y, tx, ty, col, MASK_MODE);
            //Move
            if(right){
                right = false;
                x--;
            }else{
                right = true;
                if(up){
                    if(y==9){
                        up = false;
                        x--;
                    }else{
                        x++;
                        y--;
                    }
                }else{
                    if(y==20){
                        up = true;
                        x--;
                    }else{
                        x++;
                        y++;
                    }
                }
            }
        }
        //Division 1
        for(; i<DIV_1; i++){  //10 Byte
            uint8_t address = i/8;
            uint8_t bit = 7-i%8;
            uint8_t col = (buf[address] & (1<<bit)) >> bit;
            WritePatternPixel(screen, x, y, tx, ty, col, MASK_MODE);
            //Move
            if(right){
                right = false;
                x--;
            }else{
                right = true;
                if(up){
                    if(y==0){
                        up = false;
                        x--;
                    }else{
                        x++;
                        y--;
                        if(y==6)y--;
                    }
                }else{
                    if(y==20){
                        up = true;
                        x--;
                    }else{
                        x++;
                        y++;
                        if(y==6)y++;
                    }
                }
            }
        }
        //Division 2
        x = 8;
        y = 12;
        for(; i<DIV_2; i++){  //4 Byte
            uint8_t address = i/8;
            uint8_t bit = 7-i%8;
            uint8_t col = (buf[address] & (1<<bit)) >> bit;
            WritePatternPixel(screen, x, y, tx, ty, col, MASK_MODE);
            //Move
            if(right){
                right = false;
                x--;
            }else{
                right = true;
                if(up){
                    if(y==9){
                        up = false;
                        x--;
                        if(x==6) x--;
                    }else{
                        x++;
                        y--;
                    }
                }else{
                    if(y==12){
                        up = true;
                        x--;
                        if(x==6) x--;
                    }else{
                        x++;
                        y++;
                    }
                }
            }
        }
    }

    //............................................
    // Step 4
    // Header
    //............................................
    uint16_t header = 0;
    header |= 0b01 << 13;     // Recovery mode L (01)
    header |= MASK_MODE << 10;    // Mask mode (001)
    // BCH Encode
    uint16_t genePoly = 0b0000010100110111; // 0000 0101 0011 0111 
    uint8_t geneDeg = 10;
    uint16_t infoPoly = header;             // 0010 0100 0000 0000
    uint8_t infoDeg = 5;
    for(int i=4; i>=0; i--){
        if((infoPoly >> (10+i)) & 1){
            infoPoly ^= genePoly << i;
        }
    }
    header |= infoPoly;
    header ^= 0b101010000010010;
    {
        //for(int i=0; i<QR_BMP_SIZE; i++) dest[i] = 0x00;
        uint8_t i = 0;
        int x0 = 8;     // upper-left
        int y0 = 0;
        int x1 = 20;    // right-under
        int y1 = 8;
        for(; i<QR_HEAD_CON; i++){
            uint8_t col;
            if(header& (1<<i)) col = 0xFF;
            else col = 0x00;
            WritePatternPixel(screen, x0, y0, tx, ty, col, 0b111);
            WritePatternPixel(screen, x1, y1, tx, ty, col, 0b111);
            y0++;
            if(y0==6) y0++;
            x1--;
        }
        y0--;
        x0--;
        x1 = 8;
        y1 = 14;
        for(; i<QR_HEAD; i++){
            uint8_t col;
            if(header& (1<<i)) col = 0xFF;
            else col = 0x00;
            WritePatternPixel(screen, x0, y0, tx, ty, col, 0b111);
            WritePatternPixel(screen, x1, y1, tx, ty, col, 0b111);
            x0--;
            if(x0==6) x0--;
            y1++;
        }
    }
    
    
    return;
}

void WritePatternPixel(GameLCD* dest, uint16_t x, uint16_t y, uint16_t tx, uint16_t ty, uint8_t col, uint8_t mask){
	int by, bi;
    by=((y/8)*QR_SIZE)+x;
    bi=y % 8;
    
    switch(mask){
        case 0b000 :
            if((x+y)%2 == 0) col = !col;
            break;
        case 0b001 :
            if(y%2 == 0) col = !col;
            break;
        case 0b010 :
            if(x%3 == 0) col = !col;
            break;
        case 0b011 :
            if((x+y)%3 == 0) col = !col;
            break;
        case 0b100 :
            if(((y/2)+(x/3))%2 == 0) col = !col;
            break;
        case 0b111 :
            break;
        default:
            col = 0x00;
            break;
    }
     dest->SetPixel(x+tx, y+ty, col);
}