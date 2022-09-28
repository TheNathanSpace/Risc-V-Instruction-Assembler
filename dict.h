/*
 *
 *     File: dict.h
 *     Description: library for making dictionaries
 *     License: MIT
 *
 */

#ifndef DICT_H
#define DICT_H

#include <stdlib.h>

typedef struct dict_item {
    void* key;
    void* value;
} dict_item;

typedef struct dict {
    dict_item* items;
    unsigned int items_length;
} dict;

dict new_dict()
{
    dict dict_l;
    dict_l.items = malloc(0);
    dict_l.items_length = 0;
    return dict_l;
}

void* getvalue(void* key, dict dict_p)
{
    void* value = NULL;
    for(int i = 0; i < dict_p.items_length; ++i)
    {
        if(dict_p.items[i].key == key)
        {
            value = dict_p.items[i].value;
            break;
        }
    }
    return value;
}

void setkey(void* key, void* value, dict* dict_p)
{
    if(getvalue(key, *dict_p) == NULL)
    {
        dict_p->items_length += 1;
        dict_p->items = realloc(dict_p->items,dict_p->items_length * sizeof(dict_item));
        dict_p->items[dict_p->items_length-1].key = key;
        dict_p->items[dict_p->items_length-1].value = value;
    }
    else
    {
        int ikey = 0;
        for(int i = 0; i < dict_p->items_length; ++i)
        {
            if(dict_p->items[i].key == key)
            {
                ikey = i;
                break;
            }
        }
        dict_p->items[ikey].value = value;
    }
}

void deletekey(void* key, dict* dict_p)
{
    if(getvalue(key,*dict_p) != NULL)
    {
        int ikey = 0;
        for(int i = 0; i < dict_p->items_length; ++i)
        {
            if(dict_p->items[i].key == key)
            {
                ikey = i;
                break;
            }
        }
        if((ikey+1) < dict_p->items_length)
        {
            for(int i = (ikey+1); i < dict_p->items_length; ++i)
            {
                dict_p->items[i-1].key = dict_p->items[i].key;
                dict_p->items[i-1].value = dict_p->items[i].value;
            }
            dict_p->items = realloc(dict_p->items, (dict_p->items_length-1) * sizeof(dict_item));
            dict_p->items_length -= 1;
        }
        else
        {
            dict_p->items[ikey].key = NULL;
            dict_p->items[ikey].value = NULL;
            dict_p->items = realloc(dict_p->items, (dict_p->items_length-1) * sizeof(dict_item));
            dict_p->items_length -= 1;
        }
    }
}

void free_dict(dict* dict_p)
{
    free(dict_p->items);
}
#endif
