// Подключение заголовочных файлов
#include <math.h>        // Математические функции (здесь не используется)
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror)
#include <stdlib.h>      // Общие утилиты (exit, getpid)
#include <pthread.h>     // POSIX потоки (pthread_create, pthread_join)
#include <sys/msg.h>     // Очереди сообщений (msgget, msgsnd, msgrcv)
#include <unistd.h>      // POSIX API (sleep, getpid)
#include <string.h>      // Строковые функции (strcmp, sprintf)


// Структура для сообщения в очереди
typedef struct {
    long mtype;          // Тип сообщения (используется для фильтрации при получении)
    char mtext[2048];    // Текст сообщения (максимум 2048 символа)
} message_buf;

// Функция клиента - будет выполняться в отдельном потоке
void* client(void* args) {
    key_t key = *(key_t*)args;  // Получаем ключ очереди из аргумента

    message_buf msg;                                      // Создаём структуру для сообщения
    msg.mtype = 1;                                        // Устанавливаем тип сообщения = 1
    sprintf(msg.mtext, "Hello from client %lu\n", (unsigned long)pthread_self); // Формируем текст с ID клиента

    // Получаем доступ к очереди сообщений (или создаём, если не существует)
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) { perror("msgget"); exit(1); }  // Проверка на ошибку

    printf("sending %lu\n", (unsigned long) pthread_self);  // Выводим номер отправляемого сообщения
    // Отправляем сообщение в очередь
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");  // Выводим ошибку, если отправка не удалась
    }
        sleep(1);  // Ждём 1 секунду перед следующей отправкой
    return NULL;  // Завершаем поток
}

// Функция сервера - будет выполняться в отдельном потоке
void* server(void* args) {
    key_t key = *(key_t*)args;  // Получаем ключ очереди из аргумента

    message_buf msg;  // Структура для получения сообщений
    // Получаем доступ к очереди сообщений
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) { perror("msgget"); exit(1); }  // Проверка на ошибку

    // Бесконечный цикл приёма сообщений
    while (1) {
        // Получаем сообщение типа 1 из очереди (блокирующий вызов)
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
            perror("msgrcv");  // Если ошибка - выводим её
            break;             // И выходим из цикла
        }
        // Выводим полученное сообщение
        printf("Server receive this message from client:\n");
        printf("%s\n", msg.mtext);
    }
    return NULL;  // Завершаем поток
}


// Главная функция программы
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [c|s]\n", argv[0]);
        return 1;
    }
    printf("%s\n", argv[1]);  // Выводим первый аргумент командной строки (режим работы)

    // Генерируем уникальный ключ на основе пути к файлу и числа 0
    key_t key = ftok("/Users/ivan/CLionProjects/Phys_OS/msg_queue/cs.c", 0);
    if (key == -1) { perror("ftok"); exit(1); }  // Проверка на ошибку

    pthread_t clients_thread[10];  // Массив для 10 потоков клиентов
    pthread_t server_thread;       // Поток для сервера
    // Проверяем режим работы: "c" = клиент, иначе = сервер
    if (strcmp(argv[1], "c") == 0) {
        // Режим клиента: создаём 10 потоков-клиентов
        for (int i = 0; i < 10; i++) {
            // Создаём новый поток, который выполнит функцию client
            pthread_create(&clients_thread[i], NULL, client, (void*)&key);
        }

        // Ждём завершения всех клиентских потоков
        for (int i = 0; i < 10; i++) {
            pthread_join(clients_thread[i], NULL);
        }
    }
    else {
        // Режим сервера: создаём (или открываем) очередь сообщений
        int msgid = msgget(key, 0666 | IPC_CREAT);
        if (msgid == -1) { perror("msgget"); exit(1); }  // Проверка на ошибку

        // Режим сервера: создаём один поток-сервер
        pthread_create(&server_thread, NULL, server, (void*)&key);
        // Ждём завершения потока сервера (который работает бесконечно)
        pthread_join(server_thread, NULL);
        // Удаляем очередь сообщений из системы (очистка ресурсов)
        msgctl(msgid, IPC_RMID, NULL);
    }



    printf("%s\n", argv[1]);  // Снова выводим режим работы



    return 0;  // Успешное завершение программы
}