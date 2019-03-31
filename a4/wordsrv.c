#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include "socket.h"
#include "gameplay.h"


#ifndef PORT
    #define PORT 59994
#endif
#define MAX_QUEUE 5


void add_player(struct client **top, int fd, struct in_addr addr);
void remove_player(struct client **top, int fd);

/* These are some of the function prototypes that we used in our solution 
 * You are not required to write functions that match these prototypes, but
 * you may find the helpful when thinking about operations in your program.
 */
/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf);
void announce_turn(struct game_state *game);
void announce_winner(struct game_state *game, struct client *winner);
/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game);


/* The set of socket descriptors for select to monitor.
 * This is a global variable because we need to remove socket descriptors
 * from allset when a write to a socket fails.
 */
fd_set allset;


// HELPER FUNCTIONS

/**
 * This function prints the number of bytes read onto the
 * screen.
 */
void print_read_status(int check){
    if (check < 0){
        perror("read");
        exit(1);
    } else if (check == 0){
        printf("Read 0 things lol\n");
    } else {
        printf("Read %d things, more than 0!\n", check);
    }
}

/**
 * This function adds a player that entered a valid name to 
 * the list of active players in the game struct.
 */
void add_player_to_game(struct game_state * game, int fd, struct in_addr addr, char * name){
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    //printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    strncpy(p->name, name, MAX_NAME);
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = game->head;
    game->head = p;
}

/**
 * This function checks if the letter guessed is in the word randomly
 * chosen by the game or not.
 */
int check_letter_in_word(struct game_state * game, char * guess){
    char guessed_letter = guess[0];
    printf("THE GUESSED LETTER IS %c\n", guessed_letter);
    for (int i = 0; i < strlen(game->word); i++){
        if (guessed_letter == game->word[i]){
            return 1;
        }
    }
    return 0;
}

/**
 * This function will go through the word that is being guessed and
 * make the letters visible if and only if it has been guessed.
 */
void change_guess_scape(struct game_state * game, char * guess){
    char guessed_letter = guess[0];
    for (int i = 0; i < strlen(game->word); i++){
        if (game->word[i] == guessed_letter){
            game->guess[i] = guessed_letter;
        }
    }
}

/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found.
 * Definitely do not use strchr or other string functions to search here. (Why not?)
 */
int find_network_newline(const char *buf, int n) {
    char minibuffer[2];
    for (int i = 0; i < n-1; i++){
        minibuffer[0] = buf[i];
        minibuffer[1] = buf[i+1];
        if (strcmp(minibuffer, "\r\n")==0){
              return i+2;
        }
    }
    return -1;
}

/**
 * This function makes sure that no partial reads are allowed
 * into the system.
 */
int get_full_read(struct client * p){
    p -> in_ptr = p->inbuf;
    int nbytes;
    nbytes = read(p->fd, p->in_ptr, 3);
    p->in_ptr += nbytes;
    int newline = find_network_newline(p->in_ptr, 3);
    if (newline == -1){
        return -1;
    } else {
        return newline-2;
    }
    // if ((newline = find_network_newline(p->inbuf, 3)) == -1){
    //     return -1;
    // } else {
    //     return newline - 2;
    // }
}

/**
 * This function broadcasts a message to all the currently
 * active players in the game.
 */
void broadcast(struct game_state *game, char *outbuf){
    struct client * top = game->head;
    while(top != NULL){
        write(top->fd, outbuf, MAX_BUF);
        top = top->next;
    }
}

/**
 * This function initializes the turns, when the first person 
 * enters the game. 
 */
void initialize_turn(struct game_state * game){
    if (game->has_next_turn == NULL){
        struct client * current = game->head;
        while (current->next != NULL){
            current = current->next;
        }
        game->has_next_turn = current;
    }
}

/**
 * This function announces the turn of the next player,
 * and broadcasts it to every active player.
 */
void announce_turn(struct game_state *game){
    char turn[MAX_BUF] = {'\0'};
    sprintf(turn, "It is %s's turn.\r\n", game->has_next_turn->name);
    broadcast(game, turn);
}

/**
 * This function advances turn within the linked list
 */
