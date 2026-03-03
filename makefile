CC ?= cc
CFLAGS ?= -O3 -Wall -Wno-unused-result -Werror -Wno-error=unused-result

main:
	$(CC) -o chipppppppp.out $(CFLAGS) controlpanel/controlpanel.c emulator/chip8.c emulator/chip8.h -lraylib

cli:
	$(CC) -o cli.out $(CFLAGS) commandline/commandlineinterface.c emulator/chip8.c emulator/chip8.h

clean:
	rm -f chipppppppp.out
	rm -f cli.out

controls:
	@echo "GUI controls:"
	@echo "  1: 		  CHIP8 1 button"
	@echo "  2: 		  CHIP8 2 button"
	@echo "  3: 		  CHIP8 3 button"
	@echo "  4: 		  CHIP8 C button"
	@echo "  Q: 		  CHIP8 4 button"
	@echo "  W: 		  CHIP8 5 button"
	@echo "  E: 		  CHIP8 6 button"
	@echo "  R: 		  CHIP8 D button"
	@echo "  A: 		  CHIP8 7 button"
	@echo "  S: 		  CHIP8 8 button"
	@echo "  D: 		  CHIP8 9 button"
	@echo "  F: 		  CHIP8 E button"
	@echo "  Z: 	   	  CHIP8 A button"
	@echo "  X: 	   	  CHIP8 0 button"
	@echo "  C: 	   	  CHIP8 B button"
	@echo "  V: 	   	  CHIP8 F button"
	@echo "  SPACE: 	  Pause/Unpause"
	@echo "  MOUSE LEFT:  Execute 1 instruction"
	@echo "  MOUSE RIGHT: Advance timers"
	@echo "  KEYPAD +: 	  Increase emulator speed"
	@echo "  KEYPAD -: 	  Decrease emulator speed"
help:
	@echo "Available targets:"
	@echo "  main      	  Build Chipppppppp with GUI"
	@echo "  cli       	  Build Chipppppppp with command line interface"
	@echo "  clean     	  Delete .out files if they exist"
	@echo "  controls  	  Show controls in the GUI"
	@echo "  help      	  Show this message"