#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <gmodule.h>

#include "utils.h"

int get_first_index(METADATA *metadata)
{
    int index = metadata->start;
    for (int i=0; i<metadata->block_count-1; i++)
        index = metadata->records[index]->prev;
    return index;
}

int get_next_index(METADATA *metadata, int cur_index)
{
    int next_index = cur_index;
    for (int i=0; i<metadata->block_count; i++)
    {
        if (next_index == -1)
            break;
        next_index = metadata->records[next_index]->prev;
    }

    return next_index;
}

int get_new_index(METADATA *metadata)
{
    int next_index = metadata->head;
    for (int i=0; i<metadata->block_count; i++)
    {
        next_index = insert(metadata->records, &(metadata->free),
                            &(metadata->head), &(metadata->start));
        if (next_index == -1)
            break;
    }

    return next_index;
}

void delete_index(METADATA *metadata, int index)
{
    for (int i=0; i<metadata->block_count; i++)
    {
        int next_index = metadata->records[index]->next;
        delete_with_index(index, metadata->records, &(metadata->free),
                          &(metadata->head), &(metadata->start));
        index = next_index;
    }
}

char *read_row(METADATA *metadata, int fd, int index)
{
    int offset = metadata->data_offset + (index * metadata->size);
    lseek(fd, offset, SEEK_SET);

    char *buffer = calloc(metadata->size, sizeof(char));
    int t = 0;
    for (int i=0; i<metadata->block_count-1; i++)
    {
        read(fd, buffer+t, BLOCK_SIZE);
        t += BLOCK_SIZE;
        index = metadata->records[index]->next;
        offset = metadata->data_offset + (index * metadata->size);
        lseek(fd, offset, SEEK_SET);
    }
    
    read(fd, buffer+t, (metadata->size - t));
    return buffer;
}

void write_row(METADATA *metadata, char *buffer, int fd, int index)
{
    int offset = metadata->data_offset + (index * metadata->size);
    lseek(fd, offset, SEEK_SET);

    int t = 0;
    for (int i=0; i<metadata->block_count-1; i++)
    {
        write(fd, buffer+t, BLOCK_SIZE);
        t += BLOCK_SIZE;
        index = metadata->records[index]->next;
        offset = metadata->data_offset + (index * metadata->size);
        lseek(fd, offset, SEEK_SET);
    }
    
    write(fd, buffer+t, (metadata->size - t));
}

METADATA *get_row_as_data_struct(METADATA *metadata, int fd, int index)
{
    char *buffer = read_row(metadata, fd, index);
    METADATA *data = calloc(1, sizeof(METADATA));

    char *types = calloc(metadata->count, sizeof(char));
    data->types = types;
    int t = 0;
    for (int i=0; i<metadata->count; i++)
    {
        int len = 0;
        get_type_size(&len, metadata->types[i]);
        char *buf = calloc(len, sizeof(char));
        strncpy(buf, buffer+t, len);
        data->options[i] = buf;
        data->types[i] = metadata->types[i];
        t += len;
    }
    free(buffer);

    return data;
}

void write_row_as_data_struct(METADATA *metadata, METADATA *data, int fd, int index)
{
    char *buffer = calloc(metadata->size, sizeof(char));
    int t = 0;

    for (int i=0; i<metadata->count; i++)
    {
        int len = 0;
        get_type_size(&len, metadata->types[i]);
        if (data->options[i])
            strncpy(buffer+t, data->options[i], len);
        t += len;
    }

    write_row(metadata, buffer, fd, index);
    free(buffer);
}
