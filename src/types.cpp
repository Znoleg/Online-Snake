/**
* \file types.c
* \brief Functions related to types used in other files.
* \details This file is separated in 3 parts :
*          1 - functions that help you create variables of a certain structure type
*          2 - functions that help you free the memory used by the structures
*          3 - functions that help you operate on the structures
*/

#include <stdio.h>      //for 'printf()'
#include <stdlib.h>     //for 'malloc()'
#include <sys/ioctl.h>  //for 'ioctl()'
#include <string.h> //for 'memcpy()'

#include "types.h"
#include "game.h"

// Constructors ========================================================
/**
* \fn coord new_coord(int x, int y);
* \brief Used to create a new variable of type 'coord' that has 'x' and
*        'y' as coordinates
* \returns the newly created 'coord' structure
*/
coord new_coord(int x, int y) {
    coord c;
    c.x = x;
    c.y = y;
    return c;
}

/**
* \fn coord new_coord_empty();
* \brief Used to create a new variable of type 'coord', that has 0 for
*        coordinates
* \returns the newly created 'coord' structure
*/
coord new_coord_empty() {
    coord c;
    c.x = 0;
    c.y = 0;
    return c;
}

/**
* \fn field* new_field();
* \brief Used to create a new 'field'
* \returns a pointer to the newly created 'field' variable
*/
field* new_field(int width, int height, int timestep) {
    struct winsize sz; // Struct containing size of window
    ioctl(0, TIOCGWINSZ, &sz); // Calculate size of window
    //sz.ws_col is the width, sz.ws_col the height of the window
    // if (sz.ws_col < MIN_WINDOW_WIDTH || sz.ws_row < MIN_WINDOW_HEIGHT) {
    //     printf("Your window is too small\n");
    //     exit(1);
    // }
    // if (width > sz.ws_col || height > sz.ws_row){
    //     printf("Your window is too small to create a field of size %ix%i", width, height);
    //     exit(1);
    // }

    int a, b;

    field* map = (field*)malloc(sizeof(field));

    map->width = width;
    map->height = height;

    //creation of 'f'
    map->f = (square**)malloc(map->height*sizeof(square*));
    for (a = 0; a<map->height; a++) {
        map->f[a] = (square*)malloc(map->width*sizeof(square));
    }

    //initialisation of 'f'
    coord c = new_coord_empty();
    for (a = 0; a<map->height; a++) {
        for (b = 0; b<map->width; b++) {
            c.x = a;
            c.y = b;
            if (a == 1 || a == map->height-1 || b == 1 || b == map->width-1) {
                map->f[a][b] = WALL;
                print_to_pos_colored(c, '#', RED);
            } else {
                map->f[a][b] = EMPTY;
                print_to_pos(c, ' ');
            }
        }
    }

    map->freeze_snake = map->freeze_schlanga = 0;
    map->timestep = timestep;
    map->speed = 0;

    return map;
}

/**
* \fn snake* new_snake(t_type type, int size, int start_pos, field* map);
* \brief Used to create a new variable of type 'snake'
* \param start_pos An integer between 0 and 11 corresponding to position at which to start
* \returns a pointer to the newly created 'snake' variable
*/
snake* new_snake(t_type type, int start_pos, field* map) {
    if(start_pos < 0 || start_pos > 11){
        printf("In 'new_snake()' : start_pos unknown.\n");
        exit(1);
    }

    snake* s = (snake*)malloc(sizeof(snake));
    s->body = queue<coord>();

    s->type = type;
    s->add_size = false;

    coord head_coord;
    switch(start_pos){
        case 0:
            head_coord = new_coord(0.5 * map->height, 0.2 * map->width);
            s->dir = RIGHT;
            break;
        case 1:
            head_coord = new_coord(0.5 * map->height, 0.8 * map->width);
            s->dir = LEFT;
            break;
        case 2:
            head_coord = new_coord(0.2 * map->height, 0.5 * map->width);
            s->dir = DOWN;
            break;
        case 3:
            head_coord = new_coord(0.8 * map->height, 0.5 * map->width);
            s->dir = UP;
            break;
        case 4:
            head_coord = new_coord(0.5 * map->height + 2, 0.2 * map->width - 2);
            s->dir = RIGHT;
            break;
        case 5:
            head_coord = new_coord(0.5 * map->height - 2, 0.8 * map->width + 2);
            s->dir = LEFT;
            break;
        case 6:
            head_coord = new_coord(0.2 * map->height-2, 0.5 * map->width - 2);
            s->dir = DOWN;
            break;
        case 7:
            head_coord = new_coord(0.8 * map->height + 2, 0.5 * map->width + 2);
            s->dir = UP;
            break;
        case 8:
            head_coord = new_coord(0.5 * map->height - 2, 0.2 * map->width - 2);
            s->dir = RIGHT;
            break;
        case 9:
            head_coord = new_coord(0.5 * map->height + 2, 0.8 * map->width + 2);
            s->dir = LEFT;
            break;
        case 10:
            head_coord = new_coord(0.2 * map->height - 2, 0.5 * map->width + 2);
            s->dir = DOWN;
            break;
        case 11:
            head_coord = new_coord(0.8 * map->height + 2, 0.5 * map->width - 2);
            s->dir = UP;
            break;
    }

    if(s->type == T_SNAKE){
        print_to_pos_colored(head_coord, 's', BLUE);
    }
    if(s->type == T_SCHLANGA){
        print_to_pos_colored(head_coord, '$', YELLOW);
    }

    push_head(map, s, head_coord);
    //set_square_at(map, head_coord, SNAKE);

    return s;
}

