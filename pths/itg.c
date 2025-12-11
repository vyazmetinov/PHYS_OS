#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

const int pthread_amnt = 3;

typedef struct {
    int from_idx;
    int to_idx;
    double *absc;
    double *ord;
    double eps;
    double result;
} ThreadData;

typedef struct {
    double from_x;
    double to_x;
    double eps;
    double result;
} ThreadDataCont;

double square_dsc(int from_idx, int to_idx, double * absc, double * ord, double eps);
double square_cont(double from_x, double to_x, double eps);

void *thread_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = square_dsc(data->from_idx, data->to_idx, data->absc, data->ord, data->eps);
    return NULL;
}

void *thread_worker_cont(void *arg) {
    ThreadDataCont *data = (ThreadDataCont *)arg;
    data->result = square_cont(data->from_x, data->to_x, data->eps);
    return NULL;
}

double f(double x) {
    return sqrt(x);
}

double max(double a, double b) {
    return a > b ? a : b;
}

double precision_cont(double from, double to, double eps) { //from, to - values
    if (to - from < 1e-10) {
        return (to - from) / 100.0;
    }
    
    double max_sec_drv = 0.0;
    double step = (to - from) / 1000.0; // Берем 1000 точек для оценки
    
    for (double i = from; i < to - 2 * step; i += step) {
        double tmp_sec_drv = fabs(f(i + 2 * step) - 2 * f(i + step) + f(i));
        max_sec_drv = max(max_sec_drv, tmp_sec_drv);
    }
    
    // Защита от деления на ноль
    if (max_sec_drv < 1e-10) {
        max_sec_drv = 1e-10;
    }
    
    double h = 6 * eps / fabs(max_sec_drv);
    
    // Ограничиваем шаг разумными пределами
    double min_step = (to - from) / 100000.0;
    double max_step = (to - from) / 10.0;
    if (h < min_step) h = min_step;
    if (h > max_step) h = max_step;
    
    return h;
}

double precision_dsc(int from, int to, double absc[], double ord[], double eps) { //from, to - ind
    printf("%d %d\n", from, to);

    if (to - from < 2) {
        return (absc[to] - absc[from]) / 100.0;
    }
    
    double max_sec_drv = 0.0;
    for (int i = from; i < to - 1 ; i += 1) {
        double tmp_sec_drv = fabs(ord[i + 1] - ord[i]);
        max_sec_drv = max(max_sec_drv, tmp_sec_drv);
        printf("max_sec_drv = %f\n", max_sec_drv);
    }
    

    if (max_sec_drv < 1e-10) {
        max_sec_drv = 1e-10;
    }
    
    double h = eps / fabs(max_sec_drv);
    double min_step = (absc[to] - absc[from]) / 10000.0;
    double max_step = (absc[to] - absc[from]) / 10.0;
    if (h < min_step) h = min_step;
    if (h > max_step) h = max_step;
    
    return h;
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

            // линейная интерполяция по y
            printf("y = %f\n", y0 + t * (y1 - y0));
            return y0 + t * (y1 - y0);
        }
    }

    // на всякий случай, если не нашли (что не должно случиться)
    return ord[to];
}

double square_dsc(int from, int to, double absc[], double ord[], double eps) {
    // шаг интегрирования по оси x (может быть нецелым по отношению к индексам)
    printf("from - %d to - %d\n", from, to);
    
    // Защита от некорректных индексов
    if (from >= to) {
        return 0.0;
    }
    
    double h = precision_dsc(from, to, absc, ord, eps);
    printf("h - %f\n", h);
    
    // Дополнительная проверка шага
    if (h <= 0.0 || !isfinite(h)) {
        fprintf(stderr, "Error: invalid step h = %f\n", h);
        h = (absc[to] - absc[from]) / 100.0;  // Запасной вариант
    }
    
    double x_start = absc[from];
    double x_end   = absc[to];

    double sum = 0.0;
    int iterations = 0;
    const int max_iterations = 1000000;  // Защита от зацикливания

    for (double x = x_start; x < x_end && iterations < max_iterations; x += h) {
        double x_next = x + h;
        if (x_next > x_end) {
            x_next = x_end;
        }

        double y_left  = value_at(x,      from, to, absc, ord);
        double y_right = value_at(x_next, from, to, absc, ord);

        double dx = x_next - x;
        double trap = 0.5 * (y_left + y_right) * dx;
        printf("left %f right %f dx %f, rx %f", y_left, y_right, dx, x_next);
        sum += trap;
        printf("sum %f\n", sum);
        iterations++;
    }
    
    if (iterations >= max_iterations) {
        fprintf(stderr, "Warning: max iterations reached in square_dsc\n");
    }

    return sum;
}

