#include <stdlib.h>

#define MEMORY_SIZE 4096
#define REGISTER_SIZE 16
#define STACK_SIZE 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32


typedef struct {
    unsigned char opcode; // 2 byte opcode
    unsigned char memory[MEMORY_SIZE]; // 4k memory
    unsigned char V [REGISTER_SIZE]; // 16 registers
    unsigned short stack[STACK_SIZE]; // A stack with 16 levels
    unsigned char sp; // stack pointer
    unsigned char gfx [SCREEN_WIDTH * SCREEN_HEIGHT]; // graphics
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short I; // index register
    unsigned short pc; // program counter
    unsigned char keypad[16];
    int draw_flag;
} Chip8_t;

// 00E0 - CLS
// Clears the Display
void opcode_00E0(Chip8_t *chip8){
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        chip8->gfx[i] = 0;
    }
}

// 00EE - RET
// Return from a subroutine
void opcode_00EE(Chip8_t *chip8){
    chip8->pc = chip8->stack[chip8->sp]; // set pc to top of stack
    chip8->sp--;
}

// 1nnn - JP addr
// Jump to location 1nnn
void opcode_1nnn(Chip8_t *chip8, unsigned short nnn) {
    chip8 -> pc = nnn;
}

// 2nnn - CALL addr
// Call subroutine at nnn
void opcode_2nnn(Chip8_t *chip8, unsigned short nnn) {
    chip8 -> stack[chip8 -> sp] = chip8 -> pc;
    ++chip8 -> pc;
    chip8 -> pc = nnn;

}

// 3xkk - SE Vx, byte
// Skip next instruction if Vx == kk.
void opcode_3xkk(Chip8_t *chip8, unsigned short x, unsigned short kk) {
    if (chip8->V[x] == kk) {
        chip8->pc += 2;
    }
}

//  4xkk - SNE Vx, byte
//  Skip next instruction if Vx != kk
void opcode_4xkk(Chip8_t *chip8, unsigned short x, unsigned short kk) {
    if (chip8->V[x] != kk) {
        chip8->pc += 2;
    }
}

// 5xy0 - SE Vx, Vy
// Skip next instruction if Vx = Vy.
void opcode_5xy0(Chip8_t *chip8, unsigned short x, unsigned short y) {
    if (chip8 -> V[x] == chip8->V[y]) {
        chip8->pc += 2;
    }
}

// 6xkk - LD Vx, byte
// Set Vx = kk.
void opcode_6xkk (Chip8_t *chip8, unsigned short x, unsigned short kk) {
    chip8 -> V[x] = kk;
}

// 7xkk - ADD Vx, byte
// Set Vx = Vx + kk.
void opcode_7xkk(Chip8_t *chip8, unsigned short x, unsigned short kk) {
    chip8 -> V[x] += kk;
}

// 8xy0 - LD Vx, Vy
// Set Vx = Vy
void opcode_8xy0(Chip8_t *chip8, unsigned short x, unsigned short y) {
    chip8 -> V[x] = chip8 -> V[y];
}

//8xy1 - OR Vx, Vy
//Set Vx = Vx OR Vy.
void opcode_8xy1(Chip8_t *chip8, unsigned short x, unsigned short y) {
    chip8->V[x] |= chip8->V[y];
}

//8xy2 - AND Vx, Vy
//Set Vx = Vx AND Vy
void opcode_8xy2(Chip8_t *chip8, unsigned short x, unsigned short y) {
    chip8->V[x] &= chip8->V[y];
}

//8xy3 - XOR Vx, Vy
//Set Vx = Vx XOR Vy
void opcode_8xy3(Chip8_t *chip8, unsigned short x, unsigned short y) {
    chip8->V[x] ^= chip8->V[y];
}

//8xy4 - ADD Vx, Vy
//Set Vx = Vx + Vy, set VF = carry.
//The values of Vx and Vy are added together. If the result is greater than 8
// bits (i.e., > 255) VF is set to 1, otherwise 0. Only the lowest 8 bits of the
// result are kept, and stored in Vx.
void opcode_8xy4(Chip8_t *chip8, unsigned short x, unsigned short y) {
    if (chip8->V[x] + chip8->V[y] > 0xFF) {
        chip8->V[x] = (chip8->V[x] + chip8->V[y]) & 0xFF;
        chip8->V[0x0F] = 1;
    } else {
        chip8->V[x] += chip8->V[y];
        chip8->V[0x0F] = 0;
    }
}

