#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <gmodule.h>

#include "fes.h"
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

int operation(char *data, int operator, char *value, char type)
{
    int s = 0;

    if (type == 'i')
        s = atoi(data) - atoi(value);
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
    int offset = metadata->data_offset + (index * metadata->size);
    lseek(fd, offset, SEEK_SET);

    for (int j=0; j<metadata->count; j++)
    {
        char *tmp = read_cell(NULL, fd, metadata->types[j]);
        printf("%s\t", tmp);
    }
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
    
    int pos=0;
    int len=0;
    char type = 'i';
    get_pos_len_type(metadata, attr, &pos, &len, &type, NULL);

    int primary_key_len = 0;
    get_type_size(&primary_key_len, metadata->types[metadata->primary_key]);
    char *primary_key = calloc(primary_key_len, sizeof(char));

    for (int i=0; i<MAX_RECORDS; i++)
    {
        if (0 == metadata->records[i])
            continue;

        int offset = metadata->data_offset + (i * metadata->size) + pos;
        lseek(fd, offset, SEEK_SET);
        char *data = calloc(len, sizeof(char));
        read(fd, data, len);
        if (operation(data, operator, value, type))
        {
            metadata->records[i] = 0;

            if (metadata->is_indexed)
            {
                offset = metadata->data_offset + (i * metadata->size) +
                         metadata->key_offset;
                lseek(fd, offset, SEEK_SET);
                read(fd, primary_key, primary_key_len);
                g_hash_table_remove(metadata->index, primary_key);
            }
        }
        free(data);
    }

    free(primary_key);
    close(fd);
    write_metadata(metadata, table);
}

int get_next_free_offset(METADATA *metadata)
{
    for (int i=0; i<MAX_RECORDS; i++)
    {
        if (!(metadata->records[i]))
            return i * metadata->size;
    }

    return -1;
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
    for (int i=0; i<MAX_RECORDS; i++)
    {
        if (0 != metadata->records[i])
            print_row(metadata, fd, i);
    }
    close(fd);
}

void write_data(char *table, METADATA *data, METADATA *metadata, int pos)
{
    int fd = open(table, O_WRONLY);
    int offset = 0;
    
    if (pos >= 0)
        offset = (pos * metadata->size);
    else
        offset = get_next_free_offset(metadata);

    if (offset == -1)
    {
        printf("Records are full\n");
        return;
    }

    offset += metadata->data_offset;
    lseek(fd, offset, SEEK_SET);
    for (int i=0; i<metadata->count; i++)
    {
        if (NULL == data->options[i])
        {
            int len = 0;
            if (metadata->types[i] == 'i')
                len = INT;
            else if (metadata->types[i] == 's')
                len = STRING;
            void *t = calloc(len, sizeof(char));
            write(fd, t, len);
            free(t);
        }
        else
        {
            int t = 0;
            get_type_size(&t, data->types[i]);
            write(fd, data->options[i], t);
        }
    }

    int index = (offset - metadata->data_offset)/(metadata->size);
    metadata->records[index] = 1;

    if (metadata->is_indexed)
        g_hash_table_insert(metadata->index,
                            data->options[metadata->primary_key],
                            GINT_TO_POINTER(index));

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

    write(fd, &(metadata->records), MAX_RECORDS);

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
    else
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

    read(fd, &(metadata->records), MAX_RECORDS);
    metadata->types = types;
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
    memset(metadata->records, 0, MAX_RECORDS);
    metadata->is_indexed = 0;
    metadata->primary_key = 0;
    metadata->size = 0;
    metadata->key_offset = 0;
    metadata->count = (argc-2)/2;

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
    printf("Closing all the caches...\n");
    // write_metadata(value, key);
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
    for (int i=0; i<MAX_RECORDS; i++)
    {
        if (!(metadata->records[i]))
            continue;
        
        int offset = metadata->data_offset +
                     (i * metadata->size) +
                     metadata->key_offset;
        lseek(fd, offset, SEEK_SET);
        char *key = read_cell(NULL, fd, metadata->types[metadata->primary_key]);

        gpointer val;
        gboolean ret = g_hash_table_lookup_extended(index, key, NULL, &val);
        if ((ret) || (0 == strlen(key)))
            metadata->records[GPOINTER_TO_INT(val)] = 0;

        g_hash_table_insert(index, key, GINT_TO_POINTER(i));
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
    
    int pos=0;
    int len=0;
    char type = 'i';
    get_pos_len_type(metadata, attr, &pos, &len, &type, NULL);

    print_header(metadata);
    for (int i=0; i<MAX_RECORDS; i++)
    {
        if (0 == metadata->records[i])
            continue;

        int offset = metadata->data_offset + (i * metadata->size) + pos;
        lseek(fd, offset, SEEK_SET);
        char *data = calloc(len, sizeof(char));
        read(fd, data, len);

        if (operation(data, operator, value, type))
            print_row(metadata, fd, i);

        free(data);
    }

    close(fd);
    write_metadata(metadata, table);

}

void help(int argc, char *argv[])
{
    for (int i=0; i<argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    /*
        To be implemented:
        createdb DBNAME;
        destroydb DBNAME;
        opendb DBNAME;
        closedb;
        destroy RELATION_NAME;
        load RELATION_NAME from FILENAME;
        project into RELATION_NAME from RELATION_NAME ( ATTR_NAME [ , ATTR_NAME ]* );
        join into RELATION_NAME ( RELATION_NAME . ATTR_NAME, RELATION_NAME . ATTR_NAME );
     */

    char *manual[100] = {
                            "create RELATION_NAME ( ATTR_NAME = FORMAT [ , ATTR_NAME = FORMAT ]* );",
                            "\tCreate a new table with RELATION_NAME and attributes with given FORMAT.",
                            "print RELATION_NAME;",
                            "\tDumps all data in the table with name RELATION_NAME"
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
