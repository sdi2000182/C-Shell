#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>
#include "tokenizer.h"
#include "ADTVector.h"
#define MAX_LINE_LENGTH 100
#define myhistory_LIMIT 20
#define MAXIMUM_ALLOC 4096
#define MAX_ALL 1024



#define unused __attribute__((unused))

bool shell_is_interactive;


int shell_terminal;


struct termios shell_tmodes;


pid_t shpgid;

int cmd_exit(struct tokens* tokens);
int cmd_pwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);


typedef int commands(struct tokens* tokens);


typedef struct look_table {
  commands* search;
  char* cmd;
} look_table_t;

//commands p thewrhsa xreiazontai
look_table_t cmd_table[] = {
    {cmd_exit, "exit"},
    {cmd_pwd, "pwd"},
    {cmd_cd, "cd"},
};


int cmd_exit(unused struct tokens* tokens) { exit(0); }

int cmd_pwd( struct tokens* tokens) {
  char *pwd = getcwd(NULL, 0);
  puts(pwd);
  free(pwd);
  return 0;
}

//xeirizetai ta wild_characters
void WildCharacterExpander(char *input_arg, char ***handlerpt, int *num_handlerpt) {
    char cwd[MAX_ALL];
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Trexon katalogos: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return ;
    }
    char *dir_path = ".", *copied = input_arg;
    struct dirent *entry;
    DIR *dir;
    int i, num_wildcards = 0;
    char **wildcards = NULL;
    dir = opendir(cwd);
    if (dir == NULL) {
        fprintf(stderr, "Error: Den anoigei to directory '%s': %s\n", dir_path, strerror(errno));
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(copied, entry->d_name, 0) == 0) {
            num_wildcards++;
        }
    }
    closedir(dir);
    wildcards = malloc(num_wildcards * sizeof(char *));
    if (wildcards == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for expanded arguments\n");
        return;
    }
    dir = opendir(dir_path);
    i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(copied, entry->d_name, 0) == 0) {
            wildcards[i] = malloc((strlen(entry->d_name) + 1) * sizeof(char));
            if (wildcards[i] == NULL) {
                fprintf(stderr, "Error: fail sthn malloc \n");
                return;
            }
            strcpy(wildcards[i], entry->d_name);
            i++;
        }
    }
    closedir(dir);
    *handlerpt = wildcards;
    *num_handlerpt = num_wildcards;
}


int cmd_cd( struct tokens* tokens) {
  if (GetNumberOfTokens(tokens) == 1) return 0;
  if (GetNumberOfTokens(tokens) > 2) return 1;
  const int MAX_SIZE = MAXIMUM_ALLOC;
  char *pwd = (char *)malloc(MAX_SIZE);
  strcat(pwd, GetToken(tokens, 1));
  if (chdir(pwd)) {
    fprintf(stderr, "Cd den yparxei to directory/arxeio pou zhtate: %s\n",
      GetToken(tokens, 1));
    return 1;
  }
  return 0;
}

int lookup(char cmd[]) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(look_table_t); i++){
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)){
        return i;}
    }
  return -1;
}


