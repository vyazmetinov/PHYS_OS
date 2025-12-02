// Подключение заголовочных файлов
#include <math.h>        // Математические функции (sin)
#include <stdio.h>       // Стандартный ввод-вывод (printf)
#include <stdlib.h>      // Общие утилиты (atoi, exit)
#include <pthread.h>     // POSIX потоки
#include <sys/msg.h>     // Очереди сообщений
#include <unistd.h>      // POSIX API

// Структура для передачи аргументов в поток
typedef struct {
    double x1;      // Левая граница отрезка
    double x2;      // Правая граница отрезка
    key_t key;      // Ключ очереди сообщений
} ThreadArgs;

// Структура сообщения для передачи результата через очередь
typedef struct {
    long mtype;       // Тип сообщения (будет = 2)
    double mini_sq;   // Площадь маленькой трапеции
} message_buf;

// Функция, интеграл которой мы вычисляем: f(x) = sin(x)
double f(double x) {
    return sin(x);  // Возвращаем синус от x
}



// Функция для вычисления площади одной трапеции (выполняется в потоке)
void* square(void* args) {
    ThreadArgs* arg = (ThreadArgs*)args;  // Преобразуем аргумент к нужному типу
    double x1 = arg->x1;                  // Извлекаем левую границу
    double x2 = arg->x2;                  // Извлекаем правую границу
    int msgid;                            // ID очереди сообщений
    message_buf sbuf;                     // Буфер для сообщения (не используется)
    key_t msgkey = arg->key;              // Извлекаем ключ очереди
    
    // Получаем доступ к очереди сообщений
    if((msgid = msgget(msgkey, 0666 | IPC_CREAT)) == -1) {
        perror("msgget_hellower");  // Вывод ошибки
        exit(1);                     // Аварийное завершение
    }
    
    message_buf buf;                  // Создаём сообщение для отправки
    buf.mtype = 2;                    // Устанавливаем тип сообщения = 2
    // Вычисляем площадь трапеции по формуле: S = (f(x1) + f(x2)) / 2 * (x2 - x1)
    buf.mini_sq = (f(x1) + f(x2)) / 2 * (x2 - x1);
    // Отправляем результат в очередь
    msgsnd(msgid, &buf, sizeof(message_buf), 0);
    
    return NULL;  // Завершаем поток
}

// Главная функция программы
int main(int argc, char *argv[]) {
    double ans = 0;                       // Накопитель для суммы (итоговый интеграл)
    double a = atoi(argv[1]);             // Нижняя граница интегрирования (из аргумента)
    double b = atoi(argv[2]);             // Верхняя граница интегрирования (из аргумента)
    int n = 1e5;                          // Количество разбиений (100 000)
    double step = (b - a) / (double) n;   // Ширина одного отрезка
    pthread_t threads[n];                 // Массив для хранения потоков
    
    // Генерируем ключ для очереди сообщений на основе пути к файлу
    key_t key = ftok("/Users/ivan/CLionProjects/Phys_OS/msg_queue/itg.c", 0);
    
    // Цикл по всем отрезкам
    for (int i = 0; i < n; i++) {
        ThreadArgs args_i;                    // Создаём структуру с аргументами для потока
        args_i.x1 = a + i * step;             // Левая граница i-го отрезка
        args_i.x2 = a + (i + 1) * step;       // Правая граница i-го отрезка
        args_i.key = key;                     // Передаём ключ очереди
        
        // Создаём поток для вычисления площади i-й трапеции
        pthread_create(&threads[i], NULL, square, &args_i);
        // Сразу ждём завершения потока (последовательное выполнение)
        pthread_join(threads[i], NULL);
        
        int msgid;                            // ID очереди сообщений
        message_buf sbuf;                     // Буфер (не используется)
        size_t buf_length;                    // Длина буфера (не используется)
        key_t msgkey = key;                   // Копируем ключ
        
        // Получаем доступ к очереди сообщений
        if((msgid = msgget(msgkey, 0666 | IPC_CREAT)) == -1) {
            perror("msgget_hellower");  // Вывод ошибки
            exit(1);                     // Аварийное завершение
        }
        
        message_buf buf;  // Буфер для получения сообщения
        // Получаем сообщение типа 2 из очереди (результат вычисления)
        msgrcv(msgid, &buf, sizeof(message_buf), 2, 0);
        // Добавляем площадь трапеции к общей сумме
        ans += buf.mini_sq;
    }

    // Удаляем очередь сообщений (очистка ресурсов)
    msgctl(key, IPC_RMID, NULL);
    
    // Выводим результаты
    printf("ans = %f\n", ans);                              // Значение интеграла
    printf("sise: %d\n", (int)((b - a) / step) + 1);       // Количество точек
    
    return 0;  // Успешное завершение
}

