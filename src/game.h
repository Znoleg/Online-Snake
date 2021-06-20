/**
* \file game.h
*/

#ifndef H_GAME
#define H_GAME

#include "types.h"

// CONSTANTS ============================================================
// OPTIONS
#define C_P1_UP 'w'        /**< key to move player 1 up.       */
#define C_P1_DOWN 's'      /**< key to move player 1 down.     */
#define C_P1_LEFT 'a'      /**< key to move player 1 left.     */
#define C_P1_RIGHT 'd'     /**< key to move player 1 right.    */

#define C_P2_UP 'i'        /**< key to move player 2 up.       */
#define C_P2_DOWN 'k'      /**< key to move player 2 down.     */
#define C_P2_LEFT 'j'      /**< key to move player 2 left.     */
#define C_P2_RIGHT 'l'     /**< key to move player 2 right.    */

#define C_QUIT 27          /**< number of key to quit. 27 = esc      */

#define MIN_WINDOW_WIDTH 15
#define MIN_WINDOW_HEIGHT 15

#define REC_TIME_STEP 150   /**< time between two time steps. In msec. */

#define NB_ITEMS 5        /**< number of items in game */
#define FREEZING_TIME 10  /**< number of iterations during which a snake will be frozen */
#define ADD_SPEED 25000   /**< add x seconds to usleep */
#define MAX_INPUT_STACK 5 /**< maximum inputs that can stack for a player */

// UTILITY
#define RED     "\033[1m\033[31m"      /* Red */
#define GREEN   "\033[1m\033[32m"      /* Green */
#define YELLOW  "\033[1m\033[33m"      /* Yellow */
#define BLUE    "\033[1m\033[34m"      /* Blue */
#define RESET_COLOR "\033[0m"

#define clear() printf("\e[1;1H\e[2J") // Clear screen

// STRUCTURES ==========================================================
/**
* \typdef config
* \brief A type that can hold configuration options for a game
*/
typedef struct config{
    int size;           //snake size
    int AI_version;
    int mode;           //1 if human vs schlanga, 2 if human vs human, 3 if human vs humans through server
    int nb_players;             //number of players (for online version)
    int id;                     //id of the client's snake (for online version)
    int timestep;
} config;

// PROTOTYPES ==========================================================
// Game ================================================================
void play(config cfg);
int move(snake* s, direction d, field* map);
square pop_item(field* map, bool generate_freeze, coord& loc);

// Input/Output ========================================================
bool key_is_p1_dir(char c);
bool key_is_p2_dir(char c);
direction key_to_dir(char c);

// Display =============================================================
void print_to_pos(coord pos, char c);
void print_to_pos_colored(coord pos, char c, char* color);
void mode_raw(int activate);
void print_msg(char* msg);

#endif
