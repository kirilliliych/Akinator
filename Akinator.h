#ifndef AKINATOR_H
#define AKINATOR_H

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Stack.h"
#include "logs.h"

#define _WRONG_DATABASE                                             \
    printf("Неправильная база данных");                             \
    return WRONG_DATABASE;                          

const int  MAX_DATA_LENGTH    = 50;
const int  MAX_ANSWER_LENGTH  = 4;
const int  MAX_TREE_DEPTH     = 50;
const int  MAX_COMMAND_LENGTH = 200; 
const int  CODE_PAGE          = 1251;

const char *DATABASE_NAME = "Database.txt";
const char *GRAPH_NAME    = "AkinatorGraph.dot";
const char *IMG_NAME      = "AkinatorImg.png";
char NEW_DATABASE_BEGINNING[15] = "Неизвестно кто";

struct Tree
{
    Node *root = nullptr; 
};

struct Database
{
    int size = 0;
    char *buffer = nullptr;
};

struct Prog
{
    Tree tree = {};
    Database database = {};
};

enum Modes
{
    PLAY = 1,
    COMPARE,
    GET_DEFINITION,
    GET_DUMP
};

enum Answers
{
    YES,
    NO,
    IDK,
    UNKNOWN_ANSWER
};

enum Errors
{
    OK = 0,
    WRONG_DATABASE,
    TOO_BIG_DATA,
    NO_OBJECT_WITH_SUCH_NAME
};

void ProgCtor(Prog *prog);

void ProgDtor(Prog *prog);

void TreeCtor(Tree *tree);

void TreeDtor(Tree *tree);

Node *NodeCtor();

void NodeDtor(Node *node);

int GetMode();

char *GetNewObject();

char *GetDifference(const char *first_object, const char *second_object);

Answers AskQuestionGetAnswer(Node *cur_node);

void AkinatorPlayGame(Prog *prog);

int AkinatorCompareObjects(Prog *prog);

int AkinatorGetDefinition(Prog *prog);

void AkinatorDoGraph(Prog *prog);

int GoAgain();

void VictoryBattleCry();

void DirgeCry();

void WriteDataBase(Prog *prog, FILE *output_file_ptr);

void PrintTree(Node *node, FILE *output_file_ptr);

void ReadDataBase(Prog *prog, FILE *input_file_ptr);

size_t GetFileSize(FILE *file);

int DataBaseTreeCtor(Prog *prog);

int SearchForWordsInBuffer(Prog *prog, const char *words);

Node *TreeSearch(Node *node, const char *obj, Stack_t *stack);

void PrintDifference(Stack_t *stack1, Stack_t *stack_2);

void PrintOneStr(Node *parent, Node *child);

int GetName(Prog *prog, char *obj);

void PrintDefinition(Stack_t *stack);

void GraphNode(Node *node, FILE *graph_file);

void PrintIDK();

#endif