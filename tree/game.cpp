#include <iostream>
#include <cstring>
#include "tree.h"

using namespace std;

int Priority(char c) {
    switch (c) {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        default: return 100;
    }
}

int LastOperation(char Expr[], int first, int last) {
    int MinPrt, i, k, prt;
    MinPrt = 100;
    for (i = first; i <= last; i++) {
        prt = Priority(Expr[i]);
        if (prt <= MinPrt) {
            MinPrt = prt;
            k = i;
        }
    }
    return k;
}

PNode NumberNode(char c) {
    PNode Tree = new Node;
    Tree->data = c;
    Tree->left = NULL;
    Tree->right = NULL;
    return Tree;
}

PNode MakeTree(char Expr[], int first, int last) {
    PNode Tree;
    int k;
    if (first == last)
        return NumberNode(Expr[first]);

    k = LastOperation(Expr, first, last);
    Tree = new Node;
    Tree->data = Expr[k];
    Tree->left = MakeTree(Expr, first, k - 1);
    Tree->right = MakeTree(Expr, k + 1, last);
    return Tree;
}

int CalcTree(PNode Tree) {
    int num1, num2;
    if (!Tree->left) return Tree->data - '0';

    num1 = CalcTree(Tree->left);
    num2 = CalcTree(Tree->right);

    switch (Tree->data) {
        case '+': return num1 + num2;
        case '-': return num1 - num2;
        case '*': return num1 * num2;
        case '/': return num1 / num2;
        default: return 32767;
    }
}

void DeleteTree(PNode Tree) {
    if (Tree == NULL) return;
    DeleteTree(Tree->left);
    DeleteTree(Tree->right);
    delete Tree;
}

int main() {
    char s[80];
    PNode Tree;

    printf("Введите выражение > ");
    cin >> s;

    Tree = MakeTree(s, 0, strlen(s) - 1);
    printf("= %d \n", CalcTree(Tree));

    DeleteTree(Tree);
    return 0;
}