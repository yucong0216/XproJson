#include "xproJson.h"
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include "xparser.h"
#include "xobject.h"
#include "xlimits.h"
#include "xmem.h"


XJson* xpro_parse(const char* jsonstr) {
    return main_parser(jsonstr);
}

XJson* xpro_parseFile(const char* fileName) {
    FILE* f = fopen(fileName, "r+");
    assert(f);
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

void xpro_free(XJson* json) {
    if (json == NULL) return;
    free_json(json);
    json = NULL;
}

char* xpro_print(XJson* json) {
    return print_json(json);
}

/* get size */

int xpro_getArraySize(XJson* array) {
    if (array->t != XPRO_TARRAY) return 0;
    return array->nchild;
}

int xpro_getObjectSize(XJson* object) {
    if (object->t != XPRO_TOBJECT) return 0;
    return object->nchild;
}

/* create */

XJson* xpro_create_null() {
    return create_null();
}

XJson* xpro_create_bool(int b) {
    return create_bool(b);
}

XJson* xpro_create_int(xpro_Integer i) {
    return create_integer(i);  /* TODO */
}

XJson* xpro_create_double(xpro_Number n) {
    return create_double(n);
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

/* add */

void xpro_addItemToArray(XJson* parent, XJson* item) {
    addItem(parent, item);
}

void xpro_addItemToObject(XJson* parent, XJson* item, const char* key) {
    item->key = realloc_(char, NULL, strlen(key) + 1);
    strcpy(item->key, key);
    addItem(parent, item);
}

/* detach */

XJson* xpro_detachItemInArray(XJson* array, int index) {
    XJson* value = xpro_getItemInArray(array, index);
    if (value) {
        if (value == array->stack) array->stack = value->next;
        if (value == array->top) array->top = value->prev;
        if (value->prev) value->prev->next = value->next;
        if (value->next) value->next->prev = value->prev;
        value->prev = value->next = NULL;
    }
    return value;
}

XJson* xpro_detachItemInOjbect(XJson* object, const char* key) {
    XJson* value = xpro_getItemInObject(object, key);
    if (value) {
        if (value == object->stack) object->stack = value->next;
        if (value == object->top) object->top = value->prev;
        if (value->prev) value->prev->next = value->next;
        if (value->next) value->next->prev = value->prev;
        value->prev = value->next = NULL;
    }
    return value;
}

void xpro_deleteItemInArray(XJson* array, int index) {
    XJson* value = xpro_detachItemInArray(array, index);
    if (value) xpro_free(value);
}

void xpro_deleteItemInOjbect(XJson* object, const char* key) {
    XJson* value = xpro_detachItemInOjbect(object, key);
    if (value) xpro_free(value);
}


/* get */

XJson* xpro_getItemInArray(XJson* array, int index) {
    XJson* value = array->stack;
    while (value && index-- > 0) value = value->next;
    return value;
}

XJson* xpro_getItemInObject(XJson* object, const char* key) {
    XJson* value = object->stack;
    while (value > 0) {
        if (value->key && strcmp(value->key, key) == 0) break;
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

void xpro_dump(const char* fileName, const char* json) {
    FILE* f = fopen(fileName, "w+");
    if (!f) return;
    fwrite(json, strlen(json), 1, f);
    fclose(f);
}

void xpro_setErrFunc(ERR_FUNC func) {
    setErrFunc(func);
}


long xprotime()
{
    struct timeb t;
    ftime(&t);
    return 1000 * t.time + t.millitm;

    /*
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
   */
}