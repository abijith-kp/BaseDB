#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "fes.h"
#include "utils.h"
#include "createtable.h"

GHashTable *table_index = NULL;

void init_table_index()
{
    if (!table_index)
        table_index = g_hash_table_new(g_str_hash, g_str_equal);
}

void add_table_index(char *table, METADATA *metadata)
{
    init_table_index();
    g_hash_table_insert(table_index, table, metadata);
}

void *get_table_index(char *table)
{
    init_table_index();
    return g_hash_table_lookup(table_index, table);
}

int get_block_count(int size)
{
    int count = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE)
        count += 1;
    return count;
}

int check_input(char *t, char type, int len)
{
    if (type == 's')
        return 1;
    else if (type == 'i')
    {
        for (int i=0; i<len; i++)
        {
            if (t[i] == 0)
                break;
            if (!isdigit(t[i]))
                return 0;
        }
        return 1;
    }
    else if (type == 'f')
    {
        char *tmp;
        float val = strtod(t, &tmp);
        if (val == 0)
        {
            if (t == tmp)
                return 0;
        }
        return 1;
    }

    return 0;
}

int operation(char *data, int operator, char *value, char type)
{
    int s = 0;

    if (type == 'i')
        s = atoi(data) - atoi(value);
    else if (type == 'f')
    {
        if (strtod(data, NULL) > strtod(value, NULL))
            s = 1;
        else if (strtod(data, NULL) == strtod(value, NULL))
            s = 0;
        else
            s = -1;
    }
    else if (type == 's')
        s = strncmp(data, value, STRING);

    if (EQOP == operator)
        return s == 0;
    else if (GEOP == operator)
        return (s >= 0);
    else if (GTOP == operator)
        return s > 0;
    else if (LEOP == operator)
        return (s <= 0);
    else if (LTOP == operator)
        return s < 0;
    else if (NOTEQOP == operator)
        return s != 0;

    return 0;
}

void get_type_size(int *len, char type)
{
    if (type == 'i')
        *len = INT;
    else if (type == 'f')
        *len = FLOAT;
    else if (type == 's')
        *len = STRING;
}

void get_pos_len_type(METADATA *metadata, char *attr, int *pos, int *len, char *type, int *index)
{
    for (int j=0; j<metadata->count; j++)
    {
        int l = 0;
        get_type_size(&l, metadata->types[j]);
        if (0 == strcmp(metadata->options[j], attr))
        {
            if (len)
                *len = l;
            if (type)
                *type = metadata->types[j];
            if (index)
                *index = j;
            break;
        }
        if (pos)
            *pos += l;
    }
}

void print_header(METADATA *metadata)
{
    /* Printing the header */
    for (int i=0; i<metadata->count; i++)
        printf("%s\t", metadata->options[i]);
    printf("\n");
}

char *read_cell(char *data, int fd, char type)
{
    int len = 0;
    get_type_size(&len, type);

    if (!data)
        data = calloc(len, sizeof(char));
    memset(data, 0, len);
    read(fd, data, len);
    return data;
}

void print_row(METADATA *metadata, int fd, int index)
{
    METADATA *data = get_row_as_data_struct(metadata, fd, index);

    for (int j=0; j<metadata->count; j++)
        printf("%s\t", data->options[j]);
    printf("\n");
}

int is_created(char *table)
{
    struct stat st;
    if (stat(table, &st) == -1)
        return 0;
    return 1;
}

void delete_table(int argc, char *argv[])
{
    char *table = argv[1];
    char *attr = argv[2];
    int operator = *(int *)argv[3];
    char *value = argv[4];

    if (!is_created(table))
    {
        printf("Table not created!!\n");
        return;
    }

    METADATA *metadata = read_metadata(table);

    int fd = open(table, O_RDONLY);
    
    char type = 'i';
    int index = 0;
    get_pos_len_type(metadata, attr, NULL, NULL, &type, &index);

    int i = get_first_index(metadata);
    while (i != -1)
    {
        int _i = get_next_index(metadata, i);

        METADATA *data = get_row_as_data_struct(metadata, fd, i);

        if (operation(data->options[index], operator, value, type))
        {
            delete_index(metadata, i);

            if (metadata->is_indexed)
                g_hash_table_remove(metadata->index, data->options[metadata->primary_key]);
        }
        free(data);
        i = _i;
    }

    close(fd);
    write_metadata(metadata, table);
}

