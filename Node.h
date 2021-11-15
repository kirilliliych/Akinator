#ifndef NODE_H
#define NODE_H

struct Node
{
    Node *left  = nullptr;
    Node *right = nullptr;
    char *data = nullptr;
};

#endif