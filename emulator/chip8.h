#include<stdint.h>
#include<stdbool.h>

#ifndef CHIP8
#define CHIP8 

// defines to access registers cleanly while also having them as an array, access VA with V[A]
#define A 0xa
#define B 0xb
#define C 0xc
#define D 0xd
#define E 0xe
#define F 0xf



extern uint8_t V[16];
extern uint16_t I;
extern uint16_t PC;
extern uint8_t STACKPOINTER;
volatile extern uint8_t DELAYTIMER;
volatile extern uint8_t SOUNDTIMER;
volatile extern bool KEYS[16];
extern bool DISPLAY[32][64];
extern uint8_t RAM[4096];





// enum for error codes returned by the emulator in case of problems
typedef enum{
    NO_ERROR,                   // no error, everything is fine, starts at 0 for easy checking in conditionals
    INVALID_INSTRUCTION,        // the data at the program counter is not a valid instruction
    INVALID_ADDRESS_PC,         // tried to access an address outside the ram's 4096 bytes with the program counter
    RESERVED_ADDRESS_PC,        // tried to access an address inside the interpreter's reserved section with the program counter
    INVALID_ADDRESS_I,          // tried to access an address outside the ram's 4096 bytes with the I register
    RESERVED_ADDRESS_I,         // tried to access an address inside the interpreter's reserved section with the program I register
    STACK_OVERFLOW,             // tried to go beyond the maximum call stack depth
    STACK_UNDERFLOW,            // tried to return from a function but call stack was empty
    INVALID_BUTTON,             // tried to access a button other than 0-F
    INVALID_SPRITE              // tried to acces a font sprite beyond 0-9
} ErrorCode_t;


ErrorCode_t CHIP8_next_instruction();
void CHIP8_advance_timers();
void CHIP8_display_clear();


#endif