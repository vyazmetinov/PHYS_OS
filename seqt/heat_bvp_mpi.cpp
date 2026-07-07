#include <algorithm>
#include <pthread.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

double a; // Всегда 0
double b; // Всегда 1
double Ta;
double Tb;
double lambda;
double ro;
double c;
double u;
int N;
double alpha;
double h;
double eps = 0.05;


double exactSol(double y) {
    return (Ta - Tb) / (exp(alpha * a) - exp(alpha * b)) * exp(alpha * y) - ((Ta * exp(alpha * b) - Tb * exp(alpha * a)) / (exp(alpha * a) + exp(alpha * b)));
}

double* makeGrid(int gridSize) {
    double* grid = (double*) malloc(sizeof(double) * gridSize);
    for (int i = 0; i < gridSize; i++) {
        grid[i] = a + i * h;
    }
    return grid;
}

double* Fn(double* FnPrev, int FnPrevSize, int gridSize) {
    double* Fncur = (double*) malloc(sizeof(double) * (FnPrevSize + 1));
    for (int i = 0; i < FnPrevSize; i++) {
        Fncur[i] = FnPrev[i];
    }
    Fncur[FnPrevSize] = (2 - alpha * h) / (4 - (2 + alpha * h) * Fncur[FnPrevSize - 1]);
    if (FnPrevSize + 1 < gridSize) {
        double* result = Fn(Fncur, FnPrevSize + 1, gridSize);
        if (FnPrevSize > 1) {
            free(Fncur);
        }
        return result;
    }
    else {
        return Fncur;
    }
}

double* Gn(double* Fn, int GnPrevSize, int gridSize, double* GnPrev) {
    double* Gncur = (double*) malloc(sizeof(double) * (GnPrevSize + 1));
    for (int i = 0; i < GnPrevSize; i++) {
        Gncur[i] = GnPrev[i];
    }
    Gncur[GnPrevSize] = (2 + alpha * h) * Gncur[GnPrevSize - 1] / (4 - (2 + alpha * h) * Fn[GnPrevSize - 1]);
    if (GnPrevSize + 1 < gridSize) {
        double* result = Gn(Fn, GnPrevSize + 1, gridSize, Gncur);
        if (GnPrevSize > 1) {
            free(Gncur);
        }
        return result;
    }
    else {
        return Gncur;
    }
}

double* Tn(double* TnPrev, double* Fn, double* Gn, int gridSize, int TPrevSize) {
    double* TnCur = (double*) malloc(sizeof(double) * (TPrevSize + 1));
    for (int i = 0; i < TPrevSize; i++) {
        TnCur[i] = TnPrev[i];
    }
    TnCur[TPrevSize] = Fn[TPrevSize] * TnCur[TPrevSize - 1] + Gn[TPrevSize];
    if (TPrevSize + 1 < gridSize) {
        double* result = Tn(TnCur, Fn, Gn, gridSize, TPrevSize + 1);
        if (TPrevSize > 1) {
            free(TnCur);
        }
        return result;
    }
    return TnCur;
}

double err (double* Tn, double* grid, int gridSize ){
    double err = 0;
    for (int i = 0; i < gridSize; i++) {
        err = std::max(err, fabs(Tn[gridSize - 1 - i] - exactSol(grid[i])));
    }
    return err;
}

int main(int argc, char *argv[]) {
    if (argc < 10) {
        printf("Usage: %s a b Ta Tb lambda ro c u N\n", argv[0]);
        return 1;
    }

    a = atof(argv[1]);
    b = atof(argv[2]);
    Ta = atof(argv[3]);
    Tb = atof(argv[4]);
    lambda = atof(argv[5]);
    ro = atof(argv[6]);
    c = atof(argv[7]);
    u = atof(argv[8]);

    N = atoi(argv[9]);
    if (N < 1) {
        N = 1;
    }
    const int maxN = 1000000;

    alpha = ro * c * u / lambda;

    double currentErr = 0.0;
    int gridSize = 0;
    double* grid = nullptr;
    double* F = nullptr;
    double* G = nullptr;
    double* T = nullptr;

    do {
        h = (b - a) / N;
        gridSize = N + 1;

        grid = makeGrid(gridSize);  // Массив y

        double F0[1];
        F0[0] = 0.0;
        F = Fn(F0, 1, gridSize);

        double G0[1];
        G0[0] = Ta;
        G = Gn(F, 1, gridSize, G0);

        double T1[1];
        T1[0] = Tb;
        T = Tn(T1, F, G, gridSize, 1);

        currentErr = err(T, grid, gridSize);

        if (currentErr > eps) {
            free(grid);
            free(F);
            free(G);
            free(T);

            grid = nullptr;
            F = nullptr;
            G = nullptr;
            T = nullptr;

            N *= 2;
        }
        if (N > maxN && currentErr > eps) {
            printf("Required accuracy was not reached before maxN.\n");
            break;
        }
    } while (currentErr > eps);

    printf("N = %d\n", N);
    printf("gridSize = %d\n", gridSize);
    printf("h = %lf\n", h);
    printf("error = %lf\n", currentErr);

    free(grid);
    free(F);
    free(G);
    free(T);

    return 0;
}
