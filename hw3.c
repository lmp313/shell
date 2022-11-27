#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int jId = 1;
size_t bufsize = 32;
pid_t pid;
int check = 0;

typedef struct Process {
  int jobId;
  int processId;
  char *status;
  char *command;
  struct Process* prev;
  struct Process* next;
  // char **args;
} Process;

Process* head = (Process*)malloc(sizeof(Process));
Process* last = NULL;
Process* curr = NULL;

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
  if (buffer[tmp - 2] == '&') {
    check = tmp-2; //(tmp-2) so that I can know size of buffer for later
    buffer[tmp - 2] = '\0';
  } else {
    buffer[tmp - 1] = '\0';
  }
  return buffer;
}

char **getArgs(char *buffer) {
  int length = 0;
  int capacity = 16;
  
  char **tokens = malloc(capacity * sizeof(char *));

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
  return tokens;
}

void createJob(char *tmp, char **tmp1) {
  Process* new_job = (Process *)malloc(sizeof(Process));
  if ((pid = fork()) == -1) {
    printf("Fork not successful, exiting...");
  }
  if (pid == 0) {
    if (execvp(tmp1[0], tmp1) == -1) {
      printf("Process did not execute correctly\n");
      exit(1);
    }
  }
  if(head==NULL){
    // processes[jId - 1].jobId = jId;
    // processes[jId - 1].processId = pid; // number from fork();
    // processes[jId - 1].status = (char *)malloc(bufsize * sizeof(char));
    // strcpy(processes[jId - 1].status, "RUNNING");
    // processes[jId - 1].command = tmp;
    head->jobId = jId;
    head->processId = pid;
    head->status = (char *)malloc(bufsize * sizeof(char));
    strcpy(head->status, "RUNNING");
    head->next = NULL;
    head->prev = NULL;
    head->command = tmp;
  }else{
    curr = head;
    while(curr->next!=NULL){
      if(curr->next==NULL){
        new_job->jobId = jId;
        new_job->processId = pid;
        new_job->status = (char *)malloc(bufsize * sizeof(char));
        strcpy(curr->status, "RUNNING");
        new_job->next = NULL;
        new_job->prev = curr;
        new_job->command = tmp;
        last = new_job;
      }
      curr = curr->next;
    }
  }
  wait(NULL);
  jId++;
  // create processes item
  // processes[jId - 1].jobId = jId;
  // processes[jId - 1].processId = pid; // number from fork();
  // processes[jId - 1].status = (char *)malloc(bufsize * sizeof(char));
  // strcpy(processes[jId - 1].status, "RUNNING");
  // processes[jId - 1].command = tmp;
  //
  // free(tmp);
  // only if execvp is successful
  // wait(NULL);
  // printProcess(&processes[jobId--]);
  // jId++;
  //printf("jId = %d\n", jId);
  // if(jId > 2)
  // exit(0);
  // processes = (Process *) realloc(processes, jId * sizeof(Process));
}

void free_linked_list(Process* temp){
  for(Process* ptr=temp; ptr!=NULL; ptr = ptr->next){
    free(ptr);
  }
}

void printProcess(Process* p) {
  for(Process* ptr = p; ptr!=NULL;ptr = ptr->next){
    printf("[%d] %d %s %s\n", ptr->jobId, ptr->processId, ptr->status, ptr->command);
  }
}

void jobs(Process *processes) {
    printProcess(head);
}

int main(int argc, char **argv) {
  // Process *processes = (Process *)malloc(10 * sizeof(Process));
  char *tmp;
  char *tmp1;
  while (1) {
    tmp = getCmd();
    tmp1 = (char *)malloc((check+1) * sizeof(char));
    strcpy(tmp1, tmp);
    if(check == 1)
      printf("ITS A BACKGROUND TASK");
    //printf("tmp: %s\n", tmp);
    if (strcasecmp(tmp, "exit") == 0) {
      break;
    }
    createJob(tmp1, getArgs(tmp));
    check = 0;
  }
  free_linked_list(head);
  // jobs(processes);
  return 0;
}
