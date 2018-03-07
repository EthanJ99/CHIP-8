#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <SDL.h>

#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32

typedef struct chip8_t{
    uint8_t memory[0x1000]; // Memory
    uint8_t V[16];          // CPU Registers (V0 - VF)
    uint16_t I;             // Address register
    uint16_t PC;            // Program Counter

    // Stores pixel data
    uint8_t screen[CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT];

    uint8_t delay_timer;
    uint8_t sound_timer;

    uint16_t stack[16];
    uint8_t sp; // Stack pointer

    uint8_t key[16]; // Stores states for the CHIP-8's 16 keys
} Chip8;

void chip8_init(Chip8* chip8);
void chip8_run_cycle(Chip8* chip8);
void chip8_clear_screen(Chip8* chip8);
void chip8_load_program(Chip8* chip8, char* path);
void video_draw(Chip8* chip8, SDL_Window* w, SDL_Renderer* r, int scale);
void op_undefined_opcode(Chip8* chip8, uint16_t opcode);

int main(int argc, char* argv[]){
    // Init SDL with video out
    SDL_Init(SDL_INIT_VIDEO);

    // Init SDL window and renderer
    int video_scale = 5;
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        CHIP8_SCREEN_WIDTH * video_scale,
        CHIP8_SCREEN_HEIGHT * video_scale,
        0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetScale(renderer, video_scale, video_scale);

    // Create a Chip8 system and load its initial values.
    Chip8 chip8;
    chip8_init(&chip8);

    // Create variable for handling events (basically just keyboard input in this case)
    SDL_Event e;

    // init RNG based on current time. Only used with the CXNN opcode.
    srand(time(NULL));

    // Set up timers for the CHIP-8's update cycle (~700Hz) and the two timers (60Hz).
    uint32_t last_time = 0, current_time, delta;
    int timer_acc, update_acc;                  // accumulators
    const float timer_rate = 1000.0f / 60.0f;   // Update at 60Hz
    float update_rate = 1000.0f / 700.0f;       // Update at 700Hz

    // Main program loop. The conditional check might not be necessary, I just wanted a convenient way to quit if it reached the end of the program without calling the proper
    while(chip8.PC < 0x1000){
        // Handle input
        while(SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                SDL_Quit();
                exit(0);
            } else if(e.type == SDL_KEYDOWN){
                switch(e.key.keysym.sym){
                    case SDLK_1: chip8.key[0x1] = 1; break;
                    case SDLK_2: chip8.key[0x2] = 1; break;
                    case SDLK_3: chip8.key[0x3] = 1; break;
                    case SDLK_4: chip8.key[0xC] = 1; break;
                    case SDLK_q: chip8.key[0x4] = 1; break;
                    case SDLK_w: chip8.key[0x5] = 1; break;
                    case SDLK_e: chip8.key[0x6] = 1; break;
                    case SDLK_r: chip8.key[0xD] = 1; break;
                    case SDLK_a: chip8.key[0x7] = 1; break;
                    case SDLK_s: chip8.key[0x8] = 1; break;
                    case SDLK_d: chip8.key[0x9] = 1; break;
                    case SDLK_f: chip8.key[0xE] = 1; break;
                    case SDLK_z: chip8.key[0xA] = 1; break;
                    case SDLK_x: chip8.key[0x0] = 1; break;
                    case SDLK_c: chip8.key[0xB] = 1; break;
                    case SDLK_v: chip8.key[0xF] = 1; break;
                }
            } else if(e.type == SDL_KEYUP){
                switch(e.key.keysym.sym){
                    case SDLK_1: chip8.key[0x1] = 0; break;
                    case SDLK_2: chip8.key[0x2] = 0; break;
                    case SDLK_3: chip8.key[0x3] = 0; break;
                    case SDLK_4: chip8.key[0xC] = 0; break;
                    case SDLK_q: chip8.key[0x4] = 0; break;
                    case SDLK_w: chip8.key[0x5] = 0; break;
                    case SDLK_e: chip8.key[0x6] = 0; break;
                    case SDLK_r: chip8.key[0xD] = 0; break;
                    case SDLK_a: chip8.key[0x7] = 0; break;
                    case SDLK_s: chip8.key[0x8] = 0; break;
                    case SDLK_d: chip8.key[0x9] = 0; break;
                    case SDLK_f: chip8.key[0xE] = 0; break;
                    case SDLK_z: chip8.key[0xA] = 0; break;
                    case SDLK_x: chip8.key[0x0] = 0; break;
                    case SDLK_c: chip8.key[0xB] = 0; break;
                    case SDLK_v: chip8.key[0xF] = 0; break;
                }
            }
        }

        // Update timers
        current_time = SDL_GetTicks();
        delta = current_time - last_time;
        last_time = current_time;
        timer_acc += delta;
        update_acc += delta;

        // Handle CHIP-8 sound/delay timers @ 60Hz
        while(timer_acc >= timer_rate){
            if(chip8.delay_timer > 0){
                chip8.delay_timer--;
            }

            if(chip8.sound_timer > 0){
                // There really *should* be some code here to generate a beep sound, but I'm horribly lazy so it just prints a Z instead
                printf("Z");
                chip8.sound_timer--;
            }


            timer_acc -= timer_rate;
        }

        // Update Chip8 and draw to screen
        while(update_acc >= update_rate){
            chip8_run_cycle(&chip8);
            video_draw(&chip8, window, renderer, video_scale);

            update_acc -= update_rate;
        }


    }

    SDL_Quit();
    return 0;
}

