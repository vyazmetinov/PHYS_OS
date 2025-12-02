// Подключение заголовочных файлов
#include <math.h>        // Математические функции (sin)
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror, dprintf)
#include <stdlib.h>      // Общие утилиты (atoi, exit)
#include <pthread.h>     // POSIX потоки (pthread_create, pthread_mutex_*)
#include <fcntl.h>       // Управление файлами (open, creat, O_RDWR, O_CREAT)
#include <unistd.h>      // POSIX API (read, write, close, lseek, link)

// Глобальный мьютекс для защиты доступа к файлу
pthread_mutex_t mutex;

// Глобальная переменная (здесь не используется, но объявлена)
double res = 0;


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
    
    // КРИТИЧЕСКАЯ СЕКЦИЯ: блокируем мьютекс для безопасного доступа к файлу
    pthread_mutex_lock(&mutex);
    
    // Создаём жёсткую ссылку на файл (link создаёт дополнительное имя для файла)
    link("/Users/ivan/CLionProjects/Phys_OS/files/sq.txt", "sq.txt");
    
    // Открываем файл для чтения и записи
    int fd = open("sq.txt", O_RDWR, 0666);
    if (fd < 0) { perror("fileopen"); exit(1); }  // Проверка на ошибку
    
    double cur_sq;  // Переменная для текущего значения интеграла
    
    // Перемещаем указатель файла в начало
    lseek(fd, 0, SEEK_SET);
    
    // Читаем текущее значение из файла
    read(fd, &cur_sq, sizeof(double));
    
    // Добавляем площадь текущей трапеции к общей сумме
    cur_sq += (f(x1) + f(x2)) / 2 * (x2 - x1);
    
    // Выводим текущее значение интеграла
    printf("cur_sq: %f\n", cur_sq);
    
    // Снова перемещаем указатель в начало файла для записи
    lseek(fd, 0, SEEK_SET);
    
    // Закомментированная строка: бинарная запись
    // write(fd, &cur_sq, sizeof(double));
    
    // Записываем обновлённое значение в файл в текстовом формате
    dprintf(fd, "%f\n", cur_sq);
    
    // Закрываем файл
    close(fd);
    
    // Разблокируем мьютекс - теперь другие потоки могут работать с файлом
    pthread_mutex_unlock(&mutex);
    
    return NULL;  // Завершаем поток
}

// Главная функция программы
int main(int argc, char *argv[]) {
    // Создаём файл sq.txt для хранения результата
    int fd = creat("/Users/ivan/CLionProjects/Phys_OS/files/sq.txt", 0666|O_CREAT);
    if (fd == -1) { perror("creat"); exit(1); }  // Проверка на ошибку
    
    // Записываем начальное значение в файл (здесь ошибка: пишем адрес 0, а не число 0)
    write(fd, 0, sizeof(double));
    
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
    
    return 0;  // Успешное завершение программы
}

