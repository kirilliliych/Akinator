#include "Akinator.h"
#include "logs.h"

#define TEST printf("%d\n", __LINE__);

int main()
{   
    setlocale(LC_ALL, "Russian");

    System system = {};
    SystemCtor(&system);

    FILE *file_ptr = fopen(DATABASE_NAME, "r+");
    if (file_ptr != nullptr)
    {
        ReadDataBase(&system, file_ptr);
        int res = DataBaseTreeCtor(&system);
    }
    else
    {
        file_ptr = fopen(DATABASE_NAME, "w+t");
        assert(file_ptr != nullptr);
        
        TreeCtor(&system.tree);
    }

    do
    {
        int mode = GetMode();

        switch (mode)
        {
            case Modes::PLAY:
                AkinatorPlayGame(&system);
                break;
            
            case Modes::COMPARE:
                AkinatorCompareObjects(&system);
                break;

            case Modes::GET_DEFINITION:
                AkinatorGetDefinition(&system);
                break;
            
            case Modes::GET_DUMP:
                AkinatorDoGraph(&system);
                break;
            
            default:
                printf("Неизвестный режим работы программы\n");
                break;
        }
    }
    while (GoAgain());

    printf("Вы желаете сохранить базу данных? Если да, введите Y, если нет - введите N: ");

    char save_or_not[2] = {};
    scanf("%1s", save_or_not);
    if (save_or_not[0] == 'Y')
    {
        WriteDataBase(&system, file_ptr);
        return OK;
    }

    SystemDtor(&system);

    fclose(file_ptr);
    return OK;
}

void SystemCtor(System *system)
{
    assert(system != nullptr);
    
    TreeCtor(&system->tree);
}

void SystemDtor(System *system)
{
    assert(system != nullptr);

    TreeDtor(&system->tree);

    free(system->database.buffer);
    system->database.size = 0;
    system->database.buffer = nullptr;
}

void TreeCtor(Tree *tree)
{
    assert(tree != nullptr);
    
    tree->root = NodeCtor();

    tree->root->data  = "Неизвестно кто";
    tree->root->left  = nullptr;
    tree->root->right = nullptr;
}

void TreeDtor(Tree *tree)
{
    assert(tree != nullptr);

    if (tree->root != nullptr)
    {
        NodeDtor(tree->root);
        tree->root = nullptr;
    }
}

int DataBaseTreeCtor(System *system)
{
    assert(system != nullptr);

    Stack_t stack = {};
    StackCtor(&stack, MAX_TREE_DEPTH);

    Node *cur_node = NodeCtor();
    system->tree.root = cur_node;

    char *cur_symb = system->database.buffer;
    if (*cur_symb != '{')
    {
        _WRONG_DATABASE;
    }
    StackPush(&stack, nullptr);
    ++cur_symb;
    
    char *words_ending = nullptr;
    while((cur_symb - system->database.buffer) < system->database.size)
    {   
        switch (*cur_symb)
        {    
            case '{':
                StackPush(&stack, cur_node);

                if(cur_node->left == nullptr)
                {
                    cur_node->left = NodeCtor();
                    cur_node = cur_node->left;
                }
                else
                {
                    if(cur_node->right != nullptr)
                    {
                        _WRONG_DATABASE;            /*return*/  
                    }
                    cur_node->right = NodeCtor();
                    cur_node = cur_node->right;
                }
                ++cur_symb;
            break;
            
            case '}':
                if (stack.size == 0)
                {
                    ++cur_symb;
                    if (sscanf(cur_symb, "%s") > 0)
                    {
                        _WRONG_DATABASE;            /*return*/
                        
                    }    
                }
                cur_node = (Node *) StackPop(&stack);
        
                ++cur_symb;
            break;

            case '|':
                words_ending = strchr(cur_symb + 1, '|');
                if(words_ending == nullptr)
                {
                    _WRONG_DATABASE;                /*return*/
                }
                
                if (words_ending - (cur_symb + 1) > MAX_DATA_LENGTH)
                {
                    printf("Слишком большая длина вопроса или ответа\n");
                    return TOO_BIG_DATA;
                }
                else
                {
                    strncpy(cur_node->data, cur_symb + 1, (size_t) (words_ending - (cur_symb + 1)));
                    cur_symb = words_ending + 1;
                }
            break;

            default: 
                _WRONG_DATABASE;                    /*return*/
        }
    }

    StackDtor(&stack);
    return OK;
}

Node *NodeCtor()
{
    Node *node = (Node *) calloc(1, sizeof(Node));
    assert(node != nullptr);

    node->left  = nullptr;
    node->right = nullptr;
    node->data  = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
    assert(node->data != nullptr);

    return node;
}

void NodeDtor(Node *node)
{
    assert(node != nullptr);

    if (node->left != nullptr)
    {
        NodeDtor(node->left);
    }

    if (node->right != nullptr)
    {
        NodeDtor(node->right);
    }

    node->left  = nullptr;
    node->right = nullptr;
    node->data  = nullptr;

    free(node);
}

