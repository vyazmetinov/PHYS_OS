#pragma once

struct Node {
    int  data;          // полезные данные
    Node *left, *right; // указатели на //левого и правого сыновей
};
typedef Node *PNode; // указатель на узел

class Tree {
    public:
    Tree();
    ~Tree();

    static void insert(PNode &root, int x);

    static PNode search(PNode root, int x);
};