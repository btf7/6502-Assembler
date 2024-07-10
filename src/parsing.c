#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

struct constantArr parseConstants(const struct lineArr lines) {
    struct constantArr constants = {0, NULL};
    size_t constantsMalloced = 0;

    for (size_t i = 0; i < lines.len; i++) {
        const struct tokenArr line = lines.arr[i];
        const enum instructions instructionType = identifyInstruction(line.arr[0]);

        if (instructionType != constant && instructionType != label) {
            continue;
        }

        if (constants.len == constantsMalloced) {
            constants.arr = expandDynamicArr(constants.arr, &constantsMalloced, sizeof *constants.arr);
        }

        struct constant * const constantp = &(constants.arr[constants.len]);
        constants.len++;

        if (instructionType == constant) {
            if (line.len != 4) {
                printf("Expected 4 tokens in constant definition, got %lld\n", line.len);
                exit(EXIT_FAILURE);
            }

            constantp->valueKnown = true;

            if (strcmp("BYTE", line.arr[1]) == 0) {
                constantp->twoBytes = false;
            } else if (strcmp("WORD", line.arr[1]) == 0) {
                constantp->twoBytes = true;
            } else {
                printf("Must specify constant size with either BYTE or WORD: .DEF %s %s %s\n", line.arr[1], line.arr[2], line.arr[3]);
                exit(EXIT_FAILURE);
            }

            for (size_t j = 0; j < strlen(line.arr[2]); j++) {
                if (!isalpha(line.arr[2][j])) {
                    printf("Constant names must be alphabetical: .DEF %s %s %s\n", line.arr[1], line.arr[2], line.arr[3]);
                    exit(EXIT_FAILURE);
                }
            }

            if (strcmp("LO", line.arr[2]) == 0) {
                printf("LO is an invalid constant name\n");
                exit(EXIT_FAILURE);
            }
            if (strcmp("HI", line.arr[2]) == 0) {
                printf("HI is an invalid constant name\n");
                exit(EXIT_FAILURE);
            }
            if (strcmp("A", line.arr[2]) == 0) {
                printf("A is an invalid constant name\n");
                exit(EXIT_FAILURE);
            }

            constantp->name = malloc(strlen(line.arr[2]) + 1);
            if (!constantp->name) {
                printf("Crashed due to malloc() fail\n");
                exit(EXIT_FAILURE);
            }
            strcpy(constantp->name, line.arr[2]);
            constantp->name[strlen(line.arr[2])] = '\0';

            // Pass 0 for index so it's effectively ignored
            const struct expressionValue num = parseExpression(line.arr[3], strlen(line.arr[3]), 0, constants);

            if (!num.valueKnown) {
                printf("Constants cannot be defined by labels: .DEF %s %s %s\n", line.arr[1], line.arr[2], line.arr[3]);
                exit(EXIT_FAILURE);
            }

            if (constantp->twoBytes) {
                constantp->value = num.value;
            } else {
                constantp->value = num.value & 0xff;
            }
        } else {
            if (line.len != 1) {
                printf("Expected 1 token in label definition, got %lld\n", line.len);
                exit(EXIT_FAILURE);
            }

            // Note that label values will be set when found in step 3
            constantp->valueKnown = false;
            constantp->twoBytes = true;

            for (size_t j = 0; j < strlen(line.arr[0]) - 1; j++) {
                if (!isalpha(line.arr[0][j])) {
                    printf("Label names must be alphabetical: %s\n", line.arr[0]);
                    exit(EXIT_FAILURE);
                }
            }

            if (strcmp("LO:", line.arr[0]) == 0) {
                printf("LO is an invalid label name\n");
                exit(EXIT_FAILURE);
            }
            if (strcmp("HI:", line.arr[0]) == 0) {
                printf("HI is an invalid label name\n");
                exit(EXIT_FAILURE);
            }
            if (strcmp("A:", line.arr[0]) == 0) {
                printf("A is an invalid label name\n");
                exit(EXIT_FAILURE);
            }

            const size_t nameLen = strlen(line.arr[0]); // Added to remove GCC warning -Wstringop-trunctation
            constantp->name = malloc(nameLen);
            if (!constantp->name) {
                printf("Crashed due to malloc() fail\n");
                exit(EXIT_FAILURE);
            }
            strncpy(constantp->name, line.arr[0], nameLen - 1);
            constantp->name[nameLen - 1] = '\0';
        }
    }

    constants.arr = realloc(constants.arr, constants.len * sizeof *constants.arr);
    if (!constants.arr) {
        printf("Crashed due to realloc() fail\n");
        exit(EXIT_FAILURE);
    }
    return constants;
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

struct numberValue parseNumber(const char * const text) {
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
                break;
            }
        }

        if (num > 0xffff) {
            printf("Number too big: Expected 0 - 65535, got %d: %s\n", num, text);
            exit(EXIT_FAILURE);
        }

        if (num > 255) {
            return (struct numberValue){num, true, i};
        }
        return (struct numberValue){num, false, i};
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

        if (i == 5) {
            return (struct numberValue){num, true, i};
        }
        return (struct numberValue){num, false, i};
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

        if (i == 17) {
            return (struct numberValue){num, true, i};
        }
        return (struct numberValue){num, false, i};
    }

    printf("Failed to parse number: %s\n", text);
    exit(EXIT_FAILURE);
}

