// Подключение заголовочных файлов
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror, fflush, sprintf)
#include <unistd.h>      // POSIX API (sleep)
#include <signal.h>      // Работа с сигналами (здесь не используется)
#include <stdlib.h>      // Общие утилиты (EXIT_FAILURE)
#include <string.h>      // Строковые функции (strcmp)
#include <zlib.h>        // Библиотека сжатия (здесь не используется)
#include <sys/ipc.h>     // IPC общие определения (ftok, IPC_CREAT)
#include <sys/shm.h>     // Разделяемая память (shmget, shmat, shmdt, shmctl)
#include <sys/sem.h>     // System V семафоры (semget, semop, semctl)


// Путь к проекту
const char abs_path[] = "/Users/ivan/CLionProjects/Phys_OS/";

// Перечисление индексов семафоров в наборе
enum {
    SEM_EMPTY = 0,  // Семафор "количество свободных мест в буфере"
    SEM_FULL = 1,   // Семафор "количество занятых мест в буфере"
    SEM_MUTEX = 2   // Семафор-мьютекс для защиты доступа к буферу
};

// Размер кольцевого буфера
#define N 8

// Структура кольцевого буфера в разделяемой памяти
typedef struct {
    int buf[N];     // Массив для хранения данных
    int head;       // Индекс для чтения (голова очереди)
    int tail;       // Индекс для записи (хвост очереди)
} shm_ring_t;

// Функция P (Proberen - проверить, захватить семафор) - операция Wait
int P(int semid, unsigned short num) {
    // Создаём операцию: уменьшить семафор num на 1
    struct sembuf op = { .sem_num = num, .sem_op = -1, .sem_flg = 0 };
    // Выполняем операцию (блокируется, если семафор = 0)
    return semop(semid, &op, 1);
}

// Функция V (Verhogen - увеличить, освободить семафор) - операция Signal
int V(int semid, unsigned short num) {
    // Создаём операцию: увеличить семафор num на 1
    struct sembuf op = { .sem_num = num, .sem_op = +1, .sem_flg = 0 };
    // Выполняем операцию
    return semop(semid, &op, 1);
}

// Функция производителя: добавить элемент x в очередь
void produce(int sem_id, shm_ring_t *queue, int x) {
    P(sem_id, SEM_EMPTY);                    // Ждём свободного места
    P(sem_id, SEM_MUTEX);                    // Захватываем буфер (блокируем доступ)
    queue->buf[queue->tail] = x;             // Кладём элемент в хвост
    queue->tail = (queue->tail + 1) % N;     // Сдвигаем хвост по кругу
    V(sem_id, SEM_MUTEX);                    // Освобождаем буфер
    V(sem_id, SEM_FULL);                     // Сообщаем о новом элементе
}

// Функция потребителя: извлечь элемент из очереди
int consume(int sem_id, shm_ring_t *queue) {
    int x;                                   // Переменная для результата
    P(sem_id, SEM_FULL);                     // Ждём наличия элемента
    P(sem_id, SEM_MUTEX);                    // Захватываем буфер (блокируем доступ)
    x = queue->buf[queue->head];             // Берём элемент из головы
    queue->head = (queue->head + 1) % N;     // Сдвигаем голову по кругу
    V(sem_id, SEM_MUTEX);                    // Освобождаем буфер
    V(sem_id, SEM_EMPTY);                    // Сообщаем о свободном месте
    return x;                                // Возвращаем извлечённый элемент
}

// Главная функция программы
int main(int argc, char *argv[])
{
    int sem_id;              // ID набора семафоров
    char pathname[1024];     // Буфер для полного пути к файлу
    
    // Формируем путь к текущему файлу для генерации ключа
    sprintf(pathname, "%s/%s", abs_path, "producer-consumer.c");
    
    key_t key;  // Ключ для IPC-объектов
    
    // Генерируем уникальный ключ на основе пути к файлу
    if ((key = ftok(pathname, 0)) < 0) {
        perror("ftok");
        return EXIT_FAILURE;
    }
    
    // Создаём или получаем доступ к сегменту разделяемой памяти
    int shmid = shmget(key, sizeof(shm_ring_t), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return EXIT_FAILURE;
    }
    
    // Подключаем разделяемую память к адресному пространству процесса
    shm_ring_t *queue = (shm_ring_t *) shmat(shmid, NULL, 0);
    if (queue == (void*)-1) {
        perror("shmat");
    }
    
    // Создаём или получаем доступ к набору из 3 семафоров
    sem_id = semget(key, 3, IPC_CREAT|0666);
    if (sem_id == -1) {
        perror("semget");
        return EXIT_FAILURE;
    }
    
    struct shmid_ds ds;  // Структура для информации о разделяемой памяти
    
    // Получаем информацию о сегменте памяти
    shmctl(shmid, IPC_STAT, &ds);
    
    // Если мы первый процесс, подключившийся к памяти - инициализируем всё
    if (ds.shm_nattch == 1) {
        queue->head = 0;                          // Голова очереди в начале
        queue->tail = 0;                          // Хвост очереди в начале
        semctl(sem_id, SEM_EMPTY, SETVAL, N);     // Семафор EMPTY = N (все места свободны)
        semctl(sem_id, SEM_FULL, SETVAL, 0);      // Семафор FULL = 0 (нет элементов)
        semctl(sem_id, SEM_MUTEX, SETVAL, 1);     // Семафор MUTEX = 1 (доступен)
    }
    
    // Проверяем режим работы: производитель или потребитель
    if (argc > 1 && strcmp(argv[1], "producer") == 0) {
        // РЕЖИМ ПРОИЗВОДИТЕЛЯ: генерируем 100 чисел
        for (int i = 0; i < 100; i++) {
            P(sem_id, SEM_EMPTY);                     // Ждём свободного места
            P(sem_id, SEM_MUTEX);                     // Захватываем буфер
            queue->buf[queue->tail] = i;              // Кладём число i
            queue->tail = (queue->tail + 1) % N;      // Сдвигаем хвост
            V(sem_id, SEM_MUTEX);                     // Освобождаем буфер
            V(sem_id, SEM_FULL);                      // Сообщаем о новом элементе
            sleep(1);                                  // Ждём 1 секунду (имитация работы)
        }
    }
    else {
        // РЕЖИМ ПОТРЕБИТЕЛЯ: получаем 100 чисел
        for (int i = 0; i < 100; i++) {
            P(sem_id, SEM_FULL);                      // Ждём наличия элемента
            P(sem_id, SEM_MUTEX);                     // Захватываем буфер
            int x = queue->buf[queue->head];          // Берём элемент
            queue->head = (queue->head + 1) % N;      // Сдвигаем голову
            V(sem_id, SEM_MUTEX);                     // Освобождаем буфер
            V(sem_id, SEM_EMPTY);                     // Сообщаем о свободном месте
            printf("Consumed: %d\n", x);              // Выводим полученное число
            fflush(stdout);                           // Принудительно сбрасываем буфер вывода
            sleep(2);                                  // Ждём 2 секунды (имитация обработки)
        }
    }
    
    // Отключаем разделяемую память от процесса
    shmdt(queue);
    
    return 0;  // Успешное завершение
}