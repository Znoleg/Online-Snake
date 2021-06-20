#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //for 'struct sockaddr_in'
#include <strings.h>    //for 'bzero()'
#include <unistd.h>     //for 'read()'
#include <sys/time.h>
#include <signal.h>

#include "game.h"

#define BACKLOG 10
//#define SERV_ADDR "192.168.0.38"
#define SERV_ADDR "127.0.0.1"
#define PORT 3490
#define MAX_PLAYERS 10
#define SNAKESIZE 1         //size of the snake
#define PING 10

#define WIDTH 60    //size of the square arena
#define HEIGHT 25

vector<int> players;
direction* players_dir;
int sockfd;

void safe_quit(int return_value)
{
    int i;
    close(sockfd);
    for(i = 0; i<MAX_PLAYERS; i++)
    {
        close(players[i]);
    }
    printf("safe quitted\n");
    exit(return_value);
}

/**
* \fn int diff(struct timeval big_time, struct timeval small_time);
* \returns The time in ms that separates 'big_time' from 'small_time'
*/
int diff_time(struct timeval big_time, struct timeval small_time)
{
    int sec = big_time.tv_sec - small_time.tv_sec;
    int usec = big_time.tv_usec - small_time.tv_usec;
    return sec * 1000 + usec / 1000;
}

int count_players()
{
    int i;
    for(i = 0; i<MAX_PLAYERS; i++)
    {
        if(players[i] == -1) return i;
    }
    return i;
}

