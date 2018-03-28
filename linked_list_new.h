#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H

typedef struct
{
    int next, prev;
} LIST;

int insert(LIST **ll, int *free, int *head, int *start);
void delete_with_index(int h, LIST **ll, int *free, int *head, int *start);
void print_list(LIST **ll, int free, int head, int start);
LIST **init_list(int *next, int *prev, int count, int init);
void save_list(LIST **ll, int *next, int *prev, int count);
void uninit_list(LIST **ll, int count);
void save_list_v2(LIST **ll, int data_offset, int fd, int first_record, int last_record, int data_end);
LIST **init_list_v2(int fd, int data_offset, int data_end);

#endif
