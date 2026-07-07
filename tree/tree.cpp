#include "tree.h"

void Tree::insert(PNode &root, int x) {
    if (!root) {
        root = new Node;
        root->data = x;
        root->left = nullptr;
        root->right = nullptr;
        return;
    }
    if (x < root->data) {
        insert(root->left, x);
    }
    else if (x > root->data) {
        insert(root->right, x);
    }
}

PNode Tree::search(PNode root, int x) {
    if (!root) {
        return nullptr;
    }
    if (root->data == x) {
        return root;
    }
    if (x < root->data) {
        return search(root->left, x);
    }
    else {
        return search(root->right, x);
    }
}
