/**
* \file game.c
* \brief Functions related to the game
* \details This file is separated in 3 parts :
*          1 - functions related to the game per se
*          2 - functions related to input/output management
*          3 - functions related to the display
*/

#include <stdbool.h>        //for 'bool' type
#include <stdio.h>          //for 'printf()'
#include <stdlib.h>         //for 'exit()'
#include <termios.h>        //for 'struct termios'
#include <unistd.h>         //for 'usleep()'
#include <sys/ioctl.h>      //for 'ioctl()'

#include "types.h"
#include "AI.h"
#include "queue.h"

#include "game.h"

//Game ================================================================
/**
* \fn void play(config cfg);
* \brief Function that launches a game and makes it run.
* \param cfg a variable of type 'config' containing options for the game.
* \details This function cares for the creation of the snakes and field,
*          the passing of the time, the collision management and so on.
*/
void play(config cfg) {
    clear();
    mode_raw(1);

    //creating field
    struct winsize sz; // Struct containing size of window
    ioctl(0, TIOCGWINSZ, &sz); // Calculate size of window
    field* map = new_field(sz.ws_col, sz.ws_row, cfg.timestep);

    //creating snakes
    const int snake_pos = 0;
    const int shlanga_pos = 1;
    snake* s = new_snake(T_SNAKE, snake_pos, map);            //Create snake with size 10 at start_pos on map
    snake* schlanga = new_snake(T_SCHLANGA, shlanga_pos, map); //Create snake with size 10 at start_pos on map

    myqueue p1_queue = new_queue(MAX_INPUT_STACK);    //queue used to stack p1 input
    myqueue p2_queue = new_queue(MAX_INPUT_STACK);    //queue used to stack p2 input

    char c;               //key that is pressed
    int ret;              //value returned by 'read()', 0 if no new key was pressed
    bool snake_dead;      //True if the snake is dead.
    bool schlanga_dead;   //True if the schlanga died.
    int random_item;      //Random integer deciding if an item pops or not
    direction cur_dir;

    //Main loop
    //1 - pass time
    //2 - retrieve and sort input
    //3 - make snakes move
    //4 - check if someone died
    //5 - handle items
    while(1){
        //1 - let's pass time
        usleep(map->timestep * 1000 - map->speed);

        //2 - let's retrieve and sort every input.
        while((ret = read(0, &c, sizeof(char))) != 0){
            if(ret == -1){
                perror("read in 'play()'");
                exit(1);
            }

            if(c == C_QUIT){
                mode_raw(0);
                clear();
                myfree_queue(&p1_queue);
                myfree_queue(&p2_queue);
                free_all(map, s, schlanga);
                return;
            }
            else if(key_is_p1_dir(c)){
                //if the key is a move key for player 1:
                if(! myqueue_full(&p1_queue)){
                    myenqueue(&p1_queue, key_to_dir(c));
                }
            }
            else if(cfg.mode == 2 && key_is_p2_dir(c)){
                if(! myqueue_full(&p2_queue)){
                    myenqueue(&p2_queue, key_to_dir(c));
                }
            }
            else{
                //key pressed was a useless key. Do nothing.
            }
        }

        //3 - let's make snakes move
        //snake
        if (map->freeze_snake > 0) {
            map->freeze_snake--;
        }
        else{
            cur_dir = (! myqueue_empty(&p1_queue)) ? mydequeue(&p1_queue) : s->dir;
            cur_dir = (cur_dir == opposite(s->dir)) ? s->dir : cur_dir;
            snake_dead = move(s, cur_dir, map);
        }

        //schlanga
        if (map->freeze_schlanga > 0) {
            map->freeze_schlanga--;
        }
        else{
            if(cfg.mode == 2){
                cur_dir = (! myqueue_empty(&p2_queue)) ? mydequeue(&p2_queue) : schlanga->dir;
                cur_dir = (cur_dir == opposite(schlanga->dir)) ? schlanga->dir : cur_dir;
            }
            else{
                switch(cfg.AI_version){
                    case 1:
                        cur_dir = rngesus(schlanga);
                        break;
                    case 2:
                        cur_dir = rngesus2(schlanga, map);
                        break;
                    case 3:
                        cur_dir = spread(schlanga, map);
                        break;
                    case 4:
                        cur_dir = aggro_dist(schlanga, map, s);
                        break;
                    case 5 :
                        cur_dir = defensif_dist(schlanga, map, s);
                        break;
                    case 6 :
                        cur_dir = heat_map(schlanga, map);
                        break;
                    default:
                        myfree_queue(&p1_queue);
                        myfree_queue(&p2_queue);
                        free_all(map, s, schlanga); mode_raw(0); clear();
                        printf("In 'move()' : AI_version not recognized.\n");
                        exit(1);
                        break;
                }
            }
            schlanga_dead = move(schlanga, cur_dir, map);
        }

        //4 - let's check if someone has died
        if(schlanga_dead){
            myfree_queue(&p1_queue);
            myfree_queue(&p2_queue);
			free_all(map, s, schlanga);
			mode_raw(0);
			clear();
			print_msg("     SCHLANGA DIED      ");
			return;
		}
        else if(snake_dead){
            myfree_queue(&p1_queue);
            myfree_queue(&p2_queue);
            free_all(map, s, schlanga);
            mode_raw(0);
            clear();
            print_msg("       SNAKE DIED       ");
            return;
        }

        //5 - let's gereate (or not) items
		random_item = rand() % 10;
		if (random_item == 0) {
            coord item_loc;
			pop_item(map, true, item_loc);
		}

        fflush(stdout);
    }//end while(1)
}

