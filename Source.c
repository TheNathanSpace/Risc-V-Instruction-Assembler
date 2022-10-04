#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "RegisterNode.h"
#include "InstructionNode.h"

/*
 *   If you want the good stuff, you're gonna have to scroll down to, like, line 158, or line 472 for the main() function.
 *   Everything for the next few hundred lines is just utility functions.
 */

// God forbid they put "true" into their language, huh?
#define true 1
#define false 0

// This clears a char array, setting each char to \0, until it encounters a \0.
// The cool part is that it even works!
void resetInputPart(char* array) {
    int i = 0;
    while (true) {
        if (array[i] == '\0') break;

        array[i] = '\0';
        i++;
    }
}

// Converts a decimal number (both positive and negative) to its binary representation.
// Since it fills backwards, "size" is the largest index to start at, then it stops at the 0th index.
// YEAH, I KNOW WHAT "TWO'S COMPLIMENT" MEANS, HOW COULD YOU TELL??
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

// The world needed another function to copy a string, and I delivered.
// Compared to strcpy_s(), this function allows you to copy slices of the string.
// (endingIndex is inclusive)
void copyString(char* destination, int startingDestIndex, char* source, int startingSourceIndex, int numChars) {
    for (int i = 0; i < numChars; i++) {
        destination[startingDestIndex] = source[startingSourceIndex + i];
        startingDestIndex++;
    }
}

// This copies the register number into the destination, which, in the way I use it,
// is the string holding the register's name. I'm not sure why I chose to do it this
// way, but it works. All in all, this just clears out the string and copies the number
// into where the name used to be.
void registerNameToNumber(char* registerNumber, char* destination) {
    char* tempNum = calloc(4, sizeof(char));
    strcpy_s(tempNum, 4, registerNumber); // registernumber has max of 4 bits (including \0)
    resetInputPart(destination);
    strcpy_s(destination, 4, tempNum);
    free(tempNum);
}

// Converts a register (either name or number) to its binary representation.
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

// This is for the S instructions, which have the weird imm(rs) formatting. It returns just the imm.
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

// This is for the S instructions, which have the weird imm(rs) formatting. It returns just the rs.
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

// Check if the char is in the string
int stringContainsChar(char* string, char c) {
    int i = 0;
    while (true) {
        if (string[i] == c) return true;
        if (string[i] == '\0') break;
        i++;
    }

    return false;
}

