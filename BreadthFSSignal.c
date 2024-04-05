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
    int* loc = malloc((L + 60) * sizeof(int));
    for (int i = 0; i < L + 60; i++) loc[i] = 0;

    int c = 0;
    srand(time(NULL));

    while (c < 60) {
        int RN = rand() % (L + 60);
        if (loc[RN] == 0) {
            loc[RN] = -(rand() % 60 + 1); // Values between -1 and -60
            c++;
        }
    }

    FILE* keys = fopen("keys.txt", "w");
    for (int i = 0; i < L + 60; i++) {
        if (loc[i] < 0) fprintf(keys, "%d\n", loc[i]);
        else fprintf(keys, "%d\n", rand() % 10000);
    }

    fclose(keys);
    free(loc);
}

int main(int argc, char* argv[]) {
  double c = 0.0;
  clock_t startTiming = clock();
  if (argc != 4) {
    printf("Not enough arguements");
    return -1;
  }

  L = atoi(argv[1]);
  H = atoi(argv[2]);
  PN = atoi(argv[3]);
  if (H < 30 || H > 60) {
        printf("H must be between 30 and 60.\n");
        return -1;
    }
  int childMax;
  int pRoot = getpid();
  char* input = malloc(60);
  int h[256];
  for (int i = 0; i < 256; i++) h[i] = -1;
  int hIndex = 0;
  printf("How Many Children (2, 3, 4)?\n");
  scanf("%d", &childMax);  // getting input from user

  // if the input is not valid
  if (childMax == 0 || (childMax < 2) || childMax >= 5)  {
    printf("Invalid input entered\n\n");
    return -1;
  }
  printf("\n");

  generateTextFile();

  FILE* file = fopen("keys.txt", "r");
  int* array = (malloc((L + 60) * sizeof(int)));
  char* line = malloc(256);
  FILE* output;
  fclose(fopen("output.txt", "w"));
  for (int i = 0; i < (L + 60); i++) {
    fgets(line, sizeof(line), file);
    array[i] = atoi(line);
  }
  int result = floor(log2(PN));
  int returnArguement = 1;


  pid_t childMaker;

  int numOfChild = -1;
  int fd[2 * PN];
  int pid;
  int start = 0;
  int end = L + 60;
  // Starting the BFS tree
  int trackingChildren[4] = {-1, -1, -1, -1};
  int pPipe = -1;
  int endingOld;
  int rCode;
  for (int j = 0; j < result; j++) {
    if (j != 0) {
      if (childMax == 2)
        returnArguement = 2 * (returnArguement)-1;
      else if (childMax == 3)
        returnArguement = 3 * (returnArguement)-2;
      else
        returnArguement = 4 * (returnArguement)-3;
    }
    childMaker = getpid();
    int inc;
    inc = ceil((end - start) / childMax);
    endingOld = end;
    end = start;
    bool isFound = false;
    for (int i = 0; i < childMax; i++) {
      if (childMaker == getpid() && returnArguement < PN) {
        numOfChild++;

        for (int l = 0; l < childMax; l++) {
          if (trackingChildren[l] == -1) {
            trackingChildren[l] = returnArguement - 1;
            break;
          }
        }

        pipe(&fd[2 * (returnArguement - 1)]);
        pid = fork();
        if (i != 0) start = start + inc;
        end = end + inc;
        if ((L + 60) - end < 5) end = L + 60;
        returnArguement = returnArguement + 1;
        rCode = returnArguement;
      }
    }

    if (pid == -1) {
      perror("fork");
    } else if (pid == 0) {
      // child process
      for (int l = 0; l < childMax; l++) {
        if (trackingChildren[l] != -1) {
          pPipe = trackingChildren[l];
          trackingChildren[l] = -1;
        }
      }
      output = fopen("output.txt", "a+");
      printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
             getpid(), returnArguement, getppid());
      fprintf(output,
              "Hi I'm process %d with return arg %d and my parent is %d.\n",
              getpid(), returnArguement, getppid());
      fclose(output);
      pid = getpid();
      if (j == (result - 1)) {
        int max = 0;
        int64_t average = 0;
        int c = 0;
        for (int j = start; j < end; j++) {
          if (array[j] > max) max = array[j];
          average += array[j];

          if (array[j] >= -60 && array[j] <= -1) {
            h[hIndex] = getpid();
            h[hIndex + 1] = j;
            h[hIndex + 2] = rCode;
            hIndex = hIndex + 3;
          }
        }

        average = average / (end - start);
        c = end - start;
        close(fd[2 * pPipe]);
        write(fd[2 * pPipe + 1], &max, sizeof(int));
        write(fd[2 * pPipe + 1], &average, sizeof(int64_t));
        write(fd[2 * pPipe + 1], &c, sizeof(int));
        write(fd[2 * pPipe + 1], &h, sizeof(int) * 256);
        write(fd[2 * pPipe + 1], &hIndex, sizeof(int));
        close(fd[2 * pPipe + 1]);

        exit(0);
      }
    } else {  
      // parent process 
      bool isThereChildren = false;
      int childc = 0;
      int max = 0;
      bool isBeingTracked = false;
      int64_t average = 0;
      int c = 0;
      int tempMax = -1;
      int tempaverage = -1;
      int tempc = -1;
      int tempH[256];

      int temphIndex = 0;
      for (int r = 0; r < childMax; r++) {
        if (trackingChildren[r] != -1) {
          isThereChildren = true;
          childc++;
        } else if (trackingChildren[r] == -1 && isThereChildren && !isBeingTracked) {
          start = end;
          end = endingOld;
          isBeingTracked = true;
        }
      }

      if (!isThereChildren) {
        end = endingOld;
      }

      // If process with no children somehow ends up here
      if (childc == 0) {
        for (int j = start; j < end; j++) {
          if (array[j] > max) max = array[j];
          average += array[j];
          if (array[j] >= -60 && array[j] <= -1) {
            h[hIndex] = getpid();
            h[hIndex + 1] = j;
            h[hIndex + 2] = rCode;
            hIndex = hIndex + 3;
          }
        }
        average = average / (end - start);
        c = end - start;
      } else if (childc < childMax) {
        // If a process with not the max children somehow ends up here
        for (int j = start; j < end; j++) {
          if (array[j] > max) max = array[j];
          average += array[j];
          if (array[j] >= -60 && array[j] <= -1) {
            h[hIndex] = getpid();
            h[hIndex + 1] = j;
            h[hIndex + 2] = rCode;
            hIndex = hIndex + 3;
          }
        }
        average = average / (end - start);
        c = end - start;

        for (int r = 0; r < childc; r++) {
          read(fd[2 * trackingChildren[r]], &tempMax, sizeof(int));
          read(fd[2 * trackingChildren[r]], &tempaverage, sizeof(int64_t));
          read(fd[2 * trackingChildren[r]], &tempc, sizeof(int));
          read(fd[2 * trackingChildren[r]], &tempH, sizeof(int) * 256);
          read(fd[2 * trackingChildren[r]], &temphIndex, sizeof(int));

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
          if (r != 0) {
            read(fd[2 * trackingChildren[r]], &tempMax, sizeof(int));
            read(fd[2 * trackingChildren[r]], &tempaverage, sizeof(int64_t));
            read(fd[2 * trackingChildren[r]], &tempc, sizeof(int));
            read(fd[2 * trackingChildren[r]], &tempH, sizeof(int) * 256);
            read(fd[2 * trackingChildren[r]], &temphIndex, sizeof(int));
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
            read(fd[2 * trackingChildren[r]], &max, sizeof(int));
            read(fd[2 * trackingChildren[r]], &average, sizeof(int64_t));
            read(fd[2 * trackingChildren[r]], &c, sizeof(int));
            read(fd[2 * trackingChildren[r]], &h, sizeof(int) * 256);
            read(fd[2 * trackingChildren[r]], &hIndex, sizeof(int));
          }
        }
      }

      if (pRoot != getpid()) {
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
              "Hi I am Process %d with return argument %d and I isFound the "
              "hidden key at position A[%d].\n",
              h[i], h[i + 2], h[i + 1]);
          fprintf(output,
                  "Hi I am Process %d with return argument %d and I isFound the "
                  "hidden key at position A[%d].\n",
                  h[i], h[i + 2], h[i + 1]);
        }
        fclose(output);
        clock_t end = clock();
        c += (double)(end - startTiming) / CLOCKS_PER_SEC;
        printf("\nThe program completed in %f seconds\n", (c));
        exit(0);
      }
      wait(NULL);
      exit(0);
    }
  }
}