void dump_table(int argc, char *argv[])
{
    char *table = argv[1];

    if (!is_created(table))
    {
        printf("Table not created!!\n");
        return;
    }

    METADATA *metadata = read_metadata(table);
    print_header(metadata);
    
    int fd = open(table, O_RDONLY);
    for (int i=get_first_index(metadata); i!=-1; i=get_next_index(metadata, i))
    {
        print_row(metadata, fd, i);
    }
    close(fd);
}

void write_data(char *table, METADATA *data, METADATA *metadata, int pos)
{
    int fd = open(table, O_WRONLY);

    if (pos < 0)
        pos = get_new_index(metadata);

    if (pos == -1)
    {
        printf("Records are full\n");
        return;
    }

    write_row_as_data_struct(metadata, data, fd, pos);

    if (metadata->is_indexed)
        g_hash_table_insert(metadata->index,
                            data->options[metadata->primary_key],
                            GINT_TO_POINTER(pos));

    close(fd);
    write_metadata(metadata, table);
}

void insert_table(int argc, char *argv[])
{
    char *table = argv[1];

    if (!is_created(table))
    {
        printf("Table not created!!\n");
        return;
    }

    METADATA *data = calloc(1, sizeof(METADATA));
    METADATA *metadata = read_metadata(table);

    char *types = calloc((argc-2)/2, sizeof(char));
    data->types = types;

    for (int i=0; i<metadata->count; i++)
        data->options[i] = NULL;

    for (int i=2; i<argc; i+=2)
    {
        char *t = calloc(TBL_NAME_SIZE, sizeof(char));
        for (int j=0; j<metadata->count; j++)
        {
            if (strcmp(metadata->options[j], argv[i]) != 0)
                continue;

            int len = STRING;
            get_type_size(&len, metadata->types[j]);
            strncpy(t, argv[i+1], len);
            data->types[j] = metadata->types[j];
            data->options[j] = t;
            if (!check_input(data->options[j], data->types[j], len))
            {
                printf("Colomn value is incorrect. Record not inserted.\n");
                free(data);
                return;
            }
        }
    }

    if ((metadata->is_indexed) && (NULL == data->options[metadata->primary_key]))
    {
        printf("Primary key colomn cannot be null.\n");
        free(data);
        return;
    }

    int index = -1;
    
    if (metadata->is_indexed)
    {
        char *key = data->options[metadata->primary_key];
        gpointer val;
        gboolean ret = g_hash_table_lookup_extended(metadata->index, key, NULL, &val);
        if (ret)
        {
            printf("Key already existing. Overwriting the data..\n");
            index = GPOINTER_TO_INT(val);
        }
    }

    write_data(table, data, metadata, index);
    add_table_index(table, metadata);
    free(data);
}

void write_metadata(METADATA *metadata, char *table)
{
    int fd = open(table, O_CREAT|O_WRONLY, S_IRWXU);
    int data_offset = 0;

    write(fd, &(data_offset), sizeof(data_offset));
    write(fd, &(metadata->count), sizeof(metadata->count));
    write(fd, &(metadata->size), sizeof(metadata->size));

    write(fd, &(metadata->is_indexed), sizeof(metadata->is_indexed));
    write(fd, &(metadata->primary_key), sizeof(metadata->primary_key));
    write(fd, &(metadata->key_offset), sizeof(metadata->key_offset));

    for (int i=0; i<metadata->count; i++)
    {
        write(fd, metadata->options[i], TBL_NAME_SIZE);
        write(fd, &(metadata->types[i]), TBL_TYPE_SIZE);
    }

    save_list(metadata->records, metadata->next, metadata->prev,
              MAX_RECORDS);
    write(fd, &(metadata->free), sizeof(metadata->free));
    write(fd, &(metadata->head), sizeof(metadata->head));
    write(fd, &(metadata->start), sizeof(metadata->start));
    write(fd, &(metadata->next), sizeof(metadata->next));
    write(fd, &(metadata->prev), sizeof(metadata->prev));

    data_offset = lseek(fd, 0, SEEK_CUR);
    data_offset += 10;
    lseek(fd, 0, SEEK_SET);
    write(fd, &(data_offset), sizeof(data_offset));
    metadata->data_offset = data_offset;
    close(fd);
}

