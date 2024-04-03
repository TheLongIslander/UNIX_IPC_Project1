#include <signal.h>   // Includes signal handling library
#include <stdarg.h>   // Includes variable arguments handling library
#include <stdbool.h>  // Includes standard boolean library
#include <stdio.h>    // Includes standard input/output library
#include <stdlib.h>   // Includes standard library for memory allocation, process control, etc.
#include <sys/types.h> // Includes system types
#include <sys/wait.h> // Includes declarations for waiting functions
#include <time.h>     // Includes time handling library
#include <unistd.h>   // Includes POSIX operating system API

int L, H, PN; // Global variables for storing input parameters

// Function to generate a text file with random integers and negative values as 'keys'
void generateTextFile() {
    int* loc = malloc((L + 60) * sizeof(int)); // Allocate memory for an array of integers
    for (int i = 0; i < L + 60; i++) loc[i] = 0; // Initialize array elements to 0

    int count = 0; // Counter for negative values
    srand(time(NULL)); // Seed the random number generator
    while (count < 60) { // Loop to insert 60 random negative values
        int randomNumber = rand() % (L + 60);
        if (loc[randomNumber] == 0) {
            loc[randomNumber] = -(rand() % 60 + 1);
            count++;
        }
    }

    FILE* keys = fopen("keys.txt", "w"); // Open a file for writing
    for (int i = 0; i < L + 60; i++) {
        if (loc[i] < 0) {
            fprintf(keys, "%d\n", loc[i]); // Write negative values to file
        } else {
            fprintf(keys, "%d\n", rand() % 10000); // Write random positive values to file
        }
    }

    fclose(keys); // Close the file
    free(loc); // Free allocated memory
}

