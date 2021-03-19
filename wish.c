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
bool redirection;
char *dir;

// Executes the tokenized command
void execute(int length) {

    // Check if in ECHO mode
    #ifdef ECHO

    // Somewhat ugly way to print stuff nicely
    printf("DEBUG: File to execute: %s\n", tokens[0]);
    if (length > 1) {
        int i = 1;
        printf("DEBUG: Arguemnts: ");
        while (i < length-1) {
            printf("%s, ", tokens[i++]);
        }
        printf("%s\n", tokens[length-1]);
    } else {
        printf("DEBUG: No arguments provided.\n");
    }

    #else

    // Make array of arguments for execvp
    char *arguments[length + 1];
    for (int i = 0; i < length; i++) {
        arguments[i] = tokens[i];
    }
    arguments[length] = NULL;

    // Set directory if specified
    // if (dir != NULL) {
    //     char *temp;
    //     strcat(temp, dir);
    //     strcat(temp, "/");
    //     strcat(temp, arguments[0]);
    //     arguments[0] = temp;
    // }
    // TODO: Mulig ikke dette de mener?

    // Execute
    execvp(arguments[0], arguments);
    if (errno != 0) {
        printf("ERROR: Could not execute %s.\n", arguments[0]);
        perror("execvp");
        printf("Error code %i\n", errno);
        errno = 0;
    }

    #endif
}

// Handles I/O redirection
void io(char *type, char *path, int length) {

    /*  The return value of open() is a file descriptor, a small,
    *   nonnegative integer.
    * 
    *   dup2(2) is used to redirect standard input (0) standard output (1) or error (2)
    *   Example: dup2("file descriptor", 1); // Writes what would usually be printed to the file.
    */

    // Check if in ECHO mode
    #ifdef ECHO

    // Print whatever's fitting
    printf("DEBUG: I/O redirection -- ");
    if (strcmp(type, ">") == 0) {
        printf("overwrite output (>)\n");
    } else if (strcmp(type, ">>") == 0) {
        printf("append output (>>)\n");
    } else if (strcmp(type, "<") == 0) {
        printf("overwrite input (<)\n");
    } else {
        printf("append input (<<)\n");
    }
    printf("DEBUG: To/from '%s'\n", path);

    #else

    if (strcmp(type, ">") == 0 || strcmp(type, "<") == 0) {
        
        
        
        // Open file
        int file_desc = open(path, O_CREAT | O_RDWR);
        
        // –––––––––––––––––––– Error handling ––––––––––––––––––––
        if (file_desc < 0) {
            printf("ERROR: Could not access '%s'.\n", path); 
        }
        if (errno != 0) {
            perror("open");
            printf("Error code %i\n", errno);
            errno = 0;
        }

        // Redirection of I/O to file
        if (strcmp(type, ">") == 0) {
            dup2(file_desc, 1);
        } else {
            dup2(file_desc, 0);
        }
        close(file_desc);

        // Error handling
        if (errno != 0) {
            printf("ERROR: Could not redirect I/O.\n");
            perror("dup");
            printf("Error code %i\n", errno);
            errno = 0;
        }

        // Execute
        execute(length);

    } else if (strcmp(type, ">>") == 0) {
        // MaTODO: Handle appending to file  
    }

    #endif
}

// Splits input into tokens, stores it in the global variable tokens
// and returns the index of last token + 1.
int tokenize() {
    int i = 0;
    
    // Reset redirection flag
    redirection = false;
    
    // Get first token
    char *token = strtok(input, " ");

    // This is the most practical place to check for internal commands.
    if (strcmp("exit", token) == 0) {
        printf("INFO: Thank you for using WISh!\n");
        printf("NOTICE: Exiting...\n");
        exit(0);
    } else if (strcmp("cd", token) == 0) {
        dir = strtok(NULL, " ");
        printf("INFO: Changed directory to '%s'\n", dir);
        return 0;
    }

    // Get the remaining tokens if any
    while (token != NULL) {

        // Check if there is a redirection
        if (strcmp(token, "<") == 0 ||
            strcmp(token, ">") == 0 ||
            strcmp(token, ">>") == 0) {
            redirection = true;
        }
        
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    
    // This is normally the index of tokens' first NULL,
    // but if tokens was entirely filled, i will be
    // equal to the size of tokens (50, that is). Can also be 0.
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
        
        // If we have something to do...
        if (i > 0) {

            // Fork
            pid_t childID = fork();
            if (errno != 0) {
                printf("ALERT: Error occured while forking. Exiting...\n");
                perror("fork");
                exit(errno);
            }

            // Execute if we're a child process
            if (childID == 0) {
                if (redirection) {
                    io(tokens[i-2], tokens[i-1], i-2);
                } else {
                    execute(i);
                }
                exit(0);
            }

            // Wait for child process to finish, then continue
            wait(NULL);
            if (errno != 0) {
                printf("ALERT: Error occured while waiting for child process. Exiting...\n");
                perror("wait");
                exit(errno);
            }
        }
    }
}

// The WISh main
int main(int argc, char **argv) {
    errno = 0;
    
    printf("INFO: Welcome to WISh (Woefully Inadequate Shell) -- make a wish. ;)\n");
    printf("WARNING: Input may not exceed 200 characters.\n");
    printf("WARNING: A single argument may not exceed 50 characters.\n");
    loop();
    
    return 0;
}
