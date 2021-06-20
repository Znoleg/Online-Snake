/**
* \file types.h
*/

#ifndef H_TYPES
#define H_TYPES

#include <queue>
using namespace std;


// STRUCTURES ==========================================================
/**
* \typedef coord
* \brief struct Represents a couple of coordinates.
*/
struct coord {
    int x;  /**< x coordinate */
    int y;  /**< y coordinate */
};

/**
* \typedef direction
* \brief Allows to use the four main directions.
*/
typedef enum {UP, DOWN, LEFT, RIGHT} direction;

/**
* \typedef square
* \brief Gathers the possible content of a square of the field.
*/
typedef enum {EMPTY, WALL, SNAKE, SCHLANGA, FOOD, POPWALL, HIGHSPEED, LOWSPEED, FREEZE} square;

/**
* \typedef t_type
* \brief type of a snake, can be either a snake or a schlanga.
*/
typedef enum {T_SNAKE=2, T_SCHLANGA=3} t_type;

/**
* \typedef snake
* \brief Represents a snake
* \details 'body' is an array of 'coord'.
*          'head' holds the index of the coordinates of the head in 'body'
*          'tail' holds the index of the coordinates of the tail in 'body'
*          'dir' is the direction the snake is currently moving.
*/
struct snake {
    t_type type;    /**< type of snake, can be 'SCHLANGA' or 'SNAKE' */
    std::queue<coord> body;   /**< array containing the coords of every part of the snake*/
    direction dir;  /**< current direction the snake is faceing */
    bool add_size;
    
    int get_size() const {return body.size();}
};

/**
* \typedef field
* \brief Represents the arena on which the game is played
*/
struct field {
    square** f;     		/**< 2D array representing the field*/
    int width;     			/**< width of the field */
    int height;     		/**< height of the field */
    int timestep;				/**< basic speed of game */
    int speed;
    int freeze_snake;		/**< freezing-time left for snake */
    int freeze_schlanga;	/**< freezing-time left for schlanga */
};

// PROTOTYPES ==========================================================
// Constructors ========================================================
coord new_coord(int x, int y);
coord new_coord_empty();
field* new_field(int width, int height, int timestep);
snake* new_snake(t_type type, int start_pos, field* map);

// Destructors =========================================================
void free_snake(snake* s);
void free_field(field* map);
void free_all(field* map, snake* s1, snake* s2);

// Objects managment ===================================================
bool are_equal(coord c1, coord c2);
direction opposite(direction d);
direction turn_left(direction d);
direction turn_right(direction d);
square get_square_at(field* map, coord c);
void set_square_at(field* map, coord c, square stuff);
coord get_head_coord(snake* s);
coord get_tail_coord(snake* s);
coord coord_after_dir(coord c, direction dir);
void remove_tail(field* map, snake* s);
void push_head(field* map, snake* s, coord head);

#endif
