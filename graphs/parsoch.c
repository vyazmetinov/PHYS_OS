#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int *data;
    int size;
    int capacity;
} IntVec;

static void vec_push(IntVec *v, int value) {
    if (v->size == v->capacity) {
        int new_capacity = (v->capacity == 0) ? 4 : v->capacity * 2;
        int *new_data = (int *)realloc(v->data, (size_t)new_capacity * sizeof(int));
        if (!new_data) {
            exit(1);
        }
        v->data = new_data;
        v->capacity = new_capacity;
    }
    v->data[v->size++] = value;
}

static int try_kuhn(int v, IntVec *g, int *used, int *mt, int marker) {
    if (used[v] == marker) {
        return 0;
    }
    used[v] = marker;

    for (int i = 0; i < g[v].size; ++i) {
        int to = g[v].data[i];
        if (to < 0) {
            continue;
        }
        if (mt[to] == -1 || try_kuhn(mt[to], g, used, mt, marker)) {
            mt[to] = v;
            return 1;
        }
    }
    return 0;
}

int main(void) {
    int *tokens = NULL;
    int token_count = 0;
    int token_capacity = 0;

    int x;
    while (scanf("%d", &x) == 1) {
        if (token_count == token_capacity) {
            int new_capacity = (token_capacity == 0) ? 32 : token_capacity * 2;
            int *new_tokens = (int *)realloc(tokens, (size_t)new_capacity * sizeof(int));
            if (!new_tokens) {
                free(tokens);
                return 1;
            }
            tokens = new_tokens;
            token_capacity = new_capacity;
        }
        tokens[token_count++] = x;
    }

    if (token_count < 2) {
        free(tokens);
        return 0;
    }

    int n_left = tokens[0];
    int n_right = tokens[1];
    if (n_left <= 0 || n_right <= 0) {
        printf("0\n");
        free(tokens);
        return 0;
    }

    IntVec *g = (IntVec *)calloc((size_t)n_left, sizeof(IntVec));
    if (!g) {
        free(tokens);
        return 1;
    }

    int idx = 2;
    int parsed = 0;

    if (idx < token_count) {
        int k = tokens[idx];
        if (k >= 0 && idx + 1 + 2 * k == token_count) {
            idx++;
            for (int i = 0; i < k; ++i) {
                int u = tokens[idx++] - 1;
                int v = tokens[idx++] - 1;
                if (u >= 0 && u < n_left && v >= 0 && v < n_right) {
                    vec_push(&g[u], v);
                }
            }
            parsed = 1;
        }
    }

    int *mt = (int *)malloc((size_t)n_right * sizeof(int));
    int *used = (int *)malloc((size_t)n_left * sizeof(int));
    if (!mt || !used) {
        free(mt);
        free(used);
        for (int i = 0; i < n_left; ++i) {
            free(g[i].data);
        }
        free(g);
        free(tokens);
        return 1;
    }

    for (int i = 0; i < n_right; ++i) {
        mt[i] = -1;
    }
    for (int i = 0; i < n_left; ++i) {
        used[i] = 0;
    }

    int matching_size = 0;
    for (int v = 0; v < n_left; ++v) {
        matching_size += try_kuhn(v, g, used, mt, v + 1);
    }

    printf("%d\n", matching_size);

    free(mt);
    free(used);
    for (int i = 0; i < n_left; ++i) {
        free(g[i].data);
    }
    free(g);
    free(tokens);
    return 0;
}
