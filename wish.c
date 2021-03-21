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
char **tokens; // Why in heck double **. TODO: find out
char *input;
size_t i_size;
bool redirection;
//// SUGGESTION:
bool double_redirection;

/*  Need a way of saving commands until they all should be executed.
*   Solution: Global list of pointers to different token lists
*   
*   char *scripts;
*/


// Executes the tokenized command
void execute(int length) {

    // Check if in ECHO mode
    #ifdef ECHO

    // Somewhat ugly way to print stuff nicely
    printf("DEBUG: File to execute: %s\n", tokens[0]);
    if (length > 1) {
        int i = 1;
        printf("DEBUG: Arguments: ");
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

    // Execute
    execvp(arguments[0], arguments);
    if (errno != 0) {
        printf("ERROR: Could not execute %s.\n", arguments[0]);
        perror("execvp");
        printf("Error code %d\n", errno);
        errno = 0;
    }

    #endif
}

// Handles I/O redirection
void io(char *type, char *path, int length) {

    // Check if in ECHO mode
    #ifdef ECHO

    // Print whatever's fitting
    printf("DEBUG: I/O redirection: ");
    if (strcmp(type, ">") == 0) {
        printf("overwrite output (>)\n");
    } else {
        printf("overwrite input (<)\n");
    }
    printf("DEBUG: To/from: '%s'\n", path);

    #else
    
    // Open file (creates one if it doesn't exist)
    int file_desc = open(path, O_CREAT | O_RDWR, S_IRWXU);

    
    // Error handling
    if (file_desc < 0) {
        printf("ERROR: Could not access '%s'.\n", path); 
    }
    if (errno != 0) {
        perror("open");
        printf("Error code %d\n", errno);
        errno = 0;
    }

    // Redirection of I/O to file
    dup2(file_desc, strcmp(type, ">") == 0 ? 1 : 0);
    close(file_desc);

    // Error handling
    if (errno != 0) {
        printf("ERROR: Could not redirect I/O.\n");
        perror("dup2");
        printf("Error code %d\n", errno);
        errno = 0;
    }

    #endif

    // Execute
    //// SUGGESTION: If in "double-redirection" mode we should not execute yet.
    if (!double_redirection) {
        execute(length);
    }
}

// Splits input into tokens, stores it in the global variable tokens
// and returns the index of last token + 1.
int tokenize() {
    int i = 0;
    
    // Reset redirection flag
    redirection = false;
    //// SUGGESTION:
    double_redirection = false;
    
    // Get first token
    char *token = strtok(input, " ");

    // This is the most practical place to check for internal commands.
    if (token != NULL) {

        // Exit if we're supposed to
        if (strcmp("exit", token) == 0) {
            printf("INFO: Thank you for using WISh!\n");
            printf("NOTICE: Exiting...\n");
            exit(0);
        }
        // Change directory if we're supposed to
        else if (strcmp("cd", token) == 0) {
            char *dir = strtok(NULL, " ");
            if (chdir(dir) > 0) {
                printf("ERROR: Could not change directory.\n");
                perror("chdir");
                printf("Error code %d\n", errno);
                errno = 0;
            } else {
                // Get current path for info to the user
                char *buffer = NULL;
                char *path = getcwd(buffer, 0);
                if (path == NULL) {
                    printf("ERROR: Could not get current directory for some reason.\n");
                    perror("getcwd");
                    printf("Error code %d\n", errno);
                    errno = 0;
                }
                printf("INFO: Changed directory to '%s'\n", path);
                free(path);
            }
            return 0;
        }
    }

    // Get the remaining tokens if any
    while (token != NULL) {

        // Check if there is a redirection
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
                        
            /// SUGGESTION: check if its the first or second "<"/">" token - if redirection == true && "<"/">"
            //// If it is: Set a new DOUBLE redirection flag. Se io() and loop().
            if (redirection) {
                double_redirection = true;
            }
            
            redirection = true;
        }

        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    
    // This is normally the index of tokens' first NULL,
    // but if tokens was entirely filled, i will be
    // equal to the size of tokens. Can also be 0.
    return i;
}

// Prompt the user for input and store it in the global variable input.
void scanInput() {
    // Reset tokens, input and i_size
    free(tokens);
    free(input);
    i_size = 0;

    // Get input
    printf("$ ");
    i_size = getline(&input, &i_size, stdin);
    // Error handling
    if (i_size < 0) {
        printf("ALERT: Reading input failed. Exiting...\n");
        perror("getline");
        exit(errno);
    }
    *tokens = (char *)malloc(i_size*i_size*sizeof(char));
    // Error handling
    if (tokens == NULL) {
        printf("ALERT: Memory allocation for tokens failed. Exiting...\n");
        perror("malloc");
        exit(errno);
    }
    
    // Remove newline character at end of last argument
    input[i_size-1] = '\0';
}

// The shell loop
void loop() {
    while (1) {
        // First get input
        scanInput();
        
        // Tokenize
        int i = tokenize();
        
        /*  Scripting: Maybe here:
        *   Check if first char is #?
        *   If it is current tokensss should be pointed to by our new super command token holder.
        *   Also rest of function should be stalled / not be executed.
        * 
        *   If our super command token holder is ready to be executed because of some command yet unknown,
        *   for all tokens pointers: run rest of function.
        *   
        *   if (tokens[0] == "#") {
        *       scripts.apppendwaddup(tokens);
        *       break; exit; somthin;
        *   } else if (tokens[0] == "command yet unknown") {
        *       for (tokens in scrips) {
        *           run the rest of code. how. Maybe put rest of function in another one called "fork" or "run" or "bitchspray"
        *       }
        *   } else {
        *       Do normal stuff. Run "bitchspray".
        *   }
        *   
        */
        
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
                
                //// SUGGESTION: 
                if (double_redirection) {
                    io(tokens[i-2], tokens[i-1], 0);
                    double_redirection = false; //"Double redirection" mode is done. Go back to normal.
                    io(tokens[i-4], tokens[i-3], i-4);
                } else if (redirection) {
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

    // Define variables
    errno = 0;
    input = malloc(0);
    tokens = malloc(0);
    
    printf("INFO: Welcome to WISh (Woefully Inadequate Shell) -- make a wish. ;)\n");
    printf("WARNING: Input may not exceed 200 characters.\n");
    printf("WARNING: A single argument may not exceed 50 characters.\n");
    loop();
    
    return 0;
}
