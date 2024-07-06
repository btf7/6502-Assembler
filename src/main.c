#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

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
            } else if (linep != NULL) {
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

struct constantArr readConstants(const struct lineArr lines) {
    struct constantArr constants = {0, NULL};
    size_t constantsMalloced = 0;

    for (size_t i = 0; i < lines.len; i++) {
        const struct line line = lines.arr[i];
        const enum instructions instructionType = identifyInstruction(line.instruction);

        if (instructionType == constant || instructionType == label) {
            // Reserve space
            if (constants.len == constantsMalloced) {
                if (constantsMalloced == 0) {
                    constantsMalloced = 1;
                } else {
                    constantsMalloced *= 2;
                }
                constants.arr = realloc(constants.arr, constantsMalloced * sizeof *constants.arr);
                if (constants.arr == NULL) {
                    printf("Crashed due to realloc() fail\n");
                    exit(EXIT_FAILURE);
                }
            }

            struct constant * const constantp = &(constants.arr[constants.len]);
            constants.len++;

            if (instructionType == constant) {
                constantp->valueKnown = true;

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
                        if (nameLen == 0) {
                            printf("Constant defined with no name: .DEF %s\n", line.args);
                            exit(EXIT_FAILURE);
                        }
                        constantp->name = realloc(constantp->name, nameLen + 1);
                        constantp->name[nameLen] = '\0';
                        if (strcmp("LO", constantp->name) == 0) {
                            printf("LO is an invalid constant name");
                            exit(EXIT_FAILURE);
                        }
                        if (strcmp("HI", constantp->name) == 0) {
                            printf("HI is an invalid constant name");
                            exit(EXIT_FAILURE);
                        }
                        if (strcmp("A", constantp->name) == 0) {
                            printf("A is an invalid constant name");
                            exit(EXIT_FAILURE);
                        }
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

                // Pass 0 for index so it's effectively ignored
                const struct number num = parseExpression(line.args + offset, strlen(line.args) - offset, 0, constants);

                if (!num.valueKnown) {
                    printf("Constants cannot be defined by labels: .DEF %s\n", line.args);
                    exit(EXIT_FAILURE);
                }

                if (constantp->twoBytes) {
                    constantp->value = num.value;
                } else {
                    constantp->value = num.value & 0xff;
                }
            } else {
                // Note that label values will be set when found in step 3
                constantp->valueKnown = false;
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
                        if (strcmp("A", constantp->name) == 0) {
                            printf("A is an invalid label name");
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

    constants.arr = realloc(constants.arr, constants.len * sizeof *constants.arr);
    return constants;
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

    struct constantArr constants = readConstants(lines);

    printf("Assembling...\n");

    uint8_t bin[0x10000] = {0};
    uint16_t index = 0x8000;
    bool started = false;
    struct unknownValueArg* unknownValueIndexes = NULL;
    size_t unknownValueIndexesCount = 0;
    size_t unknownValueIndexesMalloced = 0;

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
                arg.valueKnown = true;
            } else {
                arg = parseArgument(line.args, index, constants);
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
                if (arg.addressingMode != absolute) {
                    printf("Invalid addressing mode (%d) for instruction (%d)\n", arg.addressingMode, instruction);
                    exit(EXIT_FAILURE);
                }
                arg.addressingMode = relative;
                break;

                default:
                break;
            }

            if (arg.valueKnown) {
                punchInstruction(instruction, arg, bin, &index);
            } else {
                if (unknownValueIndexesCount == unknownValueIndexesMalloced) {
                    if (unknownValueIndexesMalloced == 0) {
                        unknownValueIndexesMalloced = 1;
                    } else {
                        unknownValueIndexesMalloced *= 2;
                    }
                    unknownValueIndexes = realloc(unknownValueIndexes, unknownValueIndexesMalloced * sizeof *unknownValueIndexes);
                }
                unknownValueIndexes[unknownValueIndexesCount].index = index;
                unknownValueIndexes[unknownValueIndexesCount].lineIndex = i;
                unknownValueIndexesCount++;
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
            size_t offset;
            switch (instruction) {
                case constant:
                break;

                case label:
                // Set the address of the label
                // Don't have to check if the label exists as this is its definition, it would have been read in step 2
                const size_t instructionLen = strlen(line.instruction);
                char * const labelName = malloc(instructionLen);
                strncpy(labelName, line.instruction, instructionLen - 1);
                labelName[instructionLen - 1] = '\0';
                for (size_t i = 0; i < constants.len; i++) {
                    if (strcmp(labelName, constants.arr[i].name) == 0) {
                        constants.arr[i].value = index;
                        constants.arr[i].valueKnown = true;
                        break;
                    }
                }
                free(labelName);
                break;

                case org:
                index = parseNumber(line.args).value;
                break;

                case byte:
                offset = 0;
                while (true) {
                    // Find the length of the expression
                    size_t expressionLen = 0;
                    while (true) {
                        if (strncmp("LO ", line.args + offset + expressionLen, 3) == 0 || strncmp("HI ", line.args + offset + expressionLen, 3) == 0) {
                            expressionLen += 3;
                        } else if ((line.args + offset)[expressionLen] == ' ' || (line.args + offset)[expressionLen] == '\0') {
                            break;
                        } else {
                            expressionLen++;
                        }
                    }

                    const struct number num = parseExpression(line.args + offset, expressionLen, index, constants);
                    if (num.valueKnown) {
                        if (num.twoBytes) {
                            printf(".BYTE expects 1 byte numbers, received 2 byte number: %s\n", line.args);
                            exit(EXIT_FAILURE);
                        }
                        bin[index] = (uint8_t)num.value;
                    } else {
                        if (unknownValueIndexesCount == unknownValueIndexesMalloced) {
                            if (unknownValueIndexesMalloced == 0) {
                                unknownValueIndexesMalloced = 1;
                            } else {
                                unknownValueIndexesMalloced *= 2;
                            }
                            unknownValueIndexes = realloc(unknownValueIndexes, unknownValueIndexesMalloced * sizeof *unknownValueIndexes);
                        }
                        unknownValueIndexes[unknownValueIndexesCount].index = index;
                        unknownValueIndexes[unknownValueIndexesCount].lineIndex = i;
                        unknownValueIndexes[unknownValueIndexesCount].offset = offset;
                        unknownValueIndexesCount++;
                    }
                    index++;

                    offset += expressionLen;
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
                    // Find the length of the expression
                    size_t expressionLen = 0;
                    while (true) {
                        if (strncmp("LO ", line.args + offset + expressionLen, 3) == 0 || strncmp("HI ", line.args + offset + expressionLen, 3) == 0) {
                            expressionLen += 3;
                        } else if ((line.args + offset)[expressionLen] == ' ' || (line.args + offset)[expressionLen] == '\0') {
                            break;
                        } else {
                            expressionLen++;
                        }
                    }

                    const struct number num = parseExpression(line.args + offset, expressionLen, index, constants);
                    if (num.valueKnown) {
                        bin[index] = (uint8_t)(num.value & 0xff);
                        index++;
                        bin[index] = (uint8_t)((num.value & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
                        index++;
                    } else {
                        if (unknownValueIndexesCount == unknownValueIndexesMalloced) {
                            if (unknownValueIndexesMalloced == 0) {
                                unknownValueIndexesMalloced = 1;
                            } else {
                                unknownValueIndexesMalloced *= 2;
                            }
                            unknownValueIndexes = realloc(unknownValueIndexes, unknownValueIndexesMalloced * sizeof *unknownValueIndexes);
                        }
                        unknownValueIndexes[unknownValueIndexesCount].index = index;
                        unknownValueIndexes[unknownValueIndexesCount].lineIndex = i;
                        unknownValueIndexes[unknownValueIndexesCount].offset = offset;
                        unknownValueIndexesCount++;
                        index += 2;
                    }

                    offset += expressionLen;
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
    }

    unknownValueIndexes = realloc(unknownValueIndexes, unknownValueIndexesCount * sizeof *unknownValueIndexes);

    printf("Resolving labels...\n");

    for (size_t i = 0; i < unknownValueIndexesCount; i++) {
        const struct line line = lines.arr[unknownValueIndexes[i].lineIndex];
        index = unknownValueIndexes[i].index;

        const enum instructions instruction = identifyInstruction(line.instruction);

        if (is6502Instruction(instruction)) {
            struct arg arg;
            if (strcmp(line.args, "") == 0) {
                arg.addressingMode = implied;
            } else {
                arg = parseArgument(line.args, index, constants);
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
        } else if (instruction == byte) {
            const size_t offset = unknownValueIndexes[i].offset;

            // Find the length of the expression
            size_t expressionLen = 0;
            while (true) {
                if (strncmp("LO ", line.args + offset + expressionLen, 3) == 0 || strncmp("HI ", line.args + offset + expressionLen, 3) == 0) {
                    expressionLen += 3;
                } else if ((line.args + offset)[expressionLen] == ' ' || (line.args + offset)[expressionLen] == '\0') {
                    break;
                } else {
                    expressionLen++;
                }
            }

            const struct number num = parseExpression(line.args + offset, expressionLen, index, constants);
            if (num.twoBytes) {
                printf(".BYTE expects 1 byte numbers, received 2 byte number: %s\n", line.args);
                exit(EXIT_FAILURE);
            }
            bin[index] = (uint8_t)num.value;
        } else if (instruction == word) {
            const size_t offset = unknownValueIndexes[i].offset;

            // Find the length of the expression
            size_t expressionLen = 0;
            while (true) {
                if (strncmp("LO ", line.args + offset + expressionLen, 3) == 0 || strncmp("HI ", line.args + offset + expressionLen, 3) == 0) {
                    expressionLen += 3;
                } else if ((line.args + offset)[expressionLen] == ' ' || (line.args + offset)[expressionLen] == '\0') {
                    break;
                } else {
                    expressionLen++;
                }
            }

            const struct number num = parseExpression(line.args + offset, expressionLen, index, constants);
            bin[index] = (uint8_t)(num.value & 0xff);
            index++;
            bin[index] = (uint8_t)((num.value & 0xff00) >> 8); // The & 0xff00 is redundant but makes it feel safer
        } else {
            printf("Unknown value in unexpected instruction (%d)\n", instruction);
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < lines.len; i++) {
        free(lines.arr[i].instruction);
        free(lines.arr[i].args);
    }
    free(lines.arr);

    for (size_t i = 0; i < constants.len; i++) {
        free(constants.arr[i].name);
    }
    free(constants.arr);

    free(unknownValueIndexes);

    printf("Writing to file...\n");

    char* fileName;
    if (argc == 2) {
        // No file name was given
        fileName = malloc(strlen(argv[1]) + 6);
        strcpy(fileName, argv[1]);
        strcpy(fileName + strlen(argv[1]), ".6502");
    } else {
        fileName = malloc(strlen(argv[2]) + 1);
        strcpy(fileName, argv[2]);
    }

    FILE * const outFile = fopen(fileName, "wb");
    if (outFile == NULL) {
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