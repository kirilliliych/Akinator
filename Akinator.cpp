#include "Akinator.h"


#define TEST printf("%d\n", __LINE__);

int main()
{   
    setlocale(LC_ALL, "Russian");

    Prog prog = {};
    ProgCtor(&prog);

    FILE *file_ptr = fopen(DATABASE_NAME, "r+");
    if (file_ptr != nullptr)
    {
        ReadDataBase(&prog, file_ptr);
        int res = DataBaseTreeCtor(&prog);
        if (res)
        {
            return res;
        }
    }
    else
    {
        file_ptr = fopen(DATABASE_NAME, "w+t");
        assert(file_ptr != nullptr);
        
        TreeCtor(&prog.tree);
    }

    do
    {
        int mode = GetMode();

        switch (mode)
        {
            case Modes::PLAY:
                AkinatorPlayGame(&prog);
                break;
            
            case Modes::COMPARE:
                AkinatorCompareObjects(&prog);
                break;

            case Modes::GET_DEFINITION:
                AkinatorGetDefinition(&prog);
                break;
            
            case Modes::GET_DUMP:
                AkinatorDoGraph(&prog);
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
        WriteDataBase(&prog, file_ptr);
        return OK;
    }

    ProgDtor(&prog);

    fclose(file_ptr);
    return OK;
}

void ProgCtor(Prog *prog)
{
    assert(prog != nullptr);
    
    TreeCtor(&prog->tree);
}

void ProgDtor(Prog *prog)
{
    assert(prog != nullptr);

    TreeDtor(&prog->tree);

    free(prog->database.buffer);
    prog->database.size = 0;
    prog->database.buffer = nullptr;
}

void TreeCtor(Tree *tree)
{
    assert(tree != nullptr);
    
    tree->root = NodeCtor();

    tree->root->data  =  NEW_DATABASE_BEGINNING;
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

int DataBaseTreeCtor(Prog *prog)
{
    assert(prog != nullptr);

    Stack_t stack = {};
    StackCtor(&stack, MAX_TREE_DEPTH);

    Node *cur_node = NodeCtor();
    prog->tree.root = cur_node;

    char *cur_symb = prog->database.buffer;
    if (*cur_symb != '{')
    {
        _WRONG_DATABASE;
    }
    StackPush(&stack, nullptr);
    ++cur_symb;
    
    char *words_ending = nullptr;
    while((cur_symb - prog->database.buffer) < prog->database.size)
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
                    if (cur_symb - prog->database.buffer != prog->database.size - 1)
                    {
                        _WRONG_DATABASE;            /*return*/ 
                    }    

                    break;
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

void AkinatorPlayGame(Prog *prog)
{
    assert(prog != nullptr);

    Node *cur_node = prog->tree.root;
    
    int end = 0;

    Stack_t backtrace = {};
    StackCtor(&backtrace, MAX_TREE_DEPTH);

    while (!end)
    {
        assert(cur_node != nullptr);
        
        Answers answer = AskQuestionGetAnswer(cur_node);
        while (answer == UNKNOWN_ANSWER)
        {
            printf("Непонятный ответ, попробуйте заново\n");
            answer = AskQuestionGetAnswer(cur_node);
        }
        
        switch (answer)
        {
            case Answers::YES:
                if (cur_node->left == nullptr)
                {
                    end = 1;
                    VictoryBattleCry();
                }
                else
                {
                    cur_node = cur_node->left;
                }
            break;

            case Answers::NO:
                if (cur_node->right == nullptr)
                {   
                    if (backtrace.size > 0)
                    {
                        printf("Попробуем еще раз...\n");
                        cur_node = StackPop(&backtrace);
                    }
                    else
                    {
                        end = -1;
                        DirgeCry();
                    }   
                }
                else
                {
                    cur_node = cur_node->right;
                }
            break;

            case Answers::IDK:
                if (cur_node->left == nullptr)
                {
                    PrintIDK();
                    end = 1;
                }
                else
                {
                    StackPush(&backtrace, cur_node);
                    cur_node = cur_node->left;
                }
            break;

            case Answers::UNKNOWN_ANSWER:
                printf("Unknown answer\n");
            break;

            default: 
            break; 
        }
    }

    StackDtor(&backtrace);
    if (end != -1)
    {
        return;
    }

    char *new_object = GetNewObject();
    char *difference = GetDifference(cur_node->data, new_object);

    cur_node->left  = NodeCtor();
    cur_node->right = NodeCtor();

    cur_node->left->data  = new_object;
    cur_node->right->data = cur_node->data;
    cur_node->data = difference;
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

void WriteDataBase(Prog *prog, FILE *output_file_ptr)
{
    assert(prog != nullptr);
    
    rewind(output_file_ptr);
    PrintTree(prog->tree.root, output_file_ptr);
    
    fclose(output_file_ptr);
}

void PrintTree(Node *node, FILE *output_file_ptr)
{
    assert(node != nullptr);
    assert(output_file_ptr != nullptr);

    fprintf(output_file_ptr, "{");
    fprintf(output_file_ptr, "|");
    fprintf(output_file_ptr, "%s", node->data);
    fprintf(output_file_ptr, "|");

    if (node->left != nullptr)
    {
        PrintTree(node->left,  output_file_ptr);
    }
    
    if (node->right != nullptr)
    {
        PrintTree(node->right, output_file_ptr);
    }

    fprintf(output_file_ptr, "}");
}

void ReadDataBase(Prog *prog, FILE *input_file_ptr)
{
    assert(prog != nullptr);

    prog->database.size = GetFileSize(input_file_ptr);
    prog->database.buffer = (char *) calloc(prog->database.size, sizeof(char));
    prog->database.size = fread(prog->database.buffer, sizeof(char), prog->database.size, input_file_ptr);
}

size_t GetFileSize(FILE *file)
{
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    return file_size;
}

int AkinatorCompareObjects(Prog *prog)
{
    assert(prog != nullptr);

    char obj1[MAX_DATA_LENGTH] = {};
    printf("Имя первого объекта: ");
    if (GetName(prog, obj1))
    {
        return NO_OBJECT_WITH_SUCH_NAME;    
    }           
                                                                                                                  
    char obj2[MAX_DATA_LENGTH] = {};
    printf("Имя второго объекта: ");
    if (GetName(prog, obj2))
    {
        return NO_OBJECT_WITH_SUCH_NAME;    
    }

    Stack_t descr1 = {};
    Stack_t descr2 = {};
    StackCtor(&descr1, MAX_TREE_DEPTH);
    StackCtor(&descr2, MAX_TREE_DEPTH);

    TreeSearch(prog->tree.root, obj1, &descr1);
    TreeSearch(prog->tree.root, obj2, &descr2);

    PrintDifference(&descr1, &descr2);

    StackDtor(&descr1);
    StackDtor(&descr2);

    return OK;
}

int SearchForWordsInBuffer(Prog *prog, const char *words)
{
    assert(prog != nullptr);
    assert(words  != nullptr);

    size_t len = strlen(words);

    for (size_t index = 0; index < prog->database.size - len + 1; ++index)
    {
        if (strncmp(prog->database.buffer + index, words, len) == 0)
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

int AkinatorGetDefinition(Prog *prog)
{
    assert(prog != nullptr);

    printf("О ком или о чем вы хотите узнать?\n");
    char obj[MAX_DATA_LENGTH] = {};
    GetName(prog, obj);

    Stack_t stack = {};
    StackCtor(&stack, MAX_TREE_DEPTH);

    if (TreeSearch(prog->tree.root, obj, &stack) == nullptr)
    {
        return NO_OBJECT_WITH_SUCH_NAME;
    }

    PrintDefinition(&stack);

    StackDtor(&stack);

    return OK;
}

int GetName(Prog *prog, char *obj)
{
    rewind(stdin);
    fgets(obj, MAX_DATA_LENGTH, stdin);
    obj[strlen(obj) - 1] = '\0';
    if (SearchForWordsInBuffer(prog, obj))
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

void AkinatorDoGraph(Prog *prog)
{
    assert(prog != nullptr);
    assert(prog->tree.root != nullptr);

    FILE *graph_file = fopen(GRAPH_NAME, "w");
    assert(graph_file != nullptr);

    fprintf(graph_file, "digraph G{\n"
                        "   ");

    GraphNode(prog->tree.root, graph_file);

    fprintf(graph_file, "}");

    fclose(graph_file);

    char do_graph[MAX_COMMAND_LENGTH] = {};
    sprintf(do_graph, "dot -Tpng %s -o %s", GRAPH_NAME, IMG_NAME);
    system(do_graph);
}

void GraphNode(Node *node, FILE *graph_file)
{
    assert(node != nullptr);
    assert(graph_file != nullptr);

    if ((node->left != nullptr) && (node->right != nullptr))
    {
        fprintf(graph_file, "P%p[shape=record, label=\"left\\n %p | %s\\n %p | right\\n %p\"];\n    ", node, node->left, node->data, node, node->right);
    }
    else
    {
        fprintf(graph_file, "P%p[shape=record, label=\" left \\n nullptr| %s\\n%p | right\\n nullptr\"];\n    ", node, node->data, node);
    }
    
    if (node->left != nullptr)
    {
        GraphNode(node->left, graph_file);
    }

    if (node->right != nullptr)
    {
        GraphNode(node->right, graph_file);
    }

    if (node->left != nullptr)
    {
        fprintf(graph_file, "P%p->P%p[label=\"Y\"];\n", node, node->left);
    }

    if (node->right != nullptr)
    {
        fprintf(graph_file, "P%p->P%p[label=\"N\"];\n", node, node->right);
    }                                                                                       
}

void PrintIDK()
{
    printf("Вы сами не знаете, кого загадали? Ну тогда ");
    VictoryBattleCry();
}