void advance_turn(struct game_state *game){
    struct client * current = game->head;
    struct client * previous = NULL;
    if (game->has_next_turn == NULL){
        while (current->next != NULL){
            current = current->next;
        }
        previous = current;
    } else if(game->has_next_turn->fd == game->head->fd){
        while(current!=NULL){
            previous = current;
            current = current->next;
        }
    } else {
        while ((current != NULL)&&(current->fd != game->has_next_turn->fd)){
        previous = current;
        current = current->next;
        }
    }
    game -> has_next_turn = previous;
    write(game->has_next_turn->fd, "Your guess? ", 12);
    announce_turn(game);
}

/**
 * This function broadcasts a message to everyone but the
 * player with the current turn.
 */
void broadcast_audience(struct game_state * game, char * outbuf){
    struct client * top = game->head;
    while(top != NULL){
        if(game->has_next_turn->fd != top->fd){
          write(top->fd, outbuf, MAX_BUF);
        }
        top = top->next;  
    }
}

int len_ll(struct client * list){
    struct client * a = list;
    int counter = 0;
    while (a != NULL){
        counter++;
        a = a->next;
    }
    return counter;
}

/**
 * This function prints contents of a linked list, used for debugging.
 */
void print_ll(struct game_state game){
    struct client * head = game.head;
    printf("\n\n NOW PRINTING CONTENTS OF LINKED LIST GAME.HEAD\n\n");
    while (head != NULL){
        printf("the name: %s\n", head->name);
        printf("file descriptor: %d\n", head->fd);
        head = head->next;
    }
}

/**
 * This function removes a player that has entered a valid name, and adds it
 * onto the linked list of active players.
 */
void remove_valid_player(struct client ** list, int fd){
    struct client * next;
    if (*list != NULL){
        next = (*list)->next;
        printf("Removing player from latent player list %d\n", fd);
        *list = next;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",fd);
    }
}


/* This function reads for input from the the client side of the socket. 
 * Executed inside a while loop so as to make sure that all the output is
 * captured.
 */

void read_from_socket(int filedes, char * name_space){
    char * p = NULL;
    int num_chars = read(filedes, name_space, MAX_BUF);
    name_space[num_chars] = '\0';
    p = strstr(name_space, "\r\n");
    *p = '\0';
}

/* This function searches the struct client * head linked list for any names 
 * that are the same as specified by the user_name given by the user.
 */
int name_not_found(struct client * game_head, char * user_name){
    struct client * current = game_head;
    int indicator = 0;
    printf("REACHED NAME NOT FOUND 1\n");
    while (current != NULL){
        if (strcmp(current->name, user_name) == 0){
            printf("the name you entered has already been taken!\n");
            indicator = 1;
        }
        current = current->next;
    }
    printf("REACHED NAME NOT FOUND 2\n");
    return indicator;
}

// WRAPPER FUNCTIONS

/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset 
 */
void remove_player(struct client **top, int fd) {
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
}