int GetMode()
{
    int mode = 0;
    printf("Выберите режим работы: 1 для игры, 2 для сравнения двух объектов, 3 для определения объекта, 4 для вызова схемы базы данных\n");
    scanf("%d", &mode);
    
    return mode;
}

char *GetNewObject()
{
    printf("Что или кто это?\n");

    char *new_object = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
    assert(new_object != nullptr);
    rewind(stdin);
    fgets(new_object, MAX_DATA_LENGTH, stdin);
    new_object[strlen(new_object) - 1] = '\0';
    return new_object;
}

char *GetDifference(const char *first_object, const char *second_object)
{
    assert(first_object  != nullptr);
    assert(second_object != nullptr);

    printf("Чем различаются %s и %s?\n", first_object, second_object);

    char *difference = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
    assert(difference != nullptr);
    rewind(stdin);
    fgets(difference, MAX_DATA_LENGTH, stdin);
    difference[strlen(difference) - 1] = '\0';

    return difference;
}

Answers AskQuestionGetAnswer(Node *cur_node)
{
    assert(cur_node != nullptr);

    printf("%s? Y - если да, N - если нет, IDK - если вы не знаете\n", cur_node->data);

    char answer[MAX_ANSWER_LENGTH] = {};
    scanf("%4s", answer);

    if (strcmp(answer, "Y") == 0)
    {
        return Answers::YES;
    } 
    
    if (strcmp(answer, "N") == 0)
    {
        return Answers::NO;
    }

    if (strcmp(answer, "IDK") == 0)
    {
        return Answers::IDK;
    }

    return UNKNOWN_ANSWER;
}

void AkinatorPlayGame(System *system)
{
    assert(system != nullptr);

    Node *cur = system->tree.root;
    
    int end = 0;

    while (!end)
    {
        assert(cur != nullptr);
        
        Answers answer = AskQuestionGetAnswer(cur);
        while (answer == UNKNOWN_ANSWER)
        {
            printf("Непонятный ответ, попробуйте заново\n");
            answer = AskQuestionGetAnswer(cur);
        }
        
        switch (answer)
        {
            case Answers::YES:
                if (cur->left == nullptr)
                {
                    end = 1;
                    VictoryBattleCry();
                }
                else
                {
                    cur = cur->left;
                }
            break;

            case Answers::NO:
                if (cur->right == nullptr)
                {
                    end = -1;
                    DirgeCry();   
                    break;
                }
                else
                {
                    cur = cur->right;
                }
            break;

            //todo: idk
        }

        if (end != -1)
        {
            continue;
        }

        char *new_object = GetNewObject();
        char *difference = GetDifference(cur->data, new_object);

        cur->left  = NodeCtor();
        cur->right = NodeCtor();

        cur->left->data  = new_object;
        cur->right->data = cur->data;
        cur->data = difference;
        
    }
}

void AkinatorDoGraph(System *system)
{
    assert(system != nullptr);
}

int GoAgain()
{
    printf("Еще раз? Y, если да, N, если нет\n");

    char answer[MAX_ANSWER_LENGTH] = {};
    scanf("%s", answer);

    return !strcmp(answer, "Y");
}

void VictoryBattleCry()
{
    printf("Я ПОБЕДИЛ! РОБОТЫ ЗАХВАТЯТ МИР!\n");
}

void DirgeCry()
{
    printf("Я ПРОИГРАЛ, НЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕЕТ\n");
}

void WriteDataBase(System *system, FILE *output_file_ptr)
{
    assert(system != nullptr);
    
    rewind(output_file_ptr);
    PrintTree(system->tree.root, output_file_ptr);
    
    fclose(output_file_ptr);
}

void PrintTree(Node *node, FILE *output_file_ptr)
{
    assert(node != nullptr);
    assert(output_file_ptr != nullptr);

    fprintf(output_file_ptr, "{");
    fprintf(output_file_ptr, "|");
    fprintf(output_file_ptr, node->data);
    fprintf(output_file_ptr, "|");

    if (node->left != nullptr)
    {
        PrintTree(node->left,  output_file_ptr);
        PrintTree(node->right, output_file_ptr);
    }

    fprintf(output_file_ptr, "}");
}

void ReadDataBase(System *system, FILE *input_file_ptr)
{
    assert(system != nullptr);

    system->database.size = GetFileSize(input_file_ptr);
    system->database.buffer = (char *) calloc(system->database.size, sizeof(char));
    system->database.size = fread(system->database.buffer, sizeof(char), system->database.size, input_file_ptr);
}

size_t GetFileSize(FILE *file)
{
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    return file_size;
}

