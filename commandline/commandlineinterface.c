#include<stdio.h>
#include<stdbool.h>
#include<stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#include "../emulator/chip8.h"

extern uint8_t V[16];
extern uint16_t I;
extern uint16_t PC;
extern uint8_t STACKPOINTER;
extern volatile uint8_t DELAYTIMER;
extern volatile uint8_t SOUNDTIMER;
extern volatile bool KEYS[16];
extern bool DISPLAY[32][64];
extern uint8_t RAM[4096];

int main(int argc, char **argv){
    if(argc<2){
        printf("must include a rom file to load into memory\n");
        return 1;
    }

    int fd=open(argv[1],O_RDONLY);

    if(fd<0){
        printf("%s\n",strerror(errno));
        printf("failed to open rom file %s\n",argv[1]);
        return 2;
    }

    struct stat stats;
    if(stat(argv[1],&stats)==-1){
        printf("%s\n",strerror(errno));
        printf("failed to get stats for %s\n",argv[1]);
        return 3;
    }

    printf("reading %ld bytes from ROM file %s and loading them into RAM...\n",stats.st_size,argv[1]);

    uint8_t *rom_bytes=malloc(stats.st_size);
    

    if(read(fd,rom_bytes,stats.st_size)==-1){
        printf("%s\n",strerror(errno));
        printf("failed to read %ld bytes from %s with fd=%d\n",stats.st_size,argv[1],fd);
        return 4;
    }

    for(int i=0;i<stats.st_size;i++){
        RAM[0x200+i]=rom_bytes[i];
    }
    printf("sucessfully loaded program data into RAM\n");

    free(rom_bytes);
    close(fd);

    printf("initializing registers...\n");
    memset(V,0,16);
    I=0;
    PC=0x200;
    STACKPOINTER=0;
    DELAYTIMER=0;
    SOUNDTIMER=0;
    memset((void*)KEYS,false,16);
    memset(&DISPLAY,false,sizeof(bool)*32*64);
    CHIP8_display_clear();
    printf("done\n\n");

    
    int selection=1;

    while (selection!=0){

        printf("1: advance instruction\n2: print display\n3: dump registers\n4: dump memory\n5: press/unpress key\n6: advance timers\n");

        scanf("%d",&selection);

        switch (selection){
        case 1:
            printf("executing instruction %02X%02X\n\n",RAM[PC],RAM[PC+1]);

            ErrorCode_t code=CHIP8_next_instruction();

            switch (code){
                case INVALID_INSTRUCTION:
                    printf("ERROR: INVALID INSTRUCTION\n\n");
                break;
                case INVALID_ADDRESS_PC:
                    printf("ERROR: INVALID ADDRESS PC\n\n");
                break;
                case RESERVED_ADDRESS_PC:
                    printf("ERROR: RESERVED ADDRESS PC\n\n");
                break;
                case INVALID_ADDRESS_I:
                    printf("ERROR: INVALID ADDRESS I\n\n");
                break;
                case RESERVED_ADDRESS_I:
                    printf("ERROR: RESERVED ADDRESS I\n\n");
                break;
                case STACK_OVERFLOW:
                    printf("ERROR: STACK OVERFLOW\n\n");
                break;
                case STACK_UNDERFLOW:
                    printf("ERROR: STACK UNDERFLOW\n\n");
                break;
                case INVALID_BUTTON:
                    printf("ERROR: INVALID BUTTON\n\n");
                break;
                case INVALID_SPRITE:
                    printf("ERROR: INVALID SPRITE\n\n");
                break;
            }
        break;
        case 2:
            printf(" ________________________________________________________________ \n");
            for(int i=0;i<32;i++){
                printf("|");
                for(int j=0;j<64;j++){
                    if(DISPLAY[i][j]){
                        printf("X");
                    }else{
                        printf(" ");
                    }
                }
                printf("|\n");
            }
            printf(" ________________________________________________________________ \n\n");
        break;
        case 3:
            printf("V= ");
            for(int i=0;i<16;i++){
                printf("%d ",V[i]);
            }
            printf("\nI=%03X\n",I);
            printf("PC=%03X\n",PC);
            printf("STACKPOINTER=%d\n",STACKPOINTER);
            printf("DELAYTIMER=%d\n",DELAYTIMER);
            printf("SOUNDTIMER=%d\n",SOUNDTIMER);
            printf("KEYS= ");
            for(int i=0;i<16;i++){
                printf("%d ",KEYS[i]);
            }
            printf("\n\n");
        break;
        case 4:
            int start,end;

            printf("start: "); scanf("%x",&start);
            printf("end: "); scanf("%x",&end);

            for(int i=start;i<=end;i++){
                printf("0x%03X  %02X\n",i,RAM[i]);
            }
            printf("\n");
        break;
        case 5:
            int button;
            printf("select button "); 
            scanf("%d",&button);

            KEYS[button]=!KEYS[button];
            printf("\n\n");
        break;
        case 6:
            CHIP8_advance_timers();
        break;
        }

    }
}