/*
COMP90077 - Assignment 2 - James Barnes (820946)

Range-tree implementation
*/

#include "rtree.h"

#define FC_NO_SUCC (-1)

const ops_t naive_ops = {
    "naive",
    construct_naive,
    free_rtree_orig,
    query_orig,
    print_rtree,
    print_sec
};

const ops_t sorted_ops = {
    "sorted",
    construct_sorted,
    free_rtree_orig,
    query_orig,
    print_rtree,
    print_sec
};

const ops_t fc_ops = {
    "cascading",
    construct_fc,
    free_rtree_fc,
    query_fc,
    print_rtree,
    print_fc
};

int report_all(rtree_t *rt);
int report_fc(fc_t *fc, int i, int hi);
int query_orig_(rtree_t *rt, query_t *q, DIM d);
int query_fc_(rtree_t *rt, query_t *q);
rtree_t *find_lca(rtree_t *rt, int lo, int hi, int (*get)(pt_t *));
int fc_bsearch(fc_t *fc, int v);
pt_t **copy(pt_t **pts, int n);
pt_t **copy_sort(pt_t **pts, int n);
rtree_t *construct_naive_y(pt_t **pts, int n);
rtree_t *construct_naive_(pt_t **pts, int n, DIM d);
int order_stat(int *arr, int n, int k);
rtree_t *construct_sorted_(pt_t **pts_x, pt_t **pts_y, int n);
rtree_t *construct_fc_(pt_t **pts_x, pt_t **pts_y, int n);
void make_fc(rtree_t *rt, pt_t **pts, int n);
void print_rtree_(rtree_t *rt, int ind, int verbose);
void print_fc_(rtree_t *rt, int ind);

int report(pt_t *pt) {
    // printf("%3d ", pt->id);
    return pt->id;
}

int report_all(rtree_t *rt) {
    if (!rt) return 0;

    int r = report(rt->pt);
    r ^= report_all(rt->left);
    r ^= report_all(rt->right);

    return r;
}

int report_fc(fc_t *fc, int i, int hi) {
    if (i < 0 || !fc) {
        return 0;
    }

    int r = 0;
    for ( ; i < fc->n && fc->pts[i]->y <= hi; i++) {
        r ^= report(fc->pts[i]);
    }

    return r;
}

int query_orig(rtree_t *rt, query_t *q) {
    return query_orig_(rt, q, X); 
}

int query_orig_(rtree_t *rt, query_t *q, DIM d) {
    int r = 0;

    if (!rt) return r;

    int lo, hi, (*get)(pt_t *);
    if (d == X) {
        lo = q->x_lo; hi = q->x_hi; get = get_x;
    } else {
        lo = q->y_lo; hi = q->y_hi; get = get_y;
    }

    rtree_t *lca = find_lca(rt, lo, hi, get);
    if (!lca) return r;
    if (in_range(lca->pt, q)) {
        r ^= report(lca->pt);
    }

    rtree_t *n;
    for (n = lca->left; n; /* nothing */) {
        if (in_range(n->pt, q)) {
            r ^= report(n->pt);
        }
        if (get(n->pt) < lo) {
            n = n->right;
        } else {
            if (d == X) {
                if (n->right) {
                    r ^= query_orig_(n->right->sec, q, Y);
                }
            } else {
                r ^= report_all(n->right);
            }
            n = n->left;
        }
    }

    for (n = lca->right; n; /* nothing */) {
        if (in_range(n->pt, q)) {
            r ^= report(n->pt);
        }
        if (hi < get(n->pt)) {
            n = n->left;
        } else {
            if (d == X) {
                if (n->left) {
                    r ^= query_orig_(n->left->sec, q, Y);
                }
            } else {
                r ^= report_all(n->left);
            }
            n = n->right;
        }
    }

    return r;
}

int query_fc(rtree_t *rt, query_t *q) {
    int r = 0;
    int lo = q->x_lo, hi = q->x_hi, y_lo = q->y_lo, y_hi = q->y_hi;

    rtree_t *lca = find_lca(rt, lo, hi, get_x);
    if (!lca) return r;
    if (in_range(lca->pt, q)) {
        r ^= report(lca->pt);
    }
    
    int idx_lca = fc_bsearch(lca->fc, y_lo);
    if (idx_lca < 0) return r;

    rtree_t *n;
    int idx;
    for (n = lca->left, idx = lca->fc->left[idx_lca]; n && idx >= 0; ) {
        if (in_range(n->pt, q)) {
            r ^= report(n->pt);
        }
        if (n->pt->x < lo) {
            idx = n->fc->right[idx];
            n = n->right;
        } else {
            if (n->right) {
                r ^= report_fc(
                    n->right->fc,
                    n->fc->right[idx],
                    y_hi
                );
            }
            idx = n->fc->left[idx];
            n = n->left;
        }
    }

    for (n = lca->right, idx = lca->fc->right[idx_lca]; n && idx >= 0; ) {
        if (in_range(n->pt, q)) {
            r ^= report(n->pt);
        }
        if (hi < n->pt->x) {
            idx = n->fc->left[idx];
            n = n->left;
        } else {
            if (n->left) {
                r ^= report_fc(
                    n->left->fc, 
                    n->fc->left[idx], 
                    y_hi
                );
            }
            idx = n->fc->right[idx];
            n = n->right;
        }
    }

    return r;
}

