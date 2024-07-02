#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum instructions identifyInstruction(const char * const text) {
    if (text[0] == '.') {
        // Pseudo_op

        if (strcmp(".ORG", text) == 0) {
            return org;
        } else if (strcmp(".BYTE", text) == 0) {
            return byte;
        } else if (strcmp(".WORD", text) == 0) {
            return word;
        } else if (strcmp(".DEF", text) == 0) {
            return constant;
        }

    } else if (text[strlen(text) - 1] == ':') {
        // Label

        return label;
    } else {
        // Op

        if (strcmp("ASL", text) == 0) {
            return asl;
        } else if (strcmp("BEQ", text) == 0) {
            return beq;
        } else if (strcmp("INX", text) == 0) {
            return inx;
        } else if (strcmp("JMP", text) == 0) {
            return jmp;
        } else if (strcmp("LDA", text) == 0) {
            return lda;
        } else if (strcmp("STX", text) == 0) {
            return stx;
        }
    }

    printf("Invalid token: %s\n", text);
    exit(EXIT_FAILURE);
}

void punchInstruction(const enum instructions instruction, const struct arg arg, uint8_t * const bin, uint16_t * const indexp) {
    switch (instruction) {
        case asl:
        switch (arg.addressingMode) {
            case accumulator: bin[*indexp] = 0x0a; break;
            case zeroPage: bin[*indexp] = 0x06; break;
            case zeroPageX: bin[*indexp] = 0x16; break;
            case absolute: bin[*indexp] = 0x0e; break;
            case absoluteX: bin[*indexp] = 0x1e; break;
            default:
            printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
            exit(EXIT_FAILURE);
        }
        break;
        
        case beq:
        bin[*indexp] = 0xf0;
        break;

        case inx:
        bin[*indexp] = 0xe8;
        break;

        case jmp:
        switch (arg.addressingMode) {
            case absolute: bin[*indexp] = 0x4c; break;
            case indirect: bin[*indexp] = 0x6c; break;
            default:
            printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
            exit(EXIT_FAILURE);
        }
        break;

        case lda:
        switch (arg.addressingMode) {
            case immediate: bin[*indexp] = 0xa9; break;
            case zeroPage: bin[*indexp] = 0xa5; break;
            case zeroPageX: bin[*indexp] = 0xb5; break;
            case absolute: bin[*indexp] = 0xad; break;
            case absoluteX: bin[*indexp] = 0xbd; break;
            case absoluteY: bin[*indexp] = 0xb9; break;
            case indirectX: bin[*indexp] = 0xa1; break;
            case indirectY: bin[*indexp] = 0xb1; break;
            default:
            printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
            exit(EXIT_FAILURE);
        }
        break;

        case stx:
        switch (arg.addressingMode) {
            case zeroPage: bin[*indexp] = 0x86; break;
            case zeroPageY: bin[*indexp] = 0x96; break;
            case absolute: bin[*indexp] = 0x8e; break;
            default:
            printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
            exit(EXIT_FAILURE);
        }
        break;

        default:
        printf("Couldn't identify instruction to punch: instruction=%d arg.addressingMode=%d arg.value=%d\n", instruction, arg.addressingMode, arg.value);
        exit(EXIT_FAILURE);
    }
    (*indexp)++;

    switch (arg.addressingMode) {
        case accumulator:
        case implied:
        break;

        case absolute:
        case absoluteX:
        case absoluteY:
        case indirect:
        // Value should be punched in little endian
        bin[*indexp] = (uint8_t)(arg.value & 0xff);
        (*indexp)++;
        bin[*indexp] = (uint8_t)((arg.value & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
        (*indexp)++;
        break;

        case relative:
        (*indexp)++;
        int32_t dist = arg.value - *indexp;
        if (dist < -126 || dist > 129) {
            printf("Branch distance too far: Must be -126 to +129, given %d: Branch from 0x%x to 0x%x\n", dist, *indexp, arg.value);
            exit(EXIT_FAILURE);
        }
        bin[*indexp - 1] = (int8_t)dist;
        break;

        default:
        bin[*indexp] = (uint8_t)arg.value;
        (*indexp)++;
        break;
    }
}

// Return false if it's a label or pseudo op, true if it's an actual 6502 instruction
bool is6502Instruction(const enum instructions instruction) {
    switch (instruction) {
        case label:
        case constant:
        case org:
        case byte:
        case word:
        return false;

        default:
        return true;
    }
}