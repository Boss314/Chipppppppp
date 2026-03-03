main:
	gcc -o chipppppppp.out -O3 controlpanel/controlpanel.c emulator/chip8.c emulator/chip8.h -lraylib

cli:
	gcc -o cli.out commandline/commandlineinterface.c emulator/chip8.c emulator/chip8.h

clean:
	