//8xy5 - SUB Vx, Vy
//Set Vx = Vx - Vy, set VF = NOT borrow.
//If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx,
//and the results stored in Vx.
void opcode_8xy5(Chip8_t *chip8, unsigned short x, unsigned short y) {
    chip8-> V[0x0F] = (chip8-> V[x] > chip8->V[y]) ? 1 : 0;
    chip8-> V[x] = chip8->V[x] - chip8->V[y];
}

//8xy6 - SHR Vx {, Vy}
//Set Vx = Vx SHR 1.
//If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
//Then Vx is divided by 2.
void opcode_8xy6(Chip8_t *chip8, unsigned  short x) {
    chip8 -> V[0x0F] = (chip8 -> V[x] & 1);
    chip8 -> V[x] >>= 1;
}

// 8xy7 - SUBN Vx, Vy
//Set Vx = Vy - Vx, set VF = NOT borrow.
//If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy,
// and the results stored in Vx.
void opcode_8xy7(Chip8_t *chip8, unsigned short x, unsigned short y) {
    chip8-> V[0x0F] = (chip8-> V[y] > chip8->V[x]) ? 1 : 0;
    chip8-> V[y] = chip8->V[y] - chip8->V[x];
}

//8xyE - SHL Vx {, Vy}
//Set Vx = Vx SHL 1.
//If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
// Then Vx is multiplied by 2.
void opcode_8xyE(Chip8_t *chip8, unsigned short x) {
    chip8-> V[0x0F] = (chip8 -> V[x] >> 0x0F);
    chip8 -> V[x] <<= 1;
}

//9xy0 - SNE Vx, Vy
//Skip next instruction if Vx != Vy.
//The values of Vx and Vy are compared, and if they are not equal,
// the program counter is increased by 2.
void opcode_9xy0(Chip8_t *chip8, unsigned short x, unsigned short y) {
    if (chip8 -> V[x] != chip8 -> V[y]) {
        chip8->pc += 2;
    }
}

//Annn - LD I, addr
//Set I = nnn.
//The value of register I is set to nnn.
void opcode_Annn(Chip8_t *chip8, unsigned short nnn) {
    chip8 -> I = nnn;
}

//Bnnn - JP V0, addr
//Jump to location nnn + V0.
//The program counter is set to nnn plus the value of V0.
void opcode_Bnnn(Chip8_t *chip8, unsigned short nnn) {
    chip8 -> pc = nnn + chip8->V[0x00];
}

//Cxkk - RND Vx, byte
//Set Vx = random byte AND kk.
//The interpreter generates a random number from 0 to 255, which is then
// ANDed with the value kk.The results are stored in Vx. See instruction
// 8xy2 for more information on AND
void opcode_Cxkk(Chip8_t *chip8, unsigned short x, unsigned short kk) {
    chip8 -> V[x] = (rand() % 256) & kk; // NOLINT(cert-msc30-c, cert-msc50-cpp)
}

//Dxyn - DRW Vx, Vy, nibble
//Display n-byte sprite starting at memory location I at (Vx, Vy) set VF = collision. The interpreter reads n bytes
// from memory, starting at the address stored in I. These bytes are then displayed as sprites on screen at coordinates
// (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1,
// otherwise it is set to 0. If the sprite is positioned so part of it is outside the coordinates of the display,
// it wraps around to the opposite side of the screen. See instruction 8xy3 for more information on XOR,
// and section 2.4, Display, for more information on the Chip-8 screen and sprites.
void opcode_Dxyn(Chip8_t *chip8, unsigned short x, unsigned short y, unsigned short n){
    unsigned short pixel;
    chip8->V[0x0F] = 0;
    int yline;
    int xline;
    for (yline = 0; yline < n; yline++){
        pixel = chip8-> memory[chip8->I + yline]; // reading bytes from memory
        for (xline = 0; xline < 8; xline++) {
            if ((pixel & (0x80 >> xline)) != 0) {
                chip8->V[0x0F] = chip8 -> gfx[(x + xline + ((y + yline) * 64))]; // Setting VF flag is a pixel is erased
                chip8->gfx[(x + xline + ((y + yline) * 64))] ^= 1; // Sprites are XORed onto existing screen
            }
        }
    }

    chip8->draw_flag = 1; // Set draw flag to true
    chip8->pc += 2; // increment pc
}

//Ex9E - SKP Vx
//Skip next instruction if key with the value of Vx is pressed.
//Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position,
// PC is increased by 2.
void opcode_Ex9E(Chip8_t *chip8, unsigned short x) {
    chip8->pc += (chip8->keypad[chip8->V[x]]) ? 2 : 0;
}

