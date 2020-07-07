/*
COMP90077 - Assignment 2 - James Barnes (820946)

Header file for data
*/

#include "data.h"

int random(int lo, int hi) {
    int range = hi - lo + 1;
    if (range <= 0) return lo;

    int thresh = RAND_MAX / range * range;
    int n;
    while ((n = rand()) > thresh);

    return n % range + lo;
}

pt_t *generate_point(int lo, int hi) {
    pt_t *pt;
    assert(pt = malloc(sizeof(pt_t)));

    pt->x = random(lo, hi);
    pt->y = random(lo, hi);
    pt->id = -1;

    return pt;
}

pt_t **generate_point_set(int n) {
    pt_t **set;
    assert(set = malloc(n * sizeof(pt_t*)));

    for (int i = 0; i < n; i++) {
        set[i] = generate_point(1, M);
        set[i]->id = i + 1;
    }

    return set;
}

query_t *generate_query(int s) {
    pt_t *pt = generate_point(1, M - s);

    query_t *q;
    assert(q = malloc(sizeof(query_t)));

    q->x_lo = pt->x;
    q->x_hi = pt->x + s;
    q->y_lo = pt->y;
    q->y_hi = pt->y + s;

    free_point(pt);

    return q;
}

void free_point(pt_t *pt) {
    free(pt);
}

void free_point_set(pt_t **pts, int n) {
    for (int i = 0; i < n; i++) free_point(pts[i]);
    free(pts);
}

void free_query(query_t *q) {
    free(q);
}

int get_x(pt_t *pt) {
    return pt->x;
}

int get_y(pt_t *pt) {
    return pt->y;
}

int cmp_x(const void *v1, const void *v2) {
    const pt_t *p1 = *(pt_t**)v1, *p2 = *(pt_t**)v2;
    int cmp;
    cmp = p1->x - p2->x;
    if (cmp) return cmp;
    cmp = p1->y - p2->y;
    if (cmp) return cmp;
    return p1->id - p2->id;
}

int cmp_y(const void *v1, const void *v2) {
    const pt_t *p1 = *(pt_t**)v1, *p2 = *(pt_t**)v2;
    int cmp;
    cmp = p1->y - p2->y;
    if (cmp) return cmp;
    cmp = p1->x - p2->x;
    if (cmp) return cmp;
    return p1->id - p2->id;
}

int in_range(pt_t *pt, query_t *q) {
    int x = pt->x, y = pt->y;
    return q->x_lo <= x && x <= q->x_hi && q->y_lo <= y && y <= q->y_hi;
}

void sort(pt_t **a, int n, int (*cmp)(const void *, const void *)) {
    if (n <= 1) return;

    int m = 1;
    while (m < (n + 1) / 2) m *= 2;
    pt_t **aux;
    assert(aux = malloc(m * sizeof(pt_t*)));

    for (int s = 1; s <= m; s *= 2) {
        for (int p = 0; p + s < n; p += 2 * s) {
            memcpy(aux, a + p, s * sizeof(pt_t*));
            int i = 0, k = p;
            for (int j = p + s; 
                    i < s && j < p + 2 * s && j < n && k < n; k++) {
                if (cmp(&aux[i], &a[j]) <= 0) {
                    a[k] = aux[i++];
                } else {
                    a[k] = a[j++];
                }
            }
            if (i < s) {
                memcpy(a + k, aux + i, (s - i) * sizeof(pt_t*));
            }
        }
    }
    
    free(aux);
}
