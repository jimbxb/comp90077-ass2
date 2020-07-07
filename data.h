/*
COMP90077 - Assignment 2 - James Barnes (820946)

Header file for data code
*/

#ifndef DATA_H_
#define DATA_H_

#include "main.h"

typedef struct pt_t {
    int x;
    int y;
    int id;
} pt_t;

typedef struct query_t {
    int x_lo, x_hi;
    int y_lo, y_hi;
} query_t;

int random(int lo, int hi);

pt_t *generate_point(int lo, int hi);

pt_t **generate_point_set(int n);

query_t *generate_query(int s);

void free_point(pt_t *pt);

void free_point_set(pt_t **pts, int n);

void free_query(query_t *);

int get_x(pt_t *pt);

int get_y(pt_t *pt);

int cmp_x(const void *p1, const void *p2);

int cmp_y(const void *p1, const void *p2);

int in_range(pt_t *pt, query_t *q);

void sort(pt_t **a, int n, int (*cmp)(const void *, const void *));

#endif /* DATA_H_ */