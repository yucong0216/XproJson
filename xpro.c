//
//  xpro.c
//
//  Created by yucong on 17/2/15.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#include "xpro.h"
#include "xparser.h"
#include "xobject.h"
#include "xlimits.h"
#include "xmem.h"
#include <stdlib.h>

XJson* xpro_parser(const char* jsonstr) {
    return main_parser(jsonstr);
}

XJson* xpro_parserFile(const char* fileName) {
    FILE* f = fopen(fileName, "r+");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    char* pBuf = (char*)malloc(len + 1);
    rewind(f);
    fread(pBuf, 1, len, f);
    pBuf[len] = 0;
    fclose(f);
    XJson* json = main_parser(pBuf);
    free((void*)pBuf);
    return json;
}

static void free_value(XJson* value) {
    if (value == NULL) {
        return;
    }
    while (value) {
        free_value(value->child);
        XJson* prev = value->prev;
        if (value->key)
            xMem_free((void*)value->key);
        if (value->v.s.s && value->v.s.len > 0)
            xMem_free((void*)value->v.s.s);
        value->prev = value->next = value->child = value->head = NULL;
        value->key = NULL;
        xMem_free((void*)value);
        value = prev;
    }
}

void xpro_free(XJson* json) {
    if (json == NULL)
        return;
    free_value(json);
    json = NULL;
    xMem_free(json);
    json = NULL;
}

char* xpro_print(XJson* json) {
    return print_json(json);
}

//  get size

int32_t xpro_getArraySize(XJson* array) {
    if (array->t != XPRO_TARRAY)
        return 0;
    return array->child_size;
}

int32_t xpro_getObjectSize(XJson* object) {
    if (object->t != XPRO_TOBJECT)
        return 0;
    return object->child_size;
}

XJson* xpro_create_null() {
    return create_null();
}

XJson* xpro_create_bool(int b) {
    return create_bool(b);
}

XJson* xpro_create_numeral(xpro_Number n) {
    return create_numeral(n);
}

XJson* xpro_create_string(const char* str) {
    return create_string(str);
}

XJson* xpro_create_array() {
    return create_array();
}

XJson* xpro_create_object() {
    return create_object();
}

void xpro_addItem(XJson* parent, XJson* item) {
    addItem(parent, item);
}

XJson* xpro_detachItemInArray(XJson* array, int index) {
    XJson* value = xpro_getItemInArray(array, index);
    
    if (value) {
        if (value == array->head) array->head = value->next;
        if (value == array->child) array->child = value->prev;
        XJson* prev = value->prev;
        XJson* next = value->next;
        if (prev) prev->next = next;
        if (next) next->prev = prev;
        value->prev = value->next = NULL;
    }
    
    return value;
}

XJson* xpro_detachItemInOjbect(XJson* object, const char* key) {
    XJson* value = xpro_getItemInObject(object, key);
    
    if (value) {
        if (value == object->head) object->head = value->next;
        if (value == object->child) object->child = value->prev;
        XJson* prev = value->prev;
        XJson* next = value->next;
        if (prev) prev->next = next;
        if (next) next->prev = prev;
        value->prev = value->next = NULL;
    }
    
    return value;
}

XJson* xpro_getItemInArray(XJson* array, int index) {
    XJson* value = array->head;
    while (value && --index > 0) value = value->next;
    return value;
}

XJson* xpro_getItemInObject(XJson* object, const char* key) {
    XJson* value = object->head;
    while (value > 0) {
        if (value->key && strcmp(value->key, key) == 0)
            break;
        value = value->next;
    }
    return value;
}

void xpro_minify(char* json) {
    char *into=json;
    while (*json)
    {
        if (*json==' ') json++;
        else if (*json=='\t') json++;	/* Whitespace characters. */
        else if (*json=='\r') json++;
        else if (*json=='\n') json++;
        else if (*json=='/' && json[1]=='/')  while (*json && *json!='\n') json++;	/* double-slash comments, to end of line. */
        else if (*json=='/' && json[1]=='*') {while (*json && !(*json=='*' && json[1]=='/')) json++;json+=2;}	/* multiline comments. */
        else if (*json=='\"'){*into++=*json++;while (*json && *json!='\"'){if (*json=='\\') *into++=*json++;*into++=*json++;}*into++=*json++;} /* string literals, which are \" sensitive. */
        else *into++=*json++;			/* All other characters. */
    }
    *into=0;
}

void xpro_saveFile(const char* fileName, const char* json) {
    FILE* f = fopen(fileName, "w+");
    if (!f) return;
    fwrite(json, strlen(json), 1, f);
    fclose(f);
}

#include <time.h>
long xprotime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}