struct expressionValue parseExpression(const char * const text, const size_t expressionLen, const uint16_t index, const struct constantArr constants) {
    if (expressionLen == 0) {
        printf("No expression to evaluate: %s\n", text);
        exit(EXIT_FAILURE);
    }

    struct expressionValue num = {0, false, true};
    const char* expression = text;
    int8_t sign = 1; // Either 1 or -1 depending on if there's a + or -
    bool lo = false;
    bool hi = false;

    while (expression < text + expressionLen) {
        if (sign == 0) {
            printf("Expression missing '+' or '-': %s\n", text);
            exit(EXIT_FAILURE);
        }

        if (isalpha(expression[0])) {
            // It's a constant

            if (strncmp("LO ", expression, 3) == 0) {
                lo = true;
                expression += 3;
            } else if (strncmp("HI ", expression, 3) == 0) {
                hi = true;
                expression += 3;
            }

            // How long is it?
            size_t constantLen = 0;
            while (isalpha(expression[constantLen])) {
                constantLen++;
            }
            if (constantLen == 0) {
                printf("LO or HI used but no constant followed: %s\n", text);
                exit(EXIT_FAILURE);
            }

            // Find the constant
            bool constantDefined = false;
            size_t i;
            for (i = 0; i < constants.len; i++) {
                if (strncmp(expression, constants.arr[i].name, constantLen) == 0 && strlen(constants.arr[i].name) == constantLen) {
                    constantDefined = true;
                    break;
                }
            }
            if (!constantDefined) {
                printf("Constant used but not defined: %s\n", expression);
                exit(EXIT_FAILURE);
            }

            if (!constants.arr[i].valueKnown) {
                num.valueKnown = false;
            }

            if (lo) {
                num.value += (constants.arr[i].value & 0xff) * sign;
                lo = false;
            } else if (hi) {
                num.value += (constants.arr[i].value >> 8) * sign;
                hi = false;
            } else {
                num.value += constants.arr[i].value * sign;
                num.twoBytes |= constants.arr[i].twoBytes;
            }
            expression += constantLen;
        } else if (expression[0] == '*') {
            num.value += index * sign;
            num.twoBytes = true;
            expression++;
        } else if (isdigit(expression[0]) || expression[0] == '$' || expression[0] == '%') {
            const struct numberValue otherNum = parseNumber(expression);

            num.value += otherNum.value * sign;
            num.twoBytes |= otherNum.twoBytes;
            expression += otherNum.charsRead;
        } else {
            printf("Unknown character in expression: %s\n", expression);
            exit(EXIT_FAILURE);
        }

        if (expression[0] == '+') {
            sign = 1;
            expression++;
        } else if (expression[0] == '-') {
            sign = -1;
            expression++;
        } else {
            // Either at the end of the expression or there's an error
            sign = 0;
        }
    }

    // Number may have overflowed due to additions
    if (num.twoBytes) {
        return num;
    }
    num.value &= 0xff;
    return num;
}

struct arg parseArgument(const char * const text, const uint16_t index, const struct constantArr constants) {
    struct arg arg;
    arg.valueKnown = true;

    if (strcmp("A", text) == 0) {
        arg.addressingMode = accumulator;
        return arg;
    }

    // Identify the addressing mode
    // For now, treat all absolute modes like zero page modes
    // They will be changed back if the expression is evaluated as two bytes
    // Ignore relative mode as this can be detected based on the instruction

    const size_t textLen = strlen(text);
    bool expressionStartOffset = 0; // Always either 0 or 1 so bool is sufficient
    size_t expressionLen = textLen;

    if (text[0] == '#') {
        expressionStartOffset = 1;
        expressionLen--;
        arg.addressingMode = immediate;
    } else if (text[0] == '(') {
        expressionStartOffset = 1;
        expressionLen -= 2;

        if (strcmp(",X)", text + textLen - 3) == 0) {
            expressionLen -= 2;
            arg.addressingMode = indirectX;
        } else if (strcmp("),Y", text + textLen - 3) == 0) {
            expressionLen -= 2;
            arg.addressingMode = indirectY;
        } else if (text[textLen - 1] == ')') {
            arg.addressingMode = indirect;
        } else {
            printf("Couldn't identify addressing mode: %s\n", text);
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(",X", text + textLen - 2) == 0 ) {
        expressionLen -= 2;
        arg.addressingMode = zeroPageX;
    } else if (strcmp(",Y", text + textLen - 2) == 0) {
        expressionLen -= 2;
        arg.addressingMode = zeroPageY;
    } else {
        arg.addressingMode = zeroPage;
    }

    const struct expressionValue num = parseExpression(text + expressionStartOffset, expressionLen, index, constants);
    arg.value = num.value;
    arg.valueKnown = num.valueKnown;

    if (num.twoBytes) {
        // Fix addressing modes
        if (arg.addressingMode == zeroPage) {
            arg.addressingMode = absolute;
        } else if (arg.addressingMode == zeroPageX) {
            arg.addressingMode = absoluteX;
        } else if (arg.addressingMode == zeroPageY) {
            arg.addressingMode = absoluteY;
        }
    }

    return arg;
}