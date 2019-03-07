#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

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

  int fd[2], fd1[2], result, exit_sig;

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

      write(fd[1], user_id, strlen(user_id)+1);
      write(fd[1], password, strlen(password)+1);
      close(fd[1]);//writing end closed after reading

      wait(&exit_sig);// wait for child

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

      read(fd[0], user_id, MAXLINE);
      read(fd[0], password, MAX_PASSWORD);
      close(fd[0]);

      execl("./validate", "validate", "user_id", "password");

  } else {
      perror("fork");
      exit(1);
  }













  // ------
//    int fjd[2], r;
//
//    /* Create the pipe */
//    if ((pipe(fd)) == -1) {
//        perror("pipe");
//        exit(1);
//    }
//
//    if ((r = fork()) > 0) { // parent will run sort
//        // Set up the redirection from file1 to stdin
//        int filedes = open("file1", O_RDONLY);
//
//        // Reset stdin so that when we read from stdin it comes from the file
//        if ((dup2(filedes, fileno(stdin))) == -1) {
//            perror("dup2");
//            exit(1);
//        }
//        // Reset stdout so that when we write to stdout it goes to the pipe
//        if ((dup2(fd[1], fileno(stdout))) == -1) {
//            perror("dup2");
//            exit(1);
//        }
//
//        // Parent won't be reading from pipe
//        if ((close(fd[0])) == -1) {
//            perror("close");
//        }
//
//        // Because writes go to stdout, noone should write to fd[1]
//        if ((close(fd[1])) == -1) {
//            perror("close");
//        }
//
//        // We won't be using filedes directly, so close it.
//        if ((close(filedes)) == -1) {
//            perror("close");
//        }
//
//        execl("/usr/bin/sort", "sort", (char *) 0);
//        fprintf(stderr, "ERROR: exec should not return \n");
//
//    } else if (r == 0) { // child will run uniq
//
//        // Reset stdi so that it reads from the pipe
//        if ((dup2(fd[0], fileno(stdin))) == -1) {
//            perror("dup2");
//            exit(1);
//        }
//
//        // This process will never write to the pipe.
//        if ((close(fd[1])) == -1) {
//            perror("close");
//        }
//
//        // SInce we rest stdin to read from the pipe, we don't need fd[0]
//        if ((close(fd[0])) == -1) {
//            perror("close");
//        }
//
//        execl("/usr/bin/uniq", "uniq", (char *) 0);
//        fprintf(stderr, "ERROR: exec should not return \n");
//
//    } else {
//        perror("fork");
//        exit(1);
//    }
  // ------
  


  return 0;
}