int main(int argc, char **argv) {
    printf("remember to make wrapper functions\n");
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;
    
    if(argc != 2){
        fprintf(stderr,"Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }
    
    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int)time(NULL));
    // Set up the file pointer outside of init_game because we want to 
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);
    
    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;
    
    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages.  In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;
    
    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);
    
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if(write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&(game.head), p->fd);
            };
        }
        
        /* Check which other socket descriptors have something ready to read.
         * The reason we iterate over the rset descriptors at the top level and
         * search through the two lists of clients each time is that it is
         * possible that a client will be removed in the middle of one of the
         * operations. This is also why we call break after handling the input.
         * If a client has been removed the loop variables may not longer be 
         * valid.
         */
        int cur_fd;
        for(cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if(FD_ISSET(cur_fd, &rset)) {
                // Check if this socket descriptor is an active player
                //write(cur_fd, "What would your choice be?", 26);
                for(p = game.head; p != NULL; p = p->next) {
                    //printf("\n\nwe are in the current players list right now\n\n");
                    //print_ll(game);
                    //write(cur_fd, "It comes into the for loop?", 26);
                    if(cur_fd == p->fd) {
                        printf("THE CHOSEN WORD WAS %s\n", game.word);
                        // read for input
                        char choice;

                        int read_checker = get_full_read(p);
                        if (read_checker == -1){
                            break;
                        } else {
                            choice = p->inbuf[read_checker];
                        }

                        // read(cur_fd, p->inbuf, 3);
                        // if (find_network_newline(p->inbuf, 3) == -1){
                        //     break;
                        // }




                        // check input for validity
                        printf("WHAT YOU SEE HERE IS %lu %c\n", strlen(p->inbuf), p->inbuf[read_checker]);
                        while ((strlen(p->inbuf) != 1) || ('a'>p->inbuf[0]) || ('z'<p->inbuf[0])){
                            char * invalid = "Invalid! Try again!\r\n";
                            write(cur_fd, invalid, strlen(invalid));



                            // USE THE SAME READ STRUCTURE AS ABOVE
                            read(cur_fd, p->inbuf, 3);
                            if (find_network_newline(p->inbuf, 3) == -1){
                                break;
                            }




                        }
                        // check if the dude who gave input was the one who had the turn.
                        if (cur_fd == game.has_next_turn->fd){
                            char chosenmsg[MAX_BUF] = {'\0'};
                            sprintf(chosenmsg, "You guessed %s\r\n", p->inbuf);
                            char audmsg[MAX_BUF] = {'\0'};
                            sprintf(audmsg, "%s guesses: %s\r\n", game.has_next_turn->name, p->inbuf);
                            broadcast_audience(&game, audmsg);
                            //process the given letter
                            int letter_in_word = check_letter_in_word(&game, p->inbuf);
                            if (letter_in_word == 0){
                                // letter not in word
                                char letter_not_found[MAX_BUF] = {'\0'};
                                sprintf(letter_not_found, "%c is not in the word!\r\n", p->inbuf[0]);
                                write(cur_fd, letter_not_found, strlen(letter_not_found));
                                game.guesses_left--;
                                advance_turn(&game);
                                // DO YOU BREAK HERE OR WHAT?!?!?
                            } else {
                                // letter in the word
                                int letter_guessed_ascii = p->inbuf[0];
                                game.letters_guessed[letter_guessed_ascii-97] = 1;
                                change_guess_scape(&game, p->inbuf);
                                //advance_turn(&game);
                            }
                        } else {
                            char * not_your_turn = "Not your turn!\r\n";
                            write(cur_fd, not_your_turn, strlen(not_your_turn));
                        }                 
                        break;
                    }
                }
        
                // Check if any new players are entering their names
                for(p = new_players; p != NULL; p = p->next) {
                    len_ll(new_players);
                    if(cur_fd == p->fd) {
                        // TODO - handle input from an new client who has
                        // not entered an acceptable name.
                        char * name = malloc(MAX_BUF);
                        printf("free this malloc'd space\n");
                        read_from_socket(cur_fd, name);

                        //printf("the name we recieved over on this side was: %s with length %lu\n", name, strlen(name));
                        while((name == NULL) || (strlen(name) == 0) || (name_not_found(game.head, name) == 1)){
                            char *greeting = WELCOME_MSG;
                            if(write(clientfd, greeting, strlen(greeting)) == -1) {
                                // fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                                perror("Write greeting to new client failed.\n");
                                remove_player(&(game.head), p->fd);
                            };
                            read_from_socket(cur_fd, name);
                            //printf("the name we recieved over on this side was: %s with length %lu\n", name, strlen(name));
                        }
                        //printf("GAME HEAD NAME IS %s", game.head->name);
                        add_player_to_game(&game, cur_fd, p->ipaddr, name);
                        remove_valid_player(&(new_players), p->fd);
                        char entry_msg[MAX_BUF] = {'\0'};
                        sprintf(entry_msg, "\r\n%s has just joined the game!\r\n", game.head->name);
                        broadcast(&game, entry_msg);
                        char status[MAX_BUF] = {'\0'};
                        write(cur_fd, status_message(status, &game), MAX_BUF);
                        if((len_ll(game.head)) == 1){
                            initialize_turn(&game);
                        }            
                        write(game.has_next_turn->fd, "Your guess? ", 12);
                        //advance_turn(&game);
                        //print_ll(game);
                        break;
                    } 
                }
            }
        }
    }
    return 0;
}
























// CODE QUARANTINE