// Вычисление интеграла непрерывной функции f(x) методом трапеций
double square_cont(double from_x, double to_x, double eps) {
    printf("CONT: from_x=%.6f to_x=%.6f\n", from_x, to_x);
    
    // Защита от некорректных границ
    if (from_x >= to_x) {
        return 0.0;
    }
    
    // Вычисляем оптимальный шаг интегрирования
    double h = precision_cont(from_x, to_x, eps);
    printf("CONT: h = %f\n", h);
    
    // Дополнительная проверка шага
    if (h <= 0.0 || !isfinite(h)) {
        fprintf(stderr, "Error: invalid step h = %f\n", h);
        h = (to_x - from_x) / 1000.0;  // Запасной вариант
    }
    
    double sum = 0.0;
    int iterations = 0;
    const int max_iterations = 10000000;  // Защита от зацикливания
    
    // Метод трапеций
    for (double x = from_x; x < to_x && iterations < max_iterations; x += h) {
        double x_next = x + h;
        if (x_next > to_x) {
            x_next = to_x;
        }
        
        double y_left = f(x);
        double y_right = f(x_next);
        
        double dx = x_next - x;
        double trap = 0.5 * (y_left + y_right) * dx;
        
        sum += trap;
        iterations++;
    }
    
    if (iterations >= max_iterations) {
        fprintf(stderr, "Warning: max iterations reached in square_cont\n");
    }
    
    printf("CONT: result = %f (iterations: %d)\n", sum, iterations);
    return sum;
}

// Параллельное вычисление интеграла на [from, to] по нитям
double parallel_square(int from, int to, double absc[], double ord[], double eps, int threads) {
    if (threads < 1) {
        threads = 1;
    }

    int total = to - from; // количество индексов внутри [from, to)
    if (total <= 0) {
        return 0.0;
    }

    // Минимум 2 точки на подотрезок для корректной работы
    const int MIN_POINTS = 2;
    
    // Ограничиваем количество потоков, чтобы каждому хватило точек
    int max_threads = total / MIN_POINTS;
    if (max_threads < 1) max_threads = 1;
    if (threads > max_threads) {
        threads = max_threads;
    }

    ThreadData data[threads];
    pthread_t tids[threads];

    // Базовая длина подотрезка по индексам (не меньше MIN_POINTS)
    int base_len = total / threads;
    if (base_len < MIN_POINTS) {
        base_len = MIN_POINTS;
    }

    int current = from;
    int tcount = 0;
    for (int k = 0; k < threads && current < to; ++k) {
        int next;

        if (k == threads - 1) {
            // последний поток забирает всё, что осталось
            next = to;
        } else {
            next = current + base_len;
            if (next > to) {
                next = to;
            }
        }

        // Проверка: убедимся, что сегмент не слишком мал
        if (next - current < 1) {
            break;
        }

        data[tcount].from_idx = current;
        data[tcount].to_idx   = next;
        data[tcount].absc     = absc;
        data[tcount].ord      = ord;
        data[tcount].eps      = eps;
        data[tcount].result   = 0.0;

        pthread_create(&tids[tcount], NULL, thread_worker, &data[tcount]);

        current = next;
        ++tcount;
    }

    // если ни одного нормального подотрезка не получилось — считаем последовательно
    if (tcount == 0) {
        return square_dsc(from, to, absc, ord, eps);
    }

    double sum = 0.0;
    for (int k = 0; k < tcount; ++k) {
        pthread_join(tids[k], NULL);
        sum += data[k].result;
    }

    return sum;
}

