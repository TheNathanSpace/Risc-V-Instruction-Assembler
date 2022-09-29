#pragma once
#include <string.h>
#define true 1
#define false 0

void resetInputPart(char* array);

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

// Read Registers.csv, returning the head of the list
RegisterNode* readRegistersFile() {
    RegisterNode* registerHead = NULL;

    FILE* fp;
    fopen_s(&fp, "Registers.csv", "r");
    if (fp == NULL) {
        printf("Error opening registers file");
        exit(-1);
    }

    // This seems kinda hacky, the but the end result is:
    // Read in every line, splitting it into key/value, and inserting into a dict
    int skip = true;
    int initFirst = true;
    char c;
    int onKey = true;
    char key[4] = "\0\0\0\0";
    char value[5] = "\0\0\0\0\0";
    int wordIndex = 0;
    while (true) {
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

    return registerHead;
}
