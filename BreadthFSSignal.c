#include <math.h>       // Include for mathematical functions like floor, ceil, log, etc.
#include <signal.h>     // Include for signal handling utilities
#include <stdarg.h>     // Include for variable argument functions
#include <stdbool.h>    // Include for boolean type
#include <stdio.h>      // Include for input/output operations
#include <stdlib.h>     // Include for standard library functions, like malloc and atoi
#include <sys/types.h>  // Include for system data type definitions
#include <sys/wait.h>   // Include for waiting for processes to terminate
#include <time.h>       // Include for time-related functions
#include <unistd.h>     // Include for POSIX operating system API

int L, H, PN; // Global variables to hold command line argument values

// Function to generate a file with random numbers and 'keys'
void generateTextFile() {
    int* loc = malloc((L + 60) * sizeof(int)); // Allocate memory for array to store random numbers and keys
    for (int i = 0; i < L + 60; i++) loc[i] = 0; // Initialize the array with zeros

    int c = 0; // Counter for generated keys
    srand(time(NULL)); // Seed the random number generator

    // Generate 60 unique keys
    while (c < 60) {
        int RN = rand() % (L + 60);
        if (loc[RN] == 0) {
            loc[RN] = -(rand() % 60 + 1); // Assign a negative value as a key
            c++;
        }
    }

    FILE* keys = fopen("keys.txt", "w"); // Open or create file for writing
    for (int i = 0; i < L + 60; i++) {
        fprintf(keys, "%d\n", loc[i]); // Write each integer in the array to the file
    }

    fclose(keys); // Close the file stream
    free(loc); // Free the allocated memory for the array
}

