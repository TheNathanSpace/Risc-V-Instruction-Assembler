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

// Size is basically the offset to start at, then it stops at the 0th index
void decToBin(int input, char* result, int size) {
    int i = size - 1;
    while (i != -1) {
        int quotient = input / 2;
        int remainder = input % 2;
        input = quotient;
        if (remainder == 1) {
            result[i] = '1';
        }
        else {
            result[i] = '0';
        }
        i--;
    }
}

// endingIndex is inclusive
void copyString(char* destination, int startingDestIndex, char* source, int startingSouceIndex, int numChars) {
    for (int i = startingSouceIndex; i <= numChars; i++) {
        destination[startingDestIndex] = source[i];
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

// Must free rdBin when finished!!
char* registerToDec(RegisterNode* registerHead, char* r) {
    if (getRegisterName(registerHead, r) == NULL) {
        // if entered as a name, convert to number
        registerNameToNumber(getRegisterNumber(registerHead, r), r);
    }

    int rdDec = atoi(r + 1); // skip initial x
    char* rdBin = calloc(6, sizeof(char));
    decToBin(rdDec, rdBin, 5); // rd has 5 bits
    
    return rdBin;
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

        char* opcode = getInstructionOpcode(instructionHead, instructionParts[0]);
        for (int i = 0; i < 7; i++) {
            finishedBinInstruction[25 + i] = opcode[i];
        }
        
        break;
    }
    case 'R': {
        char* funct7 = getInstructionFunct7(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 0, funct7, 0, 7);

        char* rs2 = instructionParts[3];
        char* rs2Bin = registerToDec(registerHead, rs2);
        copyString(finishedBinInstruction, 7, rs2Bin, 0, 5);
        free(rs2Bin);

        char* rs1 = instructionParts[2];
        char* rs1Bin = registerToDec(registerHead, rs1);
        copyString(finishedBinInstruction, 12, rs1Bin, 0, 5);
        free(rs1Bin);

        char* funct3 = getInstructionFunct3(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 17, funct3, 0, 3);

        char* rd = instructionParts[1];
        char* rdBin = registerToDec(registerHead, rd);
        copyString(finishedBinInstruction, 20, rdBin, 0, 5);
        free(rdBin);

        char* opcode = getInstructionOpcode(instructionHead, instructionParts[0]);
        copyString(finishedBinInstruction, 25, opcode, 0, 7);

        break;
    }
    }

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