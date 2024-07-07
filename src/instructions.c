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

             if (strcmp("ADC", text) == 0) {return adc;}
        else if (strcmp("AND", text) == 0) {return and;}
        else if (strcmp("ASL", text) == 0) {return asl;}
        else if (strcmp("BCC", text) == 0) {return bcc;}
        else if (strcmp("BCS", text) == 0) {return bcs;}
        else if (strcmp("BEQ", text) == 0) {return beq;}
        else if (strcmp("BIT", text) == 0) {return bit;}
        else if (strcmp("BMI", text) == 0) {return bmi;}
        else if (strcmp("BNE", text) == 0) {return bne;}
        else if (strcmp("BPL", text) == 0) {return bpl;}
        else if (strcmp("BRK", text) == 0) {return brk;}
        else if (strcmp("BVC", text) == 0) {return bvc;}
        else if (strcmp("BVS", text) == 0) {return bvs;}
        else if (strcmp("CLC", text) == 0) {return clc;}
        else if (strcmp("CLD", text) == 0) {return cld;}
        else if (strcmp("CLI", text) == 0) {return cli;}
        else if (strcmp("CLV", text) == 0) {return clv;}
        else if (strcmp("CMP", text) == 0) {return cmp;}
        else if (strcmp("CPX", text) == 0) {return cpx;}
        else if (strcmp("CPY", text) == 0) {return cpy;}
        else if (strcmp("DEC", text) == 0) {return dec;}
        else if (strcmp("DEX", text) == 0) {return dex;}
        else if (strcmp("DEY", text) == 0) {return dey;}
        else if (strcmp("EOR", text) == 0) {return eor;}
        else if (strcmp("INC", text) == 0) {return inc;}
        else if (strcmp("INX", text) == 0) {return inx;}
        else if (strcmp("INY", text) == 0) {return iny;}
        else if (strcmp("JMP", text) == 0) {return jmp;}
        else if (strcmp("JSR", text) == 0) {return jsr;}
        else if (strcmp("LDA", text) == 0) {return lda;}
        else if (strcmp("LDX", text) == 0) {return ldx;}
        else if (strcmp("LDY", text) == 0) {return ldy;}
        else if (strcmp("LSR", text) == 0) {return lsr;}
        else if (strcmp("NOP", text) == 0) {return nop;}
        else if (strcmp("ORA", text) == 0) {return ora;}
        else if (strcmp("PHA", text) == 0) {return pha;}
        else if (strcmp("PHP", text) == 0) {return php;}
        else if (strcmp("PLA", text) == 0) {return pla;}
        else if (strcmp("PLP", text) == 0) {return plp;}
        else if (strcmp("ROL", text) == 0) {return rol;}
        else if (strcmp("ROR", text) == 0) {return ror;}
        else if (strcmp("RTI", text) == 0) {return rti;}
        else if (strcmp("RTS", text) == 0) {return rts;}
        else if (strcmp("SBC", text) == 0) {return sbc;}
        else if (strcmp("SEC", text) == 0) {return sec;}
        else if (strcmp("SED", text) == 0) {return sed;}
        else if (strcmp("SEI", text) == 0) {return sei;}
        else if (strcmp("STA", text) == 0) {return sta;}
        else if (strcmp("STX", text) == 0) {return stx;}
        else if (strcmp("STY", text) == 0) {return sty;}
        else if (strcmp("TAX", text) == 0) {return tax;}
        else if (strcmp("TAY", text) == 0) {return tay;}
        else if (strcmp("TSX", text) == 0) {return tsx;}
        else if (strcmp("TXA", text) == 0) {return txa;}
        else if (strcmp("TXS", text) == 0) {return txs;}
        else if (strcmp("TYA", text) == 0) {return tya;}
    }

    printf("Invalid token: %s\n", text);
    exit(EXIT_FAILURE);
}