int main(int argc, char* argv[]) {
    double time_elapsed = 0.0; // Variable to calculate total execution time
    clock_t startTiming = clock(); // Record start time

    // Validate command line arguments
    if (argc != 4) {
        printf("Not enough arguments");
        return -1; // Exit if not exactly 4 arguments
    }

    // Parse command line arguments
    L = atoi(argv[1]); // Length of sequence
    H = atoi(argv[2]); // Threshold value
    PN = atoi(argv[3]); // Number of processes

    // Validate threshold value
    int childMax; // Variable to hold the maximum number of child processes
    int pRoot = getpid(); // Get the process ID of the current (parent) process
    char* input = malloc(60); // Allocate memory for input buffer
    int h[256]; // Array to hold hidden key information
    for (int i = 0; i < 256; i++) h[i] = -1; // Initialize array with -1
    int hIndex = 0; // Index for hidden key information array

    printf("How Many Children (2, 3, 4)?\n");
    scanf("%d", &childMax); // Read number of child processes from user input

    // Validate user input for number of child processes
    if (childMax == 0 || (childMax < 2) || childMax >= 5)  {
        printf("Invalid input entered\n\n");
        return -1;
    }

    generateTextFile(); // Generate the text file with keys and numbers

    // Open keys.txt file for reading generated keys and numbers
    FILE* file = fopen("keys.txt", "r");
    int* array = malloc((L + 60) * sizeof(int)); // Allocate memory for array to hold file contents
    char* line = malloc(256); // Allocate memory for line buffer
    FILE* output;
    fclose(fopen("output.txt", "w")); // Clear or create output.txt

    // Read file contents into array
    for (int i = 0; i < (L + 60); i++) {
        fgets(line, sizeof(line), file); // Read a line from file
        array[i] = atoi(line); // Convert line to integer and store in array
    }

    // Calculate depth of process tree
    int result = floor(log2(PN));
    int returnArguement = 1; // Initialize return argument for child processes

    pid_t childCreater; // Variable to store PID for child creation

    int numOfChild = -1; // Counter for child processes
    int fd[2 * PN]; // File descriptors for pipes
    int pid; // Variable to store process ID
    int start = 0; // Start index for processing
    int end = L + 60; // End index for processing

    // Array to track child processes
    int trackChildren[4] = {-1, -1, -1, -1};
    int pPipe = -1; // Variable to store parent pipe index
    int endingOld; // Variable to store old end index
    int rCode; // Variable to store return code of process

    // Loop for creating and managing child processes
    for (int j = 0; j < result; j++) {
        if (j != 0) {
            // Update returnArguement based on childMax
            if (childMax == 2)
                returnArguement = 2 * (returnArguement)-1;
            else if (childMax == 3)
                returnArguement = 3 * (returnArguement)-2;
            else
                returnArguement = 4 * (returnArguement)-3;
        }
        childCreater = getpid(); // Get current process ID
        int inc = ceil((end - start) / childMax); // Calculate increment for data processing
        endingOld = end; // Store old end index
        end = start; // Reset end index for next loop

        // Create child processes and set up their data range
        for (int i = 0; i < childMax; i++) {
            if (childCreater == getpid() && returnArguement < PN) {
                numOfChild++;

                // Find available slot in trackChildren array
                for (int l = 0; l < childMax; l++) {
                    if (trackChildren[l] == -1) {
                        trackChildren[l] = returnArguement - 1;
                        break;
                    }
                }

                pipe(&fd[2 * (returnArguement - 1)]); // Create pipe for communication
                pid = fork(); // Fork to create a new process
                if (i != 0) start = start + inc; // Calculate start index for child
                end = end + inc; // Calculate end index for child
                if ((L + 60) - end < 5) end = L + 60; // Adjust end index if needed
                returnArguement = returnArguement + 1; // Update returnArguement for next child
                rCode = returnArguement; // Set return code for child process
            }
        }

        // Error handling for fork()
        if (pid == -1) {
            perror("fork"); // Print error if fork fails
        } else if (pid == 0) {
            // Child process logic
            for (int l = 0; l < childMax; l++) {
                if (trackChildren[l] != -1) {
                    pPipe = trackChildren[l]; // Find parent pipe index
                    trackChildren[l] = -1; // Reset child track
                }
            }
            output = fopen("output.txt", "a+"); // Open output file for appending
            printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
                   getpid(), returnArguement, getppid()); // Print process info
            fprintf(output,
                    "Hi I'm process %d with return arg %d and my parent is %d.\n",
                    getpid(), returnArguement, getppid()); // Write process info to output file
            fclose(output); // Close output file

            // Process data in the last level of child processes
            if (j == (result - 1)) {
                int max = 0; // Variable to store maximum value found
                int64_t average = 0; // Variable to store average
                int c = 0; // Variable to count elements processed

                // Calculate max, average, and track hidden keys
                for (int j = start; j < end; j++) {
                    if (array[j] > max) max = array[j]; // Find maximum value
                    average += array[j]; // Sum values for average calculation

                    // Check and record hidden keys
                    if (array[j] >= -60 && array[j] <= -1) {
                        h[hIndex] = getpid(); // Store process ID
                        h[hIndex + 1] = j; // Store index of key
                        h[hIndex + 2] = rCode; // Store return code
                        hIndex += 3; // Move to next position in hidden keys array
                    }
                }

                average = average / (end - start); // Calculate average
                c = end - start; // Calculate count of numbers processed
                close(fd[2 * pPipe]); // Close read end of pipe

                // Write calculated values to pipe for parent process to read
                write(fd[2 * pPipe + 1], &max, sizeof(int));
                write(fd[2 * pPipe + 1], &average, sizeof(int64_t));
                write(fd[2 * pPipe + 1], &c, sizeof(int));
                write(fd[2 * pPipe + 1], &h, sizeof(int) * 256);
                write(fd[2 * pPipe + 1], &hIndex, sizeof(int));
                close(fd[2 * pPipe + 1]); // Close write end of pipe

                exit(0); // Exit child process
            }
        } else {
            // Parent process logic
            bool isThereChildren = false; // Flag to indicate if process has children
            int numberOfChildren = 0; // Counter for number of children
            int max = 0; // Variable to store maximum value found
            bool isBeingTracked = false; // Flag to indicate if tracking is complete
            int64_t average = 0; // Variable to store average
            int c = 0; // Variable to count elements processed
            int tempMax = -1; // Temporary variable for maximum value
            int tempaverage = -1; // Temporary variable for average
            int tempc = -1; // Temporary variable for count
            int tempH[256]; // Temporary array for hidden keys information

            int temphIndex = 0; // Temporary index for hidden keys array
            for (int r = 0; r < childMax; r++) {
                if (trackChildren[r] != -1) {
                    isThereChildren = true;
                    numberOfChildren++;
                } else if (trackChildren[r] == -1 && isThereChildren && !isBeingTracked) {
                    start = end; // Set start to end for next data chunk
                    end = endingOld; // Reset end to old end
                    isBeingTracked = true; // Set isBeingTracked flag
                }
            }

            if (!isThereChildren) {
                end = endingOld; // Reset end if no children
            }

            // Logic for processes with no children
            if (numberOfChildren == 0) {
                for (int j = start; j < end; j++) {
                    if (array[j] > max) max = array[j]; // Find maximum value
                    average += array[j]; // Sum values for average calculation

                    // Check and record hidden keys
                    if (array[j] >= -60 && array[j] <= -1) {
                        h[hIndex] = getpid(); // Store process ID
                        h[hIndex + 1] = j; // Store index of key
                        h[hIndex + 2] = rCode; // Store return code
                        hIndex += 3; // Move to next position in hidden keys array
                    }
                }
                average = average / (end - start); // Calculate average
                c = end - start; // Calculate count of numbers processed
            } else if (numberOfChildren < childMax) {
                // Logic for processes with fewer children than maximum
                for (int j = start; j < end; j++) {
                    if (array[j] > max) max = array[j]; // Find maximum value
                    average += array[j]; // Sum values for average calculation

                    // Check and record hidden keys
                    if (array[j] >= -60 && array[j] <= -1) {
                        h[hIndex] = getpid(); // Store process ID
                        h[hIndex + 1] = j; // Store index of key
                        h[hIndex + 2] = rCode; // Store return code
                        hIndex += 3; // Move to next position in hidden keys array
                    }
                }
                average = average / (end - start); // Calculate average
                c = end - start; // Calculate count of numbers processed

                // Read data from child processes and combine results
                for (int r = 0; r < numberOfChildren; r++) {
                    read(fd[2 * trackChildren[r]], &tempMax, sizeof(int));
                    read(fd[2 * trackChildren[r]], &tempaverage, sizeof(int64_t));
                    read(fd[2 * trackChildren[r]], &tempc, sizeof(int));
                    read(fd[2 * trackChildren[r]], &tempH, sizeof(int) * 256);
                    read(fd[2 * trackChildren[r]], &temphIndex, sizeof(int));

                    if (tempMax > max) max = tempMax; // Update max if needed
                    average = (average * c + tempaverage * tempc) / (tempc + c); // Combine averages
                    c += tempc; // Update count

                    // Append hidden keys from child to parent's array
                    for (int i = 0; i < temphIndex; i++) {
                        h[hIndex++] = tempH[i];
                    }
                }
            } else {
                // Logic for processes with maximum number of children
                for (int r = 0; r < numberOfChildren; r++) {
                    if (r != 0) {
                        read(fd[2 * trackChildren[r]], &tempMax, sizeof(int));
                        read(fd[2 * trackChildren[r]], &tempaverage, sizeof(int64_t));
                        read(fd[2 * trackChildren[r]], &tempc, sizeof(int));
                        read(fd[2 * trackChildren[r]], &tempH, sizeof(int) * 256);
                        read(fd[2 * trackChildren[r]], &temphIndex, sizeof(int));
                        if (tempMax > max) max = tempMax; // Update max if needed
                        average = (average * c + tempaverage * tempc) / (tempc + c); // Combine averages
                        for (int i = 0; i < temphIndex; i++) {
                            h[hIndex++] = tempH[i];
                        }
                    } else {
                        read(fd[2 * trackChildren[r]], &max, sizeof(int));
                        read(fd[2 * trackChildren[r]], &average, sizeof(int64_t));
                        read(fd[2 * trackChildren[r]], &c, sizeof(int));
                        read(fd[2 * trackChildren[r]], &h, sizeof(int) * 256);
                        read(fd[2 * trackChildren[r]], &hIndex, sizeof(int));
                    }
                }
            }

            // Write combined results to pipe or output file
            if (pRoot != getpid()) {
                // Child process writes results to pipe for parent to read
                write(fd[2 * pPipe + 1], &max, sizeof(int));
                write(fd[2 * pPipe + 1], &average, sizeof(int64_t));
                write(fd[2 * pPipe + 1], &c, sizeof(int));
                write(fd[2 * pPipe + 1], &h, sizeof(int) * 256);
                write(fd[2 * pPipe + 1], &hIndex, sizeof(int));
            } else {
                // Root parent process writes results to output file
                wait(NULL); // Wait for child processes to complete
                output = fopen("output.txt", "a+");
                printf("Max: %d, Avg: %ld\n\n", max, average);
                fprintf(output, "Max: %d, Avg: %ld\n\n", max, average);
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
                clock_t end = clock(); // Stop the timer
                time_elapsed += (double)(end - startTiming) / CLOCKS_PER_SEC; // Calculate total execution time
                printf("\nThe program completed in %f seconds\n", (time_elapsed));
                exit(0); // Exit the program
            }
            wait(NULL); // Wait for child processes to complete
            exit(0); // Exit the program
        }
    }
}
