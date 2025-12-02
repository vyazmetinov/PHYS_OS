// Подключение заголовочных файлов
#include <sys/types.h>   // Определения базовых типов данных
#include <sys/ipc.h>     // Общие определения для IPC (ftok и константы)
#include <sys/msg.h>     // Очереди сообщений (msgget, msgsnd, msgrcv, msgctl)
#include <string.h>      // Строковые функции (strcpy, strcmp)
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror)
#include <stdlib.h>      // Общие утилиты (exit)
#include <unistd.h>      // POSIX API (sleep)


// Структура для сообщения в очереди
typedef struct msgbuf{
    long mtype;          // Тип сообщения (для фильтрации)
    char mtext[1024];    // Текст сообщения
} message_buf;

struct msqid_ds info;    // Структура для информации об очереди (не используется)

// Функция "приветствующего" - отправляет "hello world"
void hellower(key_t key, int iter) {
    int msgid;                   // ID очереди сообщений
    message_buf sbuf;            // Буфер для сообщения
    size_t buf_length;           // Длина буфера
    key_t msgkey = key;          // Копируем ключ
    
    // Получаем доступ к очереди сообщений
    if((msgid = msgget(msgkey, 0666 | IPC_CREAT)) == -1) {
        perror("msgget_hellower");  // Вывод ошибки
        exit(1);                     // Аварийное завершение
    }
    
    // Если это не первая итерация, сначала получаем сообщение
    if (iter > 0) {
        // Получаем сообщение типа 1 из очереди (блокирующий вызов)
        if (msgrcv(msgid, (void *)&sbuf, sizeof(sbuf.mtext), 1, 0) == -1) {
            perror("msgrcv");              // Вывод ошибки
            msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь
            exit(1);                        // Аварийное завершение
        }
        // Выводим полученное сообщение
        printf("hellower recieved: %s\n", sbuf.mtext);
    }

    sbuf.mtype = 1;                               // Устанавливаем тип сообщения = 1
    (void) strcpy(sbuf.mtext, "hello world");     // Копируем текст в буфер
    buf_length = sizeof(sbuf.mtext);              // Вычисляем длину буфера
    
    // Отправляем сообщение в очередь
    if(msgsnd(msgid, &sbuf, buf_length, 0) == -1) {
        perror("msgsnd");              // Вывод ошибки
        msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь
        exit(1);                        // Аварийное завершение
    }
    
    printf("hellower sent: %s\n", sbuf.mtext);  // Сообщаем об отправке
    sleep(2);                                    // Ждём 2 секунды
    return;                                      // Выходим из функции
}

// Функция "прощающегося" - отправляет "bye world"
void byier(key_t key) {
    int msgid;                   // ID очереди сообщений
    message_buf sbuf;            // Буфер для сообщения
    size_t buf_length;           // Длина буфера
    key_t msgkey = key;          // Копируем ключ
    
    // Получаем доступ к очереди сообщений
    if((msgid = msgget(msgkey, 0666 | IPC_CREAT)) == -1) {
        perror("msgget_byier");  // Вывод ошибки
        exit(1);                  // Аварийное завершение
    }

    // Получаем сообщение типа 1 из очереди (блокирующий вызов - ждём сообщения)
    if (msgrcv(msgid, (void *)&sbuf, sizeof(sbuf.mtext), 1, 0) == -1) {
        perror("msgrcv");              // Вывод ошибки
        msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь
        exit(1);                        // Аварийное завершение
    }
    
    printf("byier recieved: %s\n", sbuf.mtext);  // Выводим полученное сообщение
    
    sbuf.mtype = 1;                             // Устанавливаем тип сообщения = 1
    (void) strcpy(sbuf.mtext, "bye world");     // Копируем текст в буфер
    buf_length = sizeof(sbuf.mtext);            // Вычисляем длину буфера
    
    // Отправляем сообщение в очередь
    if(msgsnd(msgid, &sbuf, buf_length, 0) == -1) {
        perror("msgsnd");              // Вывод ошибки
        msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь
        exit(1);                        // Аварийное завершение
    }
    
    printf("byier sent: %s\n", sbuf.mtext);  // Сообщаем об отправке
    sleep(2);                                 // Ждём 2 секунды
    return;                                   // Выходим из функции
}

// Главная функция программы
int main(int argc, char *argv[]) {
    key_t key = 10;          // Простой числовой ключ для очереди
    int msgid;               // ID очереди сообщений
    key_t msgkey = key;      // Копируем ключ
    
    // Создаём очередь сообщений с ключом 10
    if((msgid = msgget(msgkey, 0666 | IPC_CREAT)) == -1) {
        perror("msgget_byier");  // Вывод ошибки
        exit(1);                  // Аварийное завершение
    }
    
    // Проверяем первый аргумент командной строки
    if (strcmp(argv[1], "hi") == 0) {
        // Режим "hi" - запускаем hellower 10 раз
        for(int i = 0; i < 10; i++) {
            hellower(key, i);  // Вызываем функцию с номером итерации
            sleep(2);          // Ждём 2 секунды между вызовами
        }
    }
    else {
        // Любой другой режим - запускаем byier 10 раз
        for(int i = 0; i < 10; i++) {
            byier(key);  // Вызываем функцию прощания
            sleep(2);    // Ждём 2 секунды между вызовами
        }
    }
    
    sleep(2);                     // Финальная пауза перед очисткой
    msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь сообщений из системы
}
