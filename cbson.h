#ifndef CBSON_H_
#define CBSON_H_
#include <stdint.h>
#include <unistd.h>

typedef enum {
    CBSON_TYPE_DOUBLE = '\x01',
    CBSON_TYPE_STRING,
    CBSON_TYPE_DOC,
    CBSON_TYPE_ARRAY,
    CBSON_TYPE_BINARY,
    CBSON_TYPE_UNDEFINED,
    CBSON_TYPE_OBJECT_ID,
    CBSON_TYPE_FALSE,
    CBSON_TYPE_TRUE,
    CBSON_TYPE_UTC_DATETIME,
    CBSON_TYPE_NULL,
    CBSON_TYPE_REGEX,
    CBSON_TYPE_DBPOINTER,
    CBSON_TYPE_JAVASCRIPT_CODE,
    CBSON_TYPE_INT32,
    CBSON_TYPE_TIMESTAMP,
    CBSON_TYPE_INT64,
    CBSON_TYPE_MIN_KEY = '\xff',
    CBSON_TYPE_MAX_KEY = '\x7f',
} bson_type;

typedef enum {
    CBSON_RET_TYPE_ERROR = -1,
    CBSON_RET_DOC_NOT_VALID = -2,
    CBSON_RET_OK = 0
} cbson_read_result_t;

typedef struct _bson_obj_t {
    uint8_t* bytes;
    size_t   size;
    size_t   cap;
} bson_obj_t;

typedef struct _bson_iter_t {
    const uint8_t* ptr;
    size_t size;
} bson_iter_t;

cbson_read_result_t
bson_read_string(bson_iter_t* doc, const char* key, char* buf, size_t sz);

cbson_read_result_t
bson_read_int32(bson_iter_t* doc, const char* key, int32_t** u);

cbson_read_result_t
bson_read_int64(bson_iter_t* doc, const char* key, int64_t** u);

cbson_read_result_t
bson_read_double(bson_iter_t* doc, const char* key, double** u);

cbson_read_result_t
bson_read_doc(bson_iter_t* doc, const char* key, uint8_t** out_doc);

void bson_pack_begin(bson_obj_t* b);
void bson_pack_end(bson_obj_t* b);

bson_iter_t* bson_doc_iter_new(const uint8_t* b);
void bson_doc_iter_release(bson_iter_t** it);

#endif

