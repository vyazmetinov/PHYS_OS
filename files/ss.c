#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

struct A {
    double f;   // само число
    double f2;  // его квадрат
};

int main(void)
{
    int fd;                 /* файловый дескриптор */
    size_t length;          /* длина отображаемой части файла */
    int i;
    struct A *ptr, *tmpptr; /* указатель на начало и "бегунок" */
    double sum = 0.0;

    /* Открываем файл read-write, создаём, если его нет */
    fd = open("mapped.dat", O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("File open failed");
        exit(1);
    }

    /* Хотим хранить 100000 структур */
    length = 100000 * sizeof(struct A);

    /* Увеличиваем файл до нужной длины */
    if (ftruncate(fd, length) == -1) {
        perror("ftruncate failed");
        close(fd);
        exit(1);
    }

    /* Отображаем файл в память */
    ptr = (struct A *)mmap(NULL, length, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Mapping failed");
        close(fd);
        exit(2);
    }

    /* Дескриптор больше не нужен */
    close(fd);

    /* === ЧАСТЬ 1. ЗАПИСЬ ЧИСЕЛ И ИХ КВАДРАТОВ В ФАЙЛ === */
    tmpptr = ptr;
    for (i = 1; i <= 100000; i++) {
        tmpptr->f  = (double)i;
        tmpptr->f2 = tmpptr->f * tmpptr->f;
        tmpptr++;
    }

    /* === ЧАСТЬ 2. ЧТЕНИЕ И СУММА КВАДРАТОВ ИЗ УЖЕ ЗАПОЛНЕННОГО ФАЙЛА === */
    sum = 0.0;
    tmpptr = ptr;
    for (i = 0; i < 100000; i++) {
        sum += tmpptr->f2;  // складываем квадраты
        tmpptr++;
    }

    printf("Sum of squares from 1 to 100000 = %.0f\n", sum);

    /* Разотображаем память */
    munmap((void *)ptr, length);

    return 0;
}