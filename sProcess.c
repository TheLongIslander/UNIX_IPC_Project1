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
    // Initializes Array of Locations
    int* loc = malloc((L + 60) * sizeof(int));
    for (int i = 0; i < L + 60; i++) {
        loc[i] = 0;
    }

    int count = 0;
    srand(time(NULL));
    int randomNumber;

    // Picks random indexes to be one of the values between -1 and -60 (No Duplicates)
    while (count < 60) {
        randomNumber = rand() % (L + 60);
        if (loc[randomNumber] == 0) {
            // Assign a value between -1 and -60
            loc[randomNumber] = -(rand() % 60 + 1);
            count++;
        }
    }

    // Creates/Overwrites a text file named keys
    FILE* keys;
    keys = fopen("keys.txt", "w");

    for (int i = 0; i < (L + 60); i++) {
        // Generates a random positive integer.
        randomNumber = rand() % 10000;

        // If the corresponding index to the array has a hidden key, place it in the file
        if (loc[i] < 0) {
            fprintf(keys, "%d\n", loc[i]);
        }
        // Otherwise, place the randomly generated positive number.
        else {
            fprintf(keys, "%d\n", randomNumber);
        }
    }

    // Close file and free allocated memory.
    fclose(keys);
    free(loc);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <L> <PN>\n", argv[0]);
        return -1;
    }

    L = atoi(argv[1]);
    PN = atoi(argv[2]);

    // Prompt user for H value
    printf("Enter the number of hidden keys to find (30-60): ");
    scanf("%d", &H);
    if (H < 30 || H > 60) {
        printf("Invalid number of hidden keys. Please enter a value between 30 and 60.\n");
        return -1;
    }

    generateTextFile();

    int64_t avg = 0;
    int64_t validNumbers = 0;

    int keyCount = 0;
    int max = 0;  // Can be 0 since no negative ints aside from hidden keys.
    FILE* file = fopen("keys.txt", "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    char* line = malloc(256);
    if (!line) {
        perror("Memory allocation error");
        fclose(file);
        return -1;
    }

    // clear out output file if it exists then open it for writing
    FILE* output = fopen("output.txt", "w");
    if (!output) {
        perror("Error opening output file");
        free(line);
        fclose(file);
        return -1;
    }

    while (fgets(line, 256, file) && keyCount < H) {
        int num = atoi(line);
        if (num > max) max = num;
        if (num >= 0) {
            avg += num;
            validNumbers++;
        } else if (num < 0) { // Assumes negative numbers are hidden keys
            keyCount++;
            fprintf(output, "Hi I am process %d with return arg 1. I found the hidden key in position A[%lld].\n", getpid(), validNumbers);
            printf("Found hidden key at index %lld\n", validNumbers);
        }
    }

    free(line);
    fclose(file);

    if (validNumbers > 0) {
        avg /= validNumbers;  // Calculate the average by dividing the sum by the number of valid numbers.
    }

    printf("Max: %d\nAverage: %lld\n", max, avg);
    fprintf(output, "Max: %d\nAverage: %lld\n", max, avg);
    fclose(output);
}
