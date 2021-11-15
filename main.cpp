#include "Akinator.h"

const char *DATABASE_NAME = "Database.txt";

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