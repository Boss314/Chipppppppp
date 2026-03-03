#include<stdint.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<time.h>
#include<stdio.h>

#include "chip8.h"


//chip-8's 16 8-bit registers, named V0-VF 
uint8_t V[16];

//12-bit address (index) register, emulated with the lowest 12 bits of a 16 bit integer
uint16_t I;

//12-bit program counter, emulated with the lowest 12 bits of a 16 bit integer
uint16_t PC;

//stack pointer register, CHIP-8 only allows return addresses to get pushed on the stack
uint8_t STACKPOINTER;

//registers of the delay and sound timers
volatile uint8_t DELAYTIMER;
volatile uint8_t SOUNDTIMER;


//chip-8's 16 keyboard buttons, each of them is either pressed or not pressed
volatile bool KEYS[16];

//chip-8's display, each pixel is either on or off
bool DISPLAY[32][64];

//chip-8's 4Kb ram, initialized with font sprites starting at address 0
uint8_t RAM[4096]={ 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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


// definitions to get nibbles for instruction decoding, starting from the most significant nibble
#define NIBBLE1(x) ((uint8_t)(((x) >> 12) & 0b1111))
#define NIBBLE2(x) ((uint8_t)(((x) >> 8) & 0b1111))
#define NIBBLE3(x) ((uint8_t)(((x) >> 4) & 0b1111))
#define NIBBLE4(x) ((uint8_t)((x) & 0b1111))


// the chip-8 interpreter reserves the first 512 bytes of ram, programs should not access memory within here
#define RESERVED_MEMORY 0x200


// definition of maximum stack depth and stack's location in memory
#define STACK_MAX_DEPTH 16 
#define STACK_BASE 0x1ff



bool Display_Sprite(uint8_t x,uint8_t y,uint8_t n);
uint8_t Rand_get();


ErrorCode_t CHIP8_next_instruction(){
    // fetch the next instruction from the location pointed by the program counter
    uint16_t instruction;

    if(NIBBLE1(PC)) return INVALID_ADDRESS_PC;
    if(PC < RESERVED_MEMORY) return RESERVED_ADDRESS_PC;
    instruction = RAM[PC];
    PC=PC+1;

    instruction = instruction << 8;

    if(NIBBLE1(PC)) return INVALID_ADDRESS_PC;
    if(PC < RESERVED_MEMORY) return RESERVED_ADDRESS_PC;
    instruction |= RAM[PC];
    PC=PC+1;

    uint16_t address;
    uint8_t value,vx,vy;



    // decode the type of instruction based on the first nibble and execute
    switch (NIBBLE1(instruction)){
        case 0:
            if(NIBBLE2(instruction)!=0) return INVALID_INSTRUCTION;
            if(NIBBLE3(instruction)!=E) return INVALID_INSTRUCTION;
            if(NIBBLE4(instruction)==0){
                CHIP8_display_clear();
            }else if(NIBBLE4(instruction)==E){
                //Return from a subroutine
                if(STACKPOINTER==0) return STACK_UNDERFLOW;
                STACKPOINTER--;
                PC=((RAM[STACK_BASE-STACKPOINTER*2]) << 8) | (RAM[STACK_BASE-STACKPOINTER*2-1]);
            }else{
                return INVALID_INSTRUCTION;
            }
            
        break;
        case 1:
            //1nnn: Jump to location nnn
            PC=instruction & 0b111111111111; 
        break;
        case 2:
            if(STACKPOINTER==16) return STACK_OVERFLOW;
            //2nnn: Call subroutine at nnn
            address=instruction & 0b111111111111;

            RAM[STACK_BASE-STACKPOINTER*2]=PC >> 8;
            RAM[STACK_BASE-STACKPOINTER*2-1]=PC & 0b11111111;

            STACKPOINTER++;

            PC=address;
        break;
        case 3:
            //3xkk: Skip next instruction if Vx = kk
            value=instruction & 0b11111111;

            if(V[NIBBLE2(instruction)]==value){
                PC=PC+2;
            }
        break;
        case 4:
            //4xkk: Skip next instruction if Vx != kk
            value=instruction & 0b11111111;

            if(V[NIBBLE2(instruction)]!=value){
                PC=PC+2;
            }
        break;
        case 5:
            //5xy0: Skip next instruction if Vx = Vy
            if(NIBBLE4(instruction) != 0) return INVALID_INSTRUCTION;

            if(V[NIBBLE2(instruction)]==V[NIBBLE3(instruction)]){
                PC=PC+2;
            }
        break;
        case 6:
            //6xkk: Set Vx = kk
            value=instruction & 0b11111111;

            V[NIBBLE2(instruction)]=value;
        break;
        case 7:
            //7xkk: Set Vx = Vx + kk
            value=instruction & 0b11111111;

            V[NIBBLE2(instruction)]+=value;
        break;
        case 8:
            //8xy: various operations on registers Vx and Vy
            switch (NIBBLE4(instruction)){
                case 0:
                    // Set Vx = Vy
                    V[NIBBLE2(instruction)]=V[NIBBLE3(instruction)];
                break;
                case 1:
                    // Set Vx = Vx OR Vy
                    V[NIBBLE2(instruction)]|=V[NIBBLE3(instruction)];
                break;
                case 2:
                    // Set Vx = Vx AND Vy
                    V[NIBBLE2(instruction)]&=V[NIBBLE3(instruction)];                
                break;
                case 3:
                    // Set Vx = Vx XOR Vy
                    V[NIBBLE2(instruction)]^=V[NIBBLE3(instruction)];   
                break;
                case 4:
                    // Set Vx = Vx + Vy, set VF = carry
                    uint16_t sum= V[NIBBLE2(instruction)]+V[NIBBLE3(instruction)];

                    V[NIBBLE2(instruction)]=sum & 0b11111111;
                    V[F]=(sum > 255) ? 1 : 0;
                break;
                case 5:
                    // Set Vx = Vx - Vy, set VF = NOT borrow
                    vx=V[NIBBLE2(instruction)];
                    vy=V[NIBBLE3(instruction)];

                    V[F]=(vx >= vy) ? 1 : 0;
                    V[NIBBLE2(instruction)]=vx-vy;
                break;
                case 6:
                    // Set Vx = Vx SHR 1
                    V[F]=V[NIBBLE2(instruction)] & 1;
                    V[NIBBLE2(instruction)]>>=1;
                break;
                case 7:
                    // Set Vx = Vy - Vx, set VF = NOT borrow
                    vx=V[NIBBLE2(instruction)];
                    vy=V[NIBBLE3(instruction)];

                    V[F]=(vy >= vx) ? 1 : 0;
                    V[NIBBLE2(instruction)]=vy-vx;
                break;
                case E:
                    // Set Vx = Vx SHL 1
                    V[F]=V[NIBBLE2(instruction)] >> 7;
                    V[NIBBLE2(instruction)]<<=1;
                break;
                default:
                    return INVALID_INSTRUCTION;
                break;
            }
        break;
        case 9:
            //9xy0: Skip next instruction if Vx != Vy
            if(NIBBLE4(instruction)!=0) return INVALID_INSTRUCTION;

            if(V[NIBBLE2(instruction)]!=V[NIBBLE3(instruction)]){
                PC=PC+2;
            }
        break;
        case A:
            //Annn: Set I = nnn
            I=instruction & 0b111111111111;
        break;
        case B:
            //Bnnn: Jump to location nnn + V0
            address=instruction & 0b111111111111;
            address=address+V[0];

            PC=address;
        break;
        case C:
            //Cxkk: Set Vx = random byte AND kk
            uint8_t mask=instruction & 0b11111111;

            V[NIBBLE2(instruction)]=mask & Rand_get();
        break;
        case D:
            //Dxyn: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
            uint8_t x=V[NIBBLE2(instruction)];
            uint8_t y=V[NIBBLE3(instruction)];
            uint8_t n=NIBBLE4(instruction);

            if(NIBBLE1(I)) return INVALID_ADDRESS_I;
            if(NIBBLE1(I+n)) return INVALID_ADDRESS_I;

            V[F]=Display_Sprite(x,y,n);
        break;
        case E:
            if((instruction & 0b11111111) == 0x9e){
                // Skip next instruction if key with the value of Vx is pressed
                uint8_t button=V[NIBBLE2(instruction)];
                if(button>F) return INVALID_BUTTON;

                if(KEYS[button]) PC=PC+2;
            }else if((instruction & 0b11111111) == 0xa1){
                // Skip next instruction if key with the value of Vx is not pressed
                uint8_t button=V[NIBBLE2(instruction)];
                if(button>F) return INVALID_BUTTON;

                if(!KEYS[button]) PC=PC+2;
            }else{
                return INVALID_INSTRUCTION;
            }
        break;
        case F:
            switch (instruction & 0b11111111){
            case 0x07:
                // Set Vx = delay timer value
                V[NIBBLE2(instruction)]=DELAYTIMER;
            break;
            case 0x0a:
                // Wait for a key press, store the value of the key in Vx
                bool pressed=false;

                while(!pressed){
                    for(int i=0;i<16;i++){
                        if(KEYS[i]){
                            V[NIBBLE2(instruction)]=i;
                            pressed=true;
                            break;
                        }
                    }
                }
            break;
            case 0x15:
                // Set delay timer = Vx
                DELAYTIMER=V[NIBBLE2(instruction)];
            break;
            case 0x18:
                // Set sound timer = Vx
                SOUNDTIMER=V[NIBBLE2(instruction)];
            break;
            case 0x1e:
                // Set I = I + Vx
                address=I+V[NIBBLE2(instruction)];
                
                I=address;
            break;
            case 0x29:
                // Set I = location of sprite for digit Vx
                if(V[NIBBLE2(instruction)]>F) return INVALID_SPRITE;

                I=5*V[NIBBLE2(instruction)];
            break;
            case 0x33:
                // Store BCD representation of Vx in memory locations I, I+1, and I+2
                address=I;

                vx=V[NIBBLE2(instruction)];

                if(NIBBLE1(address)) return INVALID_ADDRESS_I;
                if(address < RESERVED_MEMORY) return RESERVED_ADDRESS_I;
                RAM[address]=vx/100;
                address++;

                if(NIBBLE1(address)) return INVALID_ADDRESS_I;
                if(address < RESERVED_MEMORY) return RESERVED_ADDRESS_I;
                RAM[address]=(vx/10)%10;
                address++;

                if(NIBBLE1(address)) return INVALID_ADDRESS_I;
                if(address < RESERVED_MEMORY) return RESERVED_ADDRESS_I;
                RAM[address]=vx%10;
                address++;
            break;
            case 0x55:
                // Store registers V0 through Vx in memory starting at location I
                for(int i=0;i<=NIBBLE2(instruction);i++){
                    address=I+i;
                    if(NIBBLE1(address)) return INVALID_ADDRESS_I;
                    if(address < RESERVED_MEMORY) return RESERVED_ADDRESS_I;

                    RAM[address]=V[i];
                }
            break;
            case 0x65:
                // Read registers V0 through Vx from memory starting at location I
                for(int i=0;i<=NIBBLE2(instruction);i++){
                    address=I+i;
                    if(NIBBLE1(address)) return INVALID_ADDRESS_I;
                    if(address < RESERVED_MEMORY) return RESERVED_ADDRESS_I;

                    V[i]=RAM[address];
                }
            break;
            default:
                return INVALID_INSTRUCTION;    
            break;
            }
        break;
    }

    return NO_ERROR;
}


void CHIP8_advance_timers(){
    if(DELAYTIMER!=0) DELAYTIMER--;
    if(SOUNDTIMER!=0) SOUNDTIMER--;
}


void CHIP8_display_clear(){
    for(int i=0;i<32;i++){
        for(int j=0;j<64;j++){
            DISPLAY[i][j]=false;
        }
    }
}


bool Display_Sprite(uint8_t x,uint8_t y,uint8_t n){
    bool collision=false;

    x=x%64;
    y=y%32;

    for(int i=0;i<n;i++){
        uint8_t b=RAM[I+i];

        for(int j=0;j<8;j++){
            if(y+i<32 && x+j<64){
                if((b << j) & 0b10000000){
                    collision=collision || DISPLAY[y+i][x+j];
                    DISPLAY[y+i][x+j]= !DISPLAY[y+i][x+j];
                }
            }
        }
    }

    return collision;
}

//variable for the state of the rng used for the random instruction
int r=0;
uint8_t Rand_get(){
    if(r == 0){
        srand(time(NULL));
    }

    r=rand();

    return (uint8_t)r;
}