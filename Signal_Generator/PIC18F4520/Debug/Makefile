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
#  C:/Users/tanoo/OneDrive/Documents/GitHub/Embedded_software_code/Signal_Generator/PIC18F4520/Debug/Debug.cof: TOOL=LD
#     C:/Users/tanoo/OneDrive/Documents/GitHub/Embedded_software_code/Signal_Generator/PIC18F4520/Debug/main.o: TOOL=CC
#        C:/Users/tanoo/OneDrive/Documents/GitHub/Embedded_software_code/Signal_Generator/PIC18F4520/main.c

# Build tree:
all:	Debug

Debug:	Debug.cof

Debug.cof:	main.o 
	$(LD) $(LDFLAGS) 18f4520_g.lkr -o"Debug.cof" "main.o"

main.o:	../main.c 
	$(CC) $(CCFLAGS)  -fo="main.o" "../main.c"

# tidy - delete all temporary files which are not involved in the target generation
tidy:

# cleanup - delete all generated files
clean:	tidy
	rm -rf Debug.cof
	rm -rf main.o

