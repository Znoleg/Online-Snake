/**
* \file AI.h
*/

#ifndef H_AI
#define H_AI

// CONSTANTS ============================================================
#define IA_MAX_PICK 20 /**< maximum times that the IA tries
                            picking a random direction before giving up.
                            Used to avoid infinite picking.*/

// PROTOTYPES ==========================================================
// Helpers =============================================================
int detect(snake* s, direction c, field* map);
bool not_in(coord c, coord* tableau, int taille);
int rec(field* map, coord c, coord* tableau, int* i);
float dist(coord depart, coord arrivee);
bool compare_aggro(float a, float b);
direction best_aggro(float a, float b, float c, float d, snake* s, field* map);
bool compare_def(float a, float b);
direction best_def(float a, float b, float c, float d, snake* s, field* map);

// AI main functions ===================================================
direction rngesus(snake* s);
direction rngesus2(snake* s, field* map);
direction spread(snake* s,field* map);
direction aggro_dist(snake* s, field* map, snake* enemy);
direction defensif_dist(snake* s, field* map, snake* enemy);
direction heat_map(snake* s, field* map);

#endif
