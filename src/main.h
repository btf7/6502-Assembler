#pragma once

#include <stdbool.h>
#include <stdint.h>

enum instructions {label, constant, byte, word, org,
                   adc, and, asl, bcc, bcs, beq, bit, bmi, bne, bpl, brk, bvc, bvs, clc,
                   cld, cli, clv, cmp, cpx, cpy, dec, dex, dey, eor, inc, inx, iny, jmp,
                   jsr, lda, ldx, ldy, lsr, nop, ora, pha, php, pla, plp, rol, ror, rti,
                   rts, sbc, sec, sed, sei, sta, stx, sty, tax, tay, tsx, txa, txs, tya};
// Note that relative addressing in assembly is identical to absolute, plus it's always the only option so it can be detected from the instruction
enum argAddressingMode {accumulator, implied, immediate, zeroPage, zeroPageX, zeroPageY, relative, absolute, absoluteX, absoluteY, indirect, indirectX, indirectY};

struct line {
    char* instruction;
    char* args; // All whitespace sections should be replaced with a single space
};

struct lineArr {
    size_t len;
    struct line* arr;
};

struct number {
    uint16_t value;
    bool twoBytes;
    uint8_t charsRead;
    bool valueKnown;
};

struct constant {
    char* name;
    bool valueKnown;
    uint16_t value;
    bool twoBytes;
};

struct arg {
    enum argAddressingMode addressingMode;
    uint16_t value;
    bool valueKnown;
};

struct unknownValueArg {
    uint16_t index;
    size_t lineIndex;
    size_t offset;
};

struct lineArr readAsmFile(const char* fileName);
enum instructions identifyInstruction(const char* text);
void punchInstruction(enum instructions instruction, struct arg arg, uint8_t* bin, uint16_t* indexp);
bool is6502Instruction(enum instructions instruction);
uint8_t hexCharToInt(char c);
struct number parseNumber(const char* text);
struct number parseExpression(const char* text, size_t expressionLen, uint16_t index, const struct constant* constants, size_t constantCount);
struct arg parseArgument(const char* text, uint16_t index, const struct constant* constants, size_t constantCount);