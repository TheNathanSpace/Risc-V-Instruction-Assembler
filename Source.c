#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "RegisterNode.h"
#include "InstructionNode.h"
#define true 1
#define false 0


void resetInputPart(char* array) {
    int i = 0;
    while (true) {
        if (array[i] == '\0') break;

        array[i] = '\0';
        i++;
    }
}

void printInstructionParts(char** parts) {
    printf("\n");
    for (int i = 0; i < 4; i++) {
        printf("%s", parts[i]);
        printf("\n");
    }
    printf("\n");
}

// Converts a decimal number (both positive and negative) to its binary representation.
// Since it fills backwards, "size" is the largest index to start at, then it stops at the 0th index.
void decToBin(int input, char* result, int size) {
    int i = size - 1;

    // For two's complement
    char onBit = '1';
    char offBit = '0';
    if (input < 0) {
        input *= -1;        // Invert the number
        input -= 1;         // Add 1
        onBit = '0';
        offBit = '1';
    }

    while (i != -1) {
        int quotient = input / 2;
        int remainder = input % 2;
        input = quotient;
        if (remainder == 1) {
            result[i] = onBit;
        }
        else {
            result[i] = offBit;
        }
        i--;
    }
}

// endingIndex is inclusive
void copyString(char* destination, int startingDestIndex, char* source, int startingSourceIndex, int numChars) {
    for (int i = 0; i < numChars; i++) {
        destination[startingDestIndex] = source[startingSourceIndex + i];
        startingDestIndex++;
    }
}

void registerNameToNumber(char* registerNumber, char* destination) {
    char* tempNum = calloc(4, sizeof(char));
    strcpy_s(tempNum, 4, registerNumber); // registernumber has max of 4 bits (including \0)
    resetInputPart(destination);
    strcpy_s(destination, 4, tempNum);
    free(tempNum);
}

// Converts a register (either name or number) to its binary representation
// Must free rdBin when finished!!
char* registerToBin(RegisterNode* registerHead, char* r) {
    if (getRegisterName(registerHead, r) == NULL) {
        // if entered as a name, convert to number
        registerNameToNumber(getRegisterNumber(registerHead, r), r);
    }

    int rdDec = atoi(r + 1); // skip initial x
    char* rdBin = calloc(6, sizeof(char));
    decToBin(rdDec, rdBin, 5); // rd has 5 bits
    
    return rdBin;
}

int extractOnlyOffset(char* instructionPart) {
    int bytes = 0;

    int i = 0;
    while (true) {
        if (instructionPart[i] == '(') {
            break;
        }
        bytes++;
        i++;
    }
    char* extracted = calloc(bytes + 1, sizeof(char));
    extracted[bytes] = '\0';
    copyString(extracted, 0, instructionPart, 0, bytes);
    int offset = atoi(extracted);

    return offset;
}

// Must deallocate after!
char* extractOnlyRegister(char* instructionPart) {
    
    int bytesBegin = false;
    int bytes = 0;

    int i = 0;
    int startedI = 0;
    while (true) {
        if (instructionPart[i] == '(') {
            bytesBegin = true;
            startedI = i + 1;
        }
        else if (instructionPart[i] == ')') {
            break;
        }
        else if (bytesBegin) {
            bytes++;
        }
        i++;
    }

    char* extracted = calloc(bytes + 1, sizeof(char));
    extracted[bytes] = '\0';
    copyString(extracted, 0, instructionPart, startedI, bytes);
    return extracted;
}

