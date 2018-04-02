#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "linked_list_new.h"

int insert(LIST ***l, int *start, int *head, int *free, int *data_end)
{
    LIST **ll = *l;
    if (*head == 0)    /* no rec present */
    {
        *data_end = 1;
        ll = realloc(ll, (*data_end + 1) * sizeof(LIST *));
        ll[*data_end] = calloc(1, sizeof(LIST));

        *start = 1;
        *head = 1;
        ll[*start]->next = 0;
        ll[*start]->prev = 0;
    }
    else if (*free == 0)
    {
        *data_end += 1;
        ll = realloc(ll, (*data_end + 1) * sizeof(LIST *));
        ll[*data_end] = calloc(1, sizeof(LIST));

        ll[*data_end]->next = 0;
        ll[*data_end]->prev = *head;
        ll[*head]->next = *data_end;
        *head = *data_end;
    }
    else
    {
        int new_free = ll[*free]->next;
        ll[*free]->next = 0;
        ll[*free]->prev = *head;
        ll[*head]->next = *free;
        *head = *free;
        *free = new_free;
    }

    *l = ll;
    return *head;
}

void delete_with_index(int h, LIST ***l, int *start, int *head, int *free, int *data_end)
{
    LIST **ll = *l;

    if (*head == 0)
        return;

    if (h == *start)
    {
        if (*start == *head)  /* only one element present */
        {
            *start = 1;
            *head = 0;
            *free = 0;
            *data_end = 0;
            ll = NULL;  /* have to free memory later */
        }
        else
        {
            *start = ll[*start]->next;
            ll[*start]->prev = 0;
        }
    }
    else if (h == *head)
    {
        *head = ll[*head]->prev;
        ll[*head]->next = 0;
    }
    else
    {
        int prev = ll[h]->prev;
        int next = ll[h]->next;
        ll[prev]->next = next;
        ll[next]->prev = prev;
    }

    /* add h to free list */
    
    *l = ll;
    if (!ll)
        return ;

    ll[h]->prev = 0;
    if (*free == 0)
        ll[h]->next = 0;
    else
    {
        ll[h]->next = *free;
        ll[*free]->prev = h;
    }
    *free = h;
}

void print_list(LIST **ll, int data_end)
{
    for (int start=1; start<=data_end; start++)
        printf("%d\t%d\t%d\n", start, ll[start]->next, ll[start]->prev);
    printf("\n\n");
}

void save_list_v2(LIST **ll, int data_offset, int fd, int *first_record, int *last_record, int start, int head, int free, int data_end)
{
    if (head)
        ll[head]->next = free;
    int cur_pos = lseek(fd, 0, SEEK_CUR);
    for (int i=1; i<=data_end; i++)
    {
        lseek(fd, (i*BLOCK_SIZE)+data_offset, SEEK_SET);
        write(fd, &(ll[i]->next), sizeof(ll[i]->next));
        write(fd, &(ll[i]->prev), sizeof(ll[i]->prev));
    }
    lseek(fd, cur_pos, SEEK_SET);
    if (head)
        ll[head]->next = 0;
    *first_record = start;
    *last_record = head;
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

LIST **init_list_v2(int fd, int data_offset, int *data_end,
                    int *free, int *head, int *start,
                    int *first_record, int *last_record)
{
    if (fd == -1)
    {
        *first_record = 1;
        *last_record = 0;
        *free = 0;
        *head = 0;
        *start = 1;
        *data_end = 0;
        return NULL;
    }

    LIST **ll = calloc((*data_end+1), sizeof(LIST *));
    for (int i=1; i<=*data_end; i++)
    {
        LIST *t = calloc(1, sizeof(LIST));
        ll[i] = t;
        t->next = get_next(fd, data_offset, i);
        t->prev = get_prev(fd, data_offset, i);
    }

    *start = *first_record;
    *head = *last_record;
    if (*head != 0)
    {
        *free = ll[*head]->next;
        ll[*head]->next = 0;
    }

    return ll;
}

void uninit_list(LIST **ll, int count)
{
    for (int i=0; i<count; i++)
        free(ll[i]);
    free(ll);
}

/* XXX To test the linked list impl uncomment this main()
int main(int argc, char *argv[])
{
    int fd = open("test.table", O_CREAT|O_RDWR, S_IRWXU);

    int data_end, free, head, start, first_record, last_record;
    LIST **l = init_list_v2(-1, 10, &data_end, &free, &head, &start,
                            &first_record, &last_record);

    for (int i=0; i<5; i++)
    {
        int r = insert(&l, &start, &head, &free, &data_end);
        printf("inserted: %d || ret: %d\n", i, r);
    }

    print_list(l, data_end);

    printf("free: %d\n", free);
    printf("head: %d\n", head);
    printf("start: %d\n", start);
    printf("data_end: %d\n", data_end);
    printf("=============================\n");

    for (int i=3; i<=5; i++)
    {
        delete_with_index(i, &l, &start, &head, &free, &data_end);
        print_list(l, data_end);
    printf("free: %d\n", free);
    printf("head: %d\n", head);
    printf("start: %d\n", start);
    printf("data_end: %d\n", data_end);
        printf("--------\n");
    }


    for (int i=0; i<5; i++)
    {
        int r = insert(&l, &start, &head, &free, &data_end);
        printf("inserted: %d || ret: %d\n", i, r);
    }

    print_list(l, data_end);

    printf("free: %d\n", free);
    printf("head: %d\n", head);
    printf("start: %d\n", start);
    printf("data_end: %d\n", data_end);
    printf("=============================\n");

    save_list_v2(l, 10, fd, &first_record, &last_record, start, head, free, data_end);
    close(fd);

    fd = open("test.table", O_CREAT|O_RDWR, S_IRWXU);
    l = init_list_v2(fd, 10, &data_end, &free, &head, &start,
                            &first_record, &last_record);
    printf("**********************************\n");
    print_list(l, data_end);
    printf("**********************************\n");

    for (int i=0; i<5; i++)
    {
        int r = insert(&l, &start, &head, &free, &data_end);
        printf("inserted: %d || ret: %d\n", i, r);
    }

    print_list(l, data_end);

    printf("free: %d\n", free);
    printf("head: %d\n", head);
    printf("start: %d\n", start);
    printf("data_end: %d\n", data_end);
    printf("=============================\n");

    close(fd);

    return 0;
}
*/