/**
* \fn int move(snake* s, direction d, field* map);
* \brief operates on a snake structure to make it move one step with the 'd'
*        direction. This function does not protect the snake from going into its neck.
*        This function also cares for collision management and display update.
* \return Number corresponding to an event : 0 if snake/schlanga moves peacefully
*                                            1 if snake/schlanga dies
*/
int move(snake* s, direction d, field* map) {
    static int iter = 0;
    //MOVING SNAKE

    //Updating snake's head coordinates
    s->dir = d;
    coord c_oldhead = get_head_coord(s);
    coord c_newhead;
    if (d == UP) {
        c_newhead = new_coord(c_oldhead.x - 1, c_oldhead.y);
    } else if (d == LEFT) {
        c_newhead = new_coord(c_oldhead.x, c_oldhead.y - 1);
    } else if (d == RIGHT) {
        c_newhead = new_coord(c_oldhead.x, c_oldhead.y + 1);
    } else if (d == DOWN) {
        c_newhead = new_coord(c_oldhead.x + 1, c_oldhead.y);
    }

    // DISPLAY    
    if(s->type == T_SNAKE){
        print_to_pos_colored(c_newhead, 's', BLUE);
    }
    if(s->type == T_SCHLANGA){
        print_to_pos_colored(c_newhead, '$', YELLOW);
    }

    //COLLISIONS
    square temp_square = get_square_at(map, c_newhead);

	int collision;

	switch(temp_square) {
		case WALL:
			//~ write(2, "hit a wall\n", 11*sizeof(char));
			collision = 1;
			break;
		case SCHLANGA:
			//~ write(2, "hit schlanga\n", 13*sizeof(char));
			collision = 1;
			break;
		case SNAKE:
			//~ write(2, "hit snake\n", 10*sizeof(char));
			collision = 1;
			break;
		case FOOD:
			collision = 0;
            s->add_size = true;
			break;
		case POPWALL:
			{
				int popwall;
				int nbwalls = map->width*map->height/(100+rand()%50);
				for (popwall = 0; popwall < nbwalls; popwall++) {
					coord pos_wall = new_coord(1 + rand() % (map->height-1), 1 + rand() % (map->width-1));
					if (get_square_at(map, pos_wall) == EMPTY) {
						set_square_at(map, pos_wall, WALL);
						print_to_pos_colored(pos_wall, '#', RED);
					}
				}
				collision = 0;
				break;
			}
		case HIGHSPEED:
			collision = 0;
			map->speed += ADD_SPEED;
			break;
		case LOWSPEED:
			collision = 0;
			map->speed -= ADD_SPEED;
			break;
		case FREEZE:
			collision = 0;
			if(s->type == T_SNAKE){
				map->freeze_schlanga = FREEZING_TIME;
			} else if(s->type == T_SCHLANGA){
				map->freeze_snake = FREEZING_TIME;
			}
			break;
		case EMPTY:
			//~ write(2, "hit ???\n", 7*sizeof(char));
			collision = 0;
			break;
    }

    push_head(map, s, c_newhead);

	if (collision == 0) {
		//UPDATE FIELD

        if (s->add_size == false)
        {
		    remove_tail(map, s);
        }
        else
        {
            s->add_size = false;
        }
    }
    
    return collision;
}

