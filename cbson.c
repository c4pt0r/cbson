#include "cbson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
hex_dump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
    if (desc != NULL)
        printf ("%s:\n", desc);
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf ("  %s\n", buff);
            printf ("  %04x ", i);
        }
        printf (" %02x", pc[i]);
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }
    printf ("  %s\n", buff);
}

static inline void
bson_reserved(bson_obj_t* b, size_t sz) {
    if (b->size + sz <= b->cap)
        return;

    if (b->cap == 0) b->cap = 1;
    do {
        b->cap *= 2;
    } while (b->cap <= b->size + sz);

    if (b->bytes == NULL) {
        b->bytes = malloc(b->cap);
        memset(b->bytes, 0, b->cap);
    } else {
        b->bytes = realloc(b->bytes, b->cap);
    }
}

static inline size_t
bson_reserved_length(bson_obj_t* b) {
    size_t sz = b->size;
    bson_reserved(b, 4);
    b->size += 4;
    return sz;
}

static inline void
bson_write_byte(bson_obj_t* b, uint8_t byte) {
    bson_reserved(b, 1);
    b->bytes[b->size++] = byte;
}

static inline void
bson_write_uint32(bson_obj_t* b, uint32_t i) {
    bson_write_byte(b, i & 0xff);
    bson_write_byte(b, (i >> 8) & 0xff);
    bson_write_byte(b, (i >> 16) & 0xff);
    bson_write_byte(b, (i >> 24) & 0xff);
}

static inline void
bson_write_uint32_with_offset(bson_obj_t* b, uint32_t i, size_t offset) {
    bson_reserved(b, 4);
    b->bytes[offset++] = i & 0xff;
    b->bytes[offset++] = (i >> 8) & 0xff;
    b->bytes[offset++] = (i >> 16) & 0xff;
    b->bytes[offset++] = (i >> 24) & 0xff;
}

static inline void
bson_write_string(bson_obj_t* b, const char* s, size_t sz) {
    bson_reserved(b, sz + 1);
    memcpy(b->bytes + b->size, s, sz);
    b->bytes[b->size + sz] = '\0';
    b->size += sz + 1;
}

void
bson_append_string(bson_obj_t* b, const char* k, const char* val) {
    int length_offset = 0;
    bson_write_byte(b, CBSON_TYPE_STRING);
    bson_write_string(b, k, strlen(k));
    length_offset = bson_reserved_length(b);
    bson_write_string(b, val, strlen(val));
    bson_write_uint32_with_offset(b, strlen(val) + 1, length_offset);
}

static inline int32_t
read_int32(bson_iter_t* it) {
    const uint8_t* b = it->ptr;
    uint32_t v = b[0] | b[1]<<8 | b[2]<<16 | b[3]<<24;
    it->ptr += 4;
    it->size -= 4;
    return (int32_t)v;
}

static inline int64_t
read_int64(bson_iter_t* it) {
    const uint8_t * b = it->ptr;
    uint32_t lo = b[0] | b[1]<<8 | b[2]<<16 | b[3]<<24;
    uint32_t hi = b[4] | b[5]<<8 | b[6]<<16 | b[7]<<24;
    uint64_t v = (uint64_t)lo | (uint64_t)hi<<32;
    it->ptr += 8;
    it->size -= 8;
    return (int64_t)v;
}

static inline uint8_t
read_byte(bson_iter_t* it) {
    const uint8_t* b = it->ptr;
    it->ptr += 1;
    it->size -= 1;
    return *b;
}

const char*
read_key(bson_iter_t* it, bson_type* type) {
    *type = read_byte(it);
    char* p = (char*)it->ptr;
    while(*(it->ptr++))
        it->size--;
    return p;
}

const char*
read_string_val(bson_iter_t* it) {
    int32_t sz = read_int32(it);
    const char* p = (const char*)it->ptr;
    it->ptr += sz;
    it->size += sz;
    return p;
}

const char*
bson_read_string(const uint8_t* doc, const char* key) {
    const uint32_t* data = (const uint32_t*)doc;
    bson_iter_t it = {doc, *data};
    bson_type type;
    while(it.size > 0) {
        if (strcmp(read_key(&it, &type), key) == 0) {
            return read_string_val(&it);
        }
    }
    return NULL;
}

int main() {
    bson_obj_t* b = malloc(sizeof(bson_obj_t));
    memset(b, 0, sizeof(bson_obj_t));
    // write length
    int offset = bson_reserved_length(b);
    bson_append_string(b, "hello", "world");
    bson_write_byte(b, 0);
    bson_write_uint32_with_offset(b, b->size - offset, offset);
    printf("size: %zu\n", b->size);
    hex_dump("dump:", b->bytes, b->size);
    const char* v = bson_read_string(b->bytes, "hello");
    printf("key: %s value: %s \n", "hello", v);

}
