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
#define PORT 59995
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
 * This function checks if the player playing the game has finished the game
 * by either exhausting the number of guesses or by guessing the complete word.
 */
int check_gameover(struct game_state *game)
{
    if (game->guesses_left == 0)
    {
        return 0; // game over because no guesses left
    }
    for (int i = 0; i < strlen(game->guess); i++)
    {
        if (game->guess[i] == '-')
        {
            return 1; // this game isnt over
        }
    }
    return 2; // this game is over because the player with current turn won.
}

/**
 * This function adds a player that entered a valid name to 
 * the list of active players in the game struct.
 */
void add_player_to_game(struct game_state *game, int fd, struct in_addr addr, char *name)
{
    struct client *p = malloc(sizeof(struct client));

    if (!p)
    {
        perror("malloc");
        exit(1);
    }

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
int check_letter_in_word(struct game_state *game, char *guess)
{
    char guessed_letter = guess[0];
    for (int i = 0; i < strlen(game->word); i++)
    {
        if (guessed_letter == game->word[i])
        {
            return 1;
        }
    }
    return 0;
}

/**
 * This function will go through the word that is being guessed and
 * make the letters visible if and only if it has been guessed.
 */
void change_guess_scape(struct game_state *game, char *guess)
{
    char guessed_letter = guess[0];
    for (int i = 0; i < strlen(game->word); i++)
    {
        if (game->word[i] == guessed_letter)
        {
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
int find_network_newline(const char *buf, int n)
{
    char minibuffer[2];
    for (int i = 0; i < n - 1; i++)
    {
        minibuffer[0] = buf[i];
        minibuffer[1] = buf[i + 1];
        if (strcmp(minibuffer, "\r\n") == 0)
        {
            return i + 2;
        }
    }
    return -1;
}

int find_network_newline2(const char *buf, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (buf[i] == '\r')
        {
            return i + 2;
        }
    }
    return -1;
}

/**
 * This function makes sure that no partial reads are allowed
 * into the system.
 */
int get_full_read(struct client *p)
{
    p->in_ptr = p->inbuf;
    int nbytes;
    nbytes = read(p->fd, p->in_ptr, 3);
    p->in_ptr += nbytes;
    int newline = find_network_newline2(p->inbuf, MAX_BUF);
    if (newline == -1)
    {
        return -1;
    }
    else
    {
        return newline - 2;
    }
}

/**
 * This function broadcasts a message to all the currently
 * active players in the game.
 */
void broadcast(struct game_state *game, char *outbuf)
{
    struct client *top = game->head;
    while (top != NULL)
    {
        if (write(top->fd, outbuf, strlen(outbuf)) == -1)
        {
            remove_player(&(game->head), top->fd);
        }
        top = top->next;
    }
}

/**
 * This function initializes the turns, when the first person 
 * enters the game. 
 */
void initialize_turn(struct game_state *game)
{
    if (game->has_next_turn == NULL)
    {
        struct client *current = game->head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        game->has_next_turn = current;
    }
}

/**
 * This function announces the turn of the next player,
 * and broadcasts it to every active player.
 */
void announce_turn(struct game_state *game)
{
    char turn[MAX_BUF] = {'\0'};
    sprintf(turn, "It is %s's turn.\r\n", game->has_next_turn->name);
    broadcast(game, turn);
}

/**
 * This function advances turn within the linked list
 */
void advance_turn(struct game_state *game)
{
    struct client *current = game->head;
    struct client *previous = NULL;
    if (game->has_next_turn == NULL)
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        previous = current;
    }
    else if (game->has_next_turn->fd == game->head->fd)
    {
        while (current != NULL)
        {
            previous = current;
            current = current->next;
        }
    }
    else
    {
        while ((current != NULL) && (current->fd != game->has_next_turn->fd))
        {
            previous = current;
            current = current->next;
        }
    }
    game->has_next_turn = previous;
    announce_turn(game);
    if (check_gameover(game) == 1)
    {
        // if game is not over, print "Your guess? "
        if (write(game->has_next_turn->fd, "Your guess? \r\n", 14) == -1)
        {
            remove_player(&(game->head), game->has_next_turn->fd);
            if (game->head != NULL)
            {
                advance_turn(game);
                char buffa[MAX_BUF] = {'\0'};
                sprintf(buffa, "Player exit! Playing next turn!");
                broadcast(game, buffa);
            }
        }
    }
}

/**
 * This function broadcasts a message to everyone but the
 * player with the current turn.
 */
void broadcast_audience(struct game_state *game, char *outbuf)
{
    struct client *top = game->head;
    while (top != NULL)
    {
        if (game->has_next_turn->fd != top->fd)
        {
            if (write(top->fd, outbuf, MAX_BUF) == -1)
            {
                remove_player(&(game->head), top->fd);
            }
        }
        top = top->next;
    }
}

/**
 * This function calculates the length of a linked list.
 */
int len_ll(struct client *list)
{
    struct client *a = list;
    int counter = 0;
    while (a != NULL)
    {
        counter++;
        a = a->next;
    }
    return counter;
}

/**
 * This function prints contents of a linked list, used for debugging.
 */
void print_ll(struct game_state game)
{
    struct client *head = game.head;
    while (head != NULL)
    {
        printf("the name: %s\n", head->name);
        printf("file descriptor: %d\n", head->fd);
        head = head->next;
    }
}

/**
 * This function removes a player that has entered a valid name, and adds it
 * onto the linked list of active players.
 */
void remove_valid_player(struct client **list, int fd)
{
    if (*list != NULL)
    {
        if (len_ll(*list) == 1)
        {
            struct client *first;
            first = (*list)->next;
            free(*list);
            *list = first;
        }
        else
        {
            // remove the dude who just entered their name, even if it was out of turn.
            struct client *head = *list;
            struct client *previous = NULL;
            while (head != NULL && head->fd != fd)
            {
                previous = head;
                head = head->next;
            }
            struct client *next = head->next;
            if (previous != NULL)
            {
                previous->next = next;
            }
            else
            {
                *list = next;
            }
        }
    }
    else
    {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}

/* This function reads for input from the the client side of the socket. 
 * Executed inside a while loop so as to make sure that all the output is
 * captured.
 */

void read_from_socket(int filedes, char *name_space)
{
    char *p = NULL;
    int num_chars = read(filedes, name_space, MAX_BUF);
    name_space[num_chars] = '\0';
    p = strchr(name_space, '\r');
    *p = '\0';
}

/* This function searches the struct client * head linked list for any names 
 * that are the same as specified by the user_name given by the user.
 */
int name_not_found(struct client *game_head, char *user_name, int fd)
{
    struct client *current = game_head;
    int indicator = 0;
    while (current != NULL)
    {
        if (strcmp(current->name, user_name) == 0)
        {
            char *outbuf = "The name you entered has already been taken! Try again!\r\n";
            if (write(fd, outbuf, strlen(outbuf)) == -1){
                perror("write");
                exit(1);
            }
            indicator = 1;
        }
        current = current->next;
    }
    return indicator;
}

// WRAPPER FUNCTIONS

/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr)
{
    struct client *p = malloc(sizeof(struct client));

    if (!p)
    {
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
/**
 * This function broadcasts that the game is over,and the new
 * game is about to start.
 */
void broadcast_gameover(struct game_state *game, char *outbuf, int limit)
{
    struct client *head = game->head;
    while (head != NULL)
    {
        if (write(head->fd, outbuf, limit) == -1)
        {
            remove_player(&(game->head), head->fd);
        }
        head = head->next;
    }
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset 
 */
void remove_player(struct client **top, int fd)
{
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p)
    {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    }
    else
    {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                fd);
    }
}

/**
 * this function bids farewell to the exiting player, in
 * case the player exits while playing the game.
 */
void bid_farewell(struct game_state *game, int fd)
{
    struct client *head = game->head;
    while ((head != NULL) && (head->fd != fd))
    {
        head = head->next;
    }
    char farewell[MAX_BUF] = {'\0'};
    sprintf(farewell, "Goodbye %s!\r\n", head->name);
    broadcast_audience(game, farewell);
}

/**
 * this function checks if the letter chosen by the player
 * has already been chosen previously.
 */
int check_if_guessed(struct game_state *game, struct client *p)
{
    int index = p->inbuf[0];
    if (game->letters_guessed[index - 97] == 1)
    {
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <dictionary filename>\n", argv[0]);
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

    while (1)
    {
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1)
        {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset))
        {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd)
            {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1)
            {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&new_players, p->fd);
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
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++)
        {
            if (FD_ISSET(cur_fd, &rset))
            {
                // Check if this socket descriptor is an active player
                for (p = game.head; p != NULL; p = p->next)
                {
                    if (cur_fd == p->fd)
                    {
                        // checking for partial reads on letter choice
                        int nbytes = 0;
                        nbytes = read(cur_fd, p->in_ptr, MAX_BUF);
                        if (nbytes == 0)
                        {
                            if (len_ll(game.head) != 1)
                            {
                                bid_farewell(&game, cur_fd);
                                if (game.has_next_turn->fd == cur_fd)
                                {
                                    advance_turn(&game);
                                }
                                remove_player(&(game.head), cur_fd);
                            }
                            else
                            {
                                game.has_next_turn = NULL;
                                remove_player(&game.head, cur_fd);
                            }
                        }
                        else if (nbytes == -1)
                        {
                            perror("read");
                            exit(1);
                        }
                        p->in_ptr += nbytes;
                        int where;
                        if ((where = find_network_newline2(p->inbuf, MAX_BUF)) < 0)
                        {
                            // partial read found, break loop and re-read
                            break;
                        }
                        else
                        {
                            p->inbuf[where - 2] = '\0';
                            p->in_ptr -= where;
                            memmove(p->inbuf, &(p->inbuf[where]), p->in_ptr - p->inbuf);
                            p->in_ptr = p->inbuf;
                        }
                        if (cur_fd == game.has_next_turn->fd)
                        {
                            // checking for invalid reads

                            if ((strlen(p->inbuf) != 1))
                            {
                                char *invalid = "Invalid! Must be a single character! Try again!\r\n";
                                if (write(cur_fd, invalid, strlen(invalid)) == -1)
                                {
                                    remove_player(&(game.head), cur_fd);
                                }
                                break;
                            }

                            // checking if letter has already been guessed.
                            int check_guessed;
                            if ((check_guessed = check_if_guessed(&game, p)) == 0)
                            {
                                char *invalid = "This letter has already been guessed! Guess again!\r\n";
                                if (write(cur_fd, invalid, strlen(invalid)) == -1)
                                {
                                    remove_player(&(game.head), cur_fd);
                                }
                                break;
                            }

                            char chosenmsg[MAX_BUF] = {'\0'};
                            sprintf(chosenmsg, "You guessed %s\r\n", p->inbuf);
                            char audmsg[MAX_BUF] = {'\0'};
                            sprintf(audmsg, "%s guesses: %s\r\n", game.has_next_turn->name, p->inbuf);
                            broadcast_audience(&game, audmsg);

                            //process the given letter

                            int letter_in_word = check_letter_in_word(&game, p->inbuf);
                            if (letter_in_word == 0)
                            {
                                // letter not in word
                                int letter_guessed_asciii = p->inbuf[0];
                                game.letters_guessed[letter_guessed_asciii - 97] = 1;
                                char letter_not_found[MAX_BUF] = {'\0'};
                                sprintf(letter_not_found, "%c is not in the word!\r\n", p->inbuf[0]);
                                if (write(cur_fd, letter_not_found, strlen(letter_not_found)) == -1)
                                {
                                    remove_player(&(game.head), cur_fd);
                                }
                                game.guesses_left--;

                                char new_turn_status[MAX_BUF] = {'\0'};
                                int over2 = check_gameover(&game);
                                if (over2 == 0)
                                {
                                    char gameover[MAX_BUF] = {'\0'};
                                    sprintf(gameover, "Game over! No more guesses left! The word was %s.\r\n", game.word);
                                    broadcast(&game, gameover);
                                    broadcast(&game, "Starting new game...\r\n");
                                    init_game(&game, argv[1]);
                                    broadcast(&game, status_message(new_turn_status, &game));
                                    advance_turn(&game);
                                }
                                else
                                {
                                    broadcast(&game, status_message(new_turn_status, &game));
                                    advance_turn(&game);
                                }
                            }
                            else
                            {
                                // letter in the word
                                int letter_guessed_ascii = p->inbuf[0];
                                game.letters_guessed[letter_guessed_ascii - 97] = 1;
                                change_guess_scape(&game, p->inbuf);
                                //advance_turn(&game);
                                char new_turn_status[MAX_BUF] = {'\0'};
                                broadcast(&game, status_message(new_turn_status, &game));
                                if (write(cur_fd, "Your guess? \r\n", 14) == -1)
                                {
                                    remove_player(&(game.head), cur_fd);
                                }
                                int over = check_gameover(&game);
                                if (over == 2)
                                {
                                    // game over, all letters were guessed.
                                    char winner[MAX_BUF] = {'\0'};
                                    sprintf(winner, "And the winner is ... %s!!\r\n", game.has_next_turn->name);
                                    broadcast(&game, winner);
                                    broadcast(&game, "\r\nStarting new game...\r\n");
                                    init_game(&game, argv[1]);
                                    char msgbuf[MAX_BUF] = {'\0'};
                                    char *smsg = status_message(msgbuf, &game);
                                    broadcast(&game, smsg);
                                    advance_turn(&game);
                                }
                            }
                        }
                        else
                        {
                            char *not_your_turn = "Not your turn!\r\n";
                            if (write(cur_fd, not_your_turn, strlen(not_your_turn)) == -1)
                            {
                                bid_farewell(&game, cur_fd);
                                remove_player(&(game.head), cur_fd);
                            }
                        }
                        break;
                    }
                }

                // Check if any new players are entering their names
                for (p = new_players; p != NULL; p = p->next)
                {
                    len_ll(new_players);
                    if (cur_fd == p->fd)
                    {
                        // TODO - handle input from an new client who has
                        // not entered an acceptable name.

                        int numbytes = 0;
                        numbytes = read(cur_fd, p->in_ptr, MAX_NAME);
                        if (numbytes == 0)
                        {
                            remove_player(&new_players, cur_fd);
                        }
                        else if (numbytes == -1)
                        {
                            perror("read");
                            exit(1);
                        }
                        p->in_ptr += numbytes;
                        int break_loc;
                        if ((break_loc = find_network_newline2(p->inbuf, MAX_BUF)) < 0)
                        {
                            // partial read found, break loop and re-read
                            break;
                        }
                        else
                        {
                            p->inbuf[break_loc - 2] = '\0';
                            p->in_ptr -= break_loc;
                            memmove(p->inbuf, &(p->inbuf[break_loc]), p->in_ptr - p->inbuf);
                            p->in_ptr = p->inbuf;
                            strncpy(p->name, p->inbuf, MAX_NAME);
                        }

                        if ((strlen(p->name) == 0) || (name_not_found(game.head, p->name, cur_fd) == 1))
                        {
                            break;
                        }

                        add_player_to_game(&game, cur_fd, p->ipaddr, p->name);

                        remove_valid_player(&(new_players), p->fd);

                        char entry_msg[MAX_BUF] = {'\0'};
                        sprintf(entry_msg, "\r\n%s has just joined the game!\r\n", game.head->name);
                        broadcast(&game, entry_msg);

                        char status[MAX_BUF] = {'\0'};
                        if (write(cur_fd, status_message(status, &game), MAX_BUF) == -1)
                        {
                            remove_player(&(game.head), cur_fd);
                        }

                        if ((len_ll(game.head)) == 1)
                        {
                            initialize_turn(&game);
                        }

                        if (write(game.has_next_turn->fd, "Your guess? ", 12) == -1)
                        {
                            advance_turn(&game);
                            remove_player(&(game.head), game.has_next_turn->fd);
                        }

                        if (cur_fd != game.has_next_turn->fd)
                        {
                            char bufffer[MAX_BUF] = {'\0'};
                            sprintf(bufffer, "It is currently %s's turn.\r\n", game.has_next_turn->name);
                            if (write(cur_fd, bufffer, strlen(bufffer)) == -1)
                            {
                                remove_player(&(game.head), cur_fd);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
