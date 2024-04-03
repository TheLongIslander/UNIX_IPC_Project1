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

    int count = 0;
    srand(time(NULL));
    while (count < 60) {
        int randomNumber = rand() % (L + 60);
        if (loc[randomNumber] == 0) {
            loc[randomNumber] = -(rand() % 60 + 1);
            count++;
        }
    }

    FILE* keys = fopen("keys.txt", "w");
    for (int i = 0; i < L + 60; i++) {
        if (loc[i] < 0) {
            fprintf(keys, "%d\n", loc[i]);
        } else {
            fprintf(keys, "%d\n", rand() % 10000);
        }
    }

    fclose(keys);
    free(loc);
}

int main(int argc, char* argv[]) {
  double time_spent = 0.0;
  clock_t begin = clock();
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
  // generate text file of L integers and 60 randomly placed -1's
  generateTextFile();

  FILE* file = fopen("keys.txt", "r");
  FILE* output;
  fclose(fopen("output.txt", "w"));
  int* array = (malloc((L + 60) * sizeof(int)));
  char* line = malloc(256);

  for (int i = 0; i < (L + 60); i++) {
    fgets(line, sizeof(line), file);
    array[i] = atoi(line);
  }

  /
  int fd[2 * (PN)];  
  int bd[2 * (PN)];  
  int pid;
  int start = 0;
  int end = 0;

  int parentRoot = getpid();  
  int returnArg = 1;

  

  for (int i = 0; i < PN; i++) {
    // Initiate pipes
    pipe(&fd[2 * i]);
    pipe(&bd[2 * i]);

    pid = fork();

    if (pid == -1) {
      perror("fork");
    }

    // child process
    else if (pid == 0) {
      output = fopen("output.txt", "a+");
      printf("Hi I'm process %d with return arg %d and my parent is %d.\n",
             getpid(), returnArg + 1, getppid());
      fprintf(output,
              "Hi I'm process %d with return arg %d and my parent is %d.\n",
              getpid(), returnArg + 1, getppid());
      fclose(output);
      returnArg++;
      start = end;
      end = end + (L + 60) / PN;
      if (end > (L + 60)) end = (L + 60);

      
      if (i == (PN - 1)) {
        close(fd[2 * i]);
        close(bd[2 * i] + 1);
        int max = 0;
        double avg = 0;
        int count = 0;
        for (int j = start; j < end; j++) {
          if (array[j] < 0) continue;

          if (array[j] > max) max = array[j];
          avg += array[j];
          count++;
        }
        avg = avg / (double)(count);
        write(fd[2 * i + 1], &max, sizeof(int));
        write(fd[2 * i + 1], &avg, sizeof(double));
        write(fd[2 * i + 1], &count, sizeof(int));
        close(fd[2 * i + 1]);

        read(bd[2 * i], &H, sizeof(int));
        close(bd[2 * i]);

        output = fopen("output.txt", "a+");
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
        fclose(output);
        exit(0);
      }
    }

    else {  
      // parent process
      int tempMax;
      double tempAvg;
      int tempCount;
      int max = 0;
      double avg = 0;
      int count = 0;

    
      if (parentRoot != getpid()) {
        close(fd[2 * i] + 1);
        close(fd[2 * (i - 1)]);
        close(bd[2 * (i)]);
        close(bd[2 * (i - 1) + 1]);

        count = 0;
        for (int j = start; j < end; j++) {
        
          if (array[j] < 0) continue;

          if (array[j] > max) max = array[j];
          avg += array[j];
          count++;
        }
        avg = avg / (double)(count);

        read(fd[2 * i], &tempMax, sizeof(int));
        read(fd[2 * i], &tempAvg, sizeof(double));
        read(fd[2 * i], &tempCount, sizeof(int));
        close(fd[2 * i]);

        if (tempMax >= max) {
          max = tempMax;
        }
        avg = (avg * count + tempAvg * tempCount) / (double)(tempCount + count);

        write(fd[2 * (i - 1) + 1], &max, sizeof(int));
        write(fd[2 * (i - 1) + 1], &avg, sizeof(double));
        write(fd[2 * (i - 1) + 1], &count, sizeof(int));
        close(fd[2 * (i - 1) + 1]);

        read(bd[2 * (i - 1)], &H, sizeof(int));
        close(bd[2 * (i - 1)]);

        output = fopen("output.txt", "a+");
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
        fclose(output);
        write(bd[2 * (i) + 1], &H, sizeof(int));
        close(bd[2 * (i) + 1]);
        wait(NULL);
        exit(0);

      }

      
      else {
        
        close(bd[2 * i]);
        close(fd[2 * i + 1]);
        read(fd[2 * i], &max, sizeof(int));
        read(fd[2 * i], &avg, sizeof(double));
        read(fd[2 * i], &count, sizeof(int));
        close(fd[2 * i]);
        output = fopen("output.txt", "a+");
        fprintf(output, "Max: %d, Avg: %f\n\n", max, avg);
        fclose(output);
        printf("Max: %d, Avg: %f\n\n", max, avg);
        write(bd[2 * i + 1], &H, sizeof(int));
        close(bd[2 * i] + 1);
        wait(NULL);
        clock_t end = clock();
        time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
        printf("\nThe program completed in %f seconds\n", (time_spent));
        exit(0);
      }
    }
  }
}
