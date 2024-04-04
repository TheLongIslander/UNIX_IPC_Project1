#include <signal.h>    // For handling interrupts
#include <stdarg.h>    // For functions with variable number of arguments
#include <stdbool.h>   // For using 'true' and 'false' values
#include <stdio.h>     // For file operations and input/output
#include <stdlib.h>    // For standard library functions like malloc, free, and atoi
#include <sys/types.h> // For system-specific data types
#include <sys/wait.h>  // For process waiting mechanisms
#include <time.h>      // For time functions like time()
#include <unistd.h>    // For POSIX operating system API

int L, H, PN; // Global variables to hold the length, number of hidden keys, and process number

// Function to generate a text file with random numbers and hidden keys
void generateTextFile() {
    int* loc = malloc((L + 60) * sizeof(int)); // Allocate memory for the array to hold numbers and keys
    for (int i = 0; i < L + 60; i++) {
        loc[i] = 0; // Initialize array elements to 0
    }

    int c = 0; // Counter for hidden keys
    srand(time(NULL)); // Seed the random number generator
    int RN;

    // Loop to generate unique hidden keys
    while (c < 60) {
        RN = rand() % (L + 60); // Generate a random index
        if (loc[RN] == 0) { // Ensure the index is not already used for a hidden key
            loc[RN] = -(rand() % 60 + 1); // Assign a negative number as a hidden key
            c++; // Increment the hidden key counter
        }
    }

    // Open (or create) the file 'keys.txt' for writing
    FILE* keys = fopen("keys.txt", "w");

    // Write hidden keys and random positive numbers to the file
    for (int i = 0; i < (L + 60); i++) {
        RN = rand() % 10000; // Generate a random positive number
        if (loc[i] < 0) {
            fprintf(keys, "%d\n", loc[i]); // Write a hidden key
        } else {
            fprintf(keys, "%d\n", RN); // Write a random positive number
        }
    }

    fclose(keys); // Close the file
    free(loc); // Free the allocated memory for the array
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <L> <PN>\n", argv[0]); // Prompt for correct usage if wrong argument count
        return -1;
    }

    L = atoi(argv[1]); // Convert argument to integer for L
    PN = atoi(argv[2]); // Convert argument to integer for PN

    printf("Enter the number of hidden keys to find (30-60): ");
    scanf("%d", &H); // Read the number of hidden keys from user input
    if (H < 30 || H > 60) {
        printf("Invalid number of hidden keys. Please enter a value between 30 and 60.\n");
        return -1; // Check for valid range of hidden keys
    }

    generateTextFile(); // Call the function to generate the file with random numbers and hidden keys

    int64_t average = 0; // Variable to calculate average
    int64_t validNums = 0; // Counter for valid (non-hidden-key) numbers

    int keyCounter = 0; // Counter for found hidden keys
    int max = 0; // Variable to track the maximum number

    FILE* file = fopen("keys.txt", "r"); // Open the file for reading
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    char* line = malloc(256); // Allocate memory for line buffer
    if (!line) {
        perror("Memory allocation error");
        fclose(file);
        return -1;
    }

    FILE* output = fopen("output.txt", "w"); // Open (or create) the output file for writing
    if (!output) {
        perror("Error opening output file");
        free(line);
        fclose(file);
        return -1;
    }

    // Read from the file and process each number
    while (fgets(line, 256, file) && keyCounter < H) {
        int num = atoi(line); // Convert line to integer
        if (num > max) max = num; // Check and update maximum number
        if (num >= 0) {
            average += num; // Add to average calculation
            validNums++; // Increment counter of valid numbers
        } else if (num < 0) { // Check if number is a hidden key
            keyCounter++; // Increment hidden key counter
            fprintf(output, "Hi I am process %d with return arg 1. I found the hidden key in position A[%lld].\n", getpid(), validNums);
            printf("Found hidden key at index %lld\n", validNums);
        }
    }

    free(line); // Free the allocated memory for line buffer
    fclose(file); // Close the file

    // Calculate and print the average if there are valid numbers
    if (validNums > 0) {
        average /= validNums; // Calculate average
    }

    printf("Max: %d\nAverage: %lld\n", max, average); // Print maximum and average
    fprintf(output, "Max: %d\nAverage: %lld\n", max, average); // Write maximum and average to output file
    fclose(output); // Close the output file
}
