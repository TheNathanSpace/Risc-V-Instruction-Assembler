#pragma once
#include <string.h>
#define true 1
#define false 0

/*
 *  This file defines a linked list made up of RISC-V instructions, constructed
 *  from the contents of the provided CSV file.
 *  
 *  In C, there's no standard library support for dictionaries or hashmaps or
 *  anything like that, so a linked list made up of these structs was the best
 *  compromise I could find.
 */

void resetInputPart(char* array);

// InstructionNode is a linked list containing all the instructions' data
typedef struct instruction_node instruction_node;
typedef struct instruction_node {
    instruction_node* next;
    instruction_node* head;

    // Tail is only guarenteed to be updated for the head
    instruction_node* tail;

    char* instructionName;
    char* opcode;        // note it does not include leading zeroes
    char* funct3;                   // note it does not include leading zeroes
    char* funct7;                   // note it does not include leading zeroes
    char* type;
    char* immValue;

} InstructionNode;

/// <summary>
/// Frees the entire linked list
/// </summary>
/// <param name="head">Head of the linked list</param>
void freeInstructionList(InstructionNode* head) {
    while (head != NULL) {
        free(head->instructionName);
        free(head->opcode);
        free(head->funct3);
        free(head->funct7);

        InstructionNode* savedHead = head;
        head = head->next;
        free(savedHead);
    }
}

/// <summary>
/// Creates and inserts a new InstructionNode, constructed from the parameters.
/// </summary>
/// <param name="head">The head of the linked list</param>
/// <param name="instructionName">The name (keyword) of the RISC-V instruction. eg: "add"</param>
/// <param name="opcode"></param>
/// <param name="funct3"></param>
/// <param name="funct7"></param>
/// <param name="type"></param>
/// <param name="immmValue"></param>
/// <returns>Pointer to the inserted node.</returns>
InstructionNode* insertInstructionNode(InstructionNode* head, char* instructionName, char* opcode, char* funct3, char* funct7, char* type, int* immValue) {
    InstructionNode* newNode = malloc(sizeof(InstructionNode));
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

    newNode->instructionName = malloc(sizeof(char) * 7); // max size of 6
    newNode->opcode = malloc(sizeof(char) * 8);    // max size of 7 bits (will probably need leading zeroes?)  
    newNode->funct3 = malloc(sizeof(char) * 4);    // max size of 3 bits (will probably need leading zeroes?)  
    newNode->funct7 = malloc(sizeof(char) * 8);    // max size of 7 bits (will probably need leading zeroes?)  
    newNode->type = malloc(sizeof(char) * 2); // max size of 1
    newNode->immValue = malloc(sizeof(char) * 2); // max size of 1 // todo: THIS IS WRONG!!!!!!

    strcpy_s(newNode->instructionName, 7, instructionName);
    strcpy_s(newNode->opcode, 8, opcode);
    strcpy_s(newNode->funct3, 4, funct3);
    strcpy_s(newNode->funct7, 8, funct7);
    strcpy_s(newNode->type, 2, type);
    strcpy_s(newNode->immValue, 2, immValue);

    return newNode;
}

char* getInstructionOpcode(InstructionNode* head, char* instructionName) {
    while (head != NULL) {
        if (strcmp(head->instructionName, instructionName) == 0) {
            return head->opcode;
        }
        head = head->next;
    }
    return NULL;
}

char* getInstructionFunct3(InstructionNode* head, char* instructionName) {
    while (head != NULL) {
        if (strcmp(head->instructionName, instructionName) == 0) {
            return head->funct3;
        }
        head = head->next;
    }
    return NULL;
}

char* getInstructionFunct7(InstructionNode* head, char* instructionName) {
    while (head != NULL) {
        if (strcmp(head->instructionName, instructionName) == 0) {
            return head->funct7;
        }
        head = head->next;
    }
    return NULL;
}

char* getInstructionType(InstructionNode* head, char* instructionName) {
    while (head != NULL) {
        if (strcmp(head->instructionName, instructionName) == 0) {
            return head->type;
        }
        head = head->next;
    }
    return NULL;
}

char* getInstructionImmValue(InstructionNode* head, char* instructionName) {
    while (head != NULL) {
        if (strcmp(head->instructionName, instructionName) == 0) {
            return head->immValue;
        }
        head = head->next;
    }
    return NULL;
}

/// <summary>
/// Reads the provided RISC-V instructions CSV file, creating a linked list from its contents.
/// </summary>
/// <returns>A pointer to the head of the linked list.</returns>
InstructionNode* readInstructionsFile() {
    InstructionNode* instructionHead = NULL;

    FILE* fp;
    fopen_s(&fp, "RISC-V_Instructions.csv", "r");
    if (fp == NULL) {
        printf("Error opening instructions file");
        exit(-1);
    }

    // This seems kinda hacky, the but the end result is:
    // Read in every line, splitting it into key/value, and inserting into a dict
    int skip = true;
    int initFirst = true;
    char c;

    // todo: must fix here
    char instructionName[7] = "\0\0\0\0\0\0\0";
    char opcode[8] = "\0\0\0\0\0\0\0\0";
    char funct3[4] = "\0\0\0\0";
    char funct7[8] = "\0\0\0\0\0\0\0\0";
    char type[2] = "\0\0";
    char immValue[2] = "\0\0";

    int onKey = 0;
    int wordIndex = 0;
    while (true) {
        c = fgetc(fp); // get next character

        // Edge case for end of file
        if (c == EOF) {
            //printf("Adding value \"%s\"/\"%s\"\n", key, value);
			insertInstructionNode(instructionHead, instructionName, opcode, funct3, funct7, type, immValue);

            break;
        }

        // If at the end of a line
        if (c == '\n') {
            if (!skip) {
                // Add to list
                if (initFirst) {
                    instructionHead = insertInstructionNode(NULL, instructionName, opcode, funct3, funct7, type, immValue);
                    initFirst = false;
                }
                else {
                    insertInstructionNode(instructionHead, instructionName, opcode, funct3, funct7, type, immValue);
                }
                // clear them out to make room for the next instruction line
                resetInputPart(instructionName);
                resetInputPart(opcode);
                resetInputPart(funct3);
                resetInputPart(funct7);
                resetInputPart(type);
                resetInputPart(immValue);

                wordIndex = 0;
                onKey = 0;
            }
            else {
                // Skip first line
                resetInputPart(instructionName);
                resetInputPart(opcode);
                resetInputPart(funct3);
                resetInputPart(funct7);
                resetInputPart(type);
                resetInputPart(immValue);

                wordIndex = 0;
                onKey = 0;

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
            onKey++;
            if (onKey == 6) onKey = 0;
            continue;
        }

        // Add character to relevant string
        switch (onKey) {
        case 0:
            instructionName[wordIndex] = c;
            wordIndex++;
            if (wordIndex == 6) wordIndex = 0;
            break;
        case 1:
            opcode[wordIndex] = c;
            wordIndex++;
            if (wordIndex == 7) wordIndex = 0;
            break;
        case 2:
            funct3[wordIndex] = c;
            wordIndex++;
            if (wordIndex == 3) wordIndex = 0;
            break;
        case 3:
            funct7[wordIndex] = c;
            wordIndex++;
            if (wordIndex == 7) wordIndex = 0;
            break;
        case 4:
            type[wordIndex] = c;
            wordIndex++;
            if (wordIndex == 1) wordIndex = 0;
            break;
        case 5:
            immValue[wordIndex] = c;
            wordIndex++;
            if (wordIndex == 1) wordIndex = 0;
            break;
        }
    }
	fclose(fp);

    return instructionHead;
}
