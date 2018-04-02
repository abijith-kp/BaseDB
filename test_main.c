#include <stdio.h>
#include <gmodule.h>
#include <string.h>

#include "createtable.h"

extern GHashTable *table_index;

char *commands[][100] = {
                            {"create", "students", "sname", "s", "sid", "i", "gpa", "f", NULL},
                            {"print", "students", NULL},
                            {"buildindex", "students", "sname", NULL},
                            {"insert", "students", "sname", "abc", "sid", "fff", "gpa", "10", NULL},
                            {"insert", "students", "sname", "abc", "sid", "1", "gpa", "10", NULL},
                            {"insert", "students", "sname", "abc", "sid", "2", "gpa", "11.345", NULL},
                            {"insert", "students", "sname", "apqr", "sid", "2", "gpa", "12", NULL},
                            {"insert", "students", "sname", "pqr", "sid", "2", "gpa", "13", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "3", "gpa", "13", NULL},
                            {"insert", "students", "sname", "ptq", "sid", "3", "gpa", "14", NULL},
                            {"insert", "students", "sname", "ptq", "sid", "4", "gpa", "15", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14.213", NULL},
                            {"print", "students", NULL},
                            {"insert", "students", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", NULL},
                            {"print", "students", NULL},
                            {"dropindex", "students", "sname", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "6", "gpa", "15", NULL},
                            {"print", "students", NULL},
                            {"select", "students", "sname", "=", "xyz", NULL},
                            {"buildindex", "students", "sname", NULL},
                            {"select", "students", "gpa", "=", "14", NULL},
                            {"delete", "students", "sname", "xyz", NULL},
                            {"print", "students", NULL},
                            {"insert", "students", "sname", "xyz", "sid", "5", "gpa", "14", NULL},
                            {"insert", "students", "sname", "abc", "sid", "2", "gpa", "11.1", NULL},
                            {"insert", "students", "sname", "qtc", "sid", "2", "gpa", "11.9", NULL},
                            {"select", "students", "gpa", "<=", "11.5", NULL},

                            {"create", "employee", "empid", "i", "empname", "s", "count", "i", NULL},
                            {"insert", "employee", "empid", "2", "empname", "a", "count", "11", NULL},
                            {"insert", "employee", "empid", "2", "empname", "b", "count", "11", NULL},
                            {"insert", "employee", "empid", "3", "empname", "c", "count", "12", NULL},
                            {"insert", "employee", "empid", "1", "empname", "d", "count", "11", NULL},
                            {"print", "employee", NULL},
                            {"select", "employee", "empid", "=", "2", NULL},
                            {"select", "employee", "empid", "<=", "2", NULL},
                            {"select", "employee", "empid", ">=", "2", NULL},
                            {"select", "employee", "empid", "<", "2", NULL},
                            {"select", "employee", "empid", ">", "2", NULL},
                            {"select", "employee", "empid", "<>", "2", NULL},

                            {"print", "employee", NULL},
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
            int tcode = 501;
            if (strcmp(commands[i][3], "=") == 0)
                tcode = 501;
            else if (strcmp(commands[i][3], "<=") == 0)
                tcode = 504;
            else if (strcmp(commands[i][3], "<") == 0)
                tcode = 506;
            else if (strcmp(commands[i][3], ">=") == 0)
                tcode = 502;
            else if (strcmp(commands[i][3], ">") == 0)
                tcode = 503;
            else if (strcmp(commands[i][3], "<>") == 0)
                tcode = 505;

            char t[100];
            memcpy(t, &tcode, sizeof(int));
            char *cmd[100] = {commands[i][0], commands[i][1], commands[i][2], t, commands[i][4], NULL};
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
