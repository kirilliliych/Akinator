#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Stack.h"

#define _WRONG_DATABASE                                 \
    printf("Неправильная база");                        \
    printf("Line: %d", __LINE__);                       \
    return WRONG_DATABASE;                          

const int  MAX_DATA_LENGTH   = 50;
const int  MAX_ANSWER_LENGTH = 4;
const int  MAX_TREE_DEPTH    = 50;

const char *DATABASE_NAME = "Database.txt";

struct Node
{
    Node *left  = nullptr;
    Node *right = nullptr;
    char *data = nullptr;
};

struct Tree
{
    Node *root = nullptr; 
};

struct Database
{
    size_t size = 0;
    char *buffer = nullptr;
};

struct System
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

void SystemCtor(System *system);

void SystemDtor(System *system);

void TreeCtor(Tree *tree);

void TreeDtor(Tree *tree);

Node *NodeCtor();

void NodeDtor(Node *node);

int GetMode();

char *GetNewObject();

char *GetDifference(const char *first_object, const char *second_object);

Answers AskQuestionGetAnswer(Node *cur_node);

void AkinatorPlayGame(System *system);

int AkinatorCompareObjects(System *system);

int AkinatorGetDefinition(System *system);

void AkinatorDoGraph(System *system);

int GoAgain();

void VictoryBattleCry();

void DirgeCry();

void WriteDataBase(System *system, FILE *output_file_ptr);

void PrintTree(Node *node, FILE *output_file_ptr);

void ReadDataBase(System *system, FILE *input_file_ptr);

size_t GetFileSize(FILE *file);

int DataBaseTreeCtor(System *system);

int SearchForWordsInBuffer(System *system, const char *words);

Node *TreeSearch(Node *node, const char *obj, Stack_t *stack);

void PrintDifference(Stack_t *stack1, Stack_t *stack_2);

void PrintOneStr(Node *parent, Node *child);

int GetName(System *system, char *obj);

void PrintDefinition(Stack_t *stack);



