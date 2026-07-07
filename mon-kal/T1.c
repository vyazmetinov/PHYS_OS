#include <pthread.h>
#include <ranlib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


pthread_mutex_t mutex;


double f(int x) {
    return sin(x);
}

double* generate_n_points(double a, double b, int n) {
    double* points = (double*) malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) {
        points[i] = (double) rand() / RAND_MAX * (b - a) + a;
    }
    return points;
}

typedef struct{
    double a;
    double b;
} ThreadArgs;

typedef struct {
    double *y;
    double a;
    double b;
} ThreadArgsDisk;

double itgValue = 0.0;

void* square(void* args) {
    ThreadArgs* arg = (ThreadArgs*) args;
    double a = arg->a;
    double b = arg->b;

    double* points = generate_n_points(a, b, 1000);
    double curSum = 0.0;
    for (int i = 0; i < 1000; i++) {
        curSum += f(points[i]);
    }
    double curItgValue = curSum * ((b - a) / 1000);
    pthread_mutex_lock(&mutex);
    itgValue += curItgValue;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* squareDisk(void* args) {
    ThreadArgsDisk* arg = (ThreadArgsDisk*) args;
    int a = arg->a;
    int b = arg->b;
    double *y = arg->y;
    double* points = generate_n_points(y[a], y[b], 1000);
    double curSum = 0.0;
    for (int i = 0; i < 1000; i++) {
        curSum += points[i];
    }
    double curItgValue = curSum * (1e-3);
    pthread_mutex_lock(&mutex);
    itgValue += curItgValue;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    double a; double b; int N;
    a = atof(argv[1]);
    b = atof(argv[2]);
    N = atoi(argv[3]);
    if (argc > 4) {
        int pointsAmount = N + 1;
        double y[pointsAmount];
        for (int i = 4; i < argc; i++) {
            y[i] = atof(argv[i]);
        }
        pthread_t th[N];
        pthread_mutex_init(&mutex, NULL);
        ThreadArgsDisk args[N];
        for (int i = 0; i < N; i++) {
            args[i].a = a + i;
            args[i].b = a + (i + 1);
            args[i].y = y;
            pthread_create(&th[i], NULL, squareDisk, &args);
        }
        for (int i = 0; i < N; i++) {
            pthread_join(th[i], NULL);
        }
        pthread_mutex_destroy(&mutex);
        printf("%lf\n", itgValue);
    }
    else {
        double step = (b - a) / (double) N;
        pthread_t th[N];
        pthread_mutex_init(&mutex, NULL);
        ThreadArgs args[N];
        for (int i = 0; i < N; i++) {
            args[i].a = a + i * step;
            args[i].b = a + (i + 1) * step;
            pthread_create(&th[i], NULL, square, &args);
        }
        for (int i = 0; i < N; i++) {
            pthread_join(th[i], NULL);
        }
        pthread_mutex_destroy(&mutex);
        printf("%lf\n", itgValue);
    }
    return 0;
}
