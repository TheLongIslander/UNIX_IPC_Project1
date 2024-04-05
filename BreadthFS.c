#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int L, H, PN;

void generateTextFile() {
    // Allocate memory for an integer array 'loc' with a size of L+60 elements.
    int* loc = malloc((L + 60) * sizeof(int));
    // Initialize all elements of 'loc' to 0.
    for (int i = 0; i < L + 60; i++) loc[i] = 0;

    int c = 0;
    // Seed the random number generator with the current time.
    srand(time(NULL));

    // Populate 'loc' with 60 unique negative random numbers between -1 and -60.
    while (c < 60) {
        int RN = rand() % (L + 60);
        if (loc[RN] == 0) {
            loc[RN] = -(rand() % 60 + 1);
            c++;
        }
    }

    // Open a file named "keys.txt" for writing.
    FILE* keys = fopen("keys.txt", "w");
    // Write the numbers from 'loc' to the file. Negative numbers are written directly,
    // while non-negative numbers are replaced with random numbers between 0 and 9999.
    for (int i = 0; i < L + 60; i++) {
        if (loc[i] < 0) fprintf(keys, "%d\n", loc[i]);
        else fprintf(keys, "%d\n", rand() % 10000);
    }

    // Close the file and free the allocated memory for 'loc'.
    fclose(keys);
    free(loc);
}

