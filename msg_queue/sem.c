// Подключение заголовочных файлов
#include <sys/types.h>   // Определения базовых типов данных
#include <sys/ipc.h>     // IPC общие определения (ftok, IPC_CREAT)
#include <sys/msg.h>     // Очереди сообщений (msgget, msgsnd, msgrcv, msgctl)
#include <string.h>      // Строковые функции (strcpy, strcmp)
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror, snprintf)
#include <stdlib.h>      // Общие утилиты (exit, atoi)
#include <unistd.h>      // POSIX API (здесь не используется напрямую)
#include <sys/sem.h>     // System V семафоры (здесь не используется, но подключен)
#include <errno.h>       // Коды ошибок (EEXIST, ENOENT)
#include <sys/shm.h>     // Разделяемая память (shmget, shmat, shmdt, shmctl)

// Структура сообщения для коммуникации между семафорами
typedef struct  msgsem {
    long mtype;        // Тип сообщения (используется для адресации конкретного семафора)
    char mtext[100];   // Текст сообщения (содержит число - изменение счётчика)
} message_buf;

// Структура пользовательского семафора
typedef struct my_sem {
    short sem_num;      // ID семафора (индекс в таблице)
    short sem_op;       // Текущее значение счётчика семафора
    short sem_flg;      // Флаги (здесь не используются)
    int sem_msg_id;     // ID очереди сообщений для этого семафора
} my_sem;

// Структура таблицы семафоров (хранится в разделяемой памяти)
typedef struct sem_buf {
    my_sem sem[128];    // Массив семафоров (максимум 128)
    int used_amount;    // Количество используемых семафоров
} sem_buf;

// Глобальный указатель на таблицу семафоров в разделяемой памяти
static sem_buf *sem_tbl = NULL;

// Функция создания/подключения к таблице семафоров в разделяемой памяти
void create_buf() {
    int shmid;  // ID сегмента разделяемой памяти
    
    // Генерируем уникальный ключ на основе пути к файлу
    key_t key = ftok("/Users/ivan/CLionProjects/Phys_OS/msg_queue/sem.c", 0);
    if (key == -1) {
        perror("ftok");  // Вывод ошибки
        exit(1);         // Аварийное завершение
    }
    
    // Пытаемся СОЗДАТЬ новый сегмент памяти (IPC_EXCL гарантирует, что создаём новый)
    if ((shmid = shmget(key, sizeof(sem_buf), IPC_CREAT | IPC_EXCL | 0666)) >= 0) {
        // Успешно создали новый сегмент - мы первый процесс
        
        // Подключаем сегмент к нашему адресному пространству
        sem_tbl = (sem_buf *) shmat(shmid, NULL, 0);
        if (sem_tbl == (void *) -1) {
            perror("shmat");  // Вывод ошибки
            exit(1);          // Аварийное завершение
        }
        
        // Инициализируем таблицу семафоров
        sem_tbl->used_amount = 1;          // Резервируем семафор 0 для защиты таблицы
        sem_tbl->sem[0].sem_num = 0;       // ID семафора = 0
        sem_tbl->sem[0].sem_op = 1;        // Счётчик = 1 (семафор-мьютекс)
        sem_tbl->sem[0].sem_flg = 0;       // Флаги не используются
    }
    else if (errno == EEXIST) {
        // Сегмент уже существует - другой процесс создал его раньше
        
        // Подключаемся к существующему сегменту (без IPC_EXCL)
        shmid = shmget(key, sizeof(sem_buf), 0666);
        if (shmid == -1) {
            perror("shmget");  // Вывод ошибки
            exit(1);           // Аварийное завершение
        }
        
        // Подключаем существующий сегмент к нашему адресному пространству
        sem_tbl = (sem_buf *) shmat(shmid, NULL, 0);
        if (sem_tbl == (void *) -1) {
            perror("shmat");  // Вывод ошибки
            exit(1);          // Аварийное завершение
        }
    }
    else {
        // Другая ошибка при создании сегмента
        perror("shmget");
        exit(1);
    }
}


