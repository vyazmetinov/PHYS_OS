#include <iostream>
#include <pthread.h>
using namespace std;




typedef int *Pint;
class Matrix {
    public:
    int size;
    Pint *data;
    Matrix(int size) {
        this->size = size;
        this->data = new Pint[size];
        for(int i = 0; i < size; i++) {
            this->data[i] = new int[size];
        }
    }
    Matrix() {
        this->size = 0;
        this->data = NULL;
    }
    int* operator[](int i) {
        return data[i];
    }

    const int* operator[](int i) const {
        return data[i];
    }

};


class Vector {
    public:
    int *data;
    int size;
    Vector(int size) {
        this->size = size;
        this->data = new int[size];
    }
    Vector() {
        this->size = 0;
        this->data = NULL;
    }
    int& operator[](int i) {
        return data[i];
    }

    const int& operator[](int i) const {
        return data[i];
    }
    void resize(int new_size) {
        this->size = new_size;
        this->data = new int[new_size];
    }

};

istream& operator >> (istream& in, Vector& v) {
    for(int i = 0; i < v.size; i++) {
        in >> v.data[i];
    }
    return in;
}


struct ThreadArgs {
    int ind;
    Vector* vector;
    Matrix* matrix;
};

Vector ans;

void* mul(void* args) {
    ThreadArgs* targs = static_cast<ThreadArgs*>(args);
    int ind = targs->ind;
    Matrix& matrix = *(targs->matrix);
    Vector& vector = *(targs->vector);

    int res = 0;
    for (int i = 0; i < matrix.size; i++) {
        res += matrix[ind][i] * vector[i];
    }
    ans[ind] = res;
    return NULL;
}

int main (int argc, char *argv[]) {
    int n;
    cin >> n;
    ans.resize(n);

    Matrix m(n);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            cin >> m[i][j];
        }
    }
    Vector v(n);
    cin >> v;
    pthread_t* threads = new pthread_t[n];
    ThreadArgs* args = new ThreadArgs[n];

    for (int i = 0; i < n; i++) {
        args[i].ind = i;
        args[i].matrix = &m;
        args[i].vector = &v;
        pthread_create(&threads[i], NULL, mul, &args[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < n; i++) {
        cout << ans[i] << ' ';
    }
    cout << endl;

    delete[] args;
    delete[] threads;
}