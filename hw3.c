#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int jId = 1;
size_t bufsize = 32;
int check = 0;

typedef struct Process {
  int jobId;
  int processId;
  char *status;
  char *command;
  // char **args;
} Process;

void exitShell() {
  /*
    When the shell exits, it should first send SIGHUP followed by SIGCONT to  
    any stopped jobs, and SIGHUP to any running jobs.

    //1. Iterate through jobs
        a.  pid_t pid;
            if(job is stopped *can check in process status) {
              pid = the jobs processId;
              kill(pid, SIGHUP);
              kill(pid, SIGCONT);
            } //job is running then
            else {
              pid = the jobs processId;
              kill(pid, SIGHUP);
            }
  */
  exit(0);
}

static void reapChild(int sig) {
  pid_t pid;
  for (;;) {
    pid = waitpid(-1, NULL, WNOHANG);
    printf("REAPED CHILD");
    if (pid <= 0) break; // note the < is now a <=
    //remove from linkedlist
  }
}

char *getCmd() {
  char *buffer;
  int tmp;

  buffer = (char *)malloc(bufsize * sizeof(char));
  if (buffer == NULL) {
    perror("Unable to allocate buffer");
    exit(1);
  }

  printf("> ");
  tmp = getline(&buffer, &bufsize, stdin);
  if(tmp == -1)
    exitShell();
  if (buffer[tmp - 2] == '&') {
    check = 1; //(tmp-2) so that I can know size of buffer for later
    buffer[tmp - 2] = '\0';
    //printf("& HERE\n");
  } else if (buffer[0] == 'b' && buffer[1] == 'g') {
    check = 2;
    buffer[tmp - 1] = '\0';
  }
  else if (buffer[0] == 'f' && buffer[1] == 'g') {
    check = 3;
    buffer[tmp - 1] = '\0';
  }
    else if (buffer[0] == 'c' && buffer[1] == 'd') {
    check = 4;
    buffer[tmp - 1] = '\0';
  }
  else {
    buffer[tmp - 1] = '\0';
  }
  return buffer;
}

char **getArgs(char *buffer) {
  int length = 0;
  int capacity = 16;
  
  char **tokens = (char **)malloc(capacity * sizeof(char *));

  char *delimiters = " \t\r\n";
  char *token = strtok(buffer, delimiters);

  while (token != NULL) {
    tokens[length] = token;
    length++;

    if (length >= capacity) {
      capacity = (int)(capacity * 2);
      tokens = realloc(tokens, capacity * sizeof(char *));
    }

    token = strtok(NULL, delimiters);
  }

  tokens[length] = NULL;
  /*for(int i = 0; i < length; i++) {
    printf("tokens[%d] = %s\n", i, tokens[i]);
  }*/
  return tokens;
}

void createJob1(char *tmp, char **tmp1, Process *processes) {
  pid_t pid;
  if ((pid = fork()) == -1) {
    printf("Fork not successful, exiting...");
  }
  if (pid == 0) {
    if (execvp(tmp1[0], tmp1) == -1) {
      printf("Process did not execute correctly\n");
      exit(1);
    }
  }
  // create processes item
  processes[jId - 1].jobId = jId;
  processes[jId - 1].processId = pid; // number from fork();
  //printf("pid inside: %d\n", pid);
  processes[jId - 1].status = (char *)malloc(bufsize * sizeof(char));
  strcpy(processes[jId - 1].status, "RUNNING");
  processes[jId - 1].command = tmp;
  //printf("tmp inside: %s\n", tmp);
  //
  // free(tmp);
  // only if execvp is successful
  waitpid(pid, NULL, WUNTRACED);
  // printProcess(&processes[jobId--]);
  jId++;
  //printf("jId = %d\n", jId);
  // if(jId > 2)
  // exit(0);
  // processes = (Process *) realloc(processes, jId * sizeof(Process));
}
void createJob2(char *tmp, char **tmp1, Process *processes) {
  pid_t pid;
  signal(SIGCHLD, reapChild);
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &set, NULL);
  if ((pid = fork()) == -1) {
    printf("Fork not successful, exiting...");
  }
  if (pid == 0) {
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    if (execvp(tmp1[0], tmp1) == -1) {
      printf("Process did not execute correctly\n");
      exit(1);
    }
  }
  // create processes item
  processes[jId - 1].jobId = jId;
  processes[jId - 1].processId = pid; // number from fork();
  //printf("pid inside: %d\n", pid);
  processes[jId - 1].status = (char *)malloc(bufsize * sizeof(char));
  strcpy(processes[jId - 1].status, "RUNNING");
  processes[jId - 1].command = tmp;
  sigprocmask(SIG_UNBLOCK, &set, NULL);
  //printf("tmp inside: %s\n", tmp);
  //
  // free(tmp);
  // only if execvp is successful
  // printProcess(&processes[jobId--]);
  jId++;
  //printf("jId = %d\n", jId);
  // if(jId > 2)
  // exit(0);
  // processes = (Process *) realloc(processes, jId * sizeof(Process));
}

void printProcess(Process *p) {
  printf("[%d] %d %s %s\n", p->jobId, p->processId, p->status, p->command);
}

void jobs(Process *processes) {
  for (int i = 0; i < 10; i++) {
    printProcess(&processes[i]);
  }
}

void putBg(char **tmp1) {
  memmove(tmp1[1], tmp1[1]+1, strlen(tmp1[1])); //remove %
  //find the process with jobId = atoi(tmp1[1]);
  /*
  pid_t pid;
  pid = process processId;
  signal(SIGCHLD, reapChild);
  kill(pid, SIGCONT);

  */
}

void putFg(char **tmp1) {
  memmove(tmp1[1], tmp1[1]+1, strlen(tmp1[1])); //remove %
  //find the process with jobId = atoi(tmp1[1]);
  /*
  pid_t pid;
  pid = process processId;
  kill(pid, SIGCONT);
  waitpid(pid, NULL, WUNTRACED);
  */
}

void doCd(char **tmp1) {
  //use chdir https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
  /*
    Change current directory to the given (absolute or relative) path. If no       path is given, use the value of environment variable HOME. Your shell          should update the environment variable PWD with the (absolute) present 
    working directory after running cd. 
  */
}

int main(int argc, char **argv) {
  Process *processes = (Process *)malloc(10 * sizeof(Process));
  char *tmp;
  char *tmp1;
  while (1) {
    tmp = getCmd();
    //printf("tmp = %s\n", tmp);
    tmp1 = (char *)malloc((check+1) * sizeof(char));
    strcpy(tmp1, tmp);
    //printf("CHECK == %d", check);
    exit(0);
    //check == 0 if foreground task, check == 1 if background task, check == 2 if bg, check == 3 if fg, check == 4 if cd
    if(check == 0) {
      createJob1(tmp1, getArgs(tmp), processes); //foreground task
    }
    else if(check == 1) {
      createJob2(tmp1, getArgs(tmp), processes); //background task
    }
    else if(check == 2) {
      putBg(getArgs(tmp));
    }
    else if(check == 3) {
      putFg(getArgs(tmp));
    }
    else if(check == 4) {
      doCd(getArgs(tmp));
    }
    else if (strcasecmp(tmp, "exit") == 0) {
      printf("EXITING\n");
      exitShell();
      }
    else if (strcasecmp(tmp, "jobs") == 0) {
      //edit this and create new method for printing the linkedlist
      jobs(processes);
      }
    check = 0;
  }
  jobs(processes);
  return 0;
}