// Функция создания нового семафора с начальным значением init_val
int create_sem(int init_val) {
    // Если таблица семафоров ещё не создана - создаём её
    if (sem_tbl == NULL) {
        create_buf();
    }
    
    // Проверяем, не переполнен ли массив семафоров
    if (sem_tbl->used_amount >= 128) {
        printf("Массив семафоров переполнен");
        exit(1);
    }
    
    // Инициализируем новый семафор в таблице
    sem_tbl->sem[sem_tbl->used_amount].sem_num = sem_tbl->used_amount;  // ID = текущий индекс
    sem_tbl->sem[sem_tbl->used_amount].sem_op = init_val;               // Начальное значение счётчика
    sem_tbl->sem[sem_tbl->used_amount].sem_flg = 0;                     // Флаги не используются
    
    // Создаём уникальный ключ для очереди сообщений этого семафора
    char key_txt[1024];
    snprintf(key_txt, 1024, "%s","/CLionProjects/Phys_OS/msg_queue/sem.c");
    
    // Генерируем ключ: используем used_amount+1 как project_id для уникальности
    key_t key = ftok(key_txt, (int)sem_tbl->used_amount + 1);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    
    // Создаём очередь сообщений для этого семафора
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) { perror("msgget"); exit(1); }
    
    // Сохраняем ID очереди в структуре семафора
    sem_tbl->sem[sem_tbl->used_amount].sem_msg_id = msgid;
    
    // Увеличиваем счётчик используемых семафоров
    sem_tbl->used_amount++;
    
    // Возвращаем ID созданного семафора
    return sem_tbl->used_amount - 1;
}

// Функция блокировки таблицы семафоров (захват мьютекса таблицы)
void lock_sem_tbl(void) {
    // Проверяем, инициализирована ли таблица
    if (!sem_tbl) {
        fprintf(stderr, "sem_tbl not initialized in lock\n");
        exit(1);
    }
    
    // Получаем ID очереди семафора 0 (мьютекс таблицы)
    int msgid = sem_tbl->sem[0].sem_msg_id;
    message_buf sbuf;  // Буфер для получения сообщения

    // Получаем сообщение из очереди (блокирующий вызов)
    // Это эквивалент операции P (Wait) для семафора-мьютекса
    if (msgrcv(msgid, &sbuf, sizeof(sbuf.mtext), 1, 0) == -1) {
        perror("msgrcv lock");
        exit(1);
    }
    // После успешного получения сообщения - таблица заблокирована
}

// Функция разблокировки таблицы семафоров (освобождение мьютекса таблицы)
void unlock_sem_tbl(void) {
    // Проверяем, инициализирована ли таблица
    if (!sem_tbl) {
        fprintf(stderr, "sem_tbl not initialized in unlock\n");
        exit(1);
    }
    
    // Получаем ID очереди семафора 0 (мьютекс таблицы)
    int msgid = sem_tbl->sem[0].sem_msg_id;
    message_buf sbuf;       // Буфер для отправки сообщения
    sbuf.mtype = 1;         // Устанавливаем тип сообщения
    strcpy(sbuf.mtext, "1"); // Содержимое не важно, важен сам факт сообщения

    // Отправляем сообщение в очередь
    // Это эквивалент операции V (Signal) для семафора-мьютекса
    if (msgsnd(msgid, &sbuf, sizeof(sbuf.mtext), 0) == -1) {
        perror("msgsnd unlock");
        exit(1);
    }
    // После отправки сообщения - таблица разблокирована
}

