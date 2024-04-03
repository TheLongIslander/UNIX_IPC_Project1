#include <math.h>       // Include for mathematical functions like floor, log, etc.
#include <signal.h>     // Include for handling signals in the program
#include <stdarg.h>     // Include for using variable argument lists
#include <stdbool.h>    // Include for using boolean data type
#include <stdio.h>      // Include for input/output operations like printf, fopen, etc.
#include <stdlib.h>     // Include for general utilities like memory allocation, random numbers
#include <sys/types.h>  // Include for defining various data types used in system calls
#include <sys/wait.h>   // Include for waiting for processes to change state
#include <time.h>       // Include for handling time-related functions
#include <unistd.h>     // Include for POSIX operating system API

int L, H, PN; // Global variables L: Length, H: Threshold, PN: Process Number

void generateTextFile() {
    int* loc = malloc((L + 60) * sizeof(int)); // Allocate memory for loc array with size L+60
    for (int i = 0; i < L + 60; i++) loc[i] = 0; // Initialize all elements of loc to 0

    int count = 0; // Counter for the number of keys generated
    srand(time(NULL)); // Seed the random number generator with current time

    while (count < 60) { // Loop until 60 unique keys are generated
        int randomNumber = rand() % (L + 60); // Generate a random number within the range of loc
        if (loc[randomNumber] == 0) { // Check if the position is not already used
            loc[randomNumber] = -(rand() % 60 + 1); // Assign a unique key (negative number) to the position
            count++; // Increment the key counter
        }
    }

    FILE* keys = fopen("keys.txt", "w"); // Open keys.txt for writing
    for (int i = 0; i < L + 60; i++) { // Iterate over the loc array
        if (loc[i] < 0) 
            fprintf(keys, "%d\n", loc[i]); // Write keys (negative numbers) to the file
        else 
            fprintf(keys, "%d\n", rand() % 10000); // Write random positive integers to the file
    }

    fclose(keys); // Close the file stream
    free(loc); // Free the allocated memory for loc
}