int fc_bsearch(fc_t *fc, int v) {
    if (fc->n <= 0) return FC_NO_SUCC;

    int m = fc->n / 2;
    for (int hi = fc->n, lo = 0; lo < hi; m = (lo + hi) / 2) {
        if (v <= fc->pts[m]->y) {
            hi = m;
        } else {
            lo = m + 1;
        }
    }

    if (m >= fc->n) return FC_NO_SUCC;

    return m;
}

rtree_t *find_lca(rtree_t* rt, int lo, int hi, int (*get)(pt_t *)) {
    while (rt && (get(rt->pt) < lo || hi < get(rt->pt))) {
        if (get(rt->pt) < lo) {
            rt = rt->right;
        } else {
            rt = rt->left;
        }
    }

    return rt;
}

pt_t **copy(pt_t **pts, int n) {
    pt_t **cpy;
    assert(cpy = malloc(n * sizeof(pt_t*)));
    memcpy(cpy, pts, n * sizeof(pt_t*));

    return cpy;
}

pt_t **copy_sort(pt_t **pts, int n) {
    pt_t **pts_y = copy(pts, n);
    sort(pts, n, cmp_x);
    sort(pts_y, n, cmp_y);
    
    return pts_y;
}

rtree_t *construct_naive(pt_t **pts, int n) {
    pts = copy(pts, n);
    sort(pts, n, cmp_x);

    rtree_t *rt = construct_naive_(pts, n, X);

    free(pts);

    return rt;
}

rtree_t *construct_naive_y(pt_t **pts, int n) {
    sort(pts, n, cmp_y);

    return construct_naive_(pts, n, Y);
}

rtree_t *construct_naive_(pt_t **pts, int n, DIM d) {
    if (n <= 0) return NULL;

    rtree_t *rt;
    assert(rt = malloc(sizeof(rtree_t)));

    if (n == 1) {
        rt->pt = pts[0];
        rt->left = rt->right = NULL;
        rt->sec = d == X ? construct_naive_y(pts, n) : NULL;

        return rt;
    }

    int m = n / 2;
    rt->pt = pts[m];
    rt->left = construct_naive_(pts, m, d);
    rt->right = construct_naive_(pts + m + 1, n - m - 1, d);
    rt->sec = d == X ? construct_naive_y(pts, n) : NULL;

    return rt;
}

rtree_t *construct_sorted(pt_t **pts, int n) {
    pts = copy(pts, n);
    pt_t **pts_y = copy_sort(pts, n);

    rtree_t *rt = construct_sorted_(pts, pts_y, n);
    
    free(pts_y);
    free(pts);

    return rt;
}

rtree_t *construct_sorted_(pt_t **pts_x, pt_t **pts_y, int n) {
    if (n <= 0) return NULL;

    rtree_t *rt;
    assert(rt = malloc(sizeof(rtree_t)));

    if (n == 1) {
        rt->pt = pts_x[0];
        rt->left = rt->right = NULL;
        rt->sec = construct_naive_(pts_y, n, Y);

        return rt;
    }

    int m = n / 2, l_n = 0, r_n = 0;

    pt_t **aux_l, **aux_r = NULL;
    assert(aux_l = malloc(m * sizeof(pt_t*)));
    assert(aux_r = malloc((n - m - 1) * sizeof(pt_t*)));

    rt->pt = pts_x[m]; 
    for (int i = 0; i < n; i++) {
        pt_t *pt = pts_y[i];
        if (pt->id == rt->pt->id) continue;
        if (cmp_x(&pt, &rt->pt) <= 0) {
            aux_l[l_n++] = pt;
        } else {
            aux_r[r_n++] = pt;
        }
    }

    rt->left = construct_sorted_(pts_x, aux_l, l_n);
    rt->right = construct_sorted_(pts_x + m + 1, aux_r, r_n);
    rt->sec = construct_naive_(pts_y, n, Y);

    free(aux_l);
    free(aux_r);
    return rt;
}

rtree_t *construct_fc(pt_t **pts, int n) {
    pts = copy(pts, n);
    pt_t **pts_y = copy_sort(pts, n);

    rtree_t *rt = construct_fc_(pts, pts_y, n);

    free(pts);

    return rt;
}