METADATA *read_metadata(char *table)
{

    METADATA *metadata = get_table_index(table);

    if (metadata)
        return metadata;
    metadata = calloc(1, sizeof(METADATA));

    int fd = open(table, O_RDONLY);

    read(fd, &(metadata->data_offset), sizeof(metadata->data_offset));
    read(fd, &(metadata->count), sizeof(metadata->count));
    read(fd, &(metadata->size), sizeof(metadata->size));
    read(fd, &(metadata->is_indexed), sizeof(metadata->is_indexed));
    read(fd, &(metadata->primary_key), sizeof(metadata->primary_key));
    read(fd, &(metadata->key_offset), sizeof(metadata->key_offset));

    char *types = calloc(metadata->count, sizeof(char));
    for (int i=0; i<metadata->count; i++)
    {
        char *t = calloc(TBL_NAME_SIZE, sizeof(char));
        read(fd, t, TBL_NAME_SIZE);
        read(fd, &(types[i]), TBL_TYPE_SIZE);
        metadata->options[i] = t;
    }

    read(fd, &(metadata->free), sizeof(metadata->free));
    read(fd, &(metadata->head), sizeof(metadata->head));
    read(fd, &(metadata->start), sizeof(metadata->start));
    read(fd, &(metadata->next), sizeof(metadata->next));
    read(fd, &(metadata->prev), sizeof(metadata->prev));

    metadata->records = init_list(metadata->next, metadata->prev,
                                     MAX_RECORDS, 0);

    metadata->types = types;
    metadata->block_count = get_block_count(metadata->size);
    close(fd);

    if (metadata->is_indexed)
        metadata->index = load_index(table, metadata);
    add_table_index(table, metadata);
    return metadata;
}

int createtable(int argc, char *argv[])
{
    char *table = argv[1];
    METADATA *metadata;

    if (is_created(table))
    {
        printf("Table already created!!\n");
        return -1;
    }

    metadata = calloc(1, sizeof(METADATA));
    metadata->is_indexed = 0;
    metadata->primary_key = 0;
    metadata->size = 0;
    metadata->key_offset = 0;
    metadata->count = (argc-2)/2;
    metadata->free = 0;
    metadata->head = -1;
    metadata->start = -1;

    metadata->records = init_list(metadata->next, metadata->prev,
                                     MAX_RECORDS, 1);

    char *types = calloc(metadata->count, sizeof(char));
    int s = 0;

    for (int i=2; i<argc; i+=2)
    {
        char *t = calloc(TBL_NAME_SIZE, sizeof(char));
        strncpy(t, argv[i], TBL_NAME_SIZE);
        metadata->options[(i-2)/2] = t;
        types[(i-2)/2] = argv[i+1][0];
        get_type_size(&s, argv[i+1][0]);
        metadata->size += s;
    }

    metadata->types = types;
    metadata->block_count = get_block_count(metadata->size);
    write_metadata(metadata, table);
    add_table_index(table, metadata);

    return 0;
}

void print_metadata(METADATA *metadata)
{
    printf("count: %d\n", metadata->count);
    printf("size: %d\n", metadata->size);
    printf("offset: %d\n", metadata->data_offset);
    printf("is_indexed: %d\n", metadata->is_indexed);
    printf("Name\tType\n");

    for (int i=0; i<metadata->count; i++)
        printf("%s\t%c\n", metadata->options[i], metadata->types[i]);
}

void save_metadata(void *key, void *value, void *user_data)
{
    printf("Closing all the caches for %s...\n", key);
    METADATA *t = value;
    uninit_list(t->records, MAX_RECORDS);
}

