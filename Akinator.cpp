#include "Akinator.h"

#define TEST printf("%d\n", __LINE__);

int main()
{   
    setlocale(LC_ALL, "Russian");

    System system = {};
    SystemCtor(&system);

    FILE *file_ptr = fopen(DATABASE_NAME, "r+");
    assert(file_ptr != nullptr);

    ReadDataBase(&system, file_ptr);
    int res = DataBaseTreeCtor(&system);
    
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

    printf("Вы желаете сохранить новую базу данных? Если да, введите Y, если нет - введите N");
    char save_or_not[2] = {};
    scanf("%1s", save_or_not);
    if (save_or_not[0] == 'Y')
    {
        WriteDataBase(&system, file_ptr);
    }
    
    fclose(file_ptr);
    return OK;
}

void SystemCtor(System *system)
{
    assert(system != nullptr);
    
    TreeCtor(&system->tree);
}

void TreeCtor(Tree *tree)
{
    assert(tree != nullptr);
    
    tree->root = CreateNode();

    tree->root->data  = "Неизвестно кто";
    tree->root->left  = nullptr;
    tree->root->right = nullptr;
}

Errors DataBaseTreeCtor(System *system)
{
    assert(system != nullptr);

    Stack_t stack = {};
    StackCtor(&stack, MAX_TREE_DEPTH);

    Node *cur_node = CreateNode();
    system->tree.root = cur_node;

    char *cur_symb = system->database.buffer;
    if (*cur_symb != '{')
    {
        _WRONG_DATABASE;
    }
    StackPush(&stack, nullptr);
    ++cur_symb;
    
    char *words_ending = nullptr;
    while(cur_symb - system->database.buffer < system->database.size)
    {   
        switch (*cur_symb)
        {    
            case '{':
                StackPush(&stack, cur_node);

                if(cur_node->left == nullptr)
                {
                    cur_node->left = CreateNode();
                    cur_node = cur_node->left;
                }
                else
                {
                    if(cur_node->right != nullptr)
                    {
                        _WRONG_DATABASE;            /*return*/  
                    }
                    cur_node->right = CreateNode();
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

    return Errors::OK;
}

Node *CreateNode()
{
    Node *node = (Node *) calloc(1, sizeof(Node));
    assert(node != nullptr);

    node->left  = nullptr;
    node->right = nullptr;
    node->data  = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
    assert(node->data != nullptr);

    return node;
}

void DeleteNode(Node *node)
{
    assert(node != nullptr);

    if (node->left != nullptr)
    {
        DeleteNode(node->left);
    }

    if (node->right != nullptr)
    {
        DeleteNode(node->right);
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
    printf("Что это?\n");

    char *new_object = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
    assert(new_object != nullptr);
    scanf("%s", new_object);
    
    return new_object;
}

char *GetDifference(const char *first_object, const char *second_object)
{
    assert(first_object  != nullptr);
    assert(second_object != nullptr);

    printf("Чем различаются %s и %s?\n", first_object, second_object);

    char *difference = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
    assert(difference != nullptr);
    scanf("%s", difference);

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

        cur->left  = CreateNode();
        cur->right = CreateNode();

        cur->left->data  = new_object;
        cur->right->data = cur->data;
        cur->data = difference;
        
        printf("left->data: %s\n", cur->left->data);
        printf("right->data: %s\n", cur->right->data);
        printf("cur->data: %s\n", cur->data);
    }
}

void AkinatorCompareObjects(System *system)
{
    assert(system != nullptr);

}

void AkinatorGetDefinition(System *system)
{
    assert(system != nullptr);
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
    
    TreeWalk(system->tree.root, output_file_ptr);

    fclose(output_file_ptr);
}

void TreeWalk(Node *node, FILE *output_file_ptr)
{
    assert(node != nullptr);
    assert(output_file_ptr != nullptr);

    if (node == nullptr)
    {
        return;
    }

    fprintf(output_file_ptr, "{");
    fprintf(output_file_ptr, "|");
    fprintf(output_file_ptr, node->data);
    fprintf(output_file_ptr, "|");
    if(node->left != nullptr)
    {
        TreeWalk(node->left,  output_file_ptr);
        TreeWalk(node->right, output_file_ptr);
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