void shell_start() {
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    while (tcgetpgrp(shell_terminal) != (shpgid = getpgrp()))
      kill(-shpgid, SIGTTIN);

    /* Saves the shell's process id */
    shpgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shpgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int exec_subprogram_pipe(struct tokens* tokens, int token_start, int token_end) {
  /* Create arguments and redirect */
  int argcnt = 0, tokens_length = GetNumberOfTokens(tokens);
  char *arg[4096];
  for (int i = token_start; i < token_end; i++) {
    char *token = GetToken(tokens, i);
    if (!strcmp(token, "<") && i + 1 < tokens_length) {
      int fd = open(GetToken(tokens, i+1), O_RDONLY);
      if (fd != -1) dup2(fd, STDIN_FILENO);
      else { printf("shell: can't open file: %s\n", GetToken(tokens, i+1)); }
      i++;
    }
    else if (!strcmp(token, ">") && i + 1 < tokens_length) {
      int fd = open(GetToken(tokens, i+1), O_CREAT|O_WRONLY|O_TRUNC, 0600);
      if (fd != -1) dup2(fd, STDOUT_FILENO);
      i++;
    }
    else if (!strcmp(token, ">>") && i + 1 < tokens_length) {
      int fd = open(GetToken(tokens, i+1), O_CREAT|O_WRONLY|O_APPEND, 0600);
      if (fd != -1) {
        dup2(fd, STDOUT_FILENO);
      }
      i++;
    }
    else {
      arg[argcnt++] = token;
    }
  }
  arg[tokens_length] = NULL;

  /* Retrieve the program and execute */
  char exe_path[4096], *word, *brkp;
  char *sep = ":";
  char *PATH = getenv("PATH");
  if (PATH == NULL) { return 1; }

  if (!execv(arg[0], arg)) return 0;
  for (word = strtok_r(PATH, sep, &brkp);
       word;
       word = strtok_r(NULL, sep, &brkp)) {
    strcpy(exe_path, word);
    strcat(exe_path, "/");
    strcat(exe_path, arg[0]);
    if (!execv(exe_path, arg)) return 0;
  }
  printf("shell: command not found: %s\n", arg[0]);
  return 1;
}

void execute1(struct tokens* tokens) {
  /* Calculate the total number of process */
  int total_proc = 1, tokens_length = GetNumberOfTokens(tokens);
  for (int i = 0; i < tokens_length; i++) {
    if (!strcmp(GetToken(tokens, i), "|")) {
      ++total_proc;
    }
  }

  int pipe_arr[total_proc][2];
  for (int i = 0; i < total_proc - 1; i++) {
    if (pipe(pipe_arr[i]) == -1) { exit(1); }
  }


  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGTTOU, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
  if (sigaction(SIGTTIN, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
  if (sigaction(SIGCONT, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
  if (sigaction(SIGTSTP, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
  if (sigaction(SIGTERM, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
  if (sigaction(SIGINT,  &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
  if (sigaction(SIGQUIT, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }

  /* Run subtasks */
  int pid[total_proc];
  int token_start = 0, proc_cnt = 0;
  for (int i = 0; i <= tokens_length; i++) {
    if (i == tokens_length || !strcmp(GetToken(tokens, i), "|")) {
      pid[proc_cnt] = fork();
      if (pid[proc_cnt] == 0) {
        /* Child Process */
        /* Set pipe */
        if (proc_cnt != 0) dup2(pipe_arr[proc_cnt - 1][0], STDIN_FILENO);
        if (proc_cnt != total_proc - 1)
          dup2(pipe_arr[proc_cnt][1], STDOUT_FILENO);
        for (int i = 0; i < total_proc - 1; i++) {
          close(pipe_arr[i][0]);
          close(pipe_arr[i][1]);
        }

        /* Enable some signals */
        sa.sa_handler = SIG_DFL;
        if (sigaction(SIGTTOU, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
        if (sigaction(SIGTTIN, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
        if (sigaction(SIGCONT, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
        if (sigaction(SIGTSTP, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
        if (sigaction(SIGTERM, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
        if (sigaction(SIGINT,  &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }
        if (sigaction(SIGQUIT, &sa, NULL) == -1) { perror("sigaction error"); exit(EXIT_FAILURE); }

        /* Set process group*/
        setpgid(0, 0);

        /* Run subprogram */
        int st = exec_subprogram_pipe(tokens, token_start, i);

        /* Close pipe and Exit*/
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        exit(st);
      } else {
        /* Parent Process */
        tcsetpgrp(STDIN_FILENO, pid[proc_cnt]);
        token_start = i + 1;
        ++proc_cnt;
      }
    }
  }

  /* Close pipes */
  for (int i = 0; i < total_proc - 1; i++) {
    close(pipe_arr[i][0]);
    close(pipe_arr[i][1]);
  }

  /* Wait subtasks */
  for (int i = 0; i < total_proc; i++) {
    wait(&pid[i]);
  }
  tcsetpgrp(STDIN_FILENO, getpgrp());
}


int main(unused int argc, unused char* argv[]) {
  shell_start();

  static char line[MAXIMUM_ALLOC];
  int line_index= 0;
  Vector alias_args_1 = vector_create(0,NULL);  // px an exo createalias lll ls - las tha mpei to lll edo
  Vector alias_args_2 = vector_create(0,NULL); // edo to ls -las
  int hcnt = 0;
  void *ptr [myhistory_LIMIT * MAX_LINE_LENGTH]; // pinakas history
  char (*myhistory)[MAX_LINE_LENGTH] = (char(*)[MAX_LINE_LENGTH])ptr ;
  char *args1[MAXIMUM_ALLOC]; //wildcard handler
  int hsize =0;
  fprintf(stdout, "%d: ", line_index);
  struct tokens* tokens;
  while (fgets(line, MAXIMUM_ALLOC, stdin)) {
    tokens = MakeTokens(line); //ftiaxno ta token
    int cnt = 0;
    char **wildcards = NULL;
    int num_wildcards = 0;
    int flag2 = 0; // flag gia na do an h entolh p exo thelei ektelesh h oxi analoga
    int get_index =0; //thn kathgoria ths
    //vector_print(alias_args_1);
      for(int i=0;i<GetNumberOfTokens(tokens);i++){
      //vector_print(alias_args_1);
      //printf("strcmp is %d\n",strcmp(GetToken(tokens,0),"destroyalias"));
      //Elegxei thn domh alias kai antikathista sthn protash m to alias me thn ermhneia tou
       if((get_index=vector_find(alias_args_1,GetToken(tokens,i),compare_ints))!=-1 && strcmp(GetToken(tokens,0),"destroyalias")!=0){
          char * new_string = (char*) malloc(strlen(vector_get_at(alias_args_2,get_index)) * sizeof(char));
          strcpy(new_string,"");  // to neo string p tha exo meta thn antikatastash tou alias
          strcpy(new_string,vector_get_at(alias_args_2,get_index));
          char* original_string = (char*) malloc(strlen(line) * sizeof(char)); //to arxiko
          strcpy(original_string,line);
          char* search_string = (char*) malloc(strlen(vector_get_at(alias_args_1,get_index)) * sizeof(char));
          strcpy(search_string,vector_get_at(alias_args_1,get_index)); // to string p tha psaxo sthn protash m aka to alias
            char *ypostring = strstr(original_string, search_string); 
            if (ypostring != NULL) { 
                char temp_string[strlen(ypostring) - strlen(search_string) + 1]; 
                strcpy(temp_string, ypostring + strlen(search_string)); 
                *ypostring = '\0'; 
                strcat(original_string, new_string);
                strcat(original_string, temp_string); 
            }
          strcpy(line,original_string);
          tokens = MakeTokens (line); // ksana kano tokens
       }
       else{
       }
    }
    for(int i=0;i<GetNumberOfTokens(tokens);i++){  // elegxo gia background kai wild cards kai ta xeirizomai analoga
      if(!strcmp(GetToken(tokens,i),"&;") || !strcmp(GetToken(tokens,i),"&")){
        char* get_arg =GetToken(tokens,i-1);
        args1[cnt++] =  get_arg;
        if(i==GetNumberOfTokens(tokens)-1){
          flag2 = 1;
          }
      }
        if(strchr(GetToken(tokens,i),'*')!=NULL || strchr(GetToken(tokens,i),'?')!=NULL){
            char * token_copy = (char*) malloc(strlen(GetToken(tokens,i)) * sizeof(char));
            strcpy(token_copy,"");
            char * token_copy_u = (char*) malloc(strlen(GetToken(tokens,i)) * sizeof(char));
            strcpy(token_copy_u,"");
            strcpy(token_copy_u,GetToken(tokens,i)); 
            strcpy(token_copy,GetToken(tokens,i-1));
            WildCharacterExpander(token_copy_u, &wildcards, &num_wildcards);
            char *concatenated_args = NULL;
            int total_length = 0;
            for (int i = 0; i < num_wildcards; i++) {
                  total_length += strlen(wildcards[i]) + 1;  
            }
            concatenated_args = malloc(total_length * sizeof(char));
            if (concatenated_args == NULL) {
                fprintf(stderr, "Error: h malloc apetyxe\n");
                return 1;}
            concatenated_args[0] = '\0';
            for (int i = 0; i < num_wildcards; i++) {
                strcat(concatenated_args, wildcards[i]);
                if(i< num_wildcards-1){
                  strcat(concatenated_args, " ");} 
            }
            strcat(token_copy," ");
            if(concatenated_args!=NULL){
                strcat(token_copy,concatenated_args);}
            if(token_copy!=NULL){
                strcpy(line,token_copy);}
            tokens =MakeTokens(line);  //sthn ousia vazo ta arxeia p tha vgoun anti gia px f*.txt
        }
        char* ptr = strchr(GetToken(tokens,i), '$');  // xeirizomai enviromental variables kai antikathisto me thn ermhneia
        if(ptr==GetToken(tokens,i)){
            char* env_value = getenv(ptr+1);
            char * line_copy = (char*) malloc(  MAX_LINE_LENGTH * sizeof(char));
            char* ptr1 = strchr(line, '$');
            int index = ptr1 - line;
            strncpy(line_copy,line,index);
            line_copy[strlen(line_copy)-1] = '\0';
            strcat(line_copy," ");
            if(env_value!=NULL){
                strcat(line_copy,env_value);}
            tokens = MakeTokens(line_copy); // ksana kano tokens
            
        }
    }
    out:                           //ekso apo thn loopa gia na prosperaso pio grhgora kapoies periptwseis
    if(hcnt==myhistory_LIMIT){
        hcnt = 0;
    }
    strcpy(myhistory[hcnt],line);    //xeirizomai to history
    if(hsize<=myhistory_LIMIT){
        hsize++;}

    hcnt ++;
    if(!strcmp(GetToken(tokens,0),"myhistory") && GetNumberOfTokens(tokens)==1){  //sketo print istoria
       for(int i=0 ; i< hsize;i++){
            if(i!=myhistory_LIMIT){
                printf("%d: %s\n",i,myhistory[i]);}
       }
       flag2=2; 
    }
    if(!strcmp(GetToken(tokens,0),"myhistory") && GetNumberOfTokens(tokens)>1){  //ektelesh istorias
        int myhistory_arg = atoi(GetToken(tokens,1));
        strcpy(line,myhistory[myhistory_arg]);
        tokens = MakeTokens(line);
    }
    if(!strcmp(GetToken(tokens,0),"createalias")){
       vector_insert_last(alias_args_1,GetToken(tokens,1));
       int total_length = 0;
       for (int i = 2; i < GetNumberOfTokens(tokens); i++) {  //h diadikasia perigrafetai sto readme
                  total_length += strlen(GetToken(tokens,i)) + 1;
        }
        char *concatenated_args = NULL;
        concatenated_args = malloc(total_length * sizeof(char));
        if (concatenated_args == NULL) {
            fprintf(stderr, "Error: H malloc apetyxe\n");
            return 1;}
        concatenated_args[0] = '\0';
        for (int i = 2; i < GetNumberOfTokens(tokens); i++) {
            strcat(concatenated_args, GetToken(tokens,i));
            if(i< GetNumberOfTokens(tokens)-1){
                strcat(concatenated_args, " ");}
        }
       vector_insert_last(alias_args_2,concatenated_args); 
       flag2=3;
    }
    if(!strcmp(GetToken(tokens,0),"destroyalias")){  //afairo to i-osto alias
        int total_length=0;
        int get_index_del = vector_find(alias_args_1,GetToken(tokens,1),compare_ints);
        vector_remove(alias_args_1,get_index_del);
        vector_remove(alias_args_2,get_index_del);
        flag2=4;
    }
    int var = lookup(GetToken(tokens, 0));  //koitao an einai oi entoles oi sygkekrimenes
    if (var >= 0) {
      cmd_table[var].search(tokens);
    } else if(flag2 == 0) {  //kano execute
      execute1(tokens);
    }
    else if(flag2 == 1){
            for(int j=0 ; j < cnt ; j++){
                char * command =args1[j];
                char* args[MAXIMUM_ALLOC];
                int arg_count = 0;
                args[arg_count++] = command;
                pid_t pid = fork();
                if(pid<0){
                    printf("Error sthn fork\n");
                }
                else if(pid==0){
                    execvp(args[0],args);
                }
                else{
                
                }
            }
        }
    fprintf(stdout, "%d: ", ++line_index);
  }
  TokenFree(tokens);
  return 0;
}