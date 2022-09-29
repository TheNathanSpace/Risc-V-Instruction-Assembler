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

int main() {
    // Allocate space for the instruction parts
    char** instructionParts = malloc(sizeof(char*) * 4);        // Array of arrays of chars. 4 is the max command parts
    for (int i = 0; i < 4; i++) {
        instructionParts[i] = malloc(sizeof(char) * 11);        // room for 10 characters per part + \0
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

    printInstructionParts(instructionParts);

    // Initialize the instruction/register lists, reading from the csv files
    RegisterNode* registerHead = readRegistersFile();
    InstructionNode* instructionHead = readInstructionsFile();

    // This is the easy part
    printf("Result: ");

    // And now for the hard part


    // deallocate the instruction parts array
    for (int i = 0; i < 4; i++) {
        free(instructionParts[i]);
    }
    free(instructionParts);
    freeRegisterList(registerHead);
    freeInstructionList(instructionHead);

    return 0;
}