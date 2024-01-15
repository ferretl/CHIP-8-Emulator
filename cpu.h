//
// Created by fezza on 15/01/2024.
//

#ifndef CHIP_8_CPU_H
#define CHIP_8_CPU_H
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

void emulate_cycle(Chip8_t *chip8);
void init_chip8(Chip8_t *chip8);

#endif //CHIP_8_CPU_H
