#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int jId = 1;
size_t bufsize = 32;
int check = 0;
sigset_t set;

typedef struct Process {
  int isBg;
  int jobId;
  int processId;
  char *status;
  char *command;
  struct Process* prev;
  struct Process* next;
  // char **args;
} Process;

Process* head = NULL;
Process* last = NULL;
Process* curr = NULL;

void freeJob(Process *ptr) {
  free(ptr->status);
  free(ptr->command);
  free(ptr);
}

void free_linked_list(){
  for(Process* ptr=head; ptr!=NULL; ptr = ptr->next){
    freeJob(ptr);
  }
}

void jobs() {
  for(Process* ptr = head; ptr!=NULL;ptr = ptr->next){
    if(ptr->isBg == 1) {
      char *tmp = (char *)malloc(bufsize * sizeof(char));
      strcpy(tmp, ptr->command);
      strncat(tmp, "&", 1);
      printf("[%d] %d %s %s\n", ptr->jobId, ptr->processId, ptr->status, tmp);
      free(tmp);
    }
    else
      printf("[%d] %d %s %s\n", ptr->jobId, ptr->processId, ptr->status, ptr->command);
  }
}

Process* findProcess(int jobId) {
  for(Process* ptr=head; ptr!=NULL; ptr = ptr->next){
    if(ptr->jobId == jobId)
      return ptr;
  }
  printf("JOB NOT FOUND");
  return NULL;
}

Process* findProcessP(int pId) {
  for(Process* ptr=head; ptr!=NULL; ptr = ptr->next){
    if(ptr->processId == pId)
      return ptr;
  }
  printf("JOB NOT FOUND");
  return NULL;
}

void removeProcess(int pid) {
    Process *ptr = findProcessP(pid);
    if(ptr->prev == NULL) {
      jId--;
      freeJob(ptr);
      head = NULL;
      curr = NULL;
      last = NULL;
      return;
      }
    Process *tmp = ptr->prev;
    //changing jobId for each job to one less since this job has terminated.
    for(Process *t = tmp; t!=NULL; t = t->next){
      t->jobId--;
    }
    if(ptr->next == NULL) {
        tmp->next = NULL;
        curr = tmp;
        last = tmp;
      }
    else {
      Process *tmp1 = ptr->next;
      tmp->next = tmp1;
      tmp1->prev = tmp;
    }
    jId--;
    freeJob(ptr);
}

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
  jobs(); //this line is for testing, remove before submitting.
  //free_linked_list();
  exit(0);
}

static void reapChild(int sig) {
  /*printf("sig in reapchild: %d\n", sig);
  pid_t pid;
  for (;;) {
    pid = waitpid(-1, NULL, WNOHANG);
    printf("REAPED CHILD\n");
    if (pid <= 0) break; // note the < is now a <=
    //remove from linkedlist
  }*/
  int pid;
	int status;

	pid = wait(&status);
	printf("parent: child process pid=%d exited with value %d\n",
		pid, WEXITSTATUS(status));
  //removeProcess(pid);
	signal(SIGCHLD, reapChild);
}

Process* findFg() {
  for(Process* ptr=head; ptr!=NULL; ptr = ptr->next){
    if(ptr->isBg != 1)
      return ptr;
  }
  return NULL;
}

