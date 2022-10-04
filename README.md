A program to convert between a RISC-V assembly instruction and its binary equivalent (for CSCI-341). Written in C because I hate myself. Featuring custom linked-list structures because self-care.

Examples:

| RISC-V Instruction | Binary Equivalent |
| --- | --- |
| add x14, x13, x15 | 00000000111101101000011100110011 |
| addi x16, x17, -32 | 11111110000010001000100000010011 |
| bne x10, x22, 1023 | 00111111011001010001111101100011 |
| xor x4, x6, x3 | 00000000001100110100001000110011 |
| slli x3, x5, 2 | 00000000001000101001000110010011 |

`test_lab3.py` is a script to run the tests in `lab3_unit_tests.csv`. If you want to use it, uncomment line 494 in `Source.c`.