// Параллельное вычисление интеграла непрерывной функции
double parallel_square_cont(double from_x, double to_x, double eps, int threads) {
    if (threads < 1) {
        threads = 1;
    }
    
    double range = to_x - from_x;
    if (range <= 0.0) {
        return 0.0;
    }
    
    // Минимальная длина подотрезка
    const double MIN_RANGE = 0.001;
    
    // Ограничиваем количество потоков
    int max_threads = (int)(range / MIN_RANGE);
    if (max_threads < 1) max_threads = 1;
    if (threads > max_threads) {
        threads = max_threads;
    }
    
    ThreadDataCont data[threads];
    pthread_t tids[threads];
    
    // Длина подотрезка на один поток
    double segment_len = range / threads;
    
    int tcount = 0;
    for (int k = 0; k < threads; ++k) {
        double current_from = from_x + k * segment_len;
        double current_to;
        
        if (k == threads - 1) {
            // Последний поток берет до конца
            current_to = to_x;
        } else {
            current_to = current_from + segment_len;
        }
        
        // Проверка на корректность подотрезка
        if (current_to - current_from < 1e-10) {
            break;
        }
        
        data[tcount].from_x = current_from;
        data[tcount].to_x = current_to;
        data[tcount].eps = eps;
        data[tcount].result = 0.0;
        
        pthread_create(&tids[tcount], NULL, thread_worker_cont, &data[tcount]);
        
        ++tcount;
    }
    
    // Если ни одного потока не создано, считаем последовательно
    if (tcount == 0) {
        return square_cont(from_x, to_x, eps);
    }
    
    double sum = 0.0;
    for (int k = 0; k < tcount; ++k) {
        pthread_join(tids[k], NULL);
        sum += data[k].result;
    }
    
    return sum;
}


int main(int argc, char *argv[]) {
    // Режим 1: Непрерывная функция f(x) = sqrt(x)
    // Формат: ./prog cont a b eps
    if (argc == 5 && strcmp(argv[1], "cont") == 0) {
        printf("=== Режим непрерывной функции f(x) = sqrt(x) ===\n");
        
        double a = atof(argv[2]);
        double b = atof(argv[3]);
        double eps = atof(argv[4]);
        
        printf("Интервал: [%f, %f], eps=%f\n", a, b, eps);
        
        double res = parallel_square_cont(a, b, eps, pthread_amnt);
        printf("\n=== РЕЗУЛЬТАТ (непрерывный) = %f ===\n", res);
    }
    // Режим 2: Дискретные данные
    // Формат: ./prog n x1 x2 ... xn y1 y2 ... yn a b eps
    else if (argc >= 2 && argc == atoi(argv[1]) * 2 + 5) {
        printf("=== Режим дискретных данных ===\n");
        
        int n = atoi(argv[1]);
        double absc[n];
        for (int i = 2; i < n + 2; i++) {
            absc[i - 2] = atof(argv[i]);
        }
        double ord[n];
        for (int i = 2 + n; i < n * 2 + 2; i++) {
            ord[i - 2 - n] = atof(argv[i]);
        }
        int a = atoi(argv[2 * n + 2]);
        int b = atoi(argv[2 * n + 3]);
        double e = atof(argv[2 * n + 4]);
        
        printf("Точки X: ");
        for (int i = 0; i < n; i++) {
            printf("%f ", absc[i]);
        }
        printf("\nЗначения Y: ");
        for (int i = 0; i < n; i++) {
            printf("%f ", ord[i]);
        }
        printf("\nИндексы: %d-%d, eps=%f\n", a, b, e);
        
        double res = parallel_square(a - 1, b - 1, absc, ord, e, pthread_amnt);
        printf("\n=== РЕЗУЛЬТАТ (дискретный) = %f ===\n", res);
    }
    else {
        fprintf(stderr, "Использование:\n");
        fprintf(stderr, "  Дискретные данные: %s n x1 x2 ... xn y1 y2 ... yn a b eps\n", argv[0]);
        fprintf(stderr, "    где a, b - индексы (от 1), eps - точность\n");
        fprintf(stderr, "  Непрерывная функция: %s cont a b eps\n", argv[0]);
        fprintf(stderr, "    где a, b - границы интервала, eps - точность\n");
        fprintf(stderr, "    функция: f(x) = sqrt(x)\n");
        return 1;
    }
    
    return 0;
}