int main(int argc, char* argv[]) {
    double time_spent = 0.0;
    clock_t begin = clock(); // Start timing the execution

    // Check for correct number of arguments
    if (argc != 4) {
        printf("Not enough arguments");
        return -1;
    }

    L = atoi(argv[1]); // Convert argument 1 to integer
    H = atoi(argv[2]); // Convert argument 2 to integer
    PN = atoi(argv[3]); // Convert argument 3 to integer

    if (H < 30 || H > 60) {
        printf("H must be between 30 and 60.\n");
        return -1;
    }

    generateTextFile(); // Generate the text file with random integers and keys

    FILE* file = fopen("keys.txt", "r"); // Open the file for reading
    FILE* output;
    fclose(fopen("output.txt", "w")); // Clear or create output.txt file
    int* array = malloc((L + 60) * sizeof(int)); // Allocate memory for array
    char* line = malloc(256); // Allocate memory for line buffer

    for (int i = 0; i < (L + 60); i++) {
        fgets(line, sizeof(line), file); // Read line from file
        array[i] = atoi(line); // Convert line to integer and store in array
    }

    int fd[2 * (PN)]; // File descriptors for forward pipes
    int bd[2 * (PN)]; // File descriptors for backward pipes

    int pid; // Process ID
    int start = 0; // Start index for processing
    int end = 0; // End index for processing

    int parentRoot = getpid(); // Get parent process ID
    int returnArg = 1; // Return argument initialization

    // Loop to create child processes and perform operations
    for (int i = 0; i < PN; i++) {
        // Initiating pipes
        pipe(&fd[2 * i]); // Create forward pipe
        pipe(&bd[2 * i]); // Create backward pipe

        pid = fork(); // Fork to create a child process

        if (pid == -1) {
            perror("fork"); // Print error if fork fails
        }

        // child process
        else if (pid == 0) {
            output = fopen("output.txt", "a+"); // Open output file for appending
            printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
                   getpid(), returnArg + 1, getppid());
            fprintf(output,
                    "Hi I'm process %d with return arg %d and my parent is %d.\n",
                    getpid(), returnArg + 1, getppid());
            fclose(output); // Close the output file
            returnArg++; // Increment return argument
            start = end; // Set start for this process's chunk
            end = end + (L + 60) / PN; // Calculate end of chunk
            if (end > (L + 60)) end = (L + 60);

            // If last child, perform operations and write to pipe
            if (i == (PN - 1)) {
                close(fd[2 * i]); // Close unused write end of forward pipe
                close(bd[2 * i] + 1); // Close unused read end of backward pipe
                int max = 0;
                double avg = 0;
                int count = 0;
                // Process data and calculate max, average
                for (int j = start; j < end; j++) {
                    if (array[j] < 0) continue; // Skip negative values

                    if (array[j] > max) max = array[j]; // Find maximum
                    avg += array[j]; // Sum for average
                    count++; // Increment count
                }
                avg = avg / (double)(count); // Calculate average
                write(fd[2 * i + 1], &max, sizeof(int)); // Write max to pipe
                write(fd[2 * i + 1], &avg, sizeof(double)); // Write average to pipe
                write(fd[2 * i + 1], &count, sizeof(int)); // Write count to pipe
                close(fd[2 * i + 1]); // Close write end of forward pipe

                read(bd[2 * i], &H, sizeof(int)); // Read from backward pipe
                close(bd[2 * i]); // Close read end of backward pipe

                output = fopen("output.txt", "a+"); // Open output file for appending
                // Write found keys to output file and standard output
                for (int i = start; i < end; i++) {
                    if (H != 0 && array[i] <= -1 && array[i] >= -60) {
                        printf(
                            "Hi I am Process %d with return argument %d and I found the "
                            "hidden key at position A[%d].\n",
                            getpid(), returnArg, i);
                        fprintf(output,
                                "Hi I am Process %d with return argument %d and I found "
                                "the hidden key at position A[%d].\n",
                                getpid(), returnArg, i);
                        H--;
                    }
                }
                fclose(output); // Close the output file
                exit(0); // Exit child process
            }
        }

        else { // parent process
            int tempMax;
            double tempAvg;
            int tempCount;
            int max = 0;
            double avg = 0;
            int count = 0;

            // If not root parent, process and pass data through pipes
            if (parentRoot != getpid()) {
                close(fd[2 * i] + 1); // Close write end of current forward pipe
                close(fd[2 * (i - 1)]); // Close read end of previous forward pipe
                close(bd[2 * i]); // Close read end of current backward pipe
                close(bd[2 * (i - 1) + 1]); // Close write end of previous backward pipe

                count = 0; // Reset count
                // Process chunk and calculate max, average
                for (int j = start; j < end; j++) {
                    if (array[j] < 0) continue; // Skip negative values

                    if (array[j] > max) max = array[j]; // Find maximum
                    avg += array[j]; // Sum for average
                    count++; // Increment count
                }
                avg = avg / (double)(count); // Calculate average

                read(fd[2 * i], &tempMax, sizeof(int)); // Read max from previous child
                read(fd[2 * i], &tempAvg, sizeof(double)); // Read average from previous child
                read(fd[2 * i], &tempCount, sizeof(int)); // Read count from previous child
                close(fd[2 * i]); // Close read end of forward pipe

                if (tempMax >= max) {
                    max = tempMax; // Set new max if greater
                }
                avg = (avg * count + tempAvg * tempCount) / (double)(tempCount + count); // Calculate new average

                write(fd[2 * (i - 1) + 1], &max, sizeof(int)); // Write max to previous forward pipe
                write(fd[2 * (i - 1) + 1], &avg, sizeof(double)); // Write average to previous forward pipe
                write(fd[2 * (i - 1) + 1], &count, sizeof(int)); // Write count to previous forward pipe
                close(fd[2 * (i - 1) + 1]); // Close write end of previous forward pipe

                read(bd[2 * (i - 1)], &H, sizeof(int)); // Read H from previous backward pipe
                close(bd[2 * (i - 1)]); // Close read end of previous backward pipe

                output = fopen("output.txt", "a+"); // Open output file for appending
                // Write found keys to output file and standard output
                for (int i = start; i < end; i++) {
                    if (H != 0 && array[i] <= -1 && array[i] >= -60) {
                        printf(
                            "Hi I am Process %d with return argument %d and I found the "
                            "hidden key at position A[%d].\n",
                            getpid(), returnArg, i);
                        fprintf(output,
                                "Hi I am Process %d with return argument %d and I found "
                                "the hidden key at position A[%d].\n",
                                getpid(), returnArg, i);
                        H--;
                    }
                }
                fclose(output); // Close the output file
                write(bd[2 * (i) + 1], &H, sizeof(int)); // Write H to next backward pipe
                close(bd[2 * (i) + 1]); // Close write end of current backward pipe
                wait(NULL); // Wait for child process to terminate
                exit(0); // Exit child process
            }

            // Root parent process
            else {
                close(bd[2 * i]); // Close read end of current backward pipe
                close(fd[2 * i + 1]); // Close write end of current forward pipe
                read(fd[2 * i], &max, sizeof(int)); // Read max from last child
                read(fd[2 * i], &avg, sizeof(double)); // Read average from last child
                read(fd[2 * i], &count, sizeof(int)); // Read count from last child
                close(fd[2 * i]); // Close read end of current forward pipe

                output = fopen("output.txt", "a+"); // Open output file for appending
                fprintf(output, "Max: %d, Avg: %f\n\n", max, avg); // Write final max and average to file
                fclose(output); // Close the output file

                printf("Max: %d, Avg: %f\n\n", max, avg); // Print final max and average
                write(bd[2 * i + 1], &H, sizeof(int)); // Write H to first backward pipe
                close(bd[2 * i] + 1); // Close write end of current backward pipe

                wait(NULL); // Wait for child process to terminate

                clock_t end = clock(); // End timing the execution
                time_spent += (double)(end - begin) / CLOCKS_PER_SEC; // Calculate total time spent
                printf("\nThe program completed in %f seconds\n", (time_spent)); // Print total time spent
                exit(0); // Exit program
            }
        }
    }
}