/**
* \fn void pop_item(field* map);
* \brief adds a random item to the field.
*/
square pop_item(field* map, bool generate_freeze, coord& item_loc) {
    coord pos_item;
    square item;
    char item_char;
    int dir = generate_freeze + rand() % 7;

    do {
        pos_item = new_coord(1 + rand() % (map->height-1), 1 + rand() % (map->width-1));
    } while (get_square_at(map, pos_item) != EMPTY);

    switch (dir) {
        case 0:
            item = FREEZE;
            item_char = '*';
            break;
        case 1:
			if (map->speed >= 5*ADD_SPEED) {
				item = (square)-1;
			} else {
				item = HIGHSPEED;
				item_char = '>';
			}
            break;
        case 2:
			if (map->speed <= -5*ADD_SPEED) {
				item = (square)-1;
			} else {
				item = LOWSPEED;
				item_char = '<';
			}
            break;
        case 3:
			item = POPWALL;
			item_char = 'W';
            break;
        case 4: case 5: case 6: case 7:
            item = FOOD;
            item_char = 'x';
            break;
        default:
            item = (square)-1;
            break;
    }

    if (item > 0) {
        set_square_at(map, pos_item, item);
        print_to_pos(pos_item, item_char);
    }

    item_loc = pos_item;
    return item;
}

//Input/Output ========================================================
/**
* \fn bool key_is_p1_dir(char c);
* \return 1 if the given char 'c' corresponds to a direction key for player 1.
*         0 otherwise.
*/
bool key_is_p1_dir(char c){
    return (c == C_P1_UP || c == C_P1_DOWN || c == C_P1_LEFT || c == C_P1_RIGHT);
}

/**
* \fn bool key_is_p2_dir(char c);
* \return 1 if the given char 'c' corresponds to a direction key for player 2.
*         0 otherwise.
*/
bool key_is_p2_dir(char c){
    return (c == C_P2_UP || c == C_P2_DOWN || c == C_P2_LEFT || c == C_P2_RIGHT);
}

/**
* \fn direction key_to_dir(char c);
* \return If the given char 'c' correspond to a direction, this function returns
*         the direction.
*/
direction key_to_dir(char c){
    switch(c){
        case C_P1_UP:
            return UP;
            break;
        case C_P2_UP:
            return UP;
            break;
        case C_P1_DOWN:
            return DOWN;
            break;
        case C_P2_DOWN:
            return DOWN;
            break;
        case C_P1_LEFT:
            return LEFT;
            break;
        case C_P2_LEFT:
            return LEFT;
            break;
        case C_P1_RIGHT:
            return RIGHT;
            break;
        case C_P2_RIGHT:
            return RIGHT;
            break;
        default:
            printf("in key_to_dir() : given char is not a direction");
            exit(1);
            break;
    }
}

