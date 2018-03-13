#include <stdio.h>
#include <gmodule.h>
#include <string.h>

#include "createtable.h"

extern GHashTable *table_index;

char *commands[][100] = {
                            {"create", "students", "sname", "s", "sid", "i", "gpa", "i", NULL},
                            {"print", "students", NULL},
                            {"buildindex", "students", "sname", NULL},
                            {"insert", "students", "sname", "abc", "sid", "1", "gpa", "10", NULL},
                            {"insert", "students", "sname", "abc", "sid", "2", "gpa", "11", NULL},
                            {"insert", "students", "sname", "apqr", "sid", "2", "gpa", "12", NULL},
                            {"insert", "students", "sname", "pqr", "sid", "2", "gpa", "13", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "3", "gpa", "13", NULL},
                            {"insert", "students", "sname", "ptq", "sid", "3", "gpa", "14", NULL},
                            {"insert", "students", "sname", "ptq", "sid", "4", "gpa", "15", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"print", "students", NULL},
                            {"insert", "students", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", NULL},
                            {"print", "students", NULL},
                            {"dropindex", "students", "sname", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"print", "students", NULL},
                            {"select", "students", "sname", "xyz", NULL},
                            {"delete", "students", "sname", "xyz", NULL},
                            {"print", "students", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"print", "students", NULL},
                            {"quit", NULL},
                            {NULL}
                         };

int length(char *argv[])
{
    int l = 0;
    while (*argv++)
        l++;
    return l;
}

int main ()
{
    printf("Welcome to MINIREL database system\n\n");
    table_index = NULL;

    int i=0;

    printf("------------\n");
    while (commands[i][0])
    {
        if (strcmp(commands[i][0], "create") == 0)
            createtable(length(commands[i]), commands[i]);
        else if (strcmp(commands[i][0], "print") == 0)
            dump_table(length(commands[i]), commands[i]);
        else if (strcmp(commands[i][0], "insert") == 0)
            insert_table(length(commands[i]), commands[i]);
        else if (strcmp(commands[i][0], "buildindex") == 0)
            build_index(length(commands[i]), commands[i]);
        else if (strcmp(commands[i][0], "dropindex") == 0)
            drop_index(length(commands[i]), commands[i]);
        else if (strcmp(commands[i][0], "quit") == 0)
            quit(length(commands[i]), commands[i]);
        else if (strcmp(commands[i][0], "delete") == 0)
        {
            char t[100];
            int tcode = 501;
            memcpy(t, &tcode, sizeof(int));
            char *cmd[100] = {"delete", "students", "sname", t, "xyz", NULL};
            delete_table(length(cmd), cmd);
        }
        else if (strcmp(commands[i][0], "select") == 0)
        {
            char t[100];
            int tcode = 501;
            memcpy(t, &tcode, sizeof(int));
            char *cmd[100] = {"select", "students", "sname", t, "xyz", NULL};
            select_table(length(cmd), cmd);
        }

        for (int j=0; j<length(commands[i]); j++)
            printf("%s ", commands[i][j]);
        printf("\n");
        i++;
        printf("------------\n");
    }

    printf("\n\nGoodbye from MINIREL\n");

    return 0;
}
