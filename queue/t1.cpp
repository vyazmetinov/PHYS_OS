#include <iostream>

using namespace std;


const int MAXSIZE = 100;

class Queue {
private:
    int Size = 0;
    int rear = 0;
    int front = 0;
    int data[MAXSIZE];
public:
    int size() {
        return Size;
    }
    void push(int el) {
        data[rear] = el;
        rear = (rear + 1) % MAXSIZE;
        Size++;
    }
    int pop() {
        front = (front + 1) % MAXSIZE;
        Size--;
        return data[front - 1];
    }
};



int main(){
    Queue Q;
    Q.push(10);
    Q.push(20);
    cout << Q.size() << '\n';
    cout << Q.pop();
    return 0;
}