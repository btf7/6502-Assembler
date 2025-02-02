#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

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

    const struct constantArr constants = parseConstants(lines);

    printf("Assembling...\n");

    uint8_t bin[0x10000] = {0};
    struct unknownValueArgArr unknownValueArgs = {0, NULL};
    assemble(lines, constants, bin, &unknownValueArgs);

    printf("Resolving labels...\n");

    resolveLabels(lines, constants, unknownValueArgs, bin);

    for (size_t i = 0; i < lines.len; i++) {
        const struct tokenArr line = lines.arr[i];
        for (size_t j = 0; j < line.len; j++) {
            free(line.arr[j]);
        }
        free(line.arr);
    }
    free(lines.arr);

    for (size_t i = 0; i < constants.len; i++) {
        free(constants.arr[i].name);
    }
    free(constants.arr);

    free(unknownValueArgs.arr);

    printf("Writing to file...\n");

    char* fileName;
    if (argc == 2) {
        // No file name was given
        fileName = safeMalloc(strlen(argv[1]) + 6);
        strcpy(fileName, argv[1]);
        strcpy(fileName + strlen(argv[1]), ".6502");
    } else {
        fileName = safeMalloc(strlen(argv[2]) + 1);
        strcpy(fileName, argv[2]);
    }

    FILE * const outFile = fopen(fileName, "wb");
    if (!outFile) {
        printf("Couldn't create output file\n");
        exit(EXIT_FAILURE);
    }
    free(fileName);

    for (uint32_t i = 0; i < 0x10000; i++) {
        if (putc(bin[i], outFile) == EOF) {
            printf("Char write at 0x%x (0x%x) failed. Continuing anyway\n", i, bin[i]);
        }
    }

    fclose(outFile);

    printf("Assembled successfully\n");
    return EXIT_SUCCESS;
}