void quit(int argc, char *argv[])
{
    if (table_index)
        g_hash_table_foreach(table_index, save_metadata, NULL);

    printf("Quiting..\n");
}

GHashTable *load_index(char *table, METADATA *metadata)
{
    GHashTable *index = g_hash_table_new(g_str_hash, g_str_equal);
    
    int fd = open(table, O_RDONLY);
    int i = get_first_index(metadata);
    while (i != -1)
    {
        METADATA *data = get_row_as_data_struct(metadata, fd, i);

        int _i = get_next_index(metadata, i);
        char *key = data->options[metadata->primary_key];
        gpointer val;
        gboolean ret = g_hash_table_lookup_extended(index, key, NULL, &val);
        if ((ret) || (0 == strlen(key)))
            delete_index(metadata, GPOINTER_TO_INT(val));

        g_hash_table_insert(index, key, GINT_TO_POINTER(i));
        i = _i;
    }

    close(fd);
    return index;
}

void build_index(int argc, char *argv[])
{
    char *table = argv[1];
    char *attr = argv[2];

    if (!is_created(table))
    {
        printf("Table not created!!\n");
        return;
    }

    METADATA *metadata = read_metadata(table);

    metadata->is_indexed = 1;
    metadata->primary_key = 0;
    metadata->key_offset = 0;
    get_pos_len_type(metadata, attr, &(metadata->key_offset), NULL, NULL, &(metadata->primary_key));

    metadata->index = load_index(table, metadata);
    write_metadata(metadata, table);
}

void drop_index(int argc, char *argv[])
{
    char *table = argv[1];

    if (!is_created(table))
    {
        printf("Table not created!!\n");
        return;
    }

    METADATA *metadata = read_metadata(table);

    metadata->is_indexed = 0;
    metadata->primary_key = 0;
    metadata->key_offset = 0;
    
    metadata->index = NULL;
    write_metadata(metadata, table);
}

void select_table(int argc, char *argv[])
{
    char *table = argv[1];
    char *attr = argv[2];
    int operator = *(int *)argv[3];
    char *value = argv[4];

    if (!is_created(table))
    {
        printf("table not created!!\n");
        return;
    }

    METADATA *metadata = read_metadata(table);

    int fd = open(table, O_RDONLY);
    
    char type = 'i';
    int index = 0;
    get_pos_len_type(metadata, attr, NULL, NULL, &type, &index);

    print_header(metadata);
    for (int i=get_first_index(metadata); i!=-1; i=get_next_index(metadata, i))
    {
        METADATA *data = get_row_as_data_struct(metadata, fd, i);

        if (operation(data->options[index], operator, value, type))
            print_row(metadata, fd, i);

        free(data);
    }

    close(fd);
}

void help(int argc, char *argv[])
{
    for (int i=0; i<argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    char *manual[100] = {
                            "create RELATION_NAME ( ATTR_NAME = FORMAT [ , ATTR_NAME = FORMAT ]* );",
                            "\tCreate a new table with RELATION_NAME and attributes with given FORMAT.\n",
                            "print RELATION_NAME;",
                            "\tDumps all data in the table with name RELATION_NAME\n"
                            "buildindex for RELATION_NAME on ATTR_NAME;",
                            "dropindex for RELATION_NAME [ on ATTR_NAME ];",
                            "select from RELATION_NAME where ( ATTR_NAME OP VALUE );",
                            "insert into RELATION_NAME ( ATTR_NAME = VALUE [ , ATTR_NAME = VALUE ]* );",
                            "delete from RELATION_NAME where ( ATTR_NAME OP VALUE );",
                            "quit;",
                            "help;",
                            "\tTo print this help",
                            NULL
                        };

    printf("\nHELP:\n");
    printf("\tRELATION_NAME: Table name. It would be same as the table file name.\n");
    printf("\tATTR_NAME: Attribute name is the colomn name. String value.\n");
    printf("\tVALUE: String or Integer.\n\n");
    printf("\tFORMAT: For string data use \"s\" and for integer data use \"i\".\n\n");
    for (int i=0; manual[i]; i++)
        printf("%s\n", manual[i]);
    printf("\n");
}
