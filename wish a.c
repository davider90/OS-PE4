#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

// Global variables
char *tokens[50];
char input[200];
bool redirectionFlag = false;
char redirection = NULL;

// Splits input into tokens, stores it in the global variable tokens
// and returns the index of last token + 1.
int tokenize() {
    int i = 0;
    
    // Get first token
    char *token = strtok(input, " ");

    // Reset redirection flag and direction
    redirectionFlag = false;
    redirection = NULL;

    // Get the remaining tokens if any
    while (token != NULL) {

        // Check if there is a redirection
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
            redirectionFlag = true;
            redirection = *token;

            // Get the redirection path
            token = strtok(NULL, " ");

            // Printing the type and direction, skip them for the final tokenized file
            printf("Redirection type: %c\n", redirection);
            printf("Direction: %s\n", token);
            
            // TODO: Do something with the redirection
            break;
        }
        else {
            tokens[i++] = token;
            token = strtok(NULL, " ");
        }
    }
    
    // This is normally the index of tokens' first NULL,
    // but if tokens was entirely filled, i will be
    // equal to the size of tokens (50, that is).
    return i;
}

// Prompts the user for input and stores it in the global variable input.
void scanInput() {
    // Get input
    printf("$ ");
    fgets(input, sizeof(input), stdin);

    // Remove newline character at end of last argument
    int length = strlen(input);
    input[length-1] = '\0';
}

// The shell loop
void loop() {
    while (1) {
        scanInput();
        int i = tokenize();
        int n = 0;
        while (n < i) {
            printf("Token %i: %s\n", n, tokens[n++]);
        }
    }
}

// The WISh main
int main(int argc, char **argv) {
    printf("Welcome to WISh (Woefully Inadequate Shell) -- make a wish. ;)\n");
    printf("WARNING: Input may not exceed 200 characters.\n");
    printf("WARNING: A single argument may not exceed 50 characters.\n");
    loop();
    
    return 0;
}
