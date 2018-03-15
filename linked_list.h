#ifndef __LINKED_LIST_H

typedef struct
{
    int next, prev, start;
} LIST;

int insert(LIST **ll, int *free, int *head, int *start);
void delete_with_index(int h, LIST **ll, int *free, int *head, int *start);
void print_list(LIST **ll, int free, int head, int start);
LIST **init_list(int *next, int *prev, int count, int init);
void save_list(LIST **ll, int *next, int *prev, int count);
void uninit_list(LIST **ll, int count);

#endif
