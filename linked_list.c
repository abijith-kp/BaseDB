#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "linked_list.h"

int insert(LIST **ll, int *free, int *head, int *start)
{
    int pos = *free;
    if (pos == -1)
        return -1;

    if (*start == -1)
        *start = pos;

    *free = ll[pos]->next;
    ll[pos]->next = *head;
    *head = pos;
    ll[*head]->prev = -1;
    if (ll[*head]->next != -1)
        ll[ll[*head]->next]->prev = pos;

    return pos;
}

void delete_with_index(int h, LIST **ll, int *free, int *head, int *start)
{

    if (h < 0)
        return;
    else if (h == *start)
        *start = ll[h]->prev;

    if (h == *head)
    {
        if (ll[h]->next != -1)
            ll[ll[h]->next]->prev = -1;
        *head = ll[h]->next;
        ll[h]->next = *free;
        *free = h;
    }
    else
    {
        if (ll[h]->prev == -1)
        {
            ll[h]->next = *free;
            *free = h;
        }
        else if (ll[h]->next == -1)
        {
            ll[ll[h]->prev]->next = -1;
            ll[h]->prev = -1;
            ll[h]->next = *free;
            *free = h;
        }
        else
        {
            ll[ll[h]->prev]->next = ll[h]->next;
            ll[ll[h]->next]->prev = ll[h]->prev;
            ll[h]->prev = -1;
            ll[h]->next = *free;
            *free = h;
        }
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

void save_list_v2(int head, LIST **ll, int data_offset, int fd, int *next, int *prev, int count)
{
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    for (int i=head; i!=-1; i=ll[i]->next)
    {
        lseek(fd, (i*BLOCK_SIZE)+data_offset, SEEK_SET);
        write(fd, &(ll[i]->next), sizeof(ll[i]->next));
        write(fd, &(ll[i]->prev), sizeof(ll[i]->prev));
    }
    lseek(fd, cur_pos, SEEK_SET);

    for (int i=0; i<count; i++)
    {
        next[i] = ll[i]->next;
        prev[i] = ll[i]->prev;
    }
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
int get_next(int fd, int data_offset, int i, int *next)
{
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, (i*BLOCK_SIZE)+data_offset, SEEK_SET);
    int n = 0;
    read(fd, &n, sizeof(int));
    lseek(fd, cur_pos, SEEK_SET);
    printf("next: %d %d\n", next[i], n);
    return next[i];
    return n;
}

int get_prev(int fd, int data_offset, int i, int *prev)
{
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, (i*BLOCK_SIZE)+data_offset+sizeof(int), SEEK_SET);
    int n = 0;
    read(fd, &n, sizeof(int));
    lseek(fd, cur_pos, SEEK_SET);
    printf("prev: %d %d\n", prev[i], n);
    return prev[i];
    return n;
}

LIST **init_list_v2(int fd, int data_offset, int data_end, int *next, int *prev, int count, int init)
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
            t->next = next[i];
            t->prev = prev[i];
        }
        else
        {
            if (i > data_end)
                continue;
            t->next = get_next(fd, data_offset, i, next);
            t->prev = get_prev(fd, data_offset, i, prev);
        }
    }

    if (init)
        ll[count-1]->next = -1;

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
