#include <stdio.h>
#include <mpi.h>


typedef int *pint;

struct matrix {
    int size;
    pint *data[];
};


int main (int argc, char *argv[]) {
    return 0;
}
