#pragma once

#include <stdbool.h>
#include <stdint.h>

enum instructions {adc, and, asl, bcc, bcs, beq, bit, bmi, bne, bpl, brk, bvc, bvs, clc,
                   cld, cli, clv, cmp, cpx, cpy, dec, dex, dey, eor, inc, inx, iny, jmp,
                   jsr, lda, ldx, ldy, lsr, nop, ora, pha, php, pla, plp, rol, ror, rti,
                   rts, sbc, sec, sed, sei, sta, stx, sty, tax, tay, tsx, txa, txs, tya,
                   label, constant, byte, word, org};
// Note that relative addressing in assembly is identical to absolute, plus it's always the only option so it can be detected from the instruction
enum argAddressingMode {accumulator, implied, immediate, zeroPage, zeroPageX, zeroPageY, relative, absolute, absoluteX, absoluteY, indirect, indirectX, indirectY};

static const char instructionNames[56][4] = {
    "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS", "CLC",
    "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP",
    "JSR", "LDA", "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", "ROR", "RTI",
    "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY", "TAX", "TAY", "TSX", "TXA", "TXS", "TYA"
};

static const char addressingModeNames[13][12] = {
    "accumulator", "implied", "immediate", "zero page", "zero page x", "zero page y", "relative", "absolute", "absolute x", "absolute y", "indirect", "indirect x", "indirect y"
};

struct tokenArr {
    size_t len;
    char** arr;
    size_t rawLineNumber;
};

struct lineArr {
    size_t len;
    struct tokenArr* arr;
};

struct numberValue {
    uint16_t value;
    bool twoBytes;
    uint8_t charsRead;
};

struct expressionValue {
    uint16_t value;
    bool twoBytes;
    bool valueKnown;
};

struct constant {
    char* name;
    bool valueKnown;
    bool twoBytes;
    uint16_t value;
};

struct constantArr {
    size_t len;
    struct constant* arr;
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

struct unknownValueArgArr {
    size_t len;
    struct unknownValueArg* arr;
};

struct lineArr readAsmFile(const char* fileName);
void assemble(struct lineArr lines, struct constantArr constants, uint8_t* bin, struct unknownValueArgArr* unknownValueArgs);
void resolveLabels(struct lineArr lines, struct constantArr constants, struct unknownValueArgArr unknownValueArgs, uint8_t* bin);
void* safeMalloc(size_t size);
void* safeRealloc(void* pointer, size_t size);
void* expandDynamicArr(void* arr, size_t* malloced, size_t elemSize);
void strcpyupper(char* dest, const char* source);

enum instructions identifyInstruction(const char* text, size_t lineNumber);
void punchInstruction(enum instructions instruction, struct arg arg, uint8_t* bin, uint16_t* indexp, size_t lineNumber);
bool is6502Instruction(enum instructions instruction);
char* get6502InstructionName(enum instructions instruction);

struct constantArr parseConstants(struct lineArr lines);
uint8_t hexCharToInt(char c);
struct numberValue parseNumber(const char* text, size_t lineNumber);
struct expressionValue parseExpression(const char* text, size_t expressionLen, uint16_t index, struct constantArr constants, size_t lineNumber);
struct arg parseArgument(const char* text, uint16_t index, struct constantArr constants, size_t lineNumber);