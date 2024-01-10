#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "fontset.h"

#define MEMORY_SIZE 4096
#define REGISTER_SIZE 16
#define STACK_SIZE 16


// SDL_t is a struct that contains the SDL window and renderer
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDL_t;

// DisplayConfig_t is a struct that contains the display configuration
typedef struct {
    int window_height;
    int window_width;
    int window_scale;
} DisplayConfig_t;

// initialise chip8
typedef struct {
    unsigned char opcode; // 2 byte opcode
    unsigned char memory[MEMORY_SIZE]; // 4k memory
    unsigned char V [REGISTER_SIZE]; // 16 registers
    unsigned short stack[STACK_SIZE]; // A stack with 16 levels
    unsigned char sp; // stack pointer
    unsigned char gfx [64 * 32]; // graphics
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short I; // index register
    unsigned short pc; // program counter
    unsigned char keypad[16]; // keypad
} Chip8_t;


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

void init_display(DisplayConfig_t *displayConfig){
    displayConfig->window_height = 32; // Original chip8 height
    displayConfig->window_width = 64; // Original chip8 width
    displayConfig->window_scale = 20; // Scale the window by 10
}

int init_sdl (SDL_t *sdl,DisplayConfig_t *displayConfig){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        SDL_Log(
            "Unable to initialise SDL Environment: %s\n",
            SDL_GetError()
        );
        return 0;
    }

//    create window

    sdl -> window = SDL_CreateWindow(
        "Chip8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (displayConfig->window_width) * (displayConfig->window_scale),
        (displayConfig->window_height) * (displayConfig->window_scale),
        SDL_WINDOW_SHOWN
    );
//    check if window was created
    if (!(sdl -> window)) {
        SDL_Log(
            "Unable to create SDL Window: %s\n",
            SDL_GetError()
        );
        return 0;
    }

//    create renderer
    sdl -> renderer = SDL_CreateRenderer(
        sdl -> window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

//    check if renderer was created
    if (!(sdl -> renderer)) {
        SDL_Log(
            "Unable to create SDL Renderer: %s\n",
            SDL_GetError()
        );
        return 0;
    }

    return 1;
}


void destroy_sdl(SDL_t *sdl){
    SDL_DestroyWindow(sdl->window);
    SDL_DestroyRenderer(sdl->renderer);
    puts("SDL Environment Destroyed");
    SDL_Quit();
}

int main (int argc, char **argv) {
    (void)argc;
    (void)argv;

    // initialise chip8
    DisplayConfig_t  displayConfig = {0};
    init_display(&displayConfig);

    // initialise sdl
    SDL_t sdl = {0};
    if (!init_sdl(&sdl, &displayConfig)) {
        return EXIT_FAILURE;
    }

    // initialise chip8
    Chip8_t chip8 = {0};
    init_chip8(&chip8);

    // main loop
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    running = 0;
                } break;
            }
        }
    }

    destroy_sdl(&sdl); // destroys sdl

    exit(EXIT_SUCCESS);
}