// Функция ожидания семафора (операция P/Wait) - уменьшает счётчик на P
int sem_wait(my_sem *sem, int P) {

    // Бесконечный цикл ожидания
    while (1) {
        // Блокируем таблицу семафоров для безопасного чтения
        lock_sem_tbl();
        
        // Проверяем, достаточно ли ресурсов (счётчик >= P)
        if (sem_tbl->sem[sem->sem_num].sem_op >= P) {
            // Ресурсы доступны - захватываем их (уменьшаем счётчик)
            sem_tbl->sem[sem->sem_num].sem_op -= P;
            
            // Разблокируем таблицу
            unlock_sem_tbl();
            
            // Возвращаем успех
            return 0;
        }
        
        // Ресурсов недостаточно - разблокируем таблицу и ждём
        unlock_sem_tbl();

        // Ждём сообщения в очереди этого семафора (блокирующий вызов)
        message_buf sbuf;
        if (msgrcv(sem_tbl->sem[sem->sem_num].sem_msg_id, (void *)&sbuf,
                   sizeof(sbuf.mtext),
                   sem_tbl->sem[sem->sem_num].sem_num, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        // Получили сообщение - значит кто-то освободил ресурсы

        // Блокируем таблицу для обновления счётчика
        lock_sem_tbl();
        
        // Увеличиваем счётчик на значение из сообщения
        sem_tbl->sem[sem->sem_num].sem_op += atoi(sbuf.mtext);
        
        // Разблокируем таблицу
        unlock_sem_tbl();
        
        // Возвращаемся в начало цикла для повторной проверки
    }
}

// Функция освобождения семафора (операция V/Signal) - увеличивает счётчик на op
void sem_post(my_sem *sem, int op) {
    // Проверяем, инициализирована ли таблица
    if (!sem_tbl) {
        fprintf(stderr, "sem_tbl not initialized\n");
        return;
    }
    
    // Проверяем корректность ID семафора
    if (sem->sem_num < 0 || sem->sem_num >= sem_tbl->used_amount) {
        fprintf(stderr, "invalid sem_num\n");
        return;
    }
    
    // Блокируем таблицу для безопасного изменения счётчика
    lock_sem_tbl();
    
    // Увеличиваем счётчик семафора на op (освобождаем ресурсы)
    sem_tbl->sem[sem->sem_num].sem_op += op;
    
    // Разблокируем таблицу
    unlock_sem_tbl();
    
    // Отправляем сообщение в очередь, чтобы разбудить ожидающие процессы
    message_buf sbuf;
    sbuf.mtype = sem_tbl->sem[sem->sem_num].sem_num;  // Тип = ID семафора
    (void) snprintf(sbuf.mtext, sizeof(sbuf.mtext), "%d", op);  // Значение увеличения
    
    // Отправляем сообщение
    if (msgsnd(sem_tbl->sem[sem->sem_num].sem_msg_id, &sbuf, sizeof(sbuf), 0)) {
        perror("msgsnd");
        exit(1);
    }

}


// Функция удаления конкретного семафора
void destroy_sem(int sem_id) {
    // Проверяем, инициализирована ли таблица
    if (!sem_tbl) {
        fprintf(stderr, "destroy_sem: sem_tbl not initialized\n");
        return;
    }

    // Проверяем корректность ID (защищаем семафор 0 - мьютекс таблицы)
    if (sem_id <= 0 || sem_id >= sem_tbl->used_amount) {
        fprintf(stderr, "destroy_sem: invalid sem_id %d\n", sem_id);
        return;
    }

    // Получаем указатель на семафор
    my_sem *s = &sem_tbl->sem[sem_id];

    // Удаляем очередь сообщений этого семафора
    if (msgctl(s->sem_msg_id, IPC_RMID, NULL) == -1) {
        perror("destroy_sem: msgctl IPC_RMID");
    }

    // Очищаем структуру семафора
    s->sem_num = -1;      // Помечаем как недействительный
    s->sem_op  = 0;       // Обнуляем счётчик
    s->sem_flg = 0;       // Обнуляем флаги
    s->sem_msg_id = -1;   // Помечаем очередь как недействительную

    // Если это был последний семафор в таблице - уменьшаем счётчик
    if (sem_id == sem_tbl->used_amount - 1) {
        sem_tbl->used_amount--;
    }
}

// Функция полного удаления всех семафоров и таблицы
void destroy_all_sems(void) {
    key_t key;   // Ключ для IPC
    int shmid;   // ID разделяемой памяти

    // Если таблица не подключена - подключаемся к ней
    if (!sem_tbl) {
        // Генерируем ключ
        key = ftok("/CLionProjects/Phys_OS/msg_queue/sem.c", 0);
        if (key == -1) {
            perror("destroy_all_sems: ftok");
            return;
        }
        
        // Получаем ID существующего сегмента памяти
        shmid = shmget(key, sizeof(sem_buf), 0666);
        if (shmid == -1) {
            if (errno == ENOENT) {
                // Сегмент не существует - нечего удалять
                return;
            }
            perror("destroy_all_sems: shmget");
            return;
        }
        
        // Подключаем сегмент к адресному пространству
        sem_tbl = (sem_buf *)shmat(shmid, NULL, 0);
        if (sem_tbl == (void *)-1) {
            perror("destroy_all_sems: shmat");
            sem_tbl = NULL;
            return;
        }
    } else {
        // Таблица уже подключена - получаем ID её сегмента
        key = ftok("/Users/ivan/CLionProjects/Phys_OS/msg_queue/sem.c", 0);
        if (key == -1) {
            perror("destroy_all_sems: ftok");
            return;
        }
        
        shmid = shmget(key, sizeof(sem_buf), 0666);
        if (shmid == -1) {
            perror("destroy_all_sems: shmget");
            return;
        }
    }

    // Удаляем все очереди сообщений всех семафоров
    for (int i = 0; i < sem_tbl->used_amount; ++i) {
        int msgid = sem_tbl->sem[i].sem_msg_id;  // Получаем ID очереди
        
        if (msgid > 0) {  // Если очередь существует
            // Удаляем очередь сообщений
            if (msgctl(msgid, IPC_RMID, NULL) == -1) {
                perror("destroy_all_sems: msgctl IPC_RMID");
            }
            // Помечаем как удалённую
            sem_tbl->sem[i].sem_msg_id = -1;
        }
    }

    // Отключаем сегмент разделяемой памяти от процесса
    if (shmdt(sem_tbl) == -1) {
        perror("destroy_all_sems: shmdt");
    }
    
    // Обнуляем глобальный указатель
    sem_tbl = NULL;
    
    // Удаляем сегмент разделяемой памяти из системы
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("destroy_all_sems: shmctl IPC_RMID");
    }
}

// Главная функция (пустая - это библиотека для использования в других программах)
int main(int argc, char *argv[]) {
    // Этот файл предназначен для использования как библиотека
    // Функции вызываются из других программ
}