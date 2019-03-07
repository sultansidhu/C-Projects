#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }

  int fd[2], fd1[2], result, exit_sig, exit_check;

  if ((pipe(fd)) == -1){
      perror("pipe");
      exit(1);
  }

  if ((pipe(fd1)) == -1){
      perror("pipe");
      exit(1);
  }

  if ((result = fork()) > 0){
      // write the user_id and password to pipe;
      close(fd[0]);//reading end closed

      write(fd[1], user_id, MAX_PASSWORD);
      write(fd[1], password, MAX_PASSWORD);
      close(fd[1]);//writing end closed after reading

      wait(&exit_check);// wait for child
      if (WIFEXITED(exit_check)){
          exit_sig = WEXITSTATUS(exit_check);
      } else {
          fprintf(stderr, "CHILD DID NOT EXIT");
          exit(1);
      }




      if (exit_sig == 0){
          printf(SUCCESS);
      } else if (exit_sig == 1){
          fprintf(stderr, "ERROR");
      } else if (exit_sig == 2){
          printf(INVALID);
      } else if (exit_sig == 3){
          printf(NO_USER);
      }
  } else if (result == 0){
      // child process, runs validate
      close(fd[1]); //close writing end

      dup2(fd[0], STDIN_FILENO);
      close(fd[0]);

      execl("./validate", "validate", NULL);

  } else {
      perror("fork");
      exit(1);
  }


  return 0;
}
