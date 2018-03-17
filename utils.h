#ifndef __UTILS_H
#define __UTILS_H

#include "createtable.h"

int get_first_index(METADATA *metadata);
int get_next_index(METADATA *metadata, int cur_index);
void write_row(METADATA *metadata, char *buffer, int fd, int index);
char *read_row(METADATA *metadata, int fd, int index);
void write_row_as_data_struct(METADATA *metadata, METADATA *data, int fd, int index);
METADATA *get_row_as_data_struct(METADATA *metadata, int fd, int index);
int get_new_index(METADATA *metadata);
void delete_index(METADATA *metadata, int index);

#endif
