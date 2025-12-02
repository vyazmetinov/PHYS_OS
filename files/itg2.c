// Подключение заголовочных файлов
#include <math.h>        // Математические функции (sin)
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror, dprintf)
#include <stdlib.h>      // Общие утилиты (atoi, exit)
#include <pthread.h>     // POSIX потоки (pthread_create, pthread_mutex_*)
#include <fcntl.h>       // Управление файлами (open, creat, O_RDWR, O_CREAT)
#include <unistd.h>      // POSIX API (read, write, close, lseek, link)
#include <sys/mman.h>    // Отображение файлов в память (mmap)

// Глобальный мьютекс для защиты доступа к файлу
pthread_mutex_t mutex;

// Глобальная переменная (здесь не используется, но объявлена)
double res = 0;

double *mapped_res = NULL; // указатель на отображённое в память значение интеграла


// Структура для передачи аргументов в поток
typedef struct {
    double x1;      // Левая граница отрезка
    double x2;      // Правая граница отрезка
} ThreadArgs;


// Функция, интеграл которой мы вычисляем: f(x) = sin(x)
double f(double x) {
    return sin(x);  // Возвращаем синус от x
}

// Функция для вычисления площади трапеции (выполняется в отдельном потоке)
void* square(void* args) {
    ThreadArgs* arg = (ThreadArgs*)args;  // Преобразуем аргументы
    double x1 = arg->x1;                  // Извлекаем левую границу
    double x2 = arg->x2;                  // Извлекаем правую границу

    // Вычисляем локальный вклад в интеграл на отрезке [x1, x2]
    double local_sq = (f(x1) + f(x2)) / 2.0 * (x2 - x1);

    // КРИТИЧЕСКАЯ СЕКЦИЯ: добавляем локальный вклад к общей сумме,
    // хранящейся в отображённом файле
    pthread_mutex_lock(&mutex);
    *mapped_res += local_sq;
    printf("cur_sq: %f\n", *mapped_res);
    pthread_mutex_unlock(&mutex);

    return NULL;  // Завершаем поток
}

// Главная функция программы
int main(int argc, char *argv[]) {
    // Открываем/создаём файл для отображения результата интеграла
    int fd = open("/Users/ivan/CLionProjects/Phys_OS/files/sq_mmap.dat", O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Увеличиваем файл до размера одного double
    if (ftruncate(fd, sizeof(double)) == -1) {
        perror("ftruncate");
        close(fd);
        exit(1);
    }

    // Отображаем файл в память
    mapped_res = (double *)mmap(NULL, sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_res == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    // Дескриптор файла больше не нужен
    close(fd);

    // Инициализируем значение интеграла нулём в отображённой памяти
    *mapped_res = 0.0;

    double a = atoi(argv[1]);             // Нижняя граница интегрирования
    double b = atoi(argv[2]);             // Верхняя граница интегрирования
    int n = 1e5;                          // Количество разбиений (100 000)
    double step = (b - a) / (double) n;   // Ширина одного отрезка

    // Инициализируем мьютекс с настройками по умолчанию
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[n];  // Массив дескрипторов потоков

    // Цикл по всем отрезкам
    for (int i = 0; i < n; i++) {
        ThreadArgs args_i;                    // Создаём структуру с аргументами
        args_i.x1 = a + i * step;             // Левая граница i-го отрезка
        args_i.x2 = a + (i + 1) * step;       // Правая граница i-го отрезка

        // Создаём поток для вычисления площади трапеции
        pthread_create(&threads[i], NULL, square, &args_i);

        // Сразу ждём завершения потока (последовательное выполнение)
        pthread_join(threads[i], NULL);
    }

    // Уничтожаем мьютекс (освобождаем ресурсы)
    pthread_mutex_destroy(&mutex);

    // Выводим итоговое значение интеграла и снимаем отображение файла
    printf("Result integral: %f\n", *mapped_res);
    munmap(mapped_res, sizeof(double));

    return 0;  // Успешное завершение программы
}