//ExA1 - SKNP Vx
//Skip next instruction if key with the value of Vx is not pressed.
//Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position,
// PC is increased by 2.
void opcode_ExA1(Chip8_t *chip8, unsigned short x) {
    chip8->pc += (!chip8->keypad[chip8->V[x]]) ? 2 : 0;
}

//Fx07 - LD Vx, DT
//Set Vx = delay timer value.
//The value of DT is placed into Vx.
void opcode_Fx07(Chip8_t *chip8, unsigned short x) {
    chip8->V[x] = chip8->delay_timer;
}

//Fx0A - LD Vx, K
//Wait for a key press, store the value of the key in Vx.
//All execution stops until a key is pressed, then the value of that key is stored in Vx.
void opcode_Fx0A(Chip8_t *chip8, unsigned short x) {
    int found_key = 0;
    for (int i = 0; i < 16; i++) {
        if (chip8->keypad[i]) {
            chip8->V[x] = i;
            found_key = 1;
            break;
        }
    }

    if (!found_key) {
        chip8->pc -= 2; // decrement pc so we move on
    }
}

//Fx15 - LD DT, Vx
//Set delay timer = Vx.
//DT is set equal to the value of Vx.
void opcode_Fx15(Chip8_t *chip8, unsigned short x) {
    chip8->delay_timer = chip8->V[x];
}

//Fx18 - LD ST, Vx
//Set sound timer = Vx.
//ST is set equal to the value of Vx.
void opcode_Fx18(Chip8_t *chip8, unsigned short x) {
    chip8->sound_timer = chip8 ->V[x];
}

//Fx29 - LD F, Vx
//Set I = location of sprite for digit Vx.
//The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
// See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
void opcode_Fx29(Chip8_t *chip8, unsigned short x){
    chip8->I = (chip8->V[x]*0x05); // Each chararacter has 5 elements hence * 0x05
}

//Fx33 - LD B, Vx
//Store BCD representation of Vx in memory locations I, I+1, and I+2.
//The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I,
// the tens digit at location I+1, and the ones digit at location I+2.
void opcode_Fx33(Chip8_t *chip8, unsigned short x) {
    unsigned short I = chip8->I;

    chip8->V[I] = chip8->V[x] / 100;
    chip8->V[I+1] = (chip8->V[x] / 10) % 10;
    chip8->V[I+2] = (chip8->V[x] % 100) % 10;
}

//Fx55 - LD [I], Vx
//Store registers V0 through Vx in memory starting at location I.
//The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
void opcode_Fx55(Chip8_t *chip8, unsigned short x) {
    unsigned short I = chip8->I;
    int i;
    for (i = 0; i <= x; i++) {
        chip8->memory[I + i];
    }
}

//Fx65 - LD Vx, [I]
//Read registers V0 through Vx from memory starting at location I.
//The interpreter reads values from memory starting at location I into registers V0 through Vx.
void opcode_Fx65(Chip8_t *chip8, unsigned short x) {
    unsigned short I = chip8->I;
    int i;
    for (i = 0; i <= x; i++) {
        chip8->V[i] = chip8->memory[I+i];
    }
}


void emulate_cycle(Chip8_t *chip8){
//    fetch opcode
    chip8 -> opcode = chip8 -> memory[chip8 -> pc] << 8 | chip8 -> memory[chip8 -> pc + 1];

// register identifiers
    unsigned short x = (chip8->opcode & 0x0F00) >> 8;
    unsigned short y = (chip8->opcode & 0x0F00) >> 4;


//    decode opcode
    switch(chip8 -> opcode & 0xF000) {
        case 0x0000:
            switch (chip8->opcode & 0x000F) {
                case 0x00E: // 0x00E0: Clears the screen
                opcode_00E0(chip8);
            }
    }

//    increment pc
    chip8->pc += 2;

}

void init_chip8(Chip8_t *chip8) {
    chip8->opcode = 0; // reset opcode
    chip8->pc = 0x200; // program counter starts at 0x200
    chip8->I = 0; // reset index register
    chip8->sp = 0; // reset stack pointer

//    clear display
    for (int i = 0; i < 2048; i++) {
        chip8->gfx[i] = 0;
    }

//    clear stack
    for (int i = 0; i < 16; i++) {
        chip8->stack[i] = 0;
    }

//    clear registers
    for (int i = 0; i < 16; i++) {
        chip8->V[i] = 0;
    }

//    clear memory
    for (int i = 0; i < 4096; i++) {
        chip8->memory[i] = 0;
    }

//    load fontset
    for (int i = 0; i < 80; i++) {
        chip8->memory[i] = fontset[i];
    }

//    reset timers
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
}