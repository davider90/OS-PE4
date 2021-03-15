#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// Global variables
char *tokens[50];
char input[200];

// Splits input into tokens, stores it in the global variable tokens
// and returns the index of last token + 1.
int tokenize() {
    int i = 0;
    
    // Get first token
    char *token = strtok(input, " ");

    // Get the rest if any
    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
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

// Executes the tokenized command. Fork first
void execute(int length) {

    // execl means execute list
    // execv means execute vector
    // execlp means execute list with path added
    // add e to add environment

    // Make array of arguments for execvp
    char *arguments[length + 1];
    for (int i = 0; i < length; i++) {
        arguments[i] = tokens[i];
    }
    arguments[length] = NULL;
    
    // Execute
    execvp(arguments[0], arguments);
}

// The shell loop
void loop() {
    while (1) {
        scanInput();
        int i = tokenize();
        int n = 0;
        while (n < i) {
            printf("%s\n", tokens[n++]);
        }
        execute(i);
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
