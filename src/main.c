#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

enum expectedArgs {noArgs, oneArg, manyArgs};
enum instructions {label, byte, word, org, asl, beq, inx, jmp, lda, stx};
// Note that relative addressing in assembly is identical to absolute, plus it's always the only option so it can be detected from the instruction
enum argAddressingMode {accumulator, implied, immediate, zeroPage, zeroPageX, zeroPageY, relative, absolute, absoluteX, absoluteY, indirect, indirectX, indirectY};

struct tokenArr {
    size_t len;
    char** arr;
};

struct lineArr {
    size_t len;
    struct tokenArr* arr;
};

struct arg {
    enum argAddressingMode addressingMode;
    uint16_t value;
};

struct number {
    uint16_t value;
    bool twoBytes;
};

struct lineArr readTokensFromAsmFile(const char * const fileName) {
    FILE * const file = fopen(fileName, "r");
    if (file == NULL) {
        printf("File not found: %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    size_t linesMalloced = 0;
    struct lineArr lines = {0, NULL};

    size_t lineMalloced = 0;
    struct tokenArr* linep = NULL;

    size_t tokenMalloced = 0;
    size_t tokenLen = 0;
    char** tokenp = NULL;

    bool lineCommented = false;

    while (true) {
        char c = getc(file);

        if (c == ';') {
            lineCommented = true;
        }

        if (!isgraph(c) || lineCommented) {
            // End the current token if there is one
            if (tokenp != NULL) {
                // Clamp size to save memory
                if (tokenMalloced > tokenLen + 1) {
                    *tokenp = realloc(*tokenp, tokenLen + 1);
                    if (*tokenp == NULL) {
                        printf("Crashed due to realloc() fail\n");
                        fclose(file);
                        exit(EXIT_FAILURE);
                    }
                }

                (*tokenp)[tokenLen] = '\0';
                tokenp = NULL;
                tokenMalloced = 0;
                tokenLen = 0;
            }

            if (c == '\n' || c == EOF) {
                // End the current line if there is one
                if (linep != NULL) {
                    // Clamp size to save memory
                    if (lineMalloced > linep->len) {
                        linep->arr = realloc(linep->arr, sizeof *linep->arr * linep->len);
                        if (linep->arr == NULL) {
                            printf("Crashed due to realloc() fail\n");
                            fclose(file);
                            exit(EXIT_FAILURE);
                        }
                    }

                    linep = NULL;
                    lineMalloced = 0;
                }

                if (c == EOF) {
                    fclose(file);

                    if (linesMalloced == 0) {
                        printf("Given file has no code contents: %s\n", fileName);
                        exit(EXIT_FAILURE);
                    }

                    // Clamp size to save memory
                    if (linesMalloced > lines.len) {
                        lines.arr = realloc(lines.arr, sizeof *lines.arr * lines.len);
                        if (lines.arr == NULL) {
                            printf("Crashed due to realloc() fail\n");
                            exit(EXIT_FAILURE);
                        }
                    }

                    return lines;
                }
                
                lineCommented = false;
            }

            continue;
        }

        // Add character to token

        if (linesMalloced == 0) {
            // This is the first line, initiate the array
            // This isn't done by default as the file may be empty
            linesMalloced = 1;
            lines.arr = malloc(sizeof *lines.arr);
            if (lines.arr == NULL) {
                printf("Crashed due to malloc() fail\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }

        if (linep == NULL) {
            // Create a new line

            // Reserve space
            if (lines.len == linesMalloced) {
                linesMalloced *= 2;
                lines.arr = realloc(lines.arr, sizeof *lines.arr * linesMalloced);
                if (lines.arr == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
            }

            // Create the line
            lineMalloced = 1;
            linep = &(lines.arr[lines.len]);
            lines.len++;
            linep->len = 0;
            linep->arr = malloc(sizeof *linep->arr);
            if (linep->arr == NULL) {
                printf("Crashed due to malloc() fail\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }

        if (tokenp == NULL) {
            // Create a new token

            // Reserve space
            if (linep->len == lineMalloced) {
                lineMalloced *= 2;
                linep->arr = realloc(linep->arr, sizeof *linep->arr * lineMalloced);
                if (linep->arr == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
            }

            // Create the token
            tokenMalloced = 2;
            tokenp = &(linep->arr[linep->len]);
            linep->len++;
            tokenLen = 0;
            *tokenp = malloc(2);
            if (*tokenp == NULL) {
                printf("Crashed due to malloc() fail\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }

        // Reserve space
        if (tokenLen + 1 == tokenMalloced) {
            tokenMalloced *= 2;
            *tokenp = realloc(*tokenp, tokenMalloced);
            if(*tokenp == NULL) {
                printf("Crashed due to realloc() fail\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }

        // Add the character
        (*tokenp)[tokenLen] = c;
        tokenLen++;
    }
}

enum instructions identifyInstruction(const char * const text) {
    if (text[0] == '.') {
        // Pseudo_op

        if (strcmp(".ORG", text) == 0) {
            return org;
        } else if (strcmp(".BYTE", text) == 0) {
            return byte;
        } else if (strcmp(".WORD", text) == 0) {
            return word;
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

enum expectedArgs getExpectedArgCount(const enum instructions instruction) {
    switch (instruction) {
        case label:
        case inx:
        return noArgs;
        
        case byte:
        case word:
        return manyArgs;

        default:
        return oneArg;
    }
}

uint8_t hexCharToInt(const char c) {
    if (isdigit(c)) {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return 255;
    }
}

struct number parseNumber(const char * const text, const uint16_t index) {
    uint32_t num = 0; // Max number is uint16_t, but go bigger to detect too big numbers

    if (isdigit(text[0])) {
        // Decimal number
        // Check 6 digits to find too big numbers
        for (uint8_t i = 0; i < 6; i++) {
            const char c = text[i];
            if (isdigit(c)) {
                if (i == 0 && c == '0' && isdigit(text[i + 1])) {
                    printf("Decimal number cannot start with 0: %s\n", text);
                    exit(EXIT_FAILURE);
                }
                num *= 10;
                num += c - '0';
            } else {
                if (c == '+') {
                    num += parseNumber(text + i + 1, index).value;
                } else if (c == '-') {
                    num -= parseNumber(text + i + 1, index).value;
                }

                break;
            }
        }

        if (num > 65535) {
            printf("Number too big: Expected 0 - 65535, got %d: %s\n", num, text);
            exit(EXIT_FAILURE);
        }

        if (num > 255) {
            return (struct number){num, true};
        }
        return (struct number){num, false};
    }

    if (text[0] == '$') {
        // Hex number
        // Check 5 digits to find too big numbers
        uint8_t i;

        for (i = 1; i < 6; i++) {
            const uint8_t new = hexCharToInt(text[i]);
            
            if (new == 255) {
                num = num >> 4;
                break;
            }

            num |= new;
            num = num << 4;
        }

        if (text[i] == '+') {
            num += parseNumber(text + i + 1, index).value;
        } else if (text[i] == '-') {
            num -= parseNumber(text + i + 1, index).value;
        }

        if (i == 3) {
            return (struct number){num, false};
        }
        if (i == 5) {
            return (struct number){num, true};
        }

        printf("Expected 2 or 4 hex digits, got %d: %s\n", i - 1, text);
        exit(EXIT_FAILURE);
    }

    if (text[0] == '%') {
        // Binary number
        // Check 17 digits to find too big numbers
        uint8_t i;

        for (i = 1; i < 18; i++) {
            if (text[i] == '1') {
                num |= 1;
            } else if (text[i] != '0') {
                num = num >> 1;
                break;
            }
            num = num << 1;
        }

        if (text[i] == '+') {
            num += parseNumber(text + i + 1, index).value;
        } else if (text[i] == '-') {
            num -= parseNumber(text + i + 1, index).value;
        }

        if (i == 9) {
            return (struct number){num, false};
        }
        if (i == 17) {
            return (struct number){num, true};
        }

        printf("Expected 8 or 16 binary digits, got %d: %s\n", i - 1, text);
        exit(EXIT_FAILURE);
    }

    if (text[0] == '*') {
        num = index;

        if (text[1] == '+') {
            num += parseNumber(text + 2, index).value;
        } else if (text[1] == '-') {
            num -= parseNumber(text + 2, index).value;
        }

        return (struct number){num, true};
    }

    printf("Invalid number in argument: %s\n", text);
    exit(EXIT_FAILURE);
}

bool isStartOfNumber(const char c) {
    return isdigit(c) || c == '$' || c == '%';
}

// TODO actually enforce correct syntax
struct arg identifyArg(const char * const text, const uint16_t index) {
    struct arg arg;

    if (strcmp("A", text) == 0) {
        arg.addressingMode = accumulator;
        return arg;
    }

    if (text[0] == '#') {
        arg.addressingMode = immediate;

        if (isStartOfNumber(text[1])) {
            const struct number number = parseNumber(text + 1, index);
            arg.value = number.value;
            if (number.twoBytes) {
                printf("Expected 1 byte number, recieved 2 byte number: %s\n", text);
                exit(EXIT_FAILURE);
            }
            return arg;
        }

        printf("Labels not supported / syntax error: %s\n", text);
        exit(EXIT_FAILURE);
    }

    const size_t len = strlen(text);

    if (text[0] == '(') {
        if (isStartOfNumber(text[1])) {
            const struct number number = parseNumber(text + 1, index);
            arg.value = number.value;
            if (number.twoBytes) {
                arg.addressingMode = indirect;
            } else if (text[len - 1] == ')') {
                arg.addressingMode = indirectX;
            } else {
                arg.addressingMode = indirectY;
            }
            return arg;
        }

        printf("Labels not supported / syntax error: %s\n", text);
        exit(EXIT_FAILURE);
    }

    const struct number number = parseNumber(text, index);
    arg.value = number.value;

    if (number.twoBytes) {
        if (text[len - 2] == ',') {
            if (text[len - 1] == 'X') {
                arg.addressingMode = absoluteX;
            } else {
                arg.addressingMode = absoluteY;
            }
        } else {
            arg.addressingMode = absolute;
        }
        return arg;
    } else {
        if (text[len - 2] == ',') {
            if (text[len - 1] == 'X') {
                arg.addressingMode = zeroPageX;
            } else {
                arg.addressingMode = zeroPageY;
            }
        } else {
            arg.addressingMode = zeroPage;
        }
        return arg;
    }
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
        case org:
        case byte:
        case word:
        return false;

        default:
        return true;
    }
}

int main(int argc, char** argv) {
    // 1: read file
    // 2: convert assembly to machine code except labels
    // 3: resolve labels
    // 4: write to file

    // During step 2, could keep track of memory locations to fill in with labels and which label to fill in
    // and which memory location each label is at when they're defined

    // ^^ Only works assuming all labels are 2 bytes

    // Could do a pass where we evaluate the sizes of all labels so that when we find them, we leave the correct amount of space
    // eg. defined constant labels have whatever size we gave them and location labels are always 2 bytes, unless its relative addressing (or zero page?) (eg. size can be inferred)
    // Note that relative addressing is only used for branch instructions and is the only option for those instructions

    // LO label HI label

    // Step 1: Read file
    // Step 2: Go through file once, getting all label definitions and defining constants
    // Step 3: Go through file once, punching all instructions to bin
    // Step 4: Write bin to file

    if (argc == 1) {
        printf("No assembly file given\n");
        exit(EXIT_FAILURE);
    }

    // First pass
    printf("Reading from file...\n");

    const struct lineArr lines = readTokensFromAsmFile(argv[1]);

    // Second pass
    printf("Assembling...\n");

    uint8_t bin[0x10000] = {0};
    uint16_t index = 0x8000;
    bool started = false;

    for (size_t i = 0; i < lines.len; i++) {
        const struct tokenArr line = lines.arr[i];

        const enum instructions instruction = identifyInstruction(line.arr[0]);
        const enum expectedArgs expectedArgs = getExpectedArgCount(instruction);

        if (expectedArgs == noArgs && line.len != 1) {
            printf("Invalid line: Expected 0 args, got %llu\n", line.len - 1);
            exit(EXIT_FAILURE);
        }
        if (expectedArgs == oneArg && line.len != 2) {
            printf("Invalid line: Expected 1 arg, got %llu\n", line.len - 1);
            exit(EXIT_FAILURE);
        }
        if (expectedArgs == manyArgs && line.len == 1) {
            printf("Invalid line: Expected args, got none\n");
            exit(EXIT_FAILURE);
        }

        if (is6502Instruction(instruction)) {
            if (!started) {
                started = true;
                // Punch the start address
                bin[0xfffc] = (uint8_t)(index & 0xff);
                bin[0xfffd] = (uint8_t)((index & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
            }

            struct arg arg;
            if (expectedArgs == noArgs) {
                arg.addressingMode = implied;
            } else {
                arg = identifyArg(line.arr[1], index);
            }

            // Absolute and relative appear the same in assembly
            // Branch instructions use relative and everything else uses absolute
            // Switch from absolute to relative if it's a branch instruction
            switch (instruction) {
                case beq:
                if (arg.addressingMode != absolute) {
                    printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
                    exit(EXIT_FAILURE);
                }
                arg.addressingMode = relative;
                break;

                default:
                break;
            }

            punchInstruction(instruction, arg, bin, &index);
        } else {
            switch (instruction) {
                case label:
                printf("labels not yet supported\n");
                exit(EXIT_FAILURE);

                case org:
                index = parseNumber(line.arr[1], index).value;
                break;

                case byte:
                for (size_t i = 1; i < line.len; i++) {
                    const struct number num = parseNumber(line.arr[i], index);
                    if (num.twoBytes) {
                        printf(".BYTE expects 1 byte numbers, received 2 byte number: %s\n", line.arr[i]);
                        exit(EXIT_FAILURE);
                    }
                    bin[index] = (uint8_t)num.value;
                    index++;
                }
                break;

                case word:
                for (size_t i = 1; i < line.len; i++) {
                    const struct number num = parseNumber(line.arr[i], index);
                    bin[index] = (uint8_t)(num.value & 0xff);
                    index++;
                    bin[index] = (uint8_t)((num.value & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
                    index++;
                }
                break;

                default:
                printf("Failed to recognise instruction %d\n", instruction);
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < lines.len; i++) {
        const struct tokenArr line = lines.arr[i];
        for (int j = 0; j < line.len; j++) {
            free(line.arr[j]);
        }
        free(line.arr);
    }
    free(lines.arr);

    // Third pass
    printf("Resolving labels...\n");



    // Write to file
    printf("Writing to file...\n");

    FILE * const outFile = fopen("out.6502", "wb");
    if (outFile == NULL) {
        printf("Couldn't create output file\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < 0x10000; i++) {
        if (putc(bin[i], outFile) == EOF) {
            printf("Char write %d - 0x%x failed. Continuing anyway\n", i, bin[i]);
        }
    }

    fclose(outFile);

    printf("Assembled successfully\n");
    return EXIT_SUCCESS;
}