# 6502 Assembler

This is a basic 6502 assembler I wrote in C as my first full practice project.
I wrote it from scratch, and did not look at any existing assemblers for reference.

Instruction opcodes, addressing mode syntax, etc. was referenced from https://www.nesdev.org/obelisk-6502-guide/index.html.

Note that this has only been tested on my Windows 10 machine compiled with mingw64.

## How to use

In the console, run `.\Assembler assemblyfilename outputfilename`

You must give an assembly file, but the output file name is optional.

If you don't pass an output file name, it will default to [assemblyfilename].6502

The output file is a 64KiB binary file designed to be the starting state of a 6502 processor.

Run `.\test` (Windows only as it uses a .bat file) to check if the assembler is working correctly.
If it succeeds, the console should show:

```
> .\test
Reading from file...
Reading labels and constants...
Assembling...
Resolving labels...
Writing to file...
Assembled successfully
Comparing files TESTS\full_test_expected_output.6502 and FULL_TEST_OUTPUT.6502
FC: no differences encountered
```

## Assembly Syntax

Disclaimer: I have little experience in assembly and don't know what standard syntax looks like.

Comments are added with a `;`. Everything after a `;` in a line is ignored.

Everything (except hex values) is case-sensitive. All instructions and pseudo-ops must be in full caps.

Numbers can be inputed in 3 ways: decimal (`123`), hex (`$A1`), or binary (`%10101010`).

Expressions are any sequence of numbers, constants, labels, or `*`s combined with `+`s and `-`s.

`*` is used to get the current location counter value, before the current line is punched.
If used in `.BYTE` or `.WORD`, it's before the current value is punched.

The pseudo-ops for this assembler are: `.ORG`, `.BYTE`, `.WORD`, and `.DEF`.

- `.ORG` sets the location counter to a specific value. It takes 1 number (not expression)
as its operand and sets the location counter to that value.
This is the only operation that will not accept an expression.
If this is not used at the start of a file, the location counter will default to $8000.

- `.BYTE` punches 1 byte numbers to the binary, accepting any number of operands.

- `.WORD` punches 2 byte numbers to the binary, accepting any number of operands.

- `.DEF` defines a constant value. The first operand must be either `BYTE` or `WORD` to declare the size.
The second operand is the name of the constant, and the third operand is an expression for the value.

Labels are defined with their name followed immediately by a `:`. They take no operands.
When referenced, labels are functionally identical to 2 byte constants.

All labels and constants can be referenced before or after their definition.
Constants cannot use `*` in their definition.
Constants can only use previously defined constants in their definition.

`LO` and `HI` can be used to get the low or high byte of a label or 2 byte constant.
It cannot be used on number values.
For example: `LDA LO val` or `.DEF BYTE highbyte HI twobytes`

See tests\full_test_input.asm for more examples.