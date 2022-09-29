#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define true 1
#define false 0

// RegisterNode is a linked list containing all the registers' numbers/names
typedef struct register_node register_node;
typedef struct register_node {
    register_node* next;
    register_node* head;

    // Tail is only guarenteed to be updated for the head
    register_node* tail;

    char* registerNumber;
    char* registerName;
} RegisterNode;

void freeRegisterList(RegisterNode* head) {
    while (head != NULL) {
        free(head->registerNumber);
        free(head->registerName);

        RegisterNode* savedHead = head;
        head = head->next;
        free(savedHead);
    }
}

// Returns pointer to inserted node
RegisterNode* insertRegisterNode(RegisterNode* head, char* registerNumber, char* registerName) {
    RegisterNode* newNode = malloc(sizeof(RegisterNode));
    newNode->next = NULL;
    if (head == NULL) {
        newNode->head = newNode;
        newNode->tail = newNode;
    }
    else {
        newNode->head = head;
        head->tail->next = newNode;
        head->tail = newNode;
    }

    newNode->tail = newNode;

    newNode->registerNumber = malloc(sizeof(char) * 4);
    newNode->registerName = malloc(sizeof(char) * 5);

    strcpy_s(newNode->registerNumber, 4, registerNumber);
    strcpy_s(newNode->registerName, 5, registerName);

    return newNode;
 }

char* getRegisterNumber(RegisterNode* head, char* registerName) {
    while (head != NULL) {
        if (strcmp(head->registerName, registerName) == 0) {
            return head->registerNumber;
        }
        head = head->next;
    }
    return NULL;
}
char* getRegisterName(RegisterNode* head, char* registerNumber) {
    while (head != NULL) {
        if (strcmp(head->registerNumber, registerNumber) == 0) {
            return head->registerName;
        }
        head = head->next;
    }
    return NULL;
}


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
    printf("Enter an instruction: \n");
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

    // Open registers file
    RegisterNode* registerHead = NULL;
    
    FILE* fp;
    fopen_s(&fp, "Registers.csv", "r");
    if (fp == NULL) {
        printf("Error opening registers file");
        exit(-1);
    }


    // There's a lot of duplicated code, the but the end result is:
    // Read in every line, splitting it into key/value, and inserting into a dict
    int skip = true;
    int initFirst = true;
    char c;
    int onKey = true;
    char key[4] = "\0\0\0\0";
    char value[5] = "\0\0\0\0\0";
    int wordIndex = 0;
    int i = 0;
    while (true) {
        i++;
        c = fgetc(fp); // get next character

        // Edge case for end of file
        if (c == EOF) { 
            //printf("Adding value \"%s\"/\"%s\"\n", key, value);
            insertRegisterNode(registerHead, key, value);
            
            break;
        }

        // If at the end of a line
        if (c == '\n') {
            if (!skip) {
                // Add to list
                if (initFirst) {
                    registerHead = insertRegisterNode(NULL, key, value);
                    initFirst = false;
                }
                else {
                    insertRegisterNode(registerHead, key, value);
                }
                resetInputPart(key);
                resetInputPart(value);
                wordIndex = 0;
                onKey = true;
            }
            else {
                // Skip first lien
                resetInputPart(key);
                resetInputPart(value);
                wordIndex = 0;
                onKey = true;

                skip = false;
            }
            continue;
        }

        // Skip if still the first line
        if (skip == true) {
            continue;
        }

        // toggle key/value
        if (c == ',') {
            wordIndex = 0;
            onKey = !onKey;
            continue;
        }

        // Add character to relevant string
        if (onKey) {
            key[wordIndex] = c;
        }
        else {
            value[wordIndex] = c;
        }
        wordIndex++;
        if (onKey && wordIndex == 3) wordIndex = 0;
        if (!onKey && wordIndex == 4) wordIndex = 0;
    }
    fclose(fp);
    printf("\n");

    // So now, the registers list is initialized and the same should be done for the instructions list

    // deallocate the instruction parts array
    for (int i = 0; i < 4; i++) {
        free(instructionParts[i]);
    }
    free(instructionParts);
    freeRegisterList(registerHead);

    return 0;
}