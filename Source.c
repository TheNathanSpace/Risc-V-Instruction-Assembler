#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dict.h"
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
	printf("Entered print function\n");
	for (int i = 0; i < 4; i++) {
		printf("%s", parts[i]);
		printf("\n");
	}
}

int main() {
	// Allocate space for the instruction parts
	char** instructionParts = malloc(sizeof(char*) * 4);		// Array of arrays of chars. 4 is the max command parts
	for (int i = 0; i < 4; i++) {
		instructionParts[i] = malloc(sizeof(char) * 11);		// room for 10 characters per part + \0
	}

	int enteredParts = 0;
	printf("Enter an instruction: \n");
	char inputCommand[11];										// room for 10 characters per part + \0 (automatically added)
	inputCommand[0] = '\0';
	int result;
	
	// While true, keep reading words from input
	while (1) {
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

	// Now that the instruction is split up into its parts, you can move foward
	dict registersDict = new_dict();
	
	FILE* fp;
	fopen_s(&fp, "Registers.csv", "r");
	if (fp == NULL) {
		printf("Error opening registers file");
		exit(-1);
	}

	int skip = true;
	char c;
	int onKey = true;
	char key[4] = "\0\0\0\0";
	char value[5] = "\0\0\0\0\0";
	int wordIndex = 0;
	int i = 0;
	while (true) {
		i++;
		c = fgetc(fp); // get next character
		if (c == EOF) { 
			printf("Reached end of file\n");
			break;
		}

		// Skip if newline character
		if (c == '\n') {
			if (!skip) {
				printf("Adding value \"%s\"/\"%s\"\n", key, value);
				setkey(key, value, &registersDict);
				resetInputPart(key);
				resetInputPart(value);
				wordIndex = 0;
				onKey = true;
			}
			else {
				resetInputPart(key);
				resetInputPart(value);
				wordIndex = 0;
				onKey = true;

				skip = false;
			}
			printf("\n");
			continue;
		}
		printf("char #%d: ", i);
		printf("%c\n", c);

		// Skip if still the first line
		if (skip == true) {
			continue;
		}

		// toggle key/value
		if (c == ',') {
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

	printf("Closed file\n");

	char* val = getvalue("x4", registersDict);
	printf("Got value\n");
	if (val == NULL) {
		printf("Error: dict value is NULL\n");
		exit(-1);
	}
	printf(val);
	printf("\n");

	// deallocate the instruction parts array
	for (int i = 0; i < 4; i++) {
		free(instructionParts[i]);
	}
	free(instructionParts);
	free_dict(&registersDict);

	return 0;
}