int main(int argc, char* argv[]) {
    double time_spent = 0.0;  // Variable to store the total execution time
    clock_t begin = clock();  // Start the timer

    // Ensure the correct number of command-line arguments
    if (argc != 4) {
        printf("Not enough arguments");
        return -1;
    }

    // Assign command-line arguments to global variables
    L = atoi(argv[1]);  // Length of sequence
    H = atoi(argv[2]);  // Threshold for hidden keys
    PN = atoi(argv[3]); // Number of processes

    // Validate the threshold value
    if (H < 30 || H > 60) {
        printf("H must be between 30 and 60.\n");
        return -1;
    }

    int maxChildren;                   // To store the maximum number of child processes
    int parentRoot = getpid();         // Get the PID of the parent process
    char* input = malloc(60);         // Allocate memory for input buffer
    int h[256];                        // Array to store hidden keys
    for (int i = 0; i < 256; i++) h[i] = -1;  // Initialize array with -1
    int hstart = 0;                    // Index to track the next position in h array

    printf("How Many Children (2, 3, 4)?\n");
    scanf("%d", &maxChildren);        // Get the number of child processes from user input

    // Validate the number of child processes
    if (maxChildren == 0 || (maxChildren < 2) || maxChildren >= 5)  {
        printf("Invalid input entered\n\n");
        return -1;
    }

    generateTextFile();               // Call function to generate the text file with keys

    FILE* file = fopen("keys.txt", "r"); // Open file for reading
    int* array = malloc((L + 60) * sizeof(int)); // Allocate memory for array to store file contents
    char* line = malloc(256);         // Allocate memory for line buffer
    FILE* output;                     // File pointer for output file
    fclose(fopen("output.txt", "w")); // Clear the output file

    // Read data from file into array
    for (int i = 0; i < (L + 60); i++) {
        fgets(line, sizeof(line), file); // Read line from file
        array[i] = atoi(line);           // Convert line to integer and store in array
    }

    int result = floor(log2(PN));      // Determine the depth of process tree based on number of processes
    int returnArg = 1;                 // Initialize return argument for child processes

    pid_t childMaker;                  // PID for child making

    int childCounter = -1;             // Counter for child processes
    int fd[2 * PN];                    // File descriptors for pipes
    int pid;                           // Variable to store process ID
    int start = 0;                     // Start index for data processing
    int end = L + 60;                  // End index for data processing
    int childTrack[4] = {-1, -1, -1, -1}; // Array to track child processes
    int parentPipe = -1;               // Variable to store parent pipe index
    int oldEnd;                        // Variable to store previous end index
    int returnCode;                    // Variable to store return code of process

    // Loop to manage process creation and data processing
    for (int j = 0; j < result; j++) {
        if (j != 0) {
            // Calculate returnArg based on number of children
            if (maxChildren == 2)
                returnArg = 2 * (returnArg)-1;
            else if (maxChildren == 3)
                returnArg = 3 * (returnArg)-2;
            else
                returnArg = 4 * (returnArg)-3;
        }
        childMaker = getpid();         // Get PID of current process
        int increments = ceil((end - start) / maxChildren); // Calculate increments for child processes
        oldEnd = end;                  // Store old end index
        end = start;                   // Reset end index
        bool found = false;            // Flag to track if a suitable child is found

        // Loop to create child processes
        for (int i = 0; i < maxChildren; i++) {
            if (childMaker == getpid() && returnArg < PN) {
                childCounter++;

                for (int l = 0; l < maxChildren; l++) {
                    if (childTrack[l] == -1) {
                        childTrack[l] = returnArg - 1;
                        break;
                    }
                }

                pipe(&fd[2 * (returnArg - 1)]); // Create pipe for communication
                pid = fork();                   // Fork to create child process
                if (i != 0) start = start + increments; // Calculate new start index
                end = end + increments;         // Calculate new end index
                if ((L + 60) - end < 5) end = L + 60; // Adjust end index if necessary
                returnArg = returnArg + 1;      // Increment return argument for next child
                returnCode = returnArg;         // Set return code for child
            }
        }

        // Error handling for fork
        if (pid == -1) {
            perror("fork");
        } else if (pid == 0) {
            // Child process logic
            for (int l = 0; l < maxChildren; l++) {
                if (childTrack[l] != -1) {
                    parentPipe = childTrack[l];
                    childTrack[l] = -1;
                }
            }
            output = fopen("output.txt", "a+"); // Open output file for appending
            printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
                   getpid(), returnArg, getppid());
            fprintf(output,
                    "Hi I'm process %d with return arg %d and my parent is %d.\n",
                    getpid(), returnArg, getppid());
            fclose(output); // Close output file
            pid = getpid(); // Get PID of the current process
            if (j == (result - 1)) {
                int max = 0;               // Variable to store maximum value found
                int64_t avg = 0;            // Variable to store average
                int count = 0;              // Variable to count elements processed

                // Calculate max, average, and track hidden keys in the child process
                for (int j = start; j < end; j++) {
                    if (array[j] > max) max = array[j]; // Find maximum value
                    avg += array[j];                    // Calculate sum for average

                    // Check and store hidden keys information
                    if (array[j] >= -60 && array[j] <= -1) {
                        h[hstart] = getpid();
                        h[hstart + 1] = j;
                        h[hstart + 2] = returnCode;
                        hstart = hstart + 3;
                    }
                }

                avg = avg / (end - start);  // Compute average
                count = end - start;        // Compute count
                close(fd[2 * parentPipe]);  // Close read end of pipe
                // Write computed values to pipe
                write(fd[2 * parentPipe + 1], &max, sizeof(int));
                write(fd[2 * parentPipe + 1], &avg, sizeof(int64_t));
                write(fd[2 * parentPipe + 1], &count, sizeof(int));
                write(fd[2 * parentPipe + 1], &h, sizeof(int) * 256);
                write(fd[2 * parentPipe + 1], &hstart, sizeof(int));
                close(fd[2 * parentPipe + 1]); // Close write end of pipe

                exit(0); // Exit child process
            }
        } else {
            // Parent process logic
            bool hasChildren = false;  // Flag to check if process has children
            int childCount = 0;        // Counter for child processes
            int max = 0;               // Variable to store maximum value found
            bool tracked = false;      // Flag to check if tracking is done
            int64_t avg = 0;           // Variable to store average
            int count = 0;             // Variable to count elements processed
            int tempMax = -1;          // Temporary variable for maximum value
            int tempAvg = -1;          // Temporary variable for average
            int tempCount = -1;        // Temporary variable for count
            int tempH[256];            // Temporary array for hidden keys information

            int tempHstart = 0;        // Temporary start index for hidden keys array
            for (int r = 0; r < maxChildren; r++) {
                if (childTrack[r] != -1) {
                    hasChildren = true;
                    childCount++;
                } else if (childTrack[r] == -1 && hasChildren && !tracked) {
                    start = end;
                    end = oldEnd;
                    tracked = true;
                }
            }

            if (!hasChildren) {
                end = oldEnd;
            }

            // Logic for process with no children
            if (childCount == 0) {
                for (int j = start; j < end; j++) {
                    if (array[j] > max) max = array[j]; // Find maximum value
                    avg += array[j];                    // Calculate sum for average

                    // Check and store hidden keys information
                    if (array[j] >= -60 && array[j] <= -1) {
                        h[hstart] = getpid();
                        h[hstart + 1] = j;
                        h[hstart + 2] = returnCode;
                        hstart = hstart + 3;
                    }
                }
                avg = avg / (end - start); // Compute average
                count = end - start;       // Compute count
            } else if (childCount < maxChildren) {
                // Logic for process with fewer than maximum children
                for (int j = start; j < end; j++) {
                    if (array[j] > max) max = array[j]; // Find maximum value
                    avg += array[j];                    // Calculate sum for average

                    // Check and store hidden keys information
                    if (array[j] >= -60 && array[j] <= -1) {
                        h[hstart] = getpid();
                        h[hstart + 1] = j;
                        h[hstart + 2] = returnCode;
                        hstart = hstart + 3;
                    }
                }
                avg = avg / (end - start); // Compute average
                count = end - start;       // Compute count

                // Read data from child processes
                for (int r = 0; r < childCount; r++) {
                    read(fd[2 * childTrack[r]], &tempMax, sizeof(int));
                    read(fd[2 * childTrack[r]], &tempAvg, sizeof(int64_t));
                    read(fd[2 * childTrack[r]], &tempCount, sizeof(int));
                    read(fd[2 * childTrack[r]], &tempH, sizeof(int) * 256);
                    read(fd[2 * childTrack[r]], &tempHstart, sizeof(int));

                    if (tempMax >= max) {
                        max = tempMax;
                    }
                    int keysToAdd = (tempHstart / 3 < 2) ? tempHstart / 3 : 2;  // Determine number of keys to add
                    for (int i = 0; i < keysToAdd * 3; i++) {
                        h[hstart++] = tempH[i];
                    }
                    avg = (avg * count + tempAvg * tempCount) / (tempCount + count);
                    count += tempCount;

                    for (int i = 0; i < tempHstart; i++) {
                        h[hstart] = tempH[i];
                        hstart++;
                    }
                }
            } else {
                // Logic for process with maximum children
                for (int r = 0; r < childCount; r++) {
                    read(fd[2 * childTrack[r]], &tempMax, sizeof(int));
                    read(fd[2 * childTrack[r]], &tempAvg, sizeof(int64_t));
                    read(fd[2 * childTrack[r]], &tempCount, sizeof(int));
                    read(fd[2 * childTrack[r]], &tempH, sizeof(int) * 256);
                    read(fd[2 * childTrack[r]], &tempHstart, sizeof(int));
                    if (tempMax >= max) {
                        max = tempMax;
                    }
                    int keysToAdd = (tempHstart / 3 < 2) ? tempHstart / 3 : 2;  // Determine number of keys to add
                    for (int i = 0; i < keysToAdd * 3; i++) {
                        h[hstart++] = tempH[i];
                    }
                    avg = (avg * count + tempAvg * tempCount) / (tempCount + count);
                    for (int i = 0; i < tempHstart; i++) {
                        h[hstart] = tempH[i];
                        hstart++;
                    }
                }
            }

            if (parentRoot != getpid()) {
                // Child process writes to pipe
                write(fd[2 * parentPipe + 1], &max, sizeof(int));
                write(fd[2 * parentPipe + 1], &avg, sizeof(int64_t));
                write(fd[2 * parentPipe + 1], &count, sizeof(int));
                write(fd[2 * parentPipe + 1], &h, sizeof(int) * 256);
                write(fd[2 * parentPipe + 1], &hstart, sizeof(int));
            } else {
                // Root parent process waits for child, then writes output
                wait(NULL);
                output = fopen("output.txt", "a+");
                printf("Max: %d, Avg: %ld\n\n", max, avg);
                fprintf(output, "Max: %d, Avg: %ld\n\n", max, avg);
                for (int i = 0; i < H * 3; i += 3) {
                    printf(
                        "Hi I am Process %d with return argument %d and I found the "
                        "hidden key at position A[%d].\n",
                        h[i], h[i + 2], h[i + 1]);
                    fprintf(output,
                            "Hi I am Process %d with return argument %d and I found the "
                            "hidden key at position A[%d].\n",
                            h[i], h[i + 2], h[i + 1]);
                }
                fclose(output); // Close output file
                clock_t end = clock(); // Stop timing
                time_spent += (double)(end - begin) / CLOCKS_PER_SEC; // Calculate total execution time
                printf("\nThe program completed in %f seconds\n", (time_spent));
                exit(0); // Exit program
            }
            wait(NULL); // Wait for child processes to finish
            exit(0); // Exit program
        }
    }
}
