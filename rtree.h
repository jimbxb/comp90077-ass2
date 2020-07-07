/*
COMP90077 - Assignment 2 - James Barnes (820946)

Header file for range-tree
*/

#ifndef RTREE_H_
#define RTREE_H_

#include "main.h"
#include "data.h"

typedef struct rtree_t rtree_t;
typedef struct fc_t fc_t;
typedef struct ops_t ops_t;
typedef enum { X, Y } DIM;

struct fc_t {
    int *left;
    int *right;
    pt_t **pts;
    int n;
};

struct rtree_t {
    pt_t *pt;
    rtree_t *left;
    rtree_t *right;
    union {
        rtree_t *sec;
        fc_t *fc;
    };
};

struct ops_t {
    char *name;
    rtree_t *(*construct)(pt_t **pts, int n);
    void (*destruct)(rtree_t *rt);
    int (*query)(rtree_t *rt, query_t *q);
    void (*print)(rtree_t *rt);
    void (*print_sec)(rtree_t *rt);
};

extern const ops_t naive_ops;

extern const ops_t sorted_ops;

extern const ops_t fc_ops;

int report(pt_t *pt);

int query_orig(rtree_t *rt, query_t *q);

int query_fc(rtree_t *rt, query_t *q);

rtree_t *construct_naive(pt_t **pts, int n);

rtree_t *construct_sorted(pt_t **pts, int n);

rtree_t *construct_fc(pt_t **pts, int n);

void free_rtree_orig(rtree_t *rt);

void free_rtree_fc(rtree_t *rt);

void print_rtree(rtree_t *rt);

void print_sec(rtree_t *rt);

void print_fc(rtree_t *rt);

#endif /* RTREE_H_ */
