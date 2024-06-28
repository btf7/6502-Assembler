#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

enum instructions {label, constant, byte, word, org, asl, beq, inx, jmp, lda, stx};
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

struct arg {
    enum argAddressingMode addressingMode;
    uint16_t value;
};

struct number {
    uint16_t value;
    bool twoBytes;
    size_t charsRead; // TODO in parseNumber a uint8_t is usually used for this - could overflow
};

struct constant {
    char* name;
    bool label;
    uint16_t value;
    bool twoBytes;
};

struct lineArr readAsmFile(const char * const fileName) {
    FILE * const file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Could not open file: %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    struct lineArr lines = {0, NULL};
    size_t linesMalloced = 0;

    struct line* linep = NULL; // Should always either be a pointer to last element in lines.arr or NULL if the current line is empty
    size_t strMalloced = 0;
    size_t strLen = 0;
    bool instructionRead = false; // Are we reading an instruction or args

    bool lineCommented = false;

    while (true) {
        const int c = getc(file);

        if (c == ';') {
            lineCommented = true;
            continue;
        }

        if (c == '\n' || c == EOF) {
            // End the current line

            if (linep != NULL) {
                // Terminate strings as appropriate
                if (instructionRead) {
                    // Remove whitespace at the end if there is any
                    if (linep->args[strLen - 1] == ' ') {
                        linep->args = realloc(linep->args, strLen);
                        strLen--;
                    } else {
                        linep->args = realloc(linep->args, strLen + 1);
                    }
                    if (linep->args == NULL) {
                        printf("Crashed due to realloc() fail\n");
                        exit(EXIT_FAILURE);
                    }

                    linep->args[strLen] = '\0';
                } else {
                    // The instruction cannot be empty because then (linep == NULL) would have returned true
                    linep->instruction = realloc(linep->instruction, strLen + 1);
                    if (linep->instruction == NULL) {
                        printf("Crashed due to realloc() fail\n");
                        exit(EXIT_FAILURE);
                    }

                    linep->instruction[strLen] = '\0';

                    linep->args = calloc(1, 1);
                    if (linep->args == NULL) {
                        printf("Crashed due to calloc() fail\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }

            if (c == EOF) {
                fclose(file);
                
                if (linesMalloced == 0) {
                    printf("Given file has no assembly contents: %s\n", fileName);
                    exit(EXIT_FAILURE);
                }

                lines.arr = realloc(lines.arr, sizeof *lines.arr * lines.len);
                if (lines.arr == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    exit(EXIT_FAILURE);
                }

                return lines;
            }

            linep = NULL;
            strMalloced = 0;
            strLen = 0;
            instructionRead = false;
            lineCommented = false;

            continue;
        }

        if (lineCommented) {
            continue;
        }

        if (!isgraph(c)) {
            if (instructionRead) {
                if (strLen != 0 && linep->args[strLen - 1] != ' ') {
                    if (strLen == strMalloced) {
                        strMalloced *= 2;
                        linep->args = realloc(linep->args, strMalloced);
                        if (linep->args == NULL) {
                            printf("Crashed due to realloc() fail\n");
                            exit(EXIT_FAILURE);
                        }
                    }

                    linep->args[strLen] = ' ';
                    strLen++;
                }
            } else {
                linep->instruction = realloc(linep->instruction, strLen + 1);
                if (linep->instruction == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    exit(EXIT_FAILURE);
                }

                linep->instruction[strLen] = '\0';

                strMalloced = 1;
                strLen = 0;
                instructionRead = true;
                linep->args = malloc(1);
                if (linep->args == NULL) {
                    printf("Crashed due to malloc() fail\n");
                    exit(EXIT_FAILURE);
                }
            }

            continue;
        }

        // Add character

        if (linesMalloced == 0) {
            // This is the first line, initiate the array
            linesMalloced = 1;
            lines.arr = malloc(sizeof *lines.arr);
            if (lines.arr == NULL) {
                printf("Crashed due to malloc() fail\n");
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
                    exit(EXIT_FAILURE);
                }
            }

            // Create the line
            linep = &(lines.arr[lines.len]);
            lines.len++;
            strMalloced = 1;
            strLen = 0;
            linep->instruction = malloc(1);
            if (linep->instruction == NULL) {
                printf("Crashed due to malloc() fail\n");
                exit(EXIT_FAILURE);
            }
        }

        // Reserve space
        if (strLen == strMalloced) {
            strMalloced *= 2;
            if (instructionRead) {
                linep->args = realloc(linep->args, strMalloced);
                if (linep->args == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                linep->instruction = realloc(linep->instruction, strMalloced);
                if (linep->instruction == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        // Add the character
        if (instructionRead) {
            linep->args[strLen] = c;
        } else {
            linep->instruction[strLen] = c;
        }
        strLen++;
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
    int32_t num = 0; // Max number is uint16_t, but go bigger to detect too big numbers

    if (isdigit(text[0])) {
        // Decimal number
        // Check 6 digits to find too big numbers
        uint8_t i;
        for (i = 0; i < 6; i++) {
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
                    const struct number other = parseNumber(text + i + 1, index);
                    num += other.value;
                    i += other.charsRead + 1;
                } else if (c == '-') {
                    const struct number other = parseNumber(text + i + 1, index);
                    num -= other.value;
                    i += other.charsRead + 1;
                }

                break;
            }
        }

        if (num > 65535) {
            printf("Number too big: Expected 0 - 65535, got %d: %s\n", num, text);
            exit(EXIT_FAILURE);
        }

        if (num > 255) {
            return (struct number){num, true, i};
        }
        return (struct number){num, false, i};
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

        if (i != 3 && i != 5) {
            printf("Expected 2 or 4 hex digits, got %d: %s\n", i - 1, text);
            exit(EXIT_FAILURE);
        }

        const bool twoBytes = i == 5;

        if (text[i] == '+') {
            const struct number other = parseNumber(text + i + 1, index);
            num += other.value;
            i += other.charsRead + 1;
        } else if (text[i] == '-') {
            const struct number other = parseNumber(text + i + 1, index);
            num -= other.value;
            i += other.charsRead + 1;
        }

        if (twoBytes) {
            return (struct number){num, true, i};
        }
        return (struct number){num, false, i};
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

        if (i != 9 && i != 17) {
            printf("Expected 8 or 16 binary digits, got %d: %s\n", i - 1, text);
            exit(EXIT_FAILURE);
        }

        const bool twoBytes = i == 17;

        if (text[i] == '+') {
            const struct number other = parseNumber(text + i + 1, index);
            num += other.value;
            i += other.charsRead + 1;
        } else if (text[i] == '-') {
            const struct number other = parseNumber(text + i + 1, index);
            num -= other.value;
            i += other.charsRead + 1;
        }

        if (twoBytes) {
            return (struct number){num, true, i};
        }
        return (struct number){num, false, i};
    }

    if (text[0] == '*') {
        num = index;

        uint8_t charsRead = 1;

        if (text[1] == '+') {
            const struct number other = parseNumber(text + 2, index);
            num += other.value;
            charsRead += other.charsRead + 1;
        } else if (text[1] == '-') {
            const struct number other = parseNumber(text + 2, index);
            num -= other.value;
            charsRead += other.charsRead + 1;
        }

        return (struct number){num, true, charsRead};
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
        case constant:
        case org:
        case byte:
        case word:
        return false;

        default:
        return true;
    }
}

int main(int argc, char** argv) {
    // Step 1: Read file
    // Step 2: Go through file once, getting all label names and defining constants
    // Step 3: Go through file once, punching all instructions to bin. Keep track of label addresses and where they're used
    // Step 4: Go through all instructions where labels are used and punch in the correct value
    // Step 5: Write bin to file

    if (argc == 1) {
        printf("No assembly file given\n");
        exit(EXIT_FAILURE);
    }

    printf("Reading from file...\n");

    const struct lineArr lines = readAsmFile(argv[1]);

    printf("Reading labels and constants...\n");
    
    uint16_t index = 0x8000; // Must be defined here, not at step 3, as parseNumber() requires it

    struct constant* constants = NULL;
    size_t constantsMalloced = 0;
    size_t constantCount = 0;

    for (size_t i = 0; i < lines.len; i++) {
        const struct line line = lines.arr[i];
        const enum instructions instructionType = identifyInstruction(line.instruction);

        if (instructionType == constant || instructionType == label) {
            // Reserve space
            if (constantCount == constantsMalloced) {
                if (constantsMalloced == 0) {
                    constantsMalloced = 1;
                } else {
                    constantsMalloced *= 2;
                }
                constants = realloc(constants, constantsMalloced * sizeof *constants);
                if (constants == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    exit(EXIT_FAILURE);
                }
            }

            struct constant * const constantp = &(constants[constantCount]);
            constantCount++;

            if (instructionType == constant) {
                constantp->label = false;

                if (strncmp("BYTE ", line.args, 5) == 0) {
                    constantp->twoBytes = false;
                } else if (strncmp("WORD ", line.args, 5) == 0) {
                    constantp->twoBytes = true;
                } else {
                    printf("Must specify constant size with either BYTE or WORD: .DEF %s\n", line.args);
                    exit(EXIT_FAILURE);
                }

                size_t offset = 5;
                size_t nameLen = 0;
                size_t nameMalloced = 0;
                constantp->name = NULL;
                while (true) {
                    const char c = line.args[offset];
                    offset++;

                    if (c == '\0') {
                        printf("Constant defined with no value: .DEF %s\n", line.args);
                        exit(EXIT_FAILURE);
                    }

                    if (c == ' ') {
                        constantp->name = realloc(constantp->name, nameLen + 1);
                        constantp->name[nameLen] = '\0';
                        break;
                    }

                    if (!isalpha(c)) {
                        printf("Constant names must be alphabetical: .DEF %s\n", line.args);
                        exit(EXIT_FAILURE);
                    }

                    // Reserve space
                    if (nameLen == nameMalloced) {
                        if (nameMalloced == 0) {
                            nameMalloced = 1;
                        } else {
                            nameMalloced *= 2;
                        }
                        constantp->name = realloc(constantp->name, nameMalloced);
                    }

                    constantp->name[nameLen] = c;
                    nameLen++;
                }

                constantp->value = parseNumber(line.args + offset, index).value;
            } else {
                // Note that label values will be set when found in step 3
                constantp->label = true;
                constantp->twoBytes = true;

                size_t nameLen = 0;
                size_t nameMalloced = 0;
                constantp->name = NULL;
                while (true) {
                    const char c = line.instruction[nameLen];

                    if (c == ':') {
                        constantp->name = realloc(constantp->name, nameLen + 1);
                        constantp->name[nameLen] = '\0';
                        if (strcmp("LO", constantp->name) == 0) {
                            printf("LO is an invalid label name");
                            exit(EXIT_FAILURE);
                        }
                        if (strcmp("HI", constantp->name) == 0) {
                            printf("HI is an invalid label name");
                            exit(EXIT_FAILURE);
                        }
                        break;
                    }

                    if (!isalpha(c)) {
                        printf("Label names must be alphabetical: %s\n", line.instruction);
                        exit(EXIT_FAILURE);
                    }

                    // Reserve space
                    if (nameLen == nameMalloced) {
                        if (nameMalloced == 0) {
                            nameMalloced = 1;
                        } else {
                            nameMalloced *= 2;
                        }
                        constantp->name = realloc(constantp->name, nameMalloced);
                    }

                    constantp->name[nameLen] = c;
                    nameLen++;
                }
            }
        }
    }

    constants = realloc(constants, constantCount * sizeof *constants);

    printf("Assembling...\n");

    uint8_t bin[0x10000] = {0};
    bool started = false;

    for (size_t i = 0; i < lines.len; i++) {
        const struct line line = lines.arr[i];

        const enum instructions instruction = identifyInstruction(line.instruction);

        if (is6502Instruction(instruction)) {
            if (!started) {
                started = true;
                // Punch the start address
                bin[0xfffc] = (uint8_t)(index & 0xff);
                bin[0xfffd] = (uint8_t)((index & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
            }

            struct arg arg;
            if (strcmp(line.args, "") == 0) {
                arg.addressingMode = implied;
            } else {
                arg = identifyArg(line.args, index);
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
            size_t offset;
            switch (instruction) {
                case constant:
                break;

                case label:
                // Set the address of the label
                const size_t instructionLen = strlen(line.instruction);
                char * const labelName = malloc(instructionLen);
                strncpy(labelName, line.instruction, instructionLen - 1);
                labelName[instructionLen - 1] = '\0';
                for (size_t i = 0; i < constantCount; i++) {
                    if (strcmp(labelName, constants[i].name) == 0) {
                        constants[i].value = index;
                        break;
                    }
                }
                free(labelName);
                break;

                case org:
                index = parseNumber(line.args, index).value;
                break;

                case byte:
                offset = 0;
                while (true) {
                    const struct number num = parseNumber(line.args + offset, index);
                    if (num.twoBytes) {
                        printf(".BYTE expects 1 byte numbers, received 2 byte number: %s\n", line.args);
                        exit(EXIT_FAILURE);
                    }
                    bin[index] = (uint8_t)num.value;
                    index++;

                    offset += num.charsRead;
                    if (line.args[offset] == ' ') {
                        offset++;
                        continue;
                    } else if (line.args[offset] == '\0') {
                        break;
                    } else {
                        printf("Unexpected character '%c' in parsed arguments: %s\n", line.args[offset], line.args);
                        exit(EXIT_FAILURE);
                    }
                }
                break;

                case word:
                offset = 0;
                while (true) {
                    const struct number num = parseNumber(line.args + offset, index);
                    bin[index] = (uint8_t)(num.value & 0xff);
                    index++;
                    bin[index] = (uint8_t)((num.value & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
                    index++;

                    offset += num.charsRead;
                    if (line.args[offset] == ' ') {
                        offset++;
                        continue;
                    } else if (line.args[offset] == '\0') {
                        break;
                    } else {
                        printf("Unexpected character '%c' in parsed arguments: %s\n", line.args[offset], line.args);
                        exit(EXIT_FAILURE);
                    }
                }
                break;

                default:
                printf("Failed to recognise instruction %d\n", instruction);
                exit(EXIT_FAILURE);
            }
        }
        
        free(line.instruction);
        free(line.args);
    }

    free(lines.arr);

    printf("Resolving labels...\n");

    for (size_t i = 0; i < constantCount; i++) {
        free(constants[i].name);
    }
    free(constants);

    printf("Writing to file...\n");

    FILE * const outFile = fopen("out.6502", "wb");
    if (outFile == NULL) {
        printf("Couldn't create output file\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < 0x10000; i++) {
        if (putc(bin[i], outFile) == EOF) {
            printf("Char write at 0x%x (0x%x) failed. Continuing anyway\n", i, bin[i]);
        }
    }

    fclose(outFile);

    printf("Assembled successfully\n");
    return EXIT_SUCCESS;
}