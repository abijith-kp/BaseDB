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
    return metadata->first_record;
}

int get_next_index(METADATA *metadata, int cur_index)
{
    return metadata->records[cur_index]->next;
}

int get_new_index(METADATA *metadata)
{
    return insert(metadata->records, &(metadata->first_record),
                  &(metadata->last_record), &(metadata->data_end));
}

void delete_index(METADATA *metadata, int index)
{
    delete_with_index(index, metadata->records,
                &(metadata->first_record), &(metadata->last_record),
                &(metadata->data_end));
}

char *read_row(METADATA *metadata, int fd, int index)
{
    int offset = metadata->data_offset + (index * BLOCK_SIZE);
    offset += (2 * sizeof(int));

    lseek(fd, offset, SEEK_SET);

    char *buffer = calloc(metadata->size, sizeof(char));
    int t = 0;
    for (int i=0; i<metadata->block_count-1; i++)
    {
        read(fd, buffer+t, BLOCK_SIZE);
        t += BLOCK_SIZE;
        index = metadata->records[index]->next;
        offset = metadata->data_offset + (index * BLOCK_SIZE);
        lseek(fd, offset, SEEK_SET);
    }
    
    read(fd, buffer+t, (metadata->size - t));
    return buffer;
}

void write_row(METADATA *metadata, char *buffer, int fd, int index)
{
    int offset = metadata->data_offset + (index * BLOCK_SIZE);
    offset += (2 * sizeof(int));

    lseek(fd, offset, SEEK_SET);

    int t = 0;
    for (int i=0; i<metadata->block_count-1; i++)
    {
        write(fd, buffer+t, BLOCK_SIZE);
        t += BLOCK_SIZE;
        index = metadata->records[index]->next;
        offset = metadata->data_offset + (index * BLOCK_SIZE);
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
