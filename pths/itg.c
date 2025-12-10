#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

const int pthread_amnt = 1000;

typedef struct {
    int from_idx;
    int to_idx;
    double *absc;
    double *ord;
    double eps;
    double result;
} ThreadData;

double square(int from_idx, int to_idx, double * absc, double * ord, double eps);

void *thread_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = square(data->from_idx, data->to_idx, data->absc, data->ord, data->eps);
    return NULL;
}


double precision(int from, int to, double absc[], double ord[], double eps) { //from, to - indexes
    return 6 * eps / (ord[to] - (double)2 * ord[(from + to) / 2] - ord[from]);
}

// Линейная интерполяция значения функции в точке x
// по массивам узлов absc[] и значений ord[] на отрезке [from, to]
double value_at(double x, int from, int to, double absc[], double ord[]) {
    // если x выходит за границы, "прижимаем" к краю
    if (x <= absc[from]) return ord[from];
    if (x >= absc[to])   return ord[to];

    // ищем отрезок [absc[i], absc[i+1]], которому принадлежит x

    for (int i = from; i < to; ++i) {
        if (x <= absc[i + 1]) {
            double x0 = absc[i];
            double y0 = ord[i];
            double x1 = absc[i + 1];
            double y1 = ord[i + 1];

            // доля пути по оси x
            double t = (x - x0) / (x1 - x0);

            // длина отрезка между узлами
            double dx = x1 - x0;
            double dy = y1 - y0;
            double len = sqrt(dx * dx + dy * dy);

            // расстояние вдоль прямой до точки с такой же долей t
            double dist = t * len;

            double x_interp, y_interp;
            return y_interp;
        }
    }

    // на всякий случай, если не нашли (что не должно случиться)
    return ord[to];
}

double square(int from, int to, double absc[], double ord[], double eps) {
    // шаг интегрирования по оси x (может быть нецелым по отношению к индексам)
    double h = precision(from, to, absc, ord, eps);

    double x_start = absc[from];
    double x_end   = absc[to];

    double sum = 0.0;

    for (double x = x_start; x < x_end; x += h) {
        double x_next = x + h;
        if (x_next > x_end) {
            x_next = x_end;
        }

        double y_left  = value_at(x,      from, to, absc, ord);
        double y_right = value_at(x_next, from, to, absc, ord);

        double dx = x_next - x;
        double trap = 0.5 * (y_left + y_right) * dx;
        sum += trap;
    }

    return sum;
}

// Параллельное вычисление интеграла на [from, to] по нитям
double parallel_square(int from, int to, double absc[], double ord[], double eps, int threads) {
    if (threads < 1) {
        threads = 1;
    }

    int total = to - from;
    if (total <= 0) {
        return 0.0;
    }

    ThreadData data[threads];
    pthread_t tids[threads];

    // Базовая длина подотрезка по индексам
    int base_len = total / threads;
    if (base_len < 1) {
        base_len = 1;
    }

    int current = from;
    int tcount = 0;
    for (int k = 0; k < threads && current < to; ++k) {
        int next = (k == threads - 1) ? to : current + base_len;
        if (next > to) {
            next = to;
        }

        data[k].from_idx = current;
        data[k].to_idx = next;
        data[k].absc = absc;
        data[k].ord  = ord;
        data[k].eps  = eps;
        data[k].result = 0.0;

        pthread_create(&tids[k], NULL, thread_worker, &data[k]);

        current = next;
        ++tcount;
    }

    double sum = 0.0;
    for (int k = 0; k < tcount; ++k) {
        pthread_join(tids[k], NULL);
        sum += data[k].result;
    }

    return sum;
}


int main(int argc, char *argv[]) { //n, X, f(x), a, b, e || f(x), a, b, e
    if (argc == 6) {
        int n = atoi(argv[1]);
        double absc[n];
        for (int i = 2; i < n; i++) {
            absc[i] = atof(argv[i]);
        }
        double ord[n];
        for (int i = 2 + n; i < n * 2; i++) {
            ord[i] = atof(argv[i]);
        }
        int a = atoi(argv[2 * n]);
        int b = atoi(argv[2 * n + 1]);
        double e = atof(argv[2 * n + 2]);
        double res = parallel_square(a, b, absc, ord, e, pthread_amnt);
        printf("%f\n", res);
    }
}