struct lineArr readAsmFile(const char * const fileName) {
    FILE * const file = fopen(fileName, "r");
    if (!file) {
        printf("Could not open file: %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    size_t rawLineCount = 1;

    struct lineArr lines = {0, NULL};
    size_t linesMalloced = 0;

    struct tokenArr* linep = NULL; // Should always either be a pointer to last element in lines.arr or NULL if the current line is empty
    size_t tokensMalloced = 0;

    char** tokenp = NULL; // Should always either be a pointer to the last element in linep->arr or NULL if the current token is empty
    size_t strLen = 0;
    size_t strMalloced = 0;

    bool lineCommented = false;

    while (true) {
        const int c = getc(file);

        if (c == ';') {
            lineCommented = true;
            continue;
        }

        if (c == '\n' || c == EOF) {
            // End the current token

            if (tokenp) {
                *tokenp = safeRealloc(*tokenp, strLen + 1);
                (*tokenp)[strLen] = '\0';
            }

            // End the current line

            if (linep) {
                linep->arr = safeRealloc(linep->arr, linep->len * sizeof *linep->arr);
            }

            if (c == EOF) {
                fclose(file);

                if (linesMalloced == 0) {
                    printf("Given file has no assembly contents: %s\n", fileName);
                    exit(EXIT_FAILURE);
                }

                lines.arr = safeRealloc(lines.arr, lines.len * sizeof *lines.arr);

                return lines;
            }

            tokenp = NULL;
            linep = NULL;
            lineCommented = false;
            rawLineCount++;

            continue;
        }

        if (lineCommented) {
            continue;
        }

        // If it's not a graphical character, and the token isn't ending in "LO " or "HI "
        if (!isgraph(c) && !(c == ' ' && tokenp && (strncmp(*tokenp + strLen - 2, "LO", 2) == 0 || strncmp(*tokenp + strLen - 2, "HI", 2) == 0))) {
            // End the current token

            if (tokenp) {
                *tokenp = safeRealloc(*tokenp, strLen + 1);
                (*tokenp)[strLen] = '\0';
            }

            tokenp = NULL;

            continue;
        }

        // Add character

        if (!linep) {
            // Create a new line

            if (lines.len == linesMalloced) {
                lines.arr = expandDynamicArr(lines.arr, &linesMalloced, sizeof *lines.arr);
            }

            linep = &(lines.arr[lines.len]);
            lines.len++;

            tokensMalloced = 0;
            linep->len = 0;
            linep->arr = expandDynamicArr(NULL, &tokensMalloced, sizeof *linep->arr);
            linep->rawLineNumber = rawLineCount;
        }

        if (!tokenp) {
            // Create a new token

            if (linep->len == tokensMalloced) {
                linep->arr = expandDynamicArr(linep->arr, &tokensMalloced, sizeof *linep->arr);
            }

            tokenp = &(linep->arr[linep->len]);
            linep->len++;

            strMalloced = 0;
            strLen = 0;
            *tokenp = expandDynamicArr(NULL, &strMalloced, sizeof **tokenp);
        }

        // Add the character

        if (strLen == strMalloced) {
            *tokenp = expandDynamicArr(*tokenp, &strMalloced, sizeof **tokenp);
        }

        (*tokenp)[strLen] = (char)c;
        strLen++;
    }
}

void assemble(const struct lineArr lines, const struct constantArr constants, uint8_t * const bin, struct unknownValueArgArr * const unknownValueArgs) {
    uint16_t index = 0x8000;
    bool started = false;
    size_t unknownValueArgsMalloced = 0;

    for (size_t i = 0; i < lines.len; i++) {
        const struct tokenArr line = lines.arr[i];

        const enum instructions instruction = identifyInstruction(line.arr[0], line.rawLineNumber);

        if (is6502Instruction(instruction)) {
            if (!started) {
                started = true;
                // Punch the start address
                bin[0xfffc] = (uint8_t)(index & 0xff);
                bin[0xfffd] = (uint8_t)(index >> 8);
            }

            struct arg arg;
            if (line.len == 1) {
                arg.addressingMode = implied;
                arg.valueKnown = true;
            } else if (line.len == 2) {
                arg = parseArgument(line.arr[1], index, constants, line.rawLineNumber);
            } else {
                printf("Line %lld: Expected 1-2 tokens in instruction, got %lld\n", line.rawLineNumber, line.len);
                exit(EXIT_FAILURE);
            }

            // Absolute and relative appear the same in assembly
            // Branch instructions use relative and everything else uses absolute
            // Switch from absolute to relative if it's a branch instruction
            switch (instruction) {
                case bcc:
                case bcs:
                case beq:
                case bmi:
                case bne:
                case bpl:
                case bvc:
                case bvs:
                if (arg.addressingMode != absolute && arg.addressingMode != zeroPage) {
                    printf("Line %lld: Invalid addressing mode (%s) for instruction (%s)\n", line.rawLineNumber, addressingModeNames[arg.addressingMode], instructionNames[instruction]);
                    exit(EXIT_FAILURE);
                }
                arg.addressingMode = relative;
                break;

                default:
                break;
            }

            if (arg.valueKnown) {
                punchInstruction(instruction, arg, bin, &index, line.rawLineNumber);
            } else {
                if (unknownValueArgs->len == unknownValueArgsMalloced) {
                    unknownValueArgs->arr = expandDynamicArr(unknownValueArgs->arr, &unknownValueArgsMalloced, sizeof *unknownValueArgs->arr);
                }
                unknownValueArgs->arr[unknownValueArgs->len].index = index;
                unknownValueArgs->arr[unknownValueArgs->len].lineIndex = i;
                unknownValueArgs->len++;
                // Increment index
                switch (arg.addressingMode) {
                    case absolute:
                    case absoluteX:
                    case absoluteY:
                    case indirect:
                    index += 3;
                    break;

                    default:
                    index += 2;
                    break;
                }
            }
        } else {
            switch (instruction) {
                case constant:
                break;

                case label:
                ; // Empty statement to silence -Wpendantic warning
                // Set the address of the label
                // Don't have to check if the label exists as this is its definition, it would have been read in step 2
                const size_t instructionLen = strlen(line.arr[0]);
                char * const labelName = safeMalloc(instructionLen);
                strncpy(labelName, line.arr[0], instructionLen - 1);
                labelName[instructionLen - 1] = '\0';
                for (size_t j = 0; j < constants.len; j++) {
                    if (strcmp(labelName, constants.arr[j].name) == 0) {
                        constants.arr[j].value = index;
                        constants.arr[j].valueKnown = true;
                        break;
                    }
                }
                free(labelName);
                break;

                case org:
                if (line.len != 2) {
                    printf("Line %lld: Expected 2 tokens in .ORG instruction, got %lld\n", line.rawLineNumber, line.len);
                    exit(EXIT_FAILURE);
                }
                index = parseNumber(line.arr[1], line.rawLineNumber).value;
                break;

                case byte:
                case word:
                if (line.len == 1) {
                    printf("Line %lld: .BYTE and .WORD expect at least 2 tokens, received %lld\n", line.rawLineNumber, line.len);
                    exit(EXIT_FAILURE);
                }

                for (size_t j = 1; j < line.len; j++) {
                    const struct expressionValue num = parseExpression(line.arr[j], strlen(line.arr[j]), index, constants, line.rawLineNumber);
                    if (num.valueKnown) {
                        if (instruction == byte) {
                            if (num.twoBytes) {
                                printf("Line %lld: .BYTE expects 1 byte numbers, received 2 byte number: %s\n", line.rawLineNumber, line.arr[j]);
                                exit(EXIT_FAILURE);
                            }
                            bin[index] = (uint8_t)num.value;
                            index++;
                        } else {
                            bin[index] = (uint8_t)(num.value & 0xff);
                            index++;
                            bin[index] = (uint8_t)(num.value >> 8);
                            index++;
                        }
                    } else {
                        if (unknownValueArgs->len == unknownValueArgsMalloced) {
                            unknownValueArgs->arr = expandDynamicArr(unknownValueArgs->arr, &unknownValueArgsMalloced, sizeof *unknownValueArgs->arr);
                        }
                        unknownValueArgs->arr[unknownValueArgs->len].index = index;
                        unknownValueArgs->arr[unknownValueArgs->len].lineIndex = i;
                        unknownValueArgs->arr[unknownValueArgs->len].offset = j;
                        unknownValueArgs->len++;

                        if (instruction == byte) {
                            index++;
                        } else {
                            index += 2;
                        }
                    }
                }
                break;

                default:
                printf("Failed to find implementation for pseudo-op %d\n", instruction);
                exit(EXIT_FAILURE);
            }
        }
    }

    unknownValueArgs->arr = safeRealloc(unknownValueArgs->arr, unknownValueArgs->len * sizeof *unknownValueArgs->arr);
}

void resolveLabels(const struct lineArr lines, const struct constantArr constants, const struct unknownValueArgArr unknownValueArgs, uint8_t * const bin) {
    for (size_t i = 0; i < unknownValueArgs.len; i++) {
        const struct tokenArr line = lines.arr[unknownValueArgs.arr[i].lineIndex];
        uint16_t index = unknownValueArgs.arr[i].index;

        const enum instructions instruction = identifyInstruction(line.arr[0], line.rawLineNumber);

        if (is6502Instruction(instruction)) {
            // It must have an argument
            struct arg arg = parseArgument(line.arr[1], index, constants, line.rawLineNumber);

            // Absolute and relative appear the same in assembly
            // Branch instructions use relative and everything else uses absolute
            // Switch from absolute to relative if it's a branch instruction
            switch (instruction) {
                case bcc:
                case bcs:
                case beq:
                case bmi:
                case bne:
                case bpl:
                case bvc:
                case bvs:
                if (arg.addressingMode != absolute && arg.addressingMode != zeroPage) {
                    printf("Line %lld: Invalid addressing mode (%s) for instruction (%s)\n", line.rawLineNumber, addressingModeNames[arg.addressingMode], instructionNames[instruction]);
                    exit(EXIT_FAILURE);
                }
                arg.addressingMode = relative;
                break;

                default:
                break;
            }

            punchInstruction(instruction, arg, bin, &index, line.rawLineNumber);
        } else if (instruction == byte || instruction == word) {
            const size_t offset = unknownValueArgs.arr[i].offset;

            const struct expressionValue num = parseExpression(line.arr[offset], strlen(line.arr[offset]), index, constants, line.rawLineNumber);
            if (instruction == byte && num.twoBytes) {
                printf("Line %lld: .BYTE expects 1 byte numbers, received 2 byte number: %s\n", line.rawLineNumber, line.arr[offset]);
                exit(EXIT_FAILURE);
            }
            bin[index] = (uint8_t)(num.value & 0xff);

            if (instruction == word) {
                index++;
                bin[index] = (uint8_t)(num.value >> 8);
            }
        } else {
            printf("Forward-referenced label in unexpected instruction (%d)\n", instruction);
            exit(EXIT_FAILURE);
        }
    }
}

void* safeMalloc(const size_t size) {
    void * const pointer = malloc(size);
    if (!pointer) {
        printf("Crashed due to malloc(%lld) fail\n", size);
        exit(EXIT_FAILURE);
    }
    return pointer;
}

void* safeRealloc(void* pointer, const size_t size) {
    pointer = realloc(pointer, size);
    if (!pointer) {
        printf("Crashed due to realloc(%lld) fail\n", size);
        exit(EXIT_FAILURE);
    }
    return pointer;
}

void* expandDynamicArr(void* arr, size_t * const malloced, const size_t elemSize) {
    if (*malloced == 0) {
        *malloced = 1;
    } else {
        *malloced *= 2;
    }
    arr = safeRealloc(arr, *malloced * elemSize);
    return arr;
}

// Copy source string into dest, converting to uppercase
// Assumes dest is big enough to fit
void strcpyupper(char * const dest, const char * const source) {
    size_t i = 0;
    while (true) {
        const char c = source[i];
        if (c >= 'a' && c <= 'z') {
            dest[i] = c + 'A' - 'a';
        } else {
            dest[i] = c;
        }
        if (c == '\0') {
            break;
        }
        i++;
    }
}