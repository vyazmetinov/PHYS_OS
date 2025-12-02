// Подключение заголовочных файлов
#include <stdio.h>       // Стандартный ввод-вывод (printf, perror, setbuf)
#include <unistd.h>      // POSIX API (sleep, getpid)
#include <signal.h>      // Работа с сигналами (здесь не используется)
#include <stdlib.h>      // Общие утилиты (exit)
#include <string.h>      // Строковые функции (здесь не используется)
#include <zlib.h>        // Библиотека сжатия (здесь не используется)
#include <sys/types.h>   // Определения типов данных
#include <sys/ipc.h>     // IPC определения (здесь не используется)
#include <sys/shm.h>     // Разделяемая память (здесь не используется)
#include <errno.h>       // Коды ошибок
#include <sys/mman.h>    // Управление памятью (здесь не используется)
#include <fcntl.h>       // Управление файлами (O_CREAT для sem_open)
#include <sys/stat.h>    // Информация о файлах (здесь не используется)
#include <pthread.h>     // POSIX потоки
#include <sys/sem.h>     // System V семафоры (здесь не используется)
#include <semaphore.h>   // POSIX семафоры (sem_t, sem_open, sem_wait, sem_post)


// Путь к проекту (здесь не используется)
const char abs_path[] = "/Users/ivan/CLionProjects/Phys_OS/";

// Количество философов (и вилок)
#define N 5

// Макрос для индекса левой вилки философа i
#define LEFT(i) (i)

// Макрос для индекса правой вилки философа i (следующая по кругу)
#define RIGHT(i) (((i) + 1) % N)

// Массив указателей на семафоры-вилки
sem_t *forks[N];

// Семафор-дворецкий (ограничивает количество философов за столом)
sem_t *butler;

// Вспомогательная функция для генерации уникального имени семафора
static void mkname(char *buf, size_t sz, const char *name, int i) {
    // Формат: /name_PID_i (например: /fork_12345_0)
    snprintf(buf, sz, "/%s_%d_%d", name, (int)getpid(), i);
}

// Массив дескрипторов потоков философов
pthread_t philocofers[N];

// Массив идентификаторов философов
int ids[N];

// Функция философа (выполняется в отдельном потоке)
void *philosopher(void *num){
    int id = *(int *)num;        // Получаем ID философа
    int left = LEFT(id);         // Вычисляем индекс левой вилки
    int right = RIGHT(id);       // Вычисляем индекс правой вилки

    setbuf(stdout, NULL);        // Отключаем буферизацию вывода (для немедленного отображения)
    
    // Бесконечный цикл жизни философа
    while(1){
        // Фаза 1: Думать
        printf("Philosopher %d is thinking\n", id);
        
        // Просим разрешения у дворецкого сесть за стол
        sem_wait(butler);
        
        // Берём левую вилку
        sem_wait(forks[left]);
        printf("Philosopher %d took left fork\n", id);
        
        // Берём правую вилку
        sem_wait(forks[right]);
        printf("Philosopher %d took right fork\n", id);
        
        // Фаза 2: Есть (у нас обе вилки)
        printf("Philosopher %d is eating\n", id);
        sleep(1);  // Едим 1 секунду
        
        // Кладём левую вилку
        sem_post(forks[left]);
        
        // Кладём правую вилку
        sem_post(forks[right]);
        printf("Philosopher %d put down both forks\n", id);
        
        // Освобождаем дворецкого (встаём из-за стола)
        sem_post(butler);
        
        sleep(1);  // Пауза перед следующим циклом
    }
    
    return NULL;  // Эта строка никогда не выполнится (бесконечный цикл)
}

// Главная функция программы
int main(){
    char name_butler[64];  // Буфер для имени семафора дворецкого
    
    // Генерируем уникальное имя для дворецкого: /butler_PID
    snprintf(name_butler, 64, "/butler_%d", (int)getpid());
    
    // Удаляем семафор, если он остался от предыдущего запуска
    sem_unlink(name_butler);
    
    // Создаём семафор дворецкого со значением N-1 (максимум 4 философа за столом)
    butler = sem_open(name_butler, O_CREAT, 0666, N - 1);
    if (butler == SEM_FAILED){ perror("sem_open"); exit(1); }
    
    char name_fork[64];  // Буфер для имени семафора вилки
    
    // Создаём семафоры для каждой вилки
    for (int i = 0; i < N; i++) {
        // Генерируем уникальное имя: /fork_PID_i
        mkname(name_fork, sizeof(name_fork), "fork", i);
        
        // Удаляем семафор, если он остался от предыдущего запуска
        sem_unlink(name_fork);
        
        // Создаём семафор вилки со значением 1 (одна вилка доступна)
        forks[i] = sem_open(name_fork, O_CREAT, 0666, 1);
        if (forks[i] == SEM_FAILED){ perror("sem_open"); exit(1); }
    }

    // Создаём потоки для всех философов
    for (int i = 0; i < N; i++) {
        ids[i] = i;  // Сохраняем ID в массив
        
        // Создаём поток для i-го философа
        if (pthread_create(&philocofers[i], NULL, philosopher, &ids[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }
    
    // Ждём завершения всех потоков (которое никогда не произойдёт - бесконечный цикл)
    for (int i = 0; i < N; i++) {
        pthread_join(philocofers[i], NULL);
    }

    // Очистка ресурсов (этот код никогда не выполнится из-за бесконечных циклов выше)
    
    // Закрываем и удаляем семафоры вилок
    for (int i = 0; i < N; i++) {
        char fname[64];
        mkname(fname, sizeof(fname), "fork", i);
        sem_close(forks[i]);     // Закрываем семафор
        sem_unlink(fname);       // Удаляем из системы
    }
    
    // Закрываем и удаляем семафор дворецкого
    snprintf(name_butler, sizeof(name_butler), "/butler_%d", (int)getpid());
    sem_close(butler);           // Закрываем семафор
    sem_unlink(name_butler);     // Удаляем из системы
    
    return 0;  // Успешное завершение
}