void create_listen_socket()
{
    printf("Creating socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        safe_quit(1);
    }
    printf("Ok.\n");

    printf("Preparing server adress...\n");
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    inet_aton(SERV_ADDR, (struct in_addr*) &my_addr.sin_addr.s_addr);
    bzero(&(my_addr.sin_zero), 8);
    printf("Ok.\n");

    printf("Binding adress to socket...\n");
    if (bind(sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        safe_quit(1);
    }
    printf("Ok.\n");

    printf("Making socket a listen socket...\n");
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Ok.\n");
}

void* read_client(void* id_p)
{
    int* id = (int*)id_p;
    direction cur_dir;

    while (true)
    {
        if (read(players[*id], &cur_dir, 1 * sizeof(direction)) == -1)
        {
            perror("read on a client");  safe_quit(1);
        }
        if (cur_dir >= 0 && cur_dir <= 3 && players_dir[*id] != 4)
        {
            //if player's move is valid and if he's not dead
            players_dir[*id] = cur_dir;
            printf("Read dir %i for client %i.\n", cur_dir, *id);
        }
    }

}

void* accept_players(void*)
{
    int newfd;
    struct sockaddr their_addr;
    socklen_t serverlen = sizeof(struct sockaddr_in);

    while (players.size() <= MAX_PLAYERS)
    {
        if ((newfd = accept(sockfd, &their_addr, &serverlen)) == -1)
        {
            perror("accept");
            safe_quit(1);
        }
        players.push_back(newfd);
        printf("A new player joined the game. Connected players : %i.\n", (int)players.size());
    }
    return 0;
}

void play_server(config cfg) 
{
    //creating field
    field* map = new_field(WIDTH, HEIGHT, cfg.timestep);

    //creating direction arr
    players_dir = new direction[cfg.nb_players];

    //creating snakes
    int i;
    snake** snakes = (snake**)malloc(cfg.nb_players*sizeof(snake*));
    for (i = 0; i < cfg.nb_players; i++)
    {
        snakes[i] = new_snake(T_SNAKE, cfg.size, map);
    }

    struct timeval last_step_time;
    gettimeofday(&last_step_time, NULL);
    struct timeval now;
    int elapsed_time;       //elapsed time
    int c_dead = 0;         //count dead players
    while (true)
    {
        //SUMMARY
        //1 - let's check if it's time to process inputs
        //2 - let's send everyone the directions
        //3 - let's make snakes move
        //4 - let's update last_step_time
        //5 - let's check if the game has to end
        //--------------------------------------

        //1 - let's check if it's time to retrieve input
        gettimeofday(&now, NULL);
        elapsed_time = diff_time(now, last_step_time); //time elapsed since last round. in ms.
        if (elapsed_time > cfg.timestep - (PING / 2))
        {
            //2 - let's send everyone the directions
            for (i = 0; i < cfg.nb_players; i++)
            {
                write(players[i], &players_dir, cfg.nb_players * sizeof(direction));
            }
            printf("Directions sent : \n");
            for (i = 0; i < cfg.nb_players; i++)
            {
                printf("%i ", players_dir[i]);
            }
            printf("\n");

            //3 - let's make snakes move
            for (i = 0; i < cfg.nb_players; i++)
            {
                if (players_dir[i] != 4)
                {   
                    if (move(snakes[i], (direction)players_dir[i], map))
                    {
                        players_dir[i] = (direction)4;
                        printf("Player %i died.\n", i);
                        c_dead++;
                    }
                }
            }

            //4 - let's update last_step_time
            gettimeofday(&last_step_time, NULL);

            //5 - let's check if the game has to end
            if (c_dead >= cfg.nb_players - 1)
            {
                printf("Game has ended, only one player left alive.\n");
                sleep(3);
                break;
            }
            
            //6 - let's generate item
            coord item_pos;
            if (rand() % 4 == 0) // 25%
            {
                square item = pop_item(map, false, item_pos);
                for (int i = 0; i < cfg.nb_players; i++)
                {
                    write(players[i], &item, sizeof(square));
                }
            }
            else
            {

            }
        }
        else
        {
            //sleep 90% of the remaining time
            usleep((((cfg.timestep - (PING / 2)) - elapsed_time) * 0.9) * 1000);
        }
    }

    for(i = 0; i<cfg.nb_players; i++)
    {
        free_snake(snakes[i]);
    }
    free_field(map);
}

int main()
{
    signal(SIGINT, safe_quit);

    int size = SNAKESIZE;
    int width = WIDTH;
    int height = HEIGHT;
    int i;
    pthread_t accept_thread;                //accept_player thread
    pthread_t threads[MAX_PLAYERS];         //read_clients threads
    int wait_players = 1;
    int ok = 1;     //signal to send players
    int ret_cli;
    char c;

    //CREATING LISTEN SOCKET
    create_listen_socket();

    //RECIEVING CONNECTIONS
    printf("The server is now open to connections.\nPress ctrl+D when everyone has joined.\n");
    if (pthread_create(&accept_thread, 0, accept_players, &wait_players) != 0)
    {
        printf("error : could not create thread\n");
        safe_quit(1);
    }

    int player_cnt;
    while (true)
    {
        while (read(0, &c, sizeof(char)) != 0);
        player_cnt = players.size();
        if (player_cnt < 2)
        {
            printf("Not enough player to start a game.\n");
        }
        else
        {
            pthread_cancel(accept_thread);
            break;
        }
    }

    wait_players = 0;   //this will cause the accept_players thread to terminate
    printf("From now on, no new players will be accepted.\npress ctrl+D to start the game.\n");

    //SENDING INFO TO CLIENTS
    for (i = 0; i < player_cnt; i++)
    {
        write(players[i], &size, sizeof(int));
        write(players[i], &player_cnt, sizeof(int));
        write(players[i], &i, sizeof(int));
        write(players[i], &width, sizeof(int));
        write(players[i], &height, sizeof(int));
    }

    //LAUNCHING THREADS TO READ CLIENTS
    for (i = 0; i < player_cnt; i++)
    {
        if (pthread_create(&threads[i], 0, read_client, 0) != 0)
        {
            printf("Couldn't create thread for client %i.\n", i);
        }
    }

    //WAITING TO START THE GAME
    while (read(0, &c, sizeof(char)) != 0);

    printf("Sending signal to clients...\n");
    //SENDING SIGNAL TO CLIENTS
    for(i = 0; i < player_cnt; i++)
    {
        ret_cli = write(players[i], &ok, 1*sizeof(int));
        if(ret_cli < 0)
        {
            perror("write sending signal"); safe_quit(1);
        }
        else if(ret_cli == 0)
        {
            printf("Client %i closed connection.\n", i); safe_quit(1);
        }
    }
    printf("Ok.\n");

    config cfg;
    cfg.size = size;
    cfg.nb_players = player_cnt;

    play_server(cfg);

    printf("Press ctrl+D to terminate server.\n");
    while(read(0, &c, sizeof(char)) != 0);

    //ENDING
    safe_quit(0);

    return 0;
}