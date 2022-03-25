#ifndef MUSEUMSIM_H
#define MUSEUMSIM_H

#define VISITORS_PER_GUIDE 10
#define GUIDES_ALLOWED_INSIDE 2

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

// Will implement the following functions in museumsim.c.

void museum_init(int num_guides, int num_visitors);

void museum_destroy();

void visitor(int id);

void guide(int id);


/**
 * Callback after a visitor arrives at the museum.
 */
void visitor_arrives(int id);

/**
 * Callback after a visitor is ready to begin touring the museum.
 * Will sleep for a configurable duration.
 */
void visitor_tours(int id);

/**
 * Callback after a visitor has left the museum.
 */
void visitor_leaves(int id);


/**
 * Callback after a guide arrives at the museum.
 */
void guide_arrives(int id);

/**
 * Callback after a guide enters the museum.
 */
void guide_enters(int id);

/**
 * Callback after a guide admits a visitor into the museum.
 */
void guide_admits(int id);

/**
 * Callback after a guide leaves the museum.
 */
void guide_leaves(int id);

#endif // MUSEUMSIM_H
