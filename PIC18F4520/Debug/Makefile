###############################################################################
# Makefile for project 
###############################################################################

## General options
PROJECT = 
MCU = PIC18F4520
COMPILER = "MPLAB C18"
TARGET = Debug
SHELL = C:\Windows\system32\cmd.exe

## Tools general options
CCFLAGS=-I="C:\Program Files (x86)\Microchip\mplabc18\v3.47\bin\..\h" -p18f4520
LDFLAGS=-u_CRUNTIME -l"C:\Program Files (x86)\Microchip\mplabc18\v3.47\bin\..\lib" -x -p18f4520

## Processing Tools
CC=mcc18.exe
ASM=c:/mcc18/mpasm/MPASMWIN.exe
LD=mplink.exe
LIB=mplib.exe
#  D:/Electronics/Proteous_Project_Files/PIC18F4520/Debug/Debug.cof: TOOL=LD
#     D:/Electronics/Proteous_Project_Files/PIC18F4520/Debug/main.o: TOOL=CC
#        D:/Electronics/Proteous_Project_Files/PIC18F4520/main.c
#     D:/Electronics/Proteous_Project_Files/PIC18F4520/Debug/lcd.o: TOOL=CC
#        D:/Electronics/Proteous_Project_Files/PIC18F4520/lcd.c

# Build tree:
all:	Debug

Debug:	Debug.cof

Debug.cof:	main.o lcd.o 
	$(LD) $(LDFLAGS) 18f4520_g.lkr -o"Debug.cof" "main.o" "lcd.o"

main.o:	../main.c 
	$(CC) $(CCFLAGS)  -fo="main.o" "../main.c"

lcd.o:	../lcd.c 
	$(CC) $(CCFLAGS)  -fo="lcd.o" "../lcd.c"

# tidy - delete all temporary files which are not involved in the target generation
tidy:

# cleanup - delete all generated files
clean:	tidy
	rm -rf Debug.cof
	rm -rf main.o
	rm -rf lcd.o

