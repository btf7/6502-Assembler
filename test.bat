@echo off
.\Assembler tests\full_test_input.asm full_test_output.6502
fc.exe /b tests\full_test_expected_output.6502 full_test_output.6502