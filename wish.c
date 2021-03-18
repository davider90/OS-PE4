#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h> 

// Global variables
char *tokens[50];
char input[200];

void io(char arguments[]) {

    /*  The return value of open() is a file descriptor, a small,
    *   nonnegative integer that is used in subsequent system calls
    *   (read(2), write(2), lseek(2), fcntl(2), etc.) to refer to the
    *   open file.
    *   must include one of the following access
    *   modes: O_RDONLY, O_WRONLY, or O_RDWR.
    *   return value of open() is a file descriptor
    *    
    *   dup(2) copies the file descriptor.
    */

    //Maybe TODO: Split arguments into before "<" and after.

    for (int i = 0; i < sizeof(arguments); i++) {

        if (strcmp(arguments[i], "<") == 0) {
            int file_desc = open(arguments[i+1], O_CREAT | O_RDWR);
            // –––––––––––––––––––– Error handling ––––––––––––––––––––
            if (file_desc < 0) {
                printf("Error opening the file\n"); 
            }
            if (errno != 0) {
                perror("open");
                printf("Error code %i.", errno);
                errno = 0;
            }

            /* int copy = dup(file_desc);
            if (errno != 0) {
                perror("dup");
                printf("Error code %i.", errno);
                errno = 0;
            } */


            read(file_desc, arguments[i-1], sizeof(arguments[i-1]));
            // –––––––––––––––––––– Error handling ––––––––––––––––––––
            if (errno != 0) {
                perror("read");
                printf("Error code %i.", errno);
                errno = 0;
            }
        }
        else if (strcmp(arguments[i], ">") == 0) {
            int file_desc = open(arguments[i+1], O_CREAT | O_RDWR);
            // –––––––––––––––––––– Error handling ––––––––––––––––––––
            if (file_desc < 0) {
                printf("Error opening the file\n"); 
            }
            if (errno != 0) {
                perror("open");
                printf("Error code %i.", errno);
                errno = 0;
            }

            write(file_desc, arguments[i-1], sizeof(arguments[i-1]));
            // –––––––––––––––––––– Error handling ––––––––––––––––––––
            if (errno != 0) {
                perror("write");
                printf("Error code %i.", errno);
                errno = 0;
            }
        }
        else if (strcmp(arguments[i], ">>") == 0) {

        }
    }
}

// Splits input into tokens, stores it in the global variable tokens
// and returns the index of last token + 1.
int tokenize() {
    int i = 0;
    
    // Get first token
    char *token = strtok(input, " ");

    // Get the remaining tokens if any
    while (token != NULL) {

        /* Check if there is a redirection
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
            redirection = *token;

            // Get the redirection path
            token = strtok(NULL, " ");

            // Printing the type and direction, skip them for the final tokenized file
            printf("Redirection type: %c\n", redirection);
            printf("Direction: %s\n", token);
            
            // TODO: Do something with the redirection

            //io(tokens[i-1], redirection);
            break; 
        } */
        //else {
        tokens[i++] = token;
        token = strtok(NULL, " ");
        //}
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

// Executes the tokenized command. Fork first
void execute(int length) {

    // Reset redirection flag and direction
    bool redirectionFlag = false;

    // Make array of arguments for execvp
    char *arguments[length + 1];
    for (int i = 0; i < length; i++) {
        arguments[i] = tokens[i];

        if (strcmp(tokens[i], "<") == 0 || (strcmp(tokens[i], ">") == 0 {
            redirectionFlag = true;
        }
    }
    arguments[length] = NULL;
    
    // Execute either I/O or execute
    if (redirectionFlag) {
        io(arguments);
    }
    else {
        execvp(
            arguments[0], 
            arguments
        );
        if (errno != 0) {
			perror("execvp");
			printf("Error code %i.", errno);
		}
    }
}

// The shell loop
void loop() {
    while (1) {
        // First get input
        scanInput();
        
        // Tokenize
        int i = tokenize();
        
        // Print tokens
        int n = 0;
        while (n < i) {
            printf("Token %d: %s\n", n, tokens[n++]);
        }

        // Fork
        pid_t childID = fork();
        if (errno != 0) {
			perror("fork");
			exit(errno);
		}
        
        // Execute if we're a child process
        if (childID == 0) {
            execute(i);
            // execute(i);
            exit(0);
        }
        
        // Wait for child process to finish, then continue
        pid_t zombie = waitpid(-1, NULL, WNOHANG);
		if (errno != 0) {
			perror("waitpid");
			exit(errno);
		}
	
		while (zombie > 0) {
			zombie = waitpid(-1, NULL, WNOHANG);
			if (errno != 0) {
				perror("waitpid");
				exit(errno);
			}
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
