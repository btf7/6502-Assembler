#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

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

// TODO this doesn't set number.valueKnown as it isn't used, this is messy
struct number parseNumber(const char * const text) {
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

        if (i == 5) {
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

        if (i == 17) {
            return (struct number){num, true, i};
        }
        return (struct number){num, false, i};
    }

    printf("Invalid number in argument: %s\n", text);
    exit(EXIT_FAILURE);
}

struct number parseExpression(const char * const text, const size_t expressionLen, const uint16_t index, const struct constant * const constants, const size_t constantCount) {
    if (expressionLen == 0) {
        printf("No expression to evaluate: %s\n", text);
        exit(EXIT_FAILURE);
    }

    struct number num = {0, false, 0, true};
    const char * expression = text;
    int8_t sign = 1; // Either 1 or -1 depending on if there's a + or -

    while (expression < text + expressionLen) {
        if (sign == 0) {
            printf("Expression missing '+' or '-': %s\n", text);
            exit(EXIT_FAILURE);
        }

        if (isalpha(expression[0])) {
            // It's a constant

            // How long is it?
            size_t constantLen = 0;
            while (isalpha(expression[constantLen])) {
                constantLen++;
            }

            // Find the constant
            bool constantDefined = false;
            size_t i;
            for (i = 0; i < constantCount; i++) {
                if (strncmp(expression, constants[i].name, constantLen) == 0) {
                    constantDefined = true;
                    break;
                }
            }
            if (!constantDefined) {
                printf("Constant used but not defined: %s\n", expression);
                exit(EXIT_FAILURE);
            }

            if (!constants[i].valueKnown) {
                num.valueKnown = false;
                return num;
            }

            num.value += constants[i].value * sign;
            num.twoBytes |= constants[i].twoBytes;
            expression += constantLen;
        } else if (expression[0] == '*') {
            num.value += index * sign;
            num.twoBytes = true;
            expression++;
        } else if (isdigit(expression[0]) || expression[0] == '$' || expression[0] == '%') {
            const struct number otherNum = parseNumber(expression);

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

struct arg parseArgument(const char * const text, const uint16_t index, const struct constant * const constants, const size_t constantCount) {
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

    const struct number num = parseExpression(text + expressionStartOffset, expressionLen, index, constants, constantCount);
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