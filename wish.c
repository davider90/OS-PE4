#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

// Uncomment to have the shell print the interpreted input.
// #define ECHO

// Global variables
char *tokens[50];
char input[200];
int redirection;

// Executes the tokenized command. Fork first
void execute(int length) {

    // Make array of arguments for execvp
    char *arguments[length + 1];
    for (int i = 0; i < length; i++) {
        arguments[i] = tokens[i];
    }
    arguments[length] = NULL;

    execvp(
        arguments[0], 
        arguments
    );
    if (errno != 0) {
        perror("execvp");
        printf("Error code %i.", errno);
    }
}

void io(char *type, char *path, int length) {

    /*  The return value of open() is a file descriptor, a small,
    *   nonnegative integer.
    * 
    *   dup2(2) is used to redirect standard input (0) standard output (1) or error (2)
    *   Example: dup2("file descriptor", 1); // Writes what would usually be printed to the file.
    */

    printf("Got here!");

    if (strcmp(type, ">") == 0) {
        
        // Open file
        int file_desc = open(path, O_CREAT | O_RDWR);
        
        // –––––––––––––––––––– Error handling ––––––––––––––––––––
        if (file_desc < 0) {
            printf("Error opening the file\n"); 
        }
        if (errno != 0) {
            perror("open");
            printf("Error code %i.", errno);
            errno = 0;
        }

        // Redirection of output to file
        dup2(file_desc, 1);
        close(file_desc);

        // –––––––––––––––––––– Error handling ––––––––––––––––––––
        if (errno != 0) {
            perror("dup");
            printf("Error code %i.", errno);
            errno = 0;
        }

        // Execute current tokens, now with redirection
        execute(length);
    }
    else if (strcmp(type, "<") == 0) {
        
        // Open file
        int file_desc = open(path, O_CREAT | O_RDWR);
        
        // –––––––––––––––––––– Error handling ––––––––––––––––––––
        if (file_desc < 0) {
            printf("Error opening the file\n"); 
        }
        if (errno != 0) {
            perror("open");
            printf("Error code %i.", errno);
            errno = 0;
        }

        // Redirection of input from file
        dup2(file_desc, 0);
        close(file_desc);

        // –––––––––––––––––––– Error handling ––––––––––––––––––––
        if (errno != 0) {
            perror("dup");
            printf("Error code %i.", errno);
            errno = 0;
        }

        // Execute current tokens, now with redirection
        execute(length);
    }
    else if (strcmp(type, ">>") == 0) {
        // MaTODO: Handle appending to file
        
    }
}

// Splits input into tokens, stores it in the global variable tokens
// and returns the index of last token + 1.
int tokenize() {
    int i = 0;
    
    // Reset redirection flag
    redirection = 0;
    
    // Get first token
    char *token = strtok(input, " ");

    // Get the remaining tokens if any
    while (token != NULL) {

        // Check if there is a redirection
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || strcmp(token, ">>") == 0) {
            redirection = 1;
        }
        
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    
    // This is normally the index of tokens' first NULL,
    // but if tokens was entirely filled, i will be
    // equal to the size of tokens (50, that is).
    return i;
}

// Prompt the user for input and store it in the global variable input.
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
        // First get input
        scanInput();
        
        // Tokenize
        int i = tokenize();
        
        pid_t childID = fork();
        if (errno != 0) {
			perror("fork");
			exit(errno);
		}
        printf("pid: %d\n", (int)childID);
        if (childID == 0) {
            printf("sanity check\n");
            printf("redir: %d\n", redirection);
            if (redirection == 1) {
                printf("Heisan: %c, %c\n", tokens[i-2], tokens[i-1]);
                io(tokens[i-2], tokens[i-1], i);
            }
            // else {
            //     printf("got her instead????");
            //     execute(i);
            // }
            // exit(0);
        }
        // Print tokens
        int n = 0;
        while (n < i) {
            printf("Token %d: %s\n", n, tokens[n++]);
        }

        // Fork
        
        // Execute if we're a child process
        
        // Wait for child process to finish, then continue
        wait(NULL);
		if (errno != 0) {
			perror("wait");
			exit(errno);
		}
    }
}

// The WISh main
int main(int argc, char **argv) {
    errno = 0;
    
    printf("Welcome to WISh (Woefully Inadequate Shell) -- make a wish. ;)\n");
    printf("WARNING: Input may not exceed 200 characters.\n");
    printf("WARNING: A single argument may not exceed 50 characters.\n");
    loop();
    
    return 0;
    
}
