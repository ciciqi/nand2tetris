// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

// n = 256 * 32
// while True:
//     val = KBD
//     KBD = 0
//     if val != 0:
//         i = 0
//         while i < n:
//             RAM[SCREEN + i] = -1
//             i += 1
//     else:
//         i = 0
//         while i < n:
//             RAM[SCREEN + i] = 0
//             i += 1

@8192
D=A
@n
M=D

(LOOP)
    @KBD
    D=M
    M=0
    @WHITE
    D;JEQ

    @i
    M=0
(BLACK_LOOP)
    @i
    D=M
    @n
    D=D-M
    @LOOP
    D;JEQ
    @SCREEN
    D=A
    @i
    A=D+M
    M=-1
    @i
    M=M+1
    @BLACK_LOOP
    0;JMP

(WHITE)
    @i
    M=0
(WHITE_LOOP)
    @i
    D=M
    @n
    D=D-M
    @LOOP
    D;JEQ
    @SCREEN
    D=A
    @i
    A=D+M
    M=0
    @i
    M=M+1
    @WHITE_LOOP
    0;JMP