void chip8_init(Chip8* chip8){
    chip8->I = 0;
    chip8->PC = 0x200;
    chip8->sp = 0;

    // Clear Memory
    for(int i = 0; i < 0x1000; i++){
        chip8->memory[i] = 0;
    }

    // Clear registers
    for(int i = 0; i < 16; i++){
        chip8->V[i] = 0;
    }

    // Clear screen
    chip8_clear_screen(chip8);

    // Clear stack
    for(int i = 0; i < 16; i++){
        chip8->stack[i] = 0;
    }

    // Clear keys array
    for(int i = 0; i < 16; i++){
        chip8->key[i] = 0;
    }

    // Load font into memory
    uint8_t chip8_font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for(int i = 0; i < 80; i++){
        chip8->memory[i] = chip8_font[i];
    }

    chip8_load_program(chip8, "programs/pong");
}

void chip8_run_cycle(Chip8* chip8){
    // Fetch next opcode (fetches two bytes and merges them)
    uint16_t opcode = chip8->memory[chip8->PC] << 8 | chip8->memory[chip8->PC + 1];

    // Pre-calculate opcode values.
    uint16_t n = (opcode & 0x000F);
    uint16_t nn = (opcode & 0x00FF);
    uint16_t nnn = (opcode & 0x0FFF);
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;

    // Print current system status (Program Counter, Instruction Register, Stack pointer value/position, current values in x/y registers
    printf("Executing 0x%X at %X\tI = 0x%X SP = %d [Vx] = %d [Vy] = %d\n", opcode, chip8->PC, chip8->I, chip8->sp, chip8->V[x], chip8->V[y]);

    // Decode/execute opcode
    switch(opcode & 0xF000){
        case 0x0000:
            switch(nnn){
                // 00E0: Clear the screen
                case 0x00E0: chip8_clear_screen(chip8); chip8->PC += 2; break;

                // 00EE: Return from a subroutine
                case 0x00EE: chip8->PC = chip8->stack[--chip8->sp]; chip8->PC += 2; break;

                default: op_undefined_opcode(chip8, opcode); break;
            }
            break;

        // 1NNN: Jump to address NNN
        case 0x1000: chip8->PC = nnn; break;

        // 2NNN: Execute subroutine starting at address NNN
        case 0x2000: chip8->stack[chip8->sp] = chip8->PC; chip8->sp++; chip8->PC = nnn; break;

        // 3XNN: Skip the following instruction if the value of register VX equals NN
        case 0x3000:
            if(chip8->V[x] == nn){
                chip8->PC += 4;
            } else{
                chip8->PC += 2;
            }
            break;

        // 4XNN: Skip the following instruction if the value of register VX is not equal to NN
        case 0x4000:
            if(chip8->V[x] != nn){
                chip8->PC += 4;
            } else{
                chip8->PC += 2;
            }
            break;

        // 5XY0: Skip the following instruction if the value of register VX is equal to the value of register VY
        case 0x5000:
            switch(n){
                case 0:
                    if(chip8->V[x] == chip8->V[y]){
                        chip8->PC += 4;
                    } else{
                        chip8->PC += 2;
                    }
                    break;
                default: op_undefined_opcode(chip8, opcode); break;
            }
            break;

        // 6XNN: Store number NN in register VX
        case 0x6000: chip8->V[x] = nn;  chip8->PC += 2; break;

        // 0x7XNN: Add the value NN to register VX
        case 0x7000: chip8->V[x] += nn; chip8->PC += 2; break;

        case 0x8000:
            switch(n){
                // 8XY0: Store the value of register VY in register VX
                case 0x0: chip8->V[x] =  chip8->V[y]; chip8->PC += 2; break;

                // 8XY1: Set VX to VX OR VY
                case 0x1: chip8->V[x] |= chip8->V[y]; chip8->PC += 2; break;

                // 8XY2: Set VX to VX AND VY
                case 0x2: chip8->V[x] &= chip8->V[y]; chip8->PC += 2; break;

                // 8XY3: Set VX to VX XOR VY
                case 0x3: chip8->V[x] ^= chip8->V[y]; chip8->PC += 2; break;

                // 8XY4: Add the value of register VY to register VX
                case 0x4:
                    if(chip8->V[x] + chip8->V[y] > 0xFF){
                        // Set VF to 01 if a carry occurs
                        chip8->V[0xF] = 1;
                    } else{
                        // Set VF to 00 if a carry does not occur
                        chip8->V[0xF] = 0;
                    }
                    chip8->V[x] += chip8->V[y];
                    chip8->PC += 2;
                    break;

                // 8XY5: Subtract the value of register VY from register VX
                case 0x5:
                    if(chip8->V[y] > chip8->V[x]){
                         // Set VF to 00 if a borrow occurs
                        chip8->V[0xF] = 0;
                    } else{
                         // Set VF to 01 if a borrow does not occur
                        chip8->V[0xF] = 1;
                    }
                    chip8->V[x] -= chip8->V[y];
                    chip8->PC += 2;
                    break;

                // 8XY6: Store the value of register VY shifted right one bit in register VX
                case 0x6:
                    // Set register VF to the least significant bit prior to the shift
                    chip8->V[0xF] = (chip8->V[y] & 0x0001);

                    chip8->V[x] = chip8->V[y] >>= 1;
                    chip8->PC += 2;
                    break;

                // 8XY7: Set register VX to the value of VY minus VX
                case 0x7:
                    if(chip8->V[x] > chip8->V[y]){
                        // Set VF to 00 if a borrow occurs
                        chip8->V[0xF] = 0;
                    } else{
                        // Set VF to 01 if a borrow does not occur
                        chip8->V[0xF] = 1;
                    }
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->PC += 2;
                    break;

                // 8XYE: Store the value of register VY shifted left one bit in register VX
                case 0xE:
                    // Set register VF to the most significant bit prior to the shift
                    chip8->V[0xF] = (chip8->V[y] >> 3);

                    chip8->V[x] = chip8->V[y] <<= 1;
                    chip8->PC += 2;
                    break;

                default: op_undefined_opcode(chip8, opcode); break;
            }
            break;

        // 9XY0: Skip the following instruction if the value of register VX is not equal to the value of register VY
        case 0x9000:
            switch(n){
                case 0:
                    if(chip8->V[x] != chip8->V[y]){
                        chip8->PC += 4;
                    } else{
                        chip8->PC += 2;
                    }
                    break;
                default: op_undefined_opcode(chip8, opcode); break;
            }
            break;

        // ANNN: Store memory address NNN in register I
        case 0xA000: chip8->I = nnn; chip8->PC += 2; break;

        // BNNN: Jump to address NNN + V0
        case 0xB000: chip8->PC = nnn + chip8->V[0]; break;

        // CXNN: Set VX to a random number with a mask of NN
        case 0xC000: chip8->V[x] = (rand() % (0xFF + 1)) & nn; chip8->PC += 2; break;

        // DXYN: Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
        case 0xD000:
            // reset register F
            chip8->V[0xF] = 0;

            // the height of the sprite is defined by N (n)
            for(int yline = 0; yline < n; yline++){
                uint8_t current_pixel = chip8->memory[chip8->I + yline];
                for(int xline = 0; xline < 8; xline++){
                    if((current_pixel & (0x80 >> xline)) != 0){
                        // Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
                        if(chip8->screen[(chip8->V[x] + xline + ((chip8->V[y] + yline) * CHIP8_SCREEN_WIDTH))] == 1){
                            chip8->V[0xF] = 1;
                        }

                        chip8->screen[chip8->V[x] + xline + ((chip8->V[y] + yline) * CHIP8_SCREEN_WIDTH)] ^= 1;
                    }
                }
            }

            chip8->PC += 2;
            break;

        case 0xE000:
            switch(nn){
                // EX9E: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed
                case 0x9E:
                    if(chip8->key[chip8->V[x]] == 1){
                        chip8->PC += 4;
                    } else{
                        chip8->PC += 2;
                    }
                    break;

                // EXA1: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed
                case 0xA1:
                    if(chip8->key[chip8->V[x]] != 1){
                        chip8->PC += 4;
                    } else{
                        chip8->PC += 2;
                    }
                    break;

                default: op_undefined_opcode(chip8, opcode); break;
            }
            break;

        case 0xF000:
            switch(nn){
                // FX07: Store the current value of the delay timer in register VX
                case 0x07: chip8->V[x] = chip8->delay_timer; chip8->PC += 2; break;

                // FX15: Set the delay timer to the value of register VX
                case 0x15: chip8->delay_timer = chip8->V[x]; chip8->PC += 2; break;

                // FX18: Set the sound timer to the value of register VX
                case 0x18: chip8->sound_timer = chip8->V[x]; chip8->PC += 2; break;

                // FX1E: Add the value stored in register VX to register I
                case 0x1E: chip8->I += chip8->V[x]; chip8->PC += 2; break;

                // FX29: Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
                case 0x29: chip8->I = chip8->V[x]; chip8->PC += 2;  break;

                // FX33: Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2
                case 0x33:
                    chip8->memory[chip8->I] = chip8->V[x] / 100;
                    chip8->memory[chip8->I+1] = (chip8->V[x] / 10) % 10;
                    chip8->memory[chip8->I+2] = chip8->V[x] % 10;
                    chip8->PC += 2;
                    break;

                // FX55: Store the values of registers V0 to VX inclusive in memory starting at address I
                case 0x55:
                    for(uint8_t current_reg = 0; current_reg <= x; current_reg++){
                        chip8->memory[chip8->I + current_reg] = chip8->V[current_reg];
                    }

                    // I is set to I + X + 1 after operation
                    chip8->I += (x + 1);
                    chip8->PC += 2;
                    break;

                // Fill registers V0 to VX inclusive with the values stored in memory starting at address I
                case 0x65:
                    for(uint8_t current_reg = 0; current_reg <= x; current_reg++){
                        chip8->V[current_reg] = chip8->memory[chip8->I + current_reg];
                    }

                    // I is set to I + X + 1 after operation
                    chip8->I += (x + 1);
                    chip8->PC += 2;
                    break;

                default: op_undefined_opcode(chip8, opcode); break;
            }
            break;

        default: op_undefined_opcode(chip8, opcode); break;
    }

}