// 1. THIS IS THE CODE FROM LAB 10, WE LOOP OVER SHIT AND FIND NETWORK NEWLINE.
 // tell everyone else who's turn it is right now.
                        // char choice[3];
                        // if (cur_fd == game.has_next_turn->fd){
                        //     write(cur_fd, "Your guess? \r\n", 12);
                        // } else {
                        //     write(cur_fd, "ITS NOT YOUR TURN LOL SO YOUR GUESS DIDNT PRINT!\r\n",50); 
                        // }



                        // // THE WHILE LOOP FROM THE LAB 10 IS HERE

                        // int inbuf = 0;
                        // char buf[MAX_BUF] = {'\0'};
                        // int room = sizeof(buf);
                        // char * after = buf;
                        // int nbytes;

                        // while ((nbytes = read(cur_fd, after, room)) > 0) {
                        //     // Step 1: update inbuf (how many bytes were just added?)
                        //     inbuf += nbytes;
                        //     printf("NBYTES IS: %d\n", nbytes);

                        //     int where;

                        //     // Step 2: the loop condition below calls find_network_newline
                        //     // to determine if a full line has been read from the client.
                        //     // Your next task should be to implement find_network_newline
                        //     // (found at the bottom of this file).
                        //     //
                        //     // Note: we use a loop here because a single read might result in
                        //     // more than one full line.
                        //     while ((where = find_network_newline(buf, inbuf)) > 0) {
                        //         printf("WHERE IS: %d\n", where);
                        //         // Step 3: Okay, we have a full line.
                        //         // Output the full line, not including the "\r\n",
                        //         // using print statement below.
                        //         // Be sure to put a '\0' in the correct place first;
                        //         // otherwise you'll get junk in the output.
                        //         buf[where-2] = '\0';


                        //         printf("Next message: %s\n", buf);
                        //         // Note that we could have also used write to avoid having to
                        //         // put the '\0' in the buffer. Try using write later!

                        //         // Step 4: update inbuf and remove the full line from the buffer
                        //         // There might be stuff after the line, so don't just do inbuf = 0.
                        //         inbuf -= where;
                                

                        //         // You want to move the stuff after the full line to the beginning
                        //         // of the buffer.  A loop can do it, or you can use memmove.
                        //         memmove(buf, &(buf[where]), inbuf);


                        //     }
                        //     // Step 5: update after and room, in preparation for the next read.
                        //     after = &(buf[inbuf]);
                        //     room = MAX_BUF - inbuf;


                        // }




                        // // THE WHILE LOOP FROM THE LAB 10 ENDS HERE


// 2. 
// THE REST OF THE SHIT THAT CAME AFTER THE CODE RIGHT ABOVE THIS COMMENT, THE LAB 10 SHIT

                        // // int check = read(cur_fd, choice, 3);
                        // // print_read_status(check);
                        // if (cur_fd == game.has_next_turn->fd){
                        //     // PROCESS THIS GUESS INTO THE GAME - check if letter in word
                        //     // then update turns and everything
                        //     // after processing...

                        //     // test processing
                        //     char test_buf[MAX_BUF] = {'\0'};
                        //     sprintf(test_buf, "THIS IS A TEST: %s chose %s\r\n", game.has_next_turn->name, after);
                        //     broadcast(&game, test_buf);
                        //     // test processing ends
                        //     char message_choice[MAX_BUF] = {'\0'};
                        //     char guess[MAX_BUF] = {'\0'};
                        //     sprintf(guess, "You guessed %s\r\n", after);
                        //     write(cur_fd, guess, 15);
                        //     sprintf(message_choice, "%s guessed %s\r\n", game.has_next_turn->name, after);
                        //     broadcast_audience(&game, message_choice);
                        // } else {
                        //     write(cur_fd, "It is not your turn!\r\n", 21);
                        // }



//3. THIS BITCH LOOPS WHILE READING LOL

                        // char guess[3];
                        // read(cur_fd, guess, 3);
                        


                        // int inbuf = 0;           // How many bytes currently in buffer?
                        // int room = sizeof(p->inbuf);  // How many bytes remaining in buffer?
                        // p->in_ptr = p->inbuf;
                        // int nbytes;
                        // while((nbytes = read(cur_fd, p->in_ptr, room)) > 0){
                        //     inbuf += nbytes;
                        //     int where;
                        //     while((where = find_network_newline(p->inbuf, inbuf)) > 0){
                        //         p->inbuf[where-2] = '\0';
                        //         inbuf -= where;
                        //         memmove(p->inbuf, &(p->inbuf[where]), inbuf);
                        //     }
                        //     p->in_ptr = &(p->inbuf[inbuf]);
                        //     room = MAX_BUF - inbuf;
                        // }
