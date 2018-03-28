#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "linked_list_new.h"

int insert(LIST **ll, int *first_record, int *last_record, int *data_end)
{
    if (*first_record == 0)
    {
        *data_end = 1;
        *first_record = 1;
        *last_record = 1;
        *ll = realloc(ll, (*data_end + 1)*sizeof(LIST *));
        ll[*data_end] = calloc(1, sizeof(LIST));
        ll[*last_record]->prev = 0;
        ll[*last_record]->next = 0;
    }
    else if (*last_record == *data_end)
    {
        *data_end += 1;
        *ll = realloc(ll, (*data_end + 1)*sizeof(LIST *));
        ll[*data_end] = calloc(1, sizeof(LIST));
        ll[*last_record]->next = *data_end;
        ll[*data_end]->next = 0;
        ll[*data_end]->prev = *last_record;
        *last_record = *data_end;
    }
    else
        *last_record = ll[*last_record]->next;

    return *last_record;
}

void delete_with_index(int h, LIST **ll, int *first_record, int *last_record, int *data_end)
{
    if (h <= 0)
        return;

    if ((ll[h]->prev == 0) && (ll[h]->next == 0))
    {
        *first_record = 0;
        *last_record = 1;
    }
    else if (ll[h]->prev == 0)
    {
        *first_record = ll[h]->next;
        ll[*first_record]->prev = 0;
        int lr = ll[*last_record]->next;
        ll[*last_record]->next = h;
        ll[h]->prev = *last_record;
        ll[h]->next = lr;
        printf("lr: %d\n", lr);
        if (lr != 0)
            ll[lr]->prev = h;
    }
    else
    {
        int n = ll[h]->next;
        int p = ll[h]->prev;
        ll[n]->prev = p;
        ll[p]->next = n;
        int lr = ll[*last_record]->next;
        ll[*last_record]->next = h;
        ll[h]->prev = *last_record;
        ll[h]->next = lr;
        ll[lr]->prev = h;
    }
}

void print_list(LIST **ll, int free, int head, int start)
{
    while (start != -1)
    {
        printf("%d\t%d\t%d\n", start, ll[start]->next, ll[start]->prev);
        start = ll[start]->prev;
    }
    printf("\n\n");
}

void save_list_v2(LIST **ll, int data_offset, int fd, int first_record, int last_record, int data_end)
{
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    for (int i=1; i<=data_end; i++)
    {
        lseek(fd, (i*BLOCK_SIZE)+data_offset, SEEK_SET);
        write(fd, &(ll[i]->next), sizeof(ll[i]->next));
        write(fd, &(ll[i]->prev), sizeof(ll[i]->prev));
    }
    lseek(fd, cur_pos, SEEK_SET);
}

void save_list(LIST **ll, int *next, int *prev, int count)
{
    for (int i=0; i<count; i++)
    {
        next[i] = ll[i]->next;
        prev[i] = ll[i]->prev;
    }
}

/* |next|prev| */
int get_next(int fd, int data_offset, int i)
{
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, (i*BLOCK_SIZE)+data_offset, SEEK_SET);
    int n = 0;
    read(fd, &n, sizeof(int));
    lseek(fd, cur_pos, SEEK_SET);
    return n;
}

int get_prev(int fd, int data_offset, int i)
{
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, (i*BLOCK_SIZE)+data_offset+sizeof(int), SEEK_SET);
    int n = 0;
    read(fd, &n, sizeof(int));
    lseek(fd, cur_pos, SEEK_SET);
    return n;
}

LIST **init_list_v2(int fd, int data_offset, int data_end)
{
    if (fd == -1)
        return NULL;

    LIST **ll = calloc(data_end, sizeof(LIST *));
    for (int i=1; i<=data_end; i++)
    {
        LIST *t = calloc(1, sizeof(LIST));
        ll[i] = t;
        t->next = get_next(fd, data_offset, i);
        t->prev = get_prev(fd, data_offset, i);
    }

    return ll;
}

LIST **init_list(int *next, int *prev, int count, int init)
{
    LIST **ll = calloc(count, sizeof(LIST *));
    for (int i=0; i<count; i++)
    {
        LIST *t = calloc(1, sizeof(LIST));
        ll[i] = t;
        if (init)
        {
            next[i] = i+1;
            prev[i] = -1;
        }
        t->next = next[i];
        t->prev = prev[i];
    }

    if (init)
        ll[count-1]->next = -1;

    return ll;
}

void uninit_list(LIST **ll, int count)
{
    for (int i=0; i<count; i++)
        free(ll[i]);
    free(ll);
}
