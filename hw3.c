#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Process {
  int jobId;
  int processId;
  char *status;
  char *command;
  // char **args;
} Process;

char *getCmd() {
  char *buffer;
  size_t bufsize = 32;
  int tmp;

  buffer = (char *)malloc(bufsize * sizeof(char));
  if (buffer == NULL) {
    perror("Unable to allocate buffer");
    exit(1);
  }

  printf("> ");
  tmp = getline(&buffer, &bufsize, stdin);
  buffer[tmp - 1] = '\0';
  return buffer;
}

char **getArgs(char *buffer) {
  // size_t bufsize = 32;
  // char *t = strtok(buffer, " ");
  // int tmp = 10;
  // char **x = (char **)malloc(tmp * sizeof(char *));
  // for (int i = 0; i < tmp; i++) {
  //   x[i] = (char *)malloc(bufsize * sizeof(char));
  // }
  // int i = 0;
  // while (t != NULL) {
  //   strcpy(x[i], t);
  //   // printf("%s\n", t);
  //   t = strtok(NULL, " ");
  //   i++;
  // }

  // char **arg = (char **)malloc(i * sizeof(char *));
  // for (int i = 0; i < tmp; i++) {
  //   arg[i] = (char *)malloc(bufsize * sizeof(char));
  // }
  // int z;
  // for (z = 0; z < i; z++) {
  //   strcpy(arg[z], x[z]);
  // }
  // arg[z] = NULL;
  // /*for(int y = 0; y <= i; y++) {
  //       printf("%s, y=%d :: ", arg[y], y);
  //     }*/
  // for (int i = 0; i < tmp; i++) {
  //   free(x[i]);
  // }
  // free(x);
  // return arg;
    int length = 0;
    int capacity = 16;

    char **tokens = malloc(capacity * sizeof(char*));

    char *delimiters = " \t\r\n";
    char *token = strtok(line, delimiters);

    while (token != NULL) {
        tokens[length] = token;
        length++;

        if (length >= capacity) {
            capacity = (int) (capacity * 2);
            tokens = realloc(tokens, capacity * sizeof(char*));
        }

        token = strtok(NULL, delimiters);
    }

    tokens[length] = NULL;
    return tokens;
}

Process *createJob() {}

void printProcess(Process *p) {
  printf("[%d] %d %s %s", p->jobId, p->processId, p->status, p->command);
}

int main(int argc, char **argv) {
  size_t bufsize = 32;
  Process *processes = (Process *)malloc(10 * sizeof(Process));
  int jId = 1;
  pid_t pid;
  char *tmp;
  char **tmp1;
  while (1) {
    tmp = getCmd();
    tmp1 = getArgs(tmp);
    if (strcasecmp(tmp, "exit") == 0)
      exit(0);
    if ((pid = fork()) == -1) {
      printf("Fork not successful, exiting...");
    }
    if (pid == 0) {
      if (execvp(tmp1[0], tmp1) == -1) {
        printf("Process did not execute correctly\n");
        exit(1);
      }
    }
    // only if execvp is successful
    processes[jId - 1].jobId = jId;
    processes[jId - 1].processId = pid; // number from fork();
    processes[jId - 1].status = (char *)malloc(bufsize * sizeof(char));
    char x[6] = "HELLO";
    strcpy(processes[jId - 1].status, x);
    processes[jId - 1].command = tmp;
    wait(NULL);
    // printProcess(&processes[jobId--]);
    jId++;
  }
  // for(int i = 0; i < 10; i++) {
  // printProcess(&processes[i]);
  //}
  return 0;
}