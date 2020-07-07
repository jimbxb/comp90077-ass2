/*
COMP90077 - Assignment 2 - James Barnes (820946)

Main testing code
*/

#include "data.h"
#include "rtree.h"

#include <sys/time.h>

#define SEED (0)

#define N_TESTS  (100)
#define N_CONST  (1000000)
#define N_N      (10)
#define N_VAR(I) ((1 << (I)) * 1000)
#define S_CONST  ((M) / 20)
#define N_S      (5)
#define S_VAR(I) ((I) <= 2 ? (I) : (1 << ((I) - 3)) * 5)

void test_construct(const ops_t *ops);
int test_query_s(const ops_t *ops);
int test_query_n(const ops_t *ops);

int main(int argc, char **argv) {
    test_construct(&naive_ops);
    test_construct(&sorted_ops);

    int qss = test_query_s(&sorted_ops);
    int qsf = test_query_s(&fc_ops);
    assert(qss == qsf);
    
    int qns = test_query_n(&sorted_ops);
    int qnf = test_query_n(&fc_ops);
    assert(qns == qnf);
    
    return 0;
}

void test_construct(const ops_t *ops) {
    srand(SEED);

    for (int i = 1; i <= N_N; i++) {
        int n = N_VAR(i);
        double t = .0;
        for (int j = 0; j < N_TESTS; j++) {
            pt_t **p = generate_point_set(n);

            struct timeval start, end;
            gettimeofday(&start, NULL);

            rtree_t *rt = ops->construct(p, n);

            gettimeofday(&end, NULL);
            t += (end.tv_sec - start.tv_sec)
                + (end.tv_usec - start.tv_usec) * 1e-6;

            ops->destruct(rt);
            free_point_set(p, n);
        }
        printf("constr,%s,%d,%f\n", ops->name, n, t / N_TESTS);
    }
}

int test_query_s(const ops_t *ops) {
    srand(SEED);

    pt_t **p = generate_point_set(N_CONST);
    rtree_t *rt = ops->construct(p, N_CONST);
    int r = 0;

    for (int i = 1; i <= 5; i++) {
        int s = M / 100 * S_VAR(i);
        double t = .0;
        for (int j = 0; j < N_TESTS; j++) {
            query_t *q = generate_query(s);

            struct timeval start, end;
            gettimeofday(&start, NULL);

            r ^= ops->query(rt, q);

            gettimeofday(&end, NULL);
            t += (end.tv_sec - start.tv_sec)
                + (end.tv_usec - start.tv_usec) * 1e-6;

            free_query(q);
        }
        printf("querys,%s,%d,%f\n", ops->name, s, t / N_TESTS);
    }
    
    ops->destruct(rt);
    free_point_set(p, N_CONST);

    return r;
}

int test_query_n(const ops_t *ops) {
    srand(SEED);

    query_t *w[N_TESTS];
    for (int i = 0; i < N_TESTS; i++) {
        w[i] = generate_query(S_CONST);
    }

    int r = 0;
    for (int i = 1; i <= N_N; i++) {
        int n = N_VAR(i);
        pt_t **p = generate_point_set(n);
        rtree_t *rt = ops->construct(p, n);

        double t = .0;
        for (int j = 0; j < N_TESTS; j++) {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            r ^= ops->query(rt, w[j]);

            gettimeofday(&end, NULL);
            t += (end.tv_sec - start.tv_sec)
                + (end.tv_usec - start.tv_usec) * 1e-6;
        }
        printf("queryn,%s,%d,%f\n", ops->name, n, t / N_TESTS);

        ops->destruct(rt);
        free_point_set(p, n);
    }

    for (int i = 0; i < N_TESTS; i++) {
        free_query(w[i]);
    }

    return r;
}