rtree_t *construct_fc_(pt_t **pts_x, pt_t **pts_y, int n) {
    if (n <= 0) {
        free(pts_y);
        return NULL;
    }

    rtree_t *rt;
    assert(rt = malloc(sizeof(rtree_t)));

    if (n == 1) {
        rt->pt = pts_x[0];
        rt->left = rt->right = NULL;
        make_fc(rt, pts_y, n);
        return rt;
    }

    int m = n / 2, l_n = 0, r_n = 0;

    pt_t **aux_l, **aux_r = NULL;
    assert(aux_l = malloc(m * sizeof(pt_t*)));
    assert(aux_r = malloc((n - m - 1) * sizeof(pt_t*)));

    rt->pt = pts_x[m]; 
    for (int i = 0; i < n; i++) {
        pt_t *pt = pts_y[i];
        if (pt->id == rt->pt->id) continue;
        if (cmp_x(&pt, &rt->pt) <= 0) {
            aux_l[l_n++] = pt;
        } else {
            aux_r[r_n++] = pt;
        }
    }

    rt->left = construct_fc_(pts_x, aux_l, l_n);
    rt->right = construct_fc_(pts_x + m + 1, aux_r, r_n);
    make_fc(rt, pts_y, n);

    return rt;
}

void make_fc(rtree_t *rt, pt_t **pts, int n) {
    assert(rt->fc = malloc(sizeof(fc_t)));
    rt->fc->pts = pts;
    assert(rt->fc->left = malloc(n * sizeof(int)));
    assert(rt->fc->right = malloc(n * sizeof(int)));
    rt->fc->n = n;

    int i, j;
    fc_t *c;
    if (rt->left) {
        for (c = rt->left->fc, i = j = 0; i < n; /* nothing */) {
            if (!c || j >= c->n) {
                rt->fc->left[i++] = FC_NO_SUCC;
            } else if (pts[i]->y <= c->pts[j]->y) {
                rt->fc->left[i++] = j;
            } else {
                j++;
            }
        }
    } else {
        for (i = 0; i < n; i++) rt->fc->left[i] = FC_NO_SUCC;
    }

    if (rt->right) {
        for (c = rt->right->fc, i = j = 0; i < n; /* nothing */) {
            if (!c || j >= c->n) {
                rt->fc->right[i++] = FC_NO_SUCC;
            } else if (pts[i]->y <= c->pts[j]->y) {
                rt->fc->right[i++] = j;
            } else {
                j++;
            }
        }
    } else {
        for (i = 0; i < n; i++) rt->fc->right[i] = FC_NO_SUCC;
    }
}

void free_rtree_orig(rtree_t *rt) {
    if (!rt) return;

    free_rtree_orig(rt->left);
    free_rtree_orig(rt->right);
    free_rtree_orig(rt->sec);

    free(rt);
}

void free_rtree_fc(rtree_t *rt) {
    if (!rt) return;

    free_rtree_fc(rt->left);
    free_rtree_fc(rt->right);

    free(rt->fc->left);
    free(rt->fc->right);
    free(rt->fc->pts);
    free(rt->fc);

    free(rt);
}

void print_rtree(rtree_t *rt) {
    print_rtree_(rt, 0, 1);
}

void print_sec(rtree_t *rt) {
    if (!rt || !rt->sec) return;

    printf("*%d*\n", rt->pt->id);
    print_rtree_(rt->sec, 0, 1);

    print_sec(rt->left);
    print_sec(rt->right);
}

void print_rtree_(rtree_t *rt, int ind, int verbose) {
    if (!rt) {
        if (verbose) {
            for (int i = 0; i < ind; i++) printf("  ");
            printf("-\n");
        }
        return;
    }

    for (int i = 0; i < ind; i++) printf("  ");
    printf("%d (%d, %d)\n", rt->pt->id, rt->pt->x, rt->pt->y);
    print_rtree_(rt->left, ind + 1, verbose);
    print_rtree_(rt->right, ind + 1, verbose);
}

void print_fc(rtree_t *rt) {
    print_fc_(rt, 0);
}

void print_fc_(rtree_t *rt, int ind) {
    if (!rt || !rt->fc) return;
    for (int i = 0; i < ind; i++) printf("  ");
    for (int i = 0; i < rt->fc->n; i++) printf("%d, ", rt->fc->pts[i]->y);
    printf("\n");
    for (int i = 0; i < ind; i++) printf("  ");
    for (int i = 0; i < rt->fc->n; i++) printf("%d, ", rt->fc->left[i]);
    printf("\n");
    for (int i = 0; i < ind; i++) printf("  ");
    for (int i = 0; i < rt->fc->n; i++) printf("%d, ", rt->fc->right[i]);
    printf("\n");
    print_fc_(rt->left, ind + 1);
    print_fc_(rt->right, ind + 1);
}