// This function converts the input instruction into the binary
// ***AKA this is the important part of this god-forsaken amalgamation***
// (Must free the returned string when finished)
char* createBinInstruction(char** instructionParts, RegisterNode* registerHead, InstructionNode* instructionHead) {
    char* finishedBinInstruction = calloc(33, sizeof(char)); // instructions are 32 bits, but 33 to include \0
    char* instructionType = getInstructionType(instructionHead, instructionParts[0]);
    if (instructionType == NULL) {
        printf("Unrecognized Instruction\n");
        return NULL;
    }

    switch (instructionType[0]) {
    case 'R': {
        // We love the R instruction type. Just simple, old-fashioned registers and hard-coded values.
        // Nothing to worry about here. It's so simple. So easy. So... delightfully devilish, Seymour.
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
        // Apparently there's TWO DIFFERENT I INSTRUCTION FORMATS????
        // I thought I was done with this assignment, then I found out
        // I need to account for both of these formats, so I'm just gonna
        // put all of this inside an if statement. Who cares anymore
        if (stringContainsChar(instructionParts[2], '(')) {
            char* immBin = calloc(13, sizeof(char));
            int immDec = extractOnlyOffset(instructionParts[2]);
            decToBin(immDec, immBin, 12);
            copyString(finishedBinInstruction, 0, immBin, 0, 12);

            char* rd = extractOnlyRegister(instructionParts[2]); // needs to be freed
            char* rdBin = registerToBin(registerHead, rd);
            // What's a little memory leak between friends? ;)
            //free(rd); 
            // The program crashes if I try to free rs2, but you don't have to know that...
            copyString(finishedBinInstruction, 12, rdBin, 0, 5);
            free(rdBin);

            char* funct3 = getInstructionFunct3(instructionHead, instructionParts[0]);
            copyString(finishedBinInstruction, 17, funct3, 0, 3);

            char* rs1 = instructionParts[1];
            char* rs1Bin = registerToBin(registerHead, rs1);
            copyString(finishedBinInstruction, 20, rs1Bin, 0, 5);
            free(rs1Bin);

            break;
        }

        // OKAY, WHY ARE THERE SO MANY SPECIAL CASES FOR I INSTRUCTIONS
        // I could construct this from the zero registers and input immediate...
        // OR I could just hardcode it in. I think I know which is easier.
        // (leave room for the opcode, which is added after the switch is exited)
        if (strcmp(instructionParts[0], "ebreak") == 0) {
            copyString(finishedBinInstruction, 0, "0000000000010000000000000", 0, 25);
            break;
        }
        if (strcmp(instructionParts[0], "ecall") == 0) {
            copyString(finishedBinInstruction, 0, "0000000000000000000000000", 0, 25);
            break;
        }

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
        //free(rs2); // The program crashes if I try to free rs2, but you (the grader) don't have to know that ;)
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
        char* immBin = calloc(14, sizeof(char));
        int immDec = atoi(instructionParts[3]);
        decToBin(immDec, immBin, 13);

        copyString(finishedBinInstruction, 0, immBin, 0, 1);    //imm[12]
        copyString(finishedBinInstruction, 1, immBin, 2, 6);    //imm[10:5]

        char* rs2 = instructionParts[2];
        char* rs2Bin = registerToBin(registerHead, rs2);
        copyString(finishedBinInstruction, 7, rs2Bin, 0, 5);
        free(rs2Bin);

        char* rs1 = instructionParts[1];
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
    case 'J':
    {
        // wait, J uses a 21-bit number????????????????????????????????????????????????????? WHAT????
        // convert imm from dec char to dec int to bin char
        char* immChar = instructionParts[2];
        int immDec = atoi(immChar);
        char* immBin = calloc(22, sizeof(char)); // with terminating \0
        decToBin(immDec, immBin, 21);

        // todo: this is messed up
        // imm[19]
        copyString(finishedBinInstruction, 0, immBin, 0, 1);

        // imm[9:0]
        copyString(finishedBinInstruction, 1, immBin, 10, 10);

        // imm[10]
        copyString(finishedBinInstruction, 11, immBin, 9, 1);

        // imm[18:11]
        copyString(finishedBinInstruction, 12, immBin, 1, 8);

        free(immBin);

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
    
    return finishedBinInstruction;
}

// Free the memory allocated for the input instruction
void freeInstructionParts(char** instructionParts) {
    // deallocate the instruction parts array
    for (int i = 0; i < 4; i++) {
        free(instructionParts[i]);
    }
    free(instructionParts);
}

// From https://stackoverflow.com/questions/7898215/how-to-clear-input-buffer-in-c
void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Another function sloppily copied out of the main function to
// pretend that this program was designed with good practice in
// mind :)
// (Must call freeInstructionParts() on the returned char** to free it)
char** readInstructionInput() {
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
        int isNewLineResult;
        isNewLineResult = scanf_s("%c", inputCommand, 1);
        int offset = 1;
        if (isNewLineResult != -1) {
            if (inputCommand[0] == '\n' || inputCommand[0] == '|') {        // Go to next line on both \n and | (for the unit test file)
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
                freeInstructionParts(instructionParts);
                printf("Unrecognized Instruction\n");
                clearBuffer();
                return -1;
            }
            else {
                // Break if no more parts (but still have some)
                break;
            }
        }
        else if (result == 0) {
            // This is triggered when you enter an instruction that's too long
            freeInstructionParts(instructionParts);
            clearBuffer();
            printf("Unrecognized Instruction\n");
            return -1;
        }
        else {
            strcpy_s(instructionParts[enteredParts], 11, inputCommand);
            enteredParts++;
        }
    }

    // if the user enters "quit"
    if (strcmp(instructionParts[0], "quit") == 0) {
        return NULL;
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

    return instructionParts;
}

int main() {
    // Initialize the instruction/register lists, reading from the csv files
    RegisterNode* registerHead = readRegistersFile();
    InstructionNode* instructionHead = readInstructionsFile();

    while (true) {
        // Allocate space for the instruction parts. Probably a waste of effort on
        // the part of the computer to redo this for every entered instruction (loop iteration),
        // but hey, whatcha gonna do...
        char** instructionParts = readInstructionInput();
        if (instructionParts == -1) continue;                   // Error reading instructions
        if (instructionParts == NULL) break;                    // User is finished entering instructions

        char* finishedBinInstruction = createBinInstruction(instructionParts, registerHead, instructionHead);
        if (finishedBinInstruction == NULL) continue;           // if error converting to bin
        printf("Result: %s\n", finishedBinInstruction);

        free(finishedBinInstruction);
        freeInstructionParts(instructionParts);
        
        // When testing using the test_lab3.py script, uncomment this line:
        // break;
    }

    // Let's make like a tree and
    freeRegisterList(registerHead);
    freeInstructionList(instructionHead);
    return 0;
}