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
    unsigned char keypad[16]; // keypad
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
//The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
// The results are stored in Vx. See instruction 8xy2 for more information on AND
void opcode_Cxkk(Chip8_t *chip8, unsigned short x, unsigned short kk) {
    chip8 -> V[x] = (rand() % 256) & kk;
}

//Dxyn - DRW Vx, Vy, nibble
//Display n-byte sprite starting at memory location I at (Vx, Vy)
//set VF = collision. The interpreter reads n bytes from memory, starting
// at the address stored in I. These bytes are then displayed as sprites
// on screen at coordinates (Vx, Vy). Sprites are XORed onto the
// existing screen. If this causes any
// pixels to be erased, VF is set to 1, otherwise it is set to 0. If the sprite
// is positioned so part of it is outside the coordinates of the display,
// it wraps around to the opposite side of the screen. See instruction 8xy3
// for more information on XOR, and section 2.4, Display, for more information
// on the Chip-8 screen and sprites.
void opcode_Dxyn(){
//    TODO
}

void emulate_cycle(Chip8_t *chip8){
//    fetch opcode
    chip8 -> opcode = chip8 -> memory[chip8 -> pc] << 8 | chip8 -> memory[chip8 -> pc + 1];

//    decode opcode
    switch(chip8 -> opcode & 0xF000) {
        case 0x0000:
            switch (chip8->opcode & 0x000F) {
                case 0x00E: // 0x00E0: Clears the screen
                opcode_00E0(chip8);
            }
    }

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