void chip8_clear_screen(Chip8* chip8){
    for(int i = 0; i < 64 * 32; i++){
        chip8->screen[i] = 0;
    }
}

void chip8_load_program(Chip8* chip8, char* path){
    FILE *f = fopen(path, "rb");

    /*  Program loaded into a buffer of size 0x1000 (size of CHIP-8 memory) minus the first
        0x200 bits which aren't for program use    */
    fread(&chip8->memory[chip8->PC], 1, 0x1000 - 0x200, f);
    fclose(f);
}

void video_draw(Chip8* chip8, SDL_Window* w, SDL_Renderer* r, int scale){
    // Clear display to black
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);

    // Set colour to white to prepare for drawing pixels
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);

    // Draw to window based on data in Chip8 screen array
    for(int y = 0; y < CHIP8_SCREEN_HEIGHT; y++){
        for(int x = 0; x < CHIP8_SCREEN_WIDTH; x++){
            /*  Chip-8 screen array is only one-dimensional, while here we're working in two-dimensions (x and y).
                The equivalent element in the array is found with 'y * CHIP8_SCREEN_WIDTH + x'  */
            if(chip8->screen[y * CHIP8_SCREEN_WIDTH + x] == 1){
                SDL_RenderDrawPoint(r, x, y);
            }
        }
    }

    SDL_RenderPresent(r);
}

void op_undefined_opcode(Chip8* chip8, uint16_t opcode){
    printf("Undefined Opcode: 0x%X\n", opcode);
    chip8->PC += 2;
}
