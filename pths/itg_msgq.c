#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/wait.h>

const int process_amnt = 3;

// Структура сообщения для передачи результатов
typedef struct {
    long mtype;           // Тип сообщения (обязательное поле)
    int process_id;       // ID процесса
    double from_x;        // Начало отрезка
    double to_x;          // Конец отрезка
    double result;        // Результат вычисления
} msg_result_t;

double f(double x) {
    return sqrt(x);
}

double max(double a, double b) {
    return a > b ? a : b;
}

// Вычисление оптимального шага интегрирования
double precision_cont(double from, double to, double eps) {
    if (to - from < 1e-10) {
        return (to - from) / 100.0;
    }
    
    double max_sec_drv = 0.0;
    double step = (to - from) / 1000.0;
    
    for (double i = from; i < to - 2 * step; i += step) {
        double tmp_sec_drv = fabs(f(i + 2 * step) - 2 * f(i + step) + f(i));
        max_sec_drv = max(max_sec_drv, tmp_sec_drv);
    }
    
    if (max_sec_drv < 1e-10) {
        max_sec_drv = 1e-10;
    }
    
    double h = 6 * eps / fabs(max_sec_drv);
    
    double min_step = (to - from) / 100000.0;
    double max_step = (to - from) / 10.0;
    if (h < min_step) h = min_step;
    if (h > max_step) h = max_step;
    
    return h;
}

// Вычисление интеграла непрерывной функции методом трапеций
double square_cont(double from_x, double to_x, double eps) {
    if (from_x >= to_x) {
        return 0.0;
    }
    
    double h = precision_cont(from_x, to_x, eps);
    
    if (h <= 0.0 || !isfinite(h)) {
        fprintf(stderr, "Error: invalid step h = %f\n", h);
        h = (to_x - from_x) / 1000.0;
    }
    
    double sum = 0.0;
    int iterations = 0;
    const int max_iterations = 10000000;
    
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
    
    return sum;
}