int main() {
    // Allocate space for the instruction parts
    char** instructionParts = malloc(sizeof(char*) * 4);        // Array of arrays of chars. 4 is the max command parts
    for (int i = 0; i < 4; i++) {
        instructionParts[i] = calloc(11, sizeof(char));        // room for 10 characters per part + \0
    }

    int enteredParts = 0;
    printf("Enter an Instruction: ");
    char inputCommand[11];                                      // room for 10 characters per part + \0 (automatically added)
    inputCommand[0] = '\0';
    int result;
    
    // While true, keep reading words from input
    while (true) {
        resetInputPart(inputCommand);
        
        // This checks for a newline character by getting the next character
        int isNewLineResult = scanf_s("%c", inputCommand, 1);
        int offset = 1;
        if (isNewLineResult != -1) {
            if (inputCommand[0] == '\n') {
                break;
            }
            if (inputCommand[0] == ' ') {
                offset = 0;
            }
        }

        // Read the next string and put it in the instructions part array
        result = scanf_s("%s", inputCommand + offset, 10);
        if (result == -1) {
            if (enteredParts == 0) {
                // Exit with error if no entered parts at all
                printf("Error reading input");
                exit(-1);
            }
            else {
                // Break if no more parts (but still have some)
                break;
            }
        }
        else {
            strcpy_s(instructionParts[enteredParts], 11, inputCommand);
            enteredParts++;
        }
    }

    // remove trailing commas from input instruction parts
    for (int i = 0; i < 4; i++) {
        int j = 0;
        while (1) {
            if (instructionParts[i][j] == '\0') break;

            if (instructionParts[i][j] == ',') {
                instructionParts[i][j] = '\0'; 
                break;
            }
            j++;
        }
    }

    // Initialize the instruction/register lists, reading from the csv files
    RegisterNode* registerHead = readRegistersFile();
    InstructionNode* instructionHead = readInstructionsFile();

    char* finishedBinInstruction = calloc(33, sizeof(char)); // instructions are 32 bits, but 33 to include \0
    char* instructionType = getInstructionType(instructionHead, instructionParts[0]);
    switch (instructionType[0]) {
    case 'J':
    {
        // convert imm from dec char to dec int to bin char
        char* immChar = instructionParts[2];
        int immDec = atoi(immChar);
        char* immBin = calloc(21, sizeof(char)); // with terminating \0
        decToBin(immDec, immBin, 20);

        // imm[20]
        finishedBinInstruction[0] = immBin[19];
        
        // imm[10:1]
        for (int i = 1; i < 11; i++) {
            finishedBinInstruction[i] = immBin[10 - i];
        }

        // imm[11]
        finishedBinInstruction[11] = immBin[10] ;

        // imm[19:12]
        for (int i = 0; i < 8; i++) {
            finishedBinInstruction[12 + i] = immBin[18 - i];
        }
        free(immBin);

        char* rd = instructionParts[1];
        if (getRegisterName(registerHead, rd) == NULL) {
            // if entered as a name, convert to number
            registerNameToNumber(getRegisterNumber(registerHead, rd), rd);
        }

        int rdDec = atoi(rd + 1); // skip initial x
        char* rdBin = calloc(6, sizeof(char));
        decToBin(rdDec, rdBin, 5); // rd has 5 bits
        for (int i = 0; i < 5; i++) {
            finishedBinInstruction[20 + i] = rdBin[i];
        }
        free(rdBin);
        
        break;
    }
    case 'R': {
        char* funct7 = getInstructionFunct7(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 0, funct7, 0, 7);

        char* rs2 = instructionParts[3];
        char* rs2Bin = registerToBin(registerHead, rs2);
        copyString(finishedBinInstruction, 7, rs2Bin, 0, 5);
        free(rs2Bin);

        char* rs1 = instructionParts[2];
        char* rs1Bin = registerToBin(registerHead, rs1);
        copyString(finishedBinInstruction, 12, rs1Bin, 0, 5);
        free(rs1Bin);

        char* funct3 = getInstructionFunct3(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 17, funct3, 0, 3);

        char* rd = instructionParts[1];
        char* rdBin = registerToBin(registerHead, rd);
        copyString(finishedBinInstruction, 20, rdBin, 0, 5);
        free(rdBin);

        break;
    }
    case 'I':
    {
        char* immBin = calloc(13, sizeof(char));
        char* immChar = instructionParts[3];
        int immDec = atoi(immChar);
        decToBin(immDec, immBin, 12);
        copyString(finishedBinInstruction, 0, immBin, 0, 12);
        free(immBin);

        char* rs1 = instructionParts[2];
        char* rs1Bin = registerToBin(registerHead, rs1);
        copyString(finishedBinInstruction, 12, rs1Bin, 0, 5);
        free(rs1Bin);

        char* funct3 = getInstructionFunct3(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 17, funct3, 0, 3);

        char* rd = instructionParts[1];
        char* rdBin = registerToBin(registerHead, rd);
        copyString(finishedBinInstruction, 20, rdBin, 0, 5);
        free(rdBin);

        break;
    }
    case 'S':
    {
        // sb t6, 64(s2)
        // for 12 bit imm
        char* immBin = calloc(13, sizeof(char));
        int immDec = extractOnlyOffset(instructionParts[2]);
        decToBin(immDec, immBin, 12);
        copyString(finishedBinInstruction, 0, immBin, 0, 7);

        // rs2 and rs1 are kinda in the weird order for these instructions, since
        // rs2 comes first in the instruction. I'm not renaming the variables, so
        // just know that THIS BLOCK IS ACTUALLY rs2.
        char* rs1 = instructionParts[1];
        char* rs1Bin = registerToBin(registerHead, rs1);
        copyString(finishedBinInstruction, 7, rs1Bin, 0, 5);
        free(rs1Bin);

        // And THIS BLOCK IS ACTUALLY rs1.
        char* rs2 = extractOnlyRegister(instructionParts[2]); // needs to be freed
        char* rs2Bin = registerToBin(registerHead, rs2);
        //free(rs2);
        copyString(finishedBinInstruction, 12, rs2Bin, 0, 5);
        free(rs2Bin);

        char* funct3 = getInstructionFunct3(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 17, funct3, 0, 3);

        copyString(finishedBinInstruction, 20, immBin, 0, 5);
        free(immBin);

        break;
    }
    case 'B':
    {
        // B type has 12 bit imm
        char* immBin = calloc(13, sizeof(char));
        int immDec = atoi(instructionParts[3]);
        decToBin(immDec, immBin, 12);

        copyString(finishedBinInstruction, 0, immBin, 0, 1);    //imm[12] (note that these indices are going off the slideshow, and for some reason start at 1, not 0)
        copyString(finishedBinInstruction, 1, immBin, 2, 6);    //imm[10:5]

        char* rs2 = instructionParts[2];
        char* rs2Bin = registerToBin(registerHead, rs2);
        copyString(finishedBinInstruction, 7, rs2Bin, 0, 5);
        free(rs2Bin);

        char* rs1 = instructionParts[2];
        char* rs1Bin = registerToBin(registerHead, rs1);
        copyString(finishedBinInstruction, 12, rs1Bin, 0, 5);
        free(rs1Bin);

        char* funct3 = getInstructionFunct3(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 17, funct3, 0, 3);

        copyString(finishedBinInstruction, 20, immBin, 8, 4);    //imm[4:1]
        copyString(finishedBinInstruction, 24, immBin, 1, 1);    //imm[11]

        break;
    }
    case 'U':
    {
        // U type really only takes a 20-bit immediate (but it's the upper part of a 32-bit immediate)
        char* immBin = calloc(21, sizeof(char));
        int immDec = atoi(instructionParts[2]);
        decToBin(immDec, immBin, 20);
        copyString(finishedBinInstruction, 0, immBin, 0, 20);

        char* rd = instructionParts[1];
        char* rdBin = registerToBin(registerHead, rd);
        copyString(finishedBinInstruction, 20, rdBin, 0, 5);
        free(rdBin);

        break;
    }
    }

    // The opcode is in the same place for each instruction type
    char* opcode = getInstructionOpcode(instructionHead, instructionParts[0]);
    copyString(finishedBinInstruction, 25, opcode, 0, 7);

    printf("Result: %s\n", finishedBinInstruction);

    // deallocate the instruction parts array
    for (int i = 0; i < 4; i++) {
        free(instructionParts[i]);
    }
    free(instructionParts);
    freeRegisterList(registerHead);
    freeInstructionList(instructionHead);
    free(finishedBinInstruction);
    return 0;
}