#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

int insert(LIST **ll, int *free, int *head)
{
    int pos = *free;
    if (pos == -1)
        return -1;

    *free = ll[pos]->next;
    ll[pos]->next = *head;
    *head = pos;
    ll[*head]->prev = -1;
    if (ll[*head]->next != -1)
        ll[ll[*head]->next]->prev = pos;

    return pos;
}

void delete_with_index(int h, LIST **ll, int *free, int *head)
{
    if (h == *head)
    {
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

void print_list(LIST **ll, int free, int head)
{
    while (head != -1)
    {
        printf("%d\t%d\t%d\n", head, ll[head]->next, ll[head]->prev);
        head = ll[head]->next;
    }
    printf("\n\n");
}

void save_list(LIST **ll, int *next, int *prev, int count)
{
    for (int i=0; i<count; i++)
    {
        next[i] = ll[i]->next;
        prev[i] = ll[i]->prev;
    }
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

/*
int main_test()
{
    int next[count], prev[count];
    for (int i=0; i<count; i++)
    {
        next[i] = i+1;
        prev[i] = -1;
    }
    next[count-1] = -1;

    LIST **ll = init_list(next, prev);
    int free=0, head=-1;

    for (int i=0; i<10; i++)
    {
        insert(ll, &free, &head);
        insert(ll, &free, &head);
    }

    printf("f%d h%d\n\n", free, head);

    print_list(ll, free, head);
    for (int i=0; i<10; i++)
        delete_with_index(i, ll, &free, &head);
    print_list(ll, free, head);

    return 0;
}
*/
