#ifndef __CREATETABLE_H
#define __CREATETABLE_H

#include <gmodule.h>

#include "linked_list.h"

#define TBL_NAME_SIZE 50
#define TBL_TYPE_SIZE sizeof(char)
#define MAX_RECORDS 1024

#define BLOCK_SIZE 1024

#define INT 10
#define STRING 50


typedef struct
{
    int count;
    int block_count;
    int size;
    int data_offset;
    int primary_key;
    int key_offset;
    int next[MAX_RECORDS];
    int prev[MAX_RECORDS];
    int free;
    int head;
    int start;
    char is_indexed;
    char *options[TBL_NAME_SIZE];
    char *types;
    LIST **records;
    GHashTable *index;
} METADATA;

GHashTable *table_index;

int get_next_free_offset(METADATA *metadata);
int createtable(int argc, char *argv[]);
void write_data(char *table, METADATA *data, METADATA *metadata, int pos);
void insert_table(int argc, char *argv[]);
void open_table(char *table);
void write_metadata(METADATA *metadata, char *table);
METADATA *read_metadata(char *table);
void print_metadata(METADATA *metadata);
void print_metadata_(METADATA *metadata, char *table);
void dump_table(int argc, char *argv[]);
void delete_table(int argc, char *argv[]);
void build_index(int argc, char *argv[]);
void drop_index(int argc, char *argv[]);
void quit(int argc, char *argv[]);
GHashTable *load_index(char *table, METADATA *metadata);
void select_table(int argc, char *argv[]);
void help(int argc, char *argv[]);
void get_type_size(int *len, char type);

#endif