//Display =============================================================
/**
* \fn void print_to_pos(coord pos, char c);
* \brief prints the character 'c' at the given position
* \details this functions sets the cursor to [x,y] pos and
*          prints the 'c' param
*/
void print_to_pos(coord pos, char c) {
    if(pos.x == -1 && pos.y == -1) return;
    #ifndef DO_NOT_DISPLAY
    printf("\033[%d;%dH%c", pos.x, pos.y, c);
    #endif
}

/**
* \fn void print_to_pos_colored(coord pos, char c, char* color);
* \brief prints the character 'c' at the given position in chosen color
* \details this functions sets the cursor to [x,y] pos and
*          prints the 'c' param with chosen color
*/
void print_to_pos_colored(coord pos, char c, char* color) {
    if(pos.x == -1 && pos.y == -1) return;
    #ifndef DO_NOT_DISPLAY
    printf("%s", color);
    printf("\033[%d;%dH%c", pos.x, pos.y, c);
    printf(RESET_COLOR);
    #endif
}

/**
* \fn void mode_raw(int activate);
* \brief Use mode_raw(1) to enable raw mode, mode_raw(0) to disable.
*        See detailed description for more info.
* \details When you call 'mode_raw(1)', the terminal switches into raw mode, if it was
*          not already. Cursor is hidden. User input is hidden. User input is
*          readable with a call to 'read()' without the user having to press "enter".
*          The call to 'read()' is not blocking, and it will return 0 if there was
*          less characters to read in the buffer than the requested number.
*          When you call 'mode_raw(0)', STDIN will be flushed, the cursor will show
*          again, and the terminal will switch back to it's original state,
*          whatever it was.
*/
void mode_raw(int activate)
{
    //"static" variables are inited only once, and then shared between calls of this function.
    static bool first_run = true;       //1 if this is the first time this function is run
    static bool raw_activated = false;  //true if the raw mode is activated
    static struct termios term_save;    //configuration to apply to have the terminal back to its initial mode
    static struct termios term_raw;     //configuration to apply to have raw-mode

    char trash;     //we need this variable to flush STDIN

    //PREPARING NEEDED
    if(first_run){
        first_run = false;

        //in this block, we build 'term_save' and 'term_raw' once and for all
        //BUILDING 'term_save'
        tcgetattr(STDIN_FILENO, &term_save);    //save the current terminal config.

        //BUILDING 'term_raw'
        term_raw = term_save;

        //the line "flags &= ~flag" has the effect of removing 'flag' from 'flags'
        term_raw.c_lflag &= ~ICANON;        //set non-canonical mode (see termios(3))
        term_raw.c_lflag &= ~ECHO;          //do not echo user input !

        //make 'read()' a "polling read" (see termios(3))
        term_raw.c_cc[VMIN] = 0;
        term_raw.c_cc[VTIME] = 0;
    }

    //BULK OF THE FUNCTION :
    if(activate){   //the user wants to activate the raw mode
        if(raw_activated) return;   //if it's already activated, then do nothing

        tcsetattr(STDIN_FILENO, TCSANOW, &term_raw); //apply raw-mode configuration
        printf("\e[?25l"); //Hide cursor

        raw_activated = true;
    }
    else{           //the user wants to disactivate the raw mode
        if(!raw_activated) return;  //if it was not activated, then do nothing

        //flush STDIN
        while(read(0, &trash, sizeof(char)) != 0){}

        tcsetattr(STDIN_FILENO, TCSANOW, &term_save); //apply original configuration
        printf("\e[?25h");  //unhide cursor

        raw_activated = false;
    }
}

/**
* \fn void print_msg(char* msg);
* \brief prints a specific message onto the screen.
* \details Warning : console has to be working in the usual way for theses messages
*          to be printed correctly. Use 'mode_raw(0)' to get the console back
*          back to normal.
*/
void print_msg(char* msg){
    printf("----------------------------\n");
    printf("| %s |\n", msg);
    printf("----------------------------\n");
}