// Destructors =========================================================
/**
* \fn void free_snake(snake* s);
* \brief Used to free memory used by the 's' snake
*/
void free_snake(snake* s){
    free(s);
}

/**
* \fn void free_field(field* map);
* \brief Used to free memory used by the 'map' field
*/
void free_field(field* map){
    int i;
    for(i = 0; i<map->height; i++){
        free(map->f[i]);
    }
    free(map->f);
    free(map);
}

/**
* \fn void free_all(field* map, snake* s1, snake* s2);
* \brief frees the field and the two snakes passed in parameter
*/
void free_all(field* map, snake* s1, snake* s2){
    //freeing memory
    free_field(map);
    free_snake(s1);
    free_snake(s2);
}

// Objects managment ===================================================
/**
* \fn bool are_equal(coord c1, coord c2);
* \return 1 if 'c1' and 'c2' are equal
*/

bool are_equal(coord c1, coord c2){
    return (c1.x == c2.x && c1.y == c2.y);
}

/**
* \fn direction opposite(direction d);
* \return the opposite direction of 'd'
*/
direction opposite(direction d){
    switch(d){
        case UP:
            return DOWN;
            break;
        case DOWN:
            return UP;
            break;
        case LEFT:
            return RIGHT;
            break;
        case RIGHT:
            return LEFT;
            break;
        default:
            printf("in opposite() : Unrecognized dir\n");
            exit(1);
            break;
    }
}


/**
* \fn direction turn_left(direction d);
* \return the direction to the left of 'd'
*/
direction turn_left(direction d){
    switch(d){
        case UP:
            return LEFT;
            break;
        case DOWN:
            return RIGHT;
            break;
        case LEFT:
            return DOWN;
            break;
        case RIGHT:
            return UP;
            break;
        default:
            printf("in turn_left() : Unrecognized dir\n");
            exit(1);
            break;
    }
}


/**
* \fn direction turn_right(direction d);
* \return the direction to the right of 'd'
*/
direction turn_right(direction d){
    switch(d){
        case UP:
            return RIGHT;
            break;
        case DOWN:
            return LEFT;
            break;
        case LEFT:
            return UP;
            break;
        case RIGHT:
            return DOWN;
            break;
        default:
            printf("in turn_right() : Unrecognized dir\n");
            exit(1);
            break;
    }
}

/**
* \fn square get_square_at(field* map, coord c);
* \return the square at 'c' on 'map'
*/
square get_square_at(field* map, coord c){
    return map->f[c.x][c.y];
}

/**
* \fn square set_square_at(field* map, coord c, square stuff);
* \brief Sets 'square' at 'c' on 'map'.
*/
void set_square_at(field* map, coord c, square stuff){
    if(c.x == -1 && c.y == -1) return;
    map->f[c.x][c.y] = stuff;
}

/**
* \fn coord get_head_coord(snake* s);
* \return the coordinates of the head of 's'
*/
coord get_head_coord(snake* s) {
    return s->body.back();
}

/**
* \fn coord get_tail_coord(snake* s);
* \return the coordinates of the tail of 's'
*/
coord get_tail_coord(snake* s){
    return s->body.front();
}

void push_head(field* map, snake* s, coord head) {
    s->body.push(head);
    set_square_at(map, head, (square)s->type);
}

void remove_tail(field* map, snake* s) {
    coord tail = get_tail_coord(s);
    s->body.pop();
    print_to_pos(tail, ' ');
    set_square_at(map, tail, EMPTY);
}

/**
* \fn coord coord_after_dir(coord c, direction dir);
* \returns The coordinates of where you'd get if you moved 1 square from 'c'
*          with the 'dir' direction.
*/
coord coord_after_dir(coord c, direction dir){
    if (dir == UP) {
        return new_coord(c.x-1, c.y);
    } else if (dir == LEFT) {
        return new_coord(c.x, c.y-1);
    } else if (dir == RIGHT) {
        return new_coord(c.x, c.y+1);
    } else if (dir == DOWN) {
        return new_coord(c.x+1, c.y);
    }
    else{
        printf("in coord_after_dir() : given dir is not a direction\n");
        exit(0);
    }
}