static void catchInt(int sig) {
  Process *tmp = findFg();
  if(tmp == NULL)
    return;
  kill(tmp->processId, SIGINT);
}
static void catchTstp(int sig) {
  Process *tmp = findFg();
  if(tmp == NULL)
    return;
  kill(tmp->processId, SIGTSTP);
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

void createJob1(char *tmp, char **tmp1) {
  pid_t pid;
  Process* new_job = (Process *)malloc(sizeof(Process));
  //forking and executing command
  if ((pid = fork()) == -1) {
    printf("Fork not successful, exiting...");
  }
  if (pid == 0) {
    if (execvp(tmp1[0], tmp1) == -1) {
      printf("Process did not execute correctly\n");
      exit(1);
    }
  }
  // create process item
  new_job->isBg = 0;
  new_job->jobId = jId;
  new_job->processId = pid;
  new_job->status = (char *)malloc(bufsize * sizeof(char));
  strcpy(new_job->status, "RUNNING");
  new_job->next = NULL;
  new_job->prev = NULL;
  printf("tmp = %s\n", tmp);
  new_job->command = tmp;
  //adding it to linkedlist
  if(head==NULL){
    head = new_job;
    last = head;
  }else{
    curr = head;
    while(curr!=NULL){
      if(curr->next==NULL){
        curr->next = new_job;
        new_job->prev = curr;
        last = new_job;
        break;
      }
      curr = curr->next;
    }
  }
  //waiting for child process to either stop or terminate
  int status;
  waitpid(pid, &status, WUNTRACED);
  if(WIFSTOPPED(status)) {
    free(new_job->status);
    new_job->status = (char *)malloc(bufsize * sizeof(char));
    strcpy(new_job->status, "STOPPED");
  }
  else {
    removeProcess(pid);
  }
  jId++;
}
void createJob2(char *tmp, char **tmp1) {
  pid_t pid;
  Process* new_job = (Process *)malloc(sizeof(Process));
  //setting up child signal handler
  signal(SIGCHLD, reapChild);
  //forking and executing command
  if ((pid = fork()) == -1) {
    printf("Fork not successful, exiting...");
  }
  if (pid == 0) {
    //unblocking signals for child process
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    if (execvp(tmp1[0], tmp1) == -1) {
      printf("Process did not execute correctly\n");
      exit(1);
    }
  }
  // create process item
  new_job->isBg = 1;
  new_job->jobId = jId;
  new_job->processId = pid;
  new_job->status = (char *)malloc(bufsize * sizeof(char));
  strcpy(new_job->status, "RUNNING");
  new_job->next = NULL;
  new_job->prev = NULL;
  new_job->command = tmp;
  printf("[%d] %d\n", jId, pid);
  //adding it to linkedlist
  if(head==NULL){
    head = new_job;
    last = head;
  }else{
    curr = head;
    while(curr!=NULL){
      if(curr->next==NULL){
        curr->next = new_job;
        new_job->prev = curr;
        last = new_job;
        break;
      }
      curr = curr->next;
    }
  }
  jId++;
}

void putBg(char **tmp1) {
  memmove(tmp1[1], tmp1[1]+1, strlen(tmp1[1])); //remove %
  Process *ptr = findProcess(atoi(tmp1[1]));
  signal(SIGCHLD, reapChild);
  kill(ptr->processId, SIGCONT);
}

void putFg(char **tmp1) {
  memmove(tmp1[1], tmp1[1]+1, strlen(tmp1[1])); //remove %
  Process *ptr = findProcess(atoi(tmp1[1]));
  pid_t pid;
  pid = ptr->processId;
  kill(pid, SIGCONT);
  int status;
  waitpid(pid, &status, WUNTRACED);
  if(WIFSTOPPED(status)) {
    free(ptr->status);
    ptr->status = (char *)malloc(bufsize * sizeof(char));
    strcpy(ptr->status, "STOPPED");
  }
  else {
    removeProcess(pid);
  }
}

void doCd(char **tmp1) {
  //use chdir https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
  /*
    Change current directory to the given (absolute or relative) path. If no       path is given, use the value of environment variable HOME. Your shell          should update the environment variable PWD with the (absolute) present 
    working directory after running cd. 
  */
}

void killProc(char **tmp1) {
  memmove(tmp1[1], tmp1[1]+1, strlen(tmp1[1])); //remove %
  Process *ptr = findProcess(atoi(tmp1[1]));
  kill(ptr->processId, SIGTERM);
}

int main(int argc, char **argv) {
  char *tmp;
  char *tmp1;
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  while (1) {
    signal(SIGINT, catchInt);
    signal(SIGTSTP, catchTstp);
    //printf("jId = %d\n", jId);
    //setting up signal blocking so child does not send SIGCHILD (stop or term) before the entry is added to jobs list 
  sigprocmask(SIG_BLOCK, &set, NULL);
    tmp = getCmd();
    //unblocking signals
  sigprocmask(SIG_UNBLOCK, &set, NULL);
    //printf("tmp = %s\n", tmp);
    tmp1 = (char *)malloc((check+1) * sizeof(char));
    strcpy(tmp1, tmp);
    //printf("CHECK == %d\n", check);
    //check == 0 if foreground task, check == 1 if background task, check == 2 if bg, check == 3 if fg, check == 4 if cd
    if (strcasecmp(tmp, "exit") == 0) {
      printf("EXITING\n");
      exitShell();
      }
    else if (strcasecmp(tmp, "jobs") == 0) {
        jobs();
      }
      else if (strstr(tmp, "kill") != NULL) {
        killProc(getArgs(tmp));
      }
    else if(check == 0) {
      createJob1(tmp1, getArgs(tmp)); //foreground task
    }
    else if(check == 1) {
      createJob2(tmp1, getArgs(tmp)); //background task
      sleep(10);
    }
    else if(check == 2) {
      putBg(getArgs(tmp));
      sleep(10);
    }
    else if(check == 3) {
      putFg(getArgs(tmp));
    }
    else if(check == 4) {
      doCd(getArgs(tmp));
    }
    free(tmp);
    check = 0;
  }
  return 0;
}
