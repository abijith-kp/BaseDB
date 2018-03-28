#ifndef __CREATETABLE_H
#define __CREATETABLE_H

#include <gmodule.h>

#include "linked_list_new.h"

#define TBL_NAME_SIZE 50
#define TBL_TYPE_SIZE sizeof(char)
#define MAX_RECORDS 1024
#define COLUMN_NAME_SIZE 32

#define BLOCK_SIZE 1024

#define INT 10
#define FLOAT 20
#define STRING 50

/* For project */

#define FILE_HEADER "CS"

typedef struct
{
    char col_name[COLUMN_NAME_SIZE];
    char data_type; // this contains both type (1st 3bits) and constraint(2nd 3bits)
    char index; // present or not ( may be extended as column ordering)
    char size; // length of the data
    char frac_part_size; // nonzero if the data type is real, zero otherwise
} Column;

/**************/


typedef struct
{
    /* For project */

    char header[2];
    char time_stamp[8];
    char table_name[32];
    int active_records;
    int num_cols;
    int data_end;
    int first_record;
    int last_record;
    int total_record;
    Column column_list[255];

    /**************/

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