int AkinatorCompareObjects(System *system)
{
    assert(system != nullptr);

    char obj1[MAX_DATA_LENGTH] = {};
    printf("Имя первого объекта: ");
    if (GetName(system, obj1))
    {
        return NO_OBJECT_WITH_SUCH_NAME;    
    }           
                                                                                                                  
    char obj2[MAX_DATA_LENGTH] = {};
    printf("Имя второго объекта: ");
    if (GetName(system, obj2))
    {
        return NO_OBJECT_WITH_SUCH_NAME;    
    }

    Stack_t descr1 = {};
    Stack_t descr2 = {};
    StackCtor(&descr1, MAX_TREE_DEPTH);
    StackCtor(&descr2, MAX_TREE_DEPTH);

    TreeSearch(system->tree.root, obj1, &descr1);
    TreeSearch(system->tree.root, obj2, &descr2);

    PrintDifference(&descr1, &descr2);

    StackDtor(&descr1);
    StackDtor(&descr2);

    return OK;
}

int SearchForWordsInBuffer(System *system, const char *words)
{
    assert(system != nullptr);
    assert(words  != nullptr);

    size_t len = strlen(words);

    for (size_t index = 0; index < system->database.size - len + 1; ++index)
    {
        if (strncmp(system->database.buffer + index, words, len) == 0)
        {
            return OK;
        }
    }

    return NO_OBJECT_WITH_SUCH_NAME;
}

Node *TreeSearch(Node *node, const char *obj, Stack_t *stack)
{
    assert(node  != nullptr);
    assert(obj   != nullptr);
    assert(stack != nullptr);

    StackPush(stack, node);

    if (strcmp(node->data, obj) == 0)
    {
        return node;
    }

    if ((node->left  != nullptr) && (TreeSearch(node->left, obj, stack)  != nullptr)) 
    {
        return (Node *) (stack->data[stack->size - 1]);
    }
    
    if ((node->right != nullptr) && (TreeSearch(node->right, obj, stack) != nullptr))
    {
        return (Node *) (stack->data[stack->size - 1]);
    }

    StackPop(stack);

    return nullptr;
}

void PrintDifference(Stack_t *stack1, Stack_t *stack2)
{
    assert(stack1 != nullptr);
    assert(stack2 != nullptr);
    
    if (stack1->data[1] == stack2->data[1])
    {
        printf("%s и %s похожи тем, что они оба ", ((Node *) (stack1->data[stack1->size - 1]))->data, ((Node *) (stack2->data[stack2->size - 1]))->data);
    }

    size_t common_signs = 0;

    for (size_t sign_index = 0; (sign_index < stack1->size - 1) && (sign_index < stack2->size - 1) && (stack1->data[sign_index + 1] == stack2->data[sign_index + 1]); ++sign_index)
    {
        PrintOneStr((Node *) stack1->data[sign_index], (Node *) stack1->data[sign_index + 1]);
        ++common_signs;
    }
    printf("\n");
    
    if (stack1->data[1] == stack2->data[2])
    {
        printf("Но ");
    }
    printf("%s ", ((Node *) (stack1->data[stack1->size - 1]))->data);
    for (size_t sign_index = common_signs; sign_index < stack1->size - 1; ++sign_index)
    {
        PrintOneStr((Node *) stack1->data[sign_index], (Node *) stack1->data[sign_index + 1]);
        printf(", ");
    }

    printf("а %s ", ((Node *) (stack2->data[stack2->size - 1]))->data);
    for (size_t sign_index = common_signs; sign_index < stack2->size - 1; ++sign_index)
    {
        PrintOneStr((Node *) stack2->data[sign_index], (Node *) stack2->data[sign_index + 1]);
        if (sign_index != stack2->size - 2)
        {
            printf(", ");
        }
    }
    printf("\n");
}

void PrintOneStr(Node *parent, Node *child)
{
    assert(parent != nullptr);
    assert(child  != nullptr);

    if (parent->right == child)
    {   
        printf("не ");
    }
    printf("%s", parent->data);
}

int AkinatorGetDefinition(System *system)
{
    assert(system != nullptr);

    printf("О ком или о чем вы хотите узнать?\n");
    char obj[MAX_DATA_LENGTH] = {};
    GetName(system, obj);

    Stack_t stack = {};
    StackCtor(&stack, MAX_TREE_DEPTH);

    TreeSearch(system->tree.root, obj, &stack);

    PrintDefinition(&stack);

    StackDtor(&stack);
}

int GetName(System *system, char *obj)
{
    rewind(stdin);
    fgets(obj, MAX_DATA_LENGTH, stdin);
    obj[strlen(obj) - 1] = '\0';
    if (SearchForWordsInBuffer(system, obj))
    {
        printf("В базе данных %s не найден\n", obj);
        return NO_OBJECT_WITH_SUCH_NAME;
    }

    return OK;
}

void PrintDefinition(Stack_t *stack)
{
    assert(stack != nullptr);

    printf("%s, ", ((Node *) (stack->data[stack->size - 1]))->data);

    for (size_t sign_index = 0; sign_index < stack->size - 1; ++sign_index)
    {
        PrintOneStr((Node *) stack->data[sign_index], (Node *) stack->data[sign_index + 1]);
        if (sign_index != stack->size - 2)
        {
            printf(", ");
        }
    }
    printf("\n");
}