void punchInstruction(const enum instructions instruction, const struct arg arg, uint8_t * const bin, uint16_t * const indexp) {
    uint8_t opcodes[61][13];
    memset(opcodes, 0xff, sizeof opcodes); // 0xff isn't an valid opcode, use it to mark an invalid addressing mode

    opcodes[adc][immediate]   = 0x69;
    opcodes[adc][zeroPage]    = 0x65;
    opcodes[adc][zeroPageX]   = 0x75;
    opcodes[adc][absolute]    = 0x6d;
    opcodes[adc][absoluteX]   = 0x7d;
    opcodes[adc][absoluteY]   = 0x79;
    opcodes[adc][indirectX]   = 0x61;
    opcodes[adc][indirectY]   = 0x71;

    opcodes[and][immediate]   = 0x29;
    opcodes[and][zeroPage]    = 0x25;
    opcodes[and][zeroPageX]   = 0x35;
    opcodes[and][absolute]    = 0x2d;
    opcodes[and][absoluteX]   = 0x3d;
    opcodes[and][absoluteY]   = 0x39;
    opcodes[and][indirectX]   = 0x21;
    opcodes[and][indirectY]   = 0x31;

    opcodes[asl][accumulator] = 0x0a;
    opcodes[asl][zeroPage]    = 0x06;
    opcodes[asl][zeroPageX]   = 0x16;
    opcodes[asl][absolute]    = 0x0e;
    opcodes[asl][absoluteX]   = 0x1e;

    opcodes[bcc][relative]    = 0x90;

    opcodes[bcs][relative]    = 0xb0;

    opcodes[beq][relative]    = 0xf0;

    opcodes[bit][zeroPage]    = 0x24;
    opcodes[bit][absolute]    = 0x2c;

    opcodes[bmi][relative]    = 0x30;

    opcodes[bne][relative]    = 0xd0;

    opcodes[bpl][relative]    = 0x10;

    opcodes[brk][implied]     = 0x00;

    opcodes[bvc][relative]    = 0x50;

    opcodes[bvs][relative]    = 0x70;

    opcodes[clc][implied]     = 0x18;

    opcodes[cld][implied]     = 0xd8;

    opcodes[cli][implied]     = 0x58;

    opcodes[clv][implied]     = 0xb8;

    opcodes[cmp][immediate]   = 0xc9;
    opcodes[cmp][zeroPage]    = 0xc5;
    opcodes[cmp][zeroPageX]   = 0xd5;
    opcodes[cmp][absolute]    = 0xcd;
    opcodes[cmp][absoluteX]   = 0xdd;
    opcodes[cmp][absoluteY]   = 0xd9;
    opcodes[cmp][indirectX]   = 0xc1;
    opcodes[cmp][indirectY]   = 0xd1;

    opcodes[cpx][immediate]   = 0xe0;
    opcodes[cpx][zeroPage]    = 0xe4;
    opcodes[cpx][absolute]    = 0xec;

    opcodes[cpy][immediate]   = 0xc0;
    opcodes[cpy][zeroPage]    = 0xc4;
    opcodes[cpy][absolute]    = 0xcc;

    opcodes[dec][zeroPage]    = 0xc6;
    opcodes[dec][zeroPageX]   = 0xd6;
    opcodes[dec][absolute]    = 0xce;
    opcodes[dec][absoluteX]   = 0xde;

    opcodes[dex][implied]     = 0xca;

    opcodes[dey][implied]     = 0x88;

    opcodes[eor][immediate]   = 0x49;
    opcodes[eor][zeroPage]    = 0x45;
    opcodes[eor][zeroPageX]   = 0x55;
    opcodes[eor][absolute]    = 0x4d;
    opcodes[eor][absoluteX]   = 0x5d;
    opcodes[eor][absoluteY]   = 0x59;
    opcodes[eor][indirectX]   = 0x41;
    opcodes[eor][indirectY]   = 0x51;

    opcodes[inc][zeroPage]    = 0xe6;
    opcodes[inc][zeroPageX]   = 0xf6;
    opcodes[inc][absolute]    = 0xee;
    opcodes[inc][absoluteX]   = 0xfe;

    opcodes[inx][implied]     = 0xe8;

    opcodes[iny][implied]     = 0xc8;

    opcodes[jmp][absolute]    = 0x4c;
    opcodes[jmp][indirect]    = 0x6c;

    opcodes[jsr][absolute]    = 0x20;

    opcodes[lda][immediate]   = 0xa9;
    opcodes[lda][zeroPage]    = 0xa5;
    opcodes[lda][zeroPageX]   = 0xb5;
    opcodes[lda][absolute]    = 0xad;
    opcodes[lda][absoluteX]   = 0xbd;
    opcodes[lda][absoluteY]   = 0xb9;
    opcodes[lda][indirectX]   = 0xa1;
    opcodes[lda][indirectY]   = 0xb1;

    opcodes[ldx][immediate]   = 0xa2;
    opcodes[ldx][zeroPage]    = 0xa6;
    opcodes[ldx][zeroPageY]   = 0xb6;
    opcodes[ldx][absolute]    = 0xae;
    opcodes[ldx][absoluteY]   = 0xbe;

    opcodes[ldy][immediate]   = 0xa0;
    opcodes[ldy][zeroPage]    = 0xa4;
    opcodes[ldy][zeroPageX]   = 0xb4;
    opcodes[ldy][absolute]    = 0xac;
    opcodes[ldy][absoluteX]   = 0xbc;

    opcodes[lsr][accumulator] = 0x4a;
    opcodes[lsr][zeroPage]    = 0x46;
    opcodes[lsr][zeroPageX]   = 0x56;
    opcodes[lsr][absolute]    = 0x4e;
    opcodes[lsr][absoluteX]   = 0x5e;

    opcodes[nop][implied]     = 0xea;

    opcodes[ora][immediate]   = 0x09;
    opcodes[ora][zeroPage]    = 0x05;
    opcodes[ora][zeroPageX]   = 0x15;
    opcodes[ora][absolute]    = 0x0d;
    opcodes[ora][absoluteX]   = 0x1d;
    opcodes[ora][absoluteY]   = 0x19;
    opcodes[ora][indirectX]   = 0x01;
    opcodes[ora][indirectY]   = 0x11;

    opcodes[pha][implied]     = 0x48;

    opcodes[php][implied]     = 0x08;

    opcodes[pla][implied]     = 0x68;

    opcodes[plp][implied]     = 0x28;

    opcodes[rol][accumulator] = 0x2a;
    opcodes[rol][zeroPage]    = 0x26;
    opcodes[rol][zeroPageX]   = 0x36;
    opcodes[rol][absolute]    = 0x2e;
    opcodes[rol][absoluteX]   = 0x3e;

    opcodes[ror][accumulator] = 0x6a;
    opcodes[ror][zeroPage]    = 0x66;
    opcodes[ror][zeroPageX]   = 0x76;
    opcodes[ror][absolute]    = 0x6e;
    opcodes[ror][absoluteX]   = 0x7e;

    opcodes[rti][implied]     = 0x40;

    opcodes[rts][implied]     = 0x60;

    opcodes[sbc][immediate]   = 0xe9;
    opcodes[sbc][zeroPage]    = 0xe5;
    opcodes[sbc][zeroPageX]   = 0xf5;
    opcodes[sbc][absolute]    = 0xed;
    opcodes[sbc][absoluteX]   = 0xfd;
    opcodes[sbc][absoluteY]   = 0xf9;
    opcodes[sbc][indirectX]   = 0xe1;
    opcodes[sbc][indirectY]   = 0xf1;

    opcodes[sec][implied]     = 0x38;

    opcodes[sed][implied]     = 0xf8;

    opcodes[sei][implied]     = 0x78;

    opcodes[sta][zeroPage]    = 0x85;
    opcodes[sta][zeroPageX]   = 0x95;
    opcodes[sta][absolute]    = 0x8d;
    opcodes[sta][absoluteX]   = 0x9d;
    opcodes[sta][absoluteY]   = 0x99;
    opcodes[sta][indirectX]   = 0x81;
    opcodes[sta][indirectY]   = 0x91;

    opcodes[stx][zeroPage]    = 0x86;
    opcodes[stx][zeroPageY]   = 0x96;
    opcodes[stx][absolute]    = 0x8e;

    opcodes[sty][zeroPage]    = 0x84;
    opcodes[sty][zeroPageX]   = 0x94;
    opcodes[sty][absolute]    = 0x8c;

    opcodes[tax][implied]     = 0xaa;

    opcodes[tay][implied]     = 0xa8;

    opcodes[tsx][implied]     = 0xba;

    opcodes[txa][implied]     = 0x8a;

    opcodes[txs][implied]     = 0x9a;

    opcodes[tya][implied]     = 0x98;

    if (opcodes[instruction][arg.addressingMode] == 0xff) {
        printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
        exit(EXIT_FAILURE);
    }
    
    bin[*indexp] = opcodes[instruction][arg.addressingMode];
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
        bin[*indexp] = (uint8_t)(arg.value >> 8);
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