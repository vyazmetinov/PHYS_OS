#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <pthread.h>

const int pthread_amnt = 3;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    double from_x;
    double to_x;
    double eps;
    int thread_id;
    int msgq_id;
} ThreadDataCont;

double square_cont(double from_x, double to_x, double eps);

void *thread_worker_cont(void *arg) {
    ThreadDataCont *data = (ThreadDataCont *)arg;

    link("/Users/ivan/CLionProjects/Phys_OS/files/sq.txt", "sq.txt");

    // Открываем файл для чтения и записи
    int fd = open("sq.txt", O_RDWR, 0666);
    if (fd < 0) { perror("fileopen"); exit(1); }  // Проверка на ошибку
    pthread_mutex_lock(&mutex);
    double cur_sq;  // Переменная для текущего значения интеграла

    // Перемещаем указатель файла в начало
    lseek(fd, 0, SEEK_SET);

    // Читаем текущее значение из файла
    read(fd, &cur_sq, sizeof(double));

    // Добавляем площадь текущей трапеции к общей сумме

    cur_sq += square_cont(data->from_x, data->to_x, data->eps);



    // Снова перемещаем указатель в начало файла для записи
    lseek(fd, 0, SEEK_SET);

    // Закомментированная строка: бинарная запись
    // write(fd, &cur_sq, sizeof(double));

    // Выводим текущее значение интеграла
    printf("cur_sq: %f\n", cur_sq);

    // Записываем обновлённое значение в файл в текстовом формате
    dprintf(fd, "%f\n", cur_sq);

    // Закрываем файл
    close(fd);

    // Разблокируем мьютекс - теперь другие потоки могут работать с файлом
    pthread_mutex_unlock(&mutex);
    return NULL;
}

double f(double x) {
    return sin(x);
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

// Параллельное вычисление интеграла непрерывной функции с использованием очереди сообщений
double parallel_square_cont(double from_x, double to_x, double eps, int threads) {
    if (threads < 1) {
        threads = 1;
    }

    double range = to_x - from_x;
    if (range <= 0.0) {
        return 0.0;
    }

    const double MIN_RANGE = 0.001;

    int max_threads = (int)(range / MIN_RANGE);
    if (max_threads < 1) max_threads = 1;
    if (threads > max_threads) {
        threads = max_threads;
    }

    // Создаем очередь сообщений
    key_t key = ftok("/tmp", 'C');
    if (key == -1) {
        perror("ftok");
        return 0.0;
    }

    int msgq_id = msgget(key, 0666 | IPC_CREAT);
    if (msgq_id == -1) {
        perror("msgget");
        return 0.0;
    }

    printf("\n[Главный поток] Создана очередь сообщений (ID=%d)\n", msgq_id);

    ThreadDataCont data[threads];
    pthread_t tids[threads];

    double segment_len = range / threads;

    int tcount = 0;
    for (int k = 0; k < threads; ++k) {
        double current_from = from_x + k * segment_len;
        double current_to;

        if (k == threads - 1) {
            current_to = to_x;
        } else {
            current_to = current_from + segment_len;
        }

        if (current_to - current_from < 1e-10) {
            break;
        }

        data[tcount].from_x = current_from;
        data[tcount].to_x = current_to;
        data[tcount].eps = eps;
        data[tcount].thread_id = tcount;
        data[tcount].msgq_id = msgq_id;

        pthread_create(&tids[tcount], NULL, thread_worker_cont, &data[tcount]);

        ++tcount;
    }

    if (tcount == 0) {
        msgctl(msgq_id, IPC_RMID, NULL);
        return square_cont(from_x, to_x, eps);
    }

    printf("[Главный поток] Запущено %d потоков, ожидаю результаты...\n", tcount);

    // Ждем завершения всех потоков
    for (int k = 0; k < tcount; ++k) {
        pthread_join(tids[k], NULL);
    }


    link("/Users/ivan/CLionProjects/Phys_OS/files/sq.txt", "sq.txt");
    int fd = open("sq.txt", O_RDWR, 0666);
    if (fd < 0) { perror("fileopen"); exit(1); }  // Проверка на ошибку
    double cur_sq;  // Переменная для текущего значения интеграла
    // Перемещаем указатель файла в начало
    lseek(fd, 0, SEEK_SET);
    double res = 0;
    read(fd, &res, sizeof(double));
    return res;
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
        printf("\n=== РЕЗУЛЬТАТ (непрерывный) = СМ В ФАЙЛЕ sq.txt ===\n");
    }
    return 0;
}