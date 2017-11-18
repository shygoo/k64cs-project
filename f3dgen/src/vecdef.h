/*
	
	VECTOR_TYPEDEF (
		base_type_t,   // Name of existing base type
		vector_type_t, // Name of new vector type
		fn_prefix      // Prefix for new functions
	)
	
	typedef struct
	{
		base_type_t* array;
		int length;
		int num_slots;
	} vector_type_t;
	
	typedef int (*iterator_fn_t)(base_type_t*);
	
	int  init(vector_type_t* vec)
	int  push(vector_type_t* vec, base_type_t item)
	int  free(vector_type_t* vec)
    int  size_bytes(vector_type_t* vec)
    void clear(vector_type_t* vec)
	int  iterate(vector_type_t* vec, int(iterator_fn_t)(base_type_t*))
	
	base_type_t* ptr(vector_type_t* vec, int index)

*/

#ifndef VECDEF_H
#define VECDEF_H

#include <stdlib.h>

#define VECTOR_TYPEDEF(base_type_t, vector_type_t, fn_prefix) \
typedef struct \
{ \
    base_type_t* array; \
    int length; \
    int num_slots; \
} vector_type_t; \
\
typedef int (*fn_prefix ## iterator_fn_t)(base_type_t*); \
\
int fn_prefix ## init(vector_type_t* vec); \
int fn_prefix ## push(vector_type_t* vec, base_type_t item); \
int fn_prefix ## free(vector_type_t* vec); \
base_type_t* fn_prefix ## ptr(vector_type_t* vec, int index); \
int fn_prefix ## size_bytes(vector_type_t* vec); \
void fn_prefix ## clear(vector_type_t* vec); \
int fn_prefix ## iterate(vector_type_t* vec, fn_prefix ## iterator_fn_t iterator_fn);

#define VECTOR_IMPLDEF(base_type_t, vector_type_t, fn_prefix) \
int fn_prefix ## init(vector_type_t* vec) \
{ \
    vec->length = 0; \
    vec->num_slots = 1; \
    vec->array = (base_type_t*) malloc(vec->num_slots * sizeof(base_type_t)); \
    return 1; \
} \
\
int fn_prefix ## push(vector_type_t* vec, base_type_t item) \
{ \
    if(vec->length + 1 > vec->num_slots) \
    { \
        vec->num_slots *= 2; \
        vec->array = (base_type_t*) realloc(vec->array, vec->num_slots * sizeof(base_type_t)); \
    } \
    vec->array[vec->length++] = item; \
    return vec->length; \
} \
\
int fn_prefix ## free(vector_type_t* vec) \
{ \
    free(vec->array); \
    return 1; \
} \
\
base_type_t* fn_prefix ## ptr(vector_type_t* vec, int index) \
{ \
    if(index >= vec->length) \
    { \
        return NULL; \
    } \
    return &vec->array[index]; \
} \
\
int fn_prefix ## size_bytes(vector_type_t* vec) \
{ \
    return vec->length * sizeof(base_type_t); \
} \
\
void fn_prefix ## clear(vector_type_t* vec) \
{ \
    if(vec->array != NULL) \
    { \
        free(vec->array); \
    } \
    fn_prefix ## init(vec); \
} \
\
int fn_prefix ## iterate(vector_type_t* vec, fn_prefix ## iterator_fn_t iterator_fn) \
{ \
	for(int i = 0; i < vec->length; i++) \
	{ \
		if(!iterator_fn(fn_prefix ## ptr(vec, i))) \
		{ \
			return 0; \
		} \
	} \
	return 1; \
}

#endif // VECDEF_H