// Дочерний процесс: вычисляет интеграл и отправляет результат через очередь сообщений
void child_process(int msgq_id, int proc_id, double from_x, double to_x, double eps) {
    printf("[Процесс %d, PID=%d] Вычисляю интеграл на [%.6f, %.6f]\n", 
           proc_id, getpid(), from_x, to_x);
    
    // Вычисляем интеграл
    double result = square_cont(from_x, to_x, eps);
    
    printf("[Процесс %d, PID=%d] Результат: %.6f\n", proc_id, getpid(), result);
    
    // Формируем сообщение
    msg_result_t msg;
    msg.mtype = 1;  // Тип сообщения
    msg.process_id = proc_id;
    msg.from_x = from_x;
    msg.to_x = to_x;
    msg.result = result;
    
    // Отправляем результат в очередь сообщений
    if (msgsnd(msgq_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    
    printf("[Процесс %d, PID=%d] Результат отправлен в очередь сообщений\n", proc_id, getpid());
    exit(0);
}

// Параллельное вычисление интеграла с использованием процессов и очередей сообщений
double parallel_square_msgq(double from_x, double to_x, double eps, int processes) {
    if (processes < 1) {
        processes = 1;
    }
    
    double range = to_x - from_x;
    if (range <= 0.0) {
        return 0.0;
    }
    
    // Минимальная длина подотрезка
    const double MIN_RANGE = 0.001;
    
    int max_processes = (int)(range / MIN_RANGE);
    if (max_processes < 1) max_processes = 1;
    if (processes > max_processes) {
        processes = max_processes;
    }
    
    printf("\n=== Создание очереди сообщений ===\n");
    
    // Создаем ключ для очереди сообщений
    key_t key = ftok("/tmp", 'I');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    
    // Создаем очередь сообщений
    int msgq_id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
    if (msgq_id == -1) {
        // Если очередь уже существует, удаляем старую и создаем новую
        msgq_id = msgget(key, 0666);
        if (msgq_id != -1) {
            msgctl(msgq_id, IPC_RMID, NULL);
        }
        msgq_id = msgget(key, 0666 | IPC_CREAT);
        if (msgq_id == -1) {
            perror("msgget");
            exit(1);
        }
    }
    
    printf("Очередь сообщений создана (ID=%d)\n", msgq_id);
    printf("\n=== Запуск %d процессов ===\n", processes);
    
    // Длина подотрезка на один процесс
    double segment_len = range / processes;
    
    pid_t pids[processes];
    int pcount = 0;
    
    // Создаем дочерние процессы
    for (int k = 0; k < processes; ++k) {
        double current_from = from_x + k * segment_len;
        double current_to;
        
        if (k == processes - 1) {
            current_to = to_x;
        } else {
            current_to = current_from + segment_len;
        }
        
        if (current_to - current_from < 1e-10) {
            break;
        }
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Дочерний процесс
            child_process(msgq_id, k, current_from, current_to, eps);
            // Не должны сюда попасть
            exit(0);
        } else {
            // Родительский процесс
            pids[pcount++] = pid;
        }
    }
    
    printf("\n=== Родительский процесс (PID=%d) ожидает результаты ===\n", getpid());
    
    // Родительский процесс собирает результаты из очереди
    double total_sum = 0.0;
    
    for (int k = 0; k < pcount; ++k) {
        msg_result_t msg;
        
        // Получаем сообщение из очереди
        if (msgrcv(msgq_id, &msg, sizeof(msg) - sizeof(long), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        
        printf("[Родитель] Получен результат от процесса %d: %.6f (отрезок [%.6f, %.6f])\n", 
               msg.process_id, msg.result, msg.from_x, msg.to_x);
        
        total_sum += msg.result;
    }
    
    printf("\n=== Ожидание завершения всех процессов ===\n");
    
    // Ждем завершения всех дочерних процессов
    for (int k = 0; k < pcount; ++k) {
        int status;
        pid_t pid = wait(&status);
        if (WIFEXITED(status)) {
            printf("[Родитель] Процесс PID=%d завершился с кодом %d\n", 
                   pid, WEXITSTATUS(status));
        }
    }
    
    // Удаляем очередь сообщений
    printf("\n=== Удаление очереди сообщений ===\n");
    if (msgctl(msgq_id, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
    } else {
        printf("Очередь сообщений успешно удалена\n");
    }
    
    return total_sum;
}

int main(int argc, char *argv[]) {
    if (argc == 5 && strcmp(argv[1], "cont") == 0) {
        printf("=================================================\n");
        printf("  Интегрирование с использованием процессов\n");
        printf("  и очередей сообщений System V (msgq)\n");
        printf("=================================================\n");
        printf("Функция: f(x) = sqrt(x)\n");
        
        double a = atof(argv[2]);
        double b = atof(argv[3]);
        double eps = atof(argv[4]);
        
        printf("Интервал: [%.6f, %.6f]\n", a, b);
        printf("Точность eps: %f\n", eps);
        printf("Количество процессов: %d\n", process_amnt);
        
        double result = parallel_square_msgq(a, b, eps, process_amnt);
        
        printf("\n=================================================\n");
        printf("  ИТОГОВЫЙ РЕЗУЛЬТАТ = %.6f\n", result);
        printf("=================================================\n");
        
        // Проверка с аналитическим решением
        double analytical = (2.0/3.0) * (pow(b, 1.5) - pow(a, 1.5));
        printf("Аналитическое решение: %.6f\n", analytical);
        printf("Погрешность: %.6f (%.4f%%)\n", 
               fabs(result - analytical), 
               fabs(result - analytical) / analytical * 100.0);
    } else {
        fprintf(stderr, "Использование:\n");
        fprintf(stderr, "  %s cont a b eps\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Параметры:\n");
        fprintf(stderr, "  cont - режим непрерывной функции f(x) = sqrt(x)\n");
        fprintf(stderr, "  a    - начало интервала\n");
        fprintf(stderr, "  b    - конец интервала\n");
        fprintf(stderr, "  eps  - точность интегрирования\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Примеры:\n");
        fprintf(stderr, "  %s cont 0 4 0.01\n", argv[0]);
        fprintf(stderr, "  %s cont 1 9 0.001\n", argv[0]);
        fprintf(stderr, "  %s cont 1 16 0.001\n", argv[0]);
        return 1;
    }
    
    return 0;
}

