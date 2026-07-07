#include <iostream>

using namespace std;

struct Node {
    int data;
    Node *next, *prev;
};

typedef Node *PNode;

struct Queue {
    PNode head = nullptr, tail = nullptr;
};

void PushTail ( Queue &Q, int x )
{
    PNode NewNode;
    NewNode = new Node; // создать новый узел
    NewNode->data = x; // заполнить узел данными
    NewNode->prev = Q.tail;
    NewNode->next = NULL;
    if ( Q.tail ) // добавить узел в конец списка
        Q.tail->next = NewNode;
    Q.tail = NewNode;
    if ( ! Q.head ) Q.head = Q.tail;
}

int PopHead(Queue &Q) {
    if (!Q.tail)
        return -1;
    PNode old = Q.head;
    int val = old->data;
    Q.head = old->next;
    if (Q.head)
        Q.head->prev = nullptr;
    else
        Q.tail = nullptr;
    delete old;
    return val;
}

int main() {
    Queue Q;
    PushTail(Q, 10);
    PushTail(Q, 20);
    cout << Q.head->data << endl;
    cout << Q.tail->data << endl;
    cout << PopHead(Q) << endl;
    cout << Q.head->data << endl;
}