#ifndef __LINKED_LIST_H

typedef struct
{
    int next, prev;
} LIST;

int insert(LIST **ll, int *free, int *head);
void delete_with_index(int h, LIST **ll, int *free, int *head);
void print_list(LIST **ll, int free, int head);
LIST **init_list(int *next, int *prev, int count, int init);
void save_list(LIST **ll, int *next, int *prev, int count);

#endif