int main(int argc, char* argv[]) {
    double time_elapsed = 0.0;
    clock_t startTiming = clock(); // Start the clock to measure execution time

    // Check if the correct number of command-line arguments is provided
    if (argc != 4) {
        printf("Not enough arguments\n");
        return -1; // Exit with error code -1 if incorrect number of arguments
    }

    // Convert command-line arguments to integers and assign them to global variables
    L = atoi(argv[1]);
    H = atoi(argv[2]);
    PN = atoi(argv[3]);

    // Validate the range of H
    if (H < 30 || H > 60) {
        printf("H must be between 30 and 60.\n");
        return -1; // Exit with error code -1 if H is out of the valid range
    }

    int childMax;
    int pRoot = getpid(); // Get the process ID of the current process
    char* input = malloc(60); // Allocate memory for a character array
    int h[256]; // Array to store some form of data, initialized to -1
    for (int i = 0; i < 256; i++) h[i] = -1;
    int hIndex = 0;

    // Prompt the user for the number of child processes
    printf("How Many Children (2, 3, 4)?\n");
    scanf("%d", &childMax); // Read the user's input

    // Validate the user's input for the number of children
    if (childMax == 0 || (childMax < 2) || childMax >= 5) {
        printf("Invalid input entered\n\n");
        return -1; // Exit with error code -1 if input is invalid
    }

    printf("\n");

    // Generate a text file with random numbers
    generateTextFile();

    // Open the generated text file for reading
    FILE* file = fopen("keys.txt", "r");
    // Allocate memory for an array to hold the integers from the file
    int* array = malloc((L + 60) * sizeof(int));
    char* line = malloc(256); // Allocate memory for reading lines from the file
    FILE* output;

    // Open and close the file "output.txt" to clear it or create it if it doesn't exist
    fclose(fopen("output.txt", "w"));

    // Read integers from the file and store them in the array
    for (int i = 0; i < (L + 60); i++) {
        fgets(line, sizeof(line), file);
        array[i] = atoi(line);
    }

    // Calculate a value used to determine the depth of process creation
    int result = floor(log2(PN));
    int returnArguement = 1; // Initialize a counter for the return argument

    pid_t childMaker; // Variable to hold the process ID

    int numOfChild = -1; // Counter for the number of child processes created
    int fd[2 * PN]; // Array to store file descriptors for pipes
    int pid; // Variable to store process ID
    int start = 0; // Start index for processing segments of the array
    int end = L + 60; // End index for processing segments of the array
  // Starting the BFS (Breadth-First Search) tree
int trackChildren[4] = {-1, -1, -1, -1}; // Array to keep track of the child process IDs
int pPipe = -1; // Variable to hold the pipe file descriptor for the parent process
int endingOld; // Variable to store the previous end value in the loop
int rCode; // Variable to hold the return code for each process

// Loop through 'result' times to create a tree of processes
for (int j = 0; j < result; j++) {
    // Adjust returnArguement based on the iteration and the number of maximum children allowed
    if (j != 0) {
        if (childMax == 2)
            returnArguement = 2 * (returnArguement) - 1;
        else if (childMax == 3)
            returnArguement = 3 * (returnArguement) - 2;
        else
            returnArguement = 4 * (returnArguement) - 3;
    }

    childMaker = getpid(); // Get the current process ID to check if in parent or child
    int inc = ceil((end - start) / childMax); // Calculate the range of data each child will process
    endingOld = end; // Store the end value before changing it for the next iteration
    end = start; // Reset end to start for next calculation

    // Iterate over the number of children to be created
    for (int i = 0; i < childMax; i++) {
        if (childMaker == getpid() && returnArguement < PN) {
            numOfChild++; // Increment the counter for each child created

            // Find a free spot in trackChildren array to mark the child process
            for (int l = 0; l < childMax; l++) {
                if (trackChildren[l] == -1) {
                    trackChildren[l] = returnArguement - 1; // Mark the spot with returnArguement - 1
                    break;
                }
            }

            pipe(&fd[2 * (returnArguement - 1)]); // Create a pipe for communication between parent and child
            pid = fork(); // Fork the process to create a child

            // Update start and end for the next child's data range
            if (i != 0) start += inc;
            end += inc;

            // Ensure we don't go beyond the array length
            if ((L + 60) - end < 5) end = L + 60;

            returnArguement++; // Increment returnArguement for next potential child
            rCode = returnArguement; // Set the rCode for this process
        }
    }

    // Check for fork failure
    if (pid == -1) {
        perror("fork"); // Print error if fork fails
   } else if (pid == 0) {
    // This block is executed by the child process
    for (int l = 0; l < childMax; l++) {
        if (trackChildren[l] != -1) {
            pPipe = trackChildren[l]; // Assign the pipe of the parent to pPipe
            trackChildren[l] = -1; // Reset the child's tracking to indicate the child is now handling its task
        }
    }

    output = fopen("output.txt", "a+"); // Open the output file in append mode
    printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
           getpid(), returnArguement, getppid()); // Print to standard output the process details
    fprintf(output, "Hi I'm process %d with return arg %d and my parent is %d.\n",
            getpid(), returnArguement, getppid()); // Write the same details to the output file
    fclose(output); // Close the file after writing to it

    pid = getpid(); // Get the current process ID, although this seems redundant as pid is already the child's process ID

    if (j == (result - 1)) { // If this is the last iteration of the loop
        int max = 0; // Variable to keep track of the maximum value
        int64_t average = 0; // Variable to calculate the average
        int c = 0; // Counter for the number of elements processed

        // Loop over the assigned range of elements in the array
        for (int j = start; j < end; j++) {
            if (array[j] > max) max = array[j]; // Update max if a larger value is found
            average += array[j]; // Add to average for average calculation

            // Check if the value is within the special range and track it
            if (array[j] >= -60 && array[j] <= -1) {
                h[hIndex] = getpid(); // Store the process ID
                h[hIndex + 1] = j; // Store the index of the value
                h[hIndex + 2] = rCode; // Store the return code
                hIndex += 3; // Move to the next available position in h
            }
        }

        average = average / (end - start); // Calculate the average
        c = end - start; // Calculate the count of processed elements

        // Close the read end of the pipe and write the computed values to the write end
        close(fd[2 * pPipe]);
        write(fd[2 * pPipe + 1], &max, sizeof(int));
        write(fd[2 * pPipe + 1], &average, sizeof(int64_t));
        write(fd[2 * pPipe + 1], &c, sizeof(int));
        write(fd[2 * pPipe + 1], &h, sizeof(int) * 256);
        write(fd[2 * pPipe + 1], &hIndex, sizeof(int));
        close(fd[2 * pPipe + 1]); // Close the write end of the pipe

        exit(0); // Terminate the child process
    }
} else { // The beginning of the else block that handles the parent process logic
      // parent process logic
bool isThereChildren = false; // Flag to check if the process has any children
int childc = 0; // Counter for the number of children
int max = 0; // Variable to hold the maximum value found
bool isBeingTracked = false; // Flag to check if the tracking has started
int64_t average = 0; // Variable to calculate the average
int c = 0; // Variable to count the number of elements processed
int tempMax = -1; // Temporary variable to hold the maximum value for comparison
int tempaverage = -1; // Temporary variable to hold the average value for comparison
int tempc = -1; // Temporary variable to hold the count for comparison
int tempH[256]; // Temporary array to hold hidden values

int temphIndex = 0; // Start index for the temporary hidden values array
for (int r = 0; r < childMax; r++) {
    if (trackChildren[r] != -1) {
        isThereChildren = true; // Set isThereChildren to true if any child is tracked
        childc++; // Increment the child counter
    } else if (trackChildren[r] == -1 && isThereChildren && !isBeingTracked) {
        // If no more children are tracked and tracking has not been flagged yet
        start = end; // Set start to end to prepare for the next range of processing
        end = endingOld; // Reset end to endingOld for processing the next segment
        isBeingTracked = true; // Set tracked flag to true after tracking the first child
    }
}

if (!isThereChildren) {
    end = endingOld; // Reset end to endingOld if no children are present
}

// Process the segment of data if no children were forked or if there are fewer children than maximum
if (childc == 0) { // If no children have been forked for this process
    for (int j = start; j < end; j++) {
        if (array[j] > max) max = array[j]; // Find the maximum value in the segment
        average += array[j]; // Sum up the values for average calculation

        // Track special values within the specified range
        if (array[j] >= -60 && array[j] <= -1) {
            h[hIndex] = getpid(); // Store the process ID
            h[hIndex + 1] = j; // Store the index of the element
            h[hIndex + 2] = rCode; // Store the return code
            hIndex += 3; // Move to the next set of values in the hidden array
        }
    }
    average = average / (end - start); // Calculate the average for the segment
    c = end - start; // Count the number of elements processed
} else if (childc < childMax) {
    // If there are fewer children than the maximum, process the remaining data
    for (int j = start; j < end; j++) {
        if (array[j] > max) max = array[j]; // Find the maximum value
        average += array[j]; // Sum up the values for average calculation

        // Track special values within the specified range
        if (array[j] >= -60 && array[j] <= -1) {
            h[hIndex] = getpid(); // Store the process ID
            h[hIndex + 1] = j; // Store the index of the element
            h[hIndex + 2] = rCode; // Store the return code
            hIndex += 3; // Move to the next set of values in the hidden array
        }
    }
    average = average / (end - start); // Calculate the average for the segment
    c = end - start; // Count the number of elements processed
        for (int r = 0; r < childc; r++) {
          // PIPE CALLED ONE
          read(fd[2 * trackChildren[r]], &tempMax, sizeof(int));
          read(fd[2 * trackChildren[r]], &tempaverage, sizeof(int64_t));
          read(fd[2 * trackChildren[r]], &tempc, sizeof(int));
          read(fd[2 * trackChildren[r]], &tempH, sizeof(int) * 256);
          read(fd[2 * trackChildren[r]], &temphIndex, sizeof(int));

          if (tempMax >= max) {
            max = tempMax;
          }
         int keysToAdd = (temphIndex / 3 < 2) ? temphIndex / 3 : 2;  // Assuming each hidden key has 3 related integers
        for (int i = 0; i < keysToAdd * 3; i++) {
            h[hIndex++] = tempH[i];
        }
          average = (average * c + tempaverage * tempc) / (tempc + c);
          c += tempc;

          for (int i = 0; i < temphIndex; i++) {
            h[hIndex] = tempH[i];
            hIndex++;
          }
        }
      } else {
        for (int r = 0; r < childc; r++) {
          // PIPE CALLED TWO
          if (r != 0) {
            read(fd[2 * trackChildren[r]], &tempMax, sizeof(int));
            read(fd[2 * trackChildren[r]], &tempaverage, sizeof(int64_t));
            read(fd[2 * trackChildren[r]], &tempc, sizeof(int));
            read(fd[2 * trackChildren[r]], &tempH, sizeof(int) * 256);
            read(fd[2 * trackChildren[r]], &temphIndex, sizeof(int));
            if (tempMax >= max) {
              max = tempMax;
            }
            int keysToAdd = (temphIndex / 3 < 2) ? temphIndex / 3 : 2;  // Assuming each hidden key has 3 related integers
        for (int i = 0; i < keysToAdd * 3; i++) {
            h[hIndex++] = tempH[i];
        }
            average = (average * c + tempaverage * tempc) / (tempc + c);
            for (int i = 0; i < temphIndex; i++) {
              h[hIndex] = tempH[i];
              hIndex++;
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

      if (pRoot != getpid()) {
        // PIPE CALLED THREE
        write(fd[2 * pPipe + 1], &max, sizeof(int));
        write(fd[2 * pPipe + 1], &average, sizeof(int64_t));
        write(fd[2 * pPipe + 1], &c, sizeof(int));
        write(fd[2 * pPipe + 1], &h, sizeof(int) * 256);
        write(fd[2 * pPipe + 1], &hIndex, sizeof(int));
      } else {
        wait(NULL);
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
        fclose(output);
        clock_t end = clock();
        time_elapsed += (double)(end - startTiming) / CLOCKS_PER_SEC;
        printf("\nThe program completed in %f seconds\n", (time_elapsed));
        exit(0);
      }
      wait(NULL);
      exit(0);
    }
  }
}
