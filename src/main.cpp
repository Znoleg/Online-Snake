/**
* \file main.c
* \brief Entry point of the program
*/

#include <stdio.h>          //for 'printf()'
#include <stdlib.h>         //for 'srand()'
#include <unistd.h>         //for 'usleep()'
#include <time.h>           //for 'time()'
#include <signal.h>         //for 'SIGINT' and 'signal()'

#include "game.h"
#include "AI.h"

/**
* \fn void instructions();
* \brief Displays info about how to play the game
*/
void instructions(){
    printf("Moving snakes :\n\
        Player 1 : %c for up, %c for down, %c for left and %c for right.\n\
        Player 2 : %c for up, %c for down, %c for left and %c for right.\n",
        C_P1_UP, C_P1_DOWN, C_P1_LEFT, C_P1_RIGHT,
        C_P2_UP, C_P2_DOWN, C_P2_LEFT, C_P2_RIGHT);
}

/**
* \fn void quit();
* \brief safely quits
*/
void quit(int sig){
    clear();
    mode_raw(0);
    exit(1);
}

/**
* \fn int main();
* \brief Entry point of the program.
*/
int main(){
    signal(SIGINT, quit);

    clear();
    srand(time(NULL));
    printf("==============================================\n");
    printf("||                 WORM GAME                ||\n");
    printf("||             [By Slava Semenov]           ||\n");
    printf("==============================================\n");

    int choice;
    bool ok = false;
    config cfg;
    cfg.size = 1;

    while(!ok){
        printf("\n");
        printf("0 - Quit game\n");
        printf("1 - Play against an AI\n");
        printf("2 - Play with another human\n");
        printf("3 - How to play ?\n");
        printf("Please enter your choice.\n");
        if(scanf("%i", &choice) == 0){
            printf("Menu error\n");
            exit(1);
        }
        switch(choice){
            case 0:
                ok = true;
                printf("Good bye !\n");
                break;
            case 1:
                cfg.mode = 1;

                printf("Enter time step in miliseconds (<%i is not recommended):\n", REC_TIME_STEP);
                if(scanf("%i", &(cfg.timestep)) == 0){
                    printf("Menu error\n");
                    exit(1);
                }
                if(cfg.timestep <= 0){
                    cfg.timestep = REC_TIME_STEP;
                }

                printf("Select the version of the AI : between 1 and 6.\n");
                if(scanf("%i", &(cfg.AI_version)) == 0){
                    printf("Menu error\n");
                    exit(1);
                }
                if(cfg.AI_version >= 1 && cfg.AI_version <= 6){
                    play(cfg);
                }
                else{
                    printf("Version of AI not implemented. Returning to main menu.\n");
                    sleep(1);
                }
                break;
            case 2:
                cfg.mode = 2;

                printf("Enter time step in miliseconds (<%i is not recommended):\n", REC_TIME_STEP);
                if(scanf("%i", &(cfg.timestep)) == 0){
                    printf("Menu error\n");
                    exit(1);
                }
                if(cfg.timestep <= 0){
                    cfg.timestep = REC_TIME_STEP;
                }

                play(cfg);
                break;
            case 3:
                instructions();
                break;
            default :
                printf("This choice does not exist. Returning to main menu.\n");
                fflush(stdin);
                fflush(stdout);
                sleep(1);
                break;
        }
    }

    return 0;
}

