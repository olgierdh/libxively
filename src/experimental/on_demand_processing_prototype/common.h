#ifndef __COMMON_H__
#define __COMMON_H__

#include <assert.h>
#include <malloc.h>

/**
 * \brief the structure that describes the buffer for storing/passing
 * within the program the structure can be easly manipulated using the 
 * module functions
 */
typedef struct
{
    char*   data_buffer;
    size_t  buffer_capacity;    //!< the maximum capacity of the buffer
    size_t  buffer_size;        //!< the actual used size of the buffer 
} data_buffer_t;

/**
 * \brief the structure that describes the simple char* with the size so it's easier to
 * pass this as a paremeter
 */
typedef struct
{
    char*   data_ptr;
    short   data_size;
    short   hint_size;
} data_descriptor_t;

typedef struct
{
    const char*       data_ptr;
    short             data_size;
    short             hint_size;
} const_data_descriptor_t;


// some syntactic sugar
#define WRAP_DATA( data_ptr, size )     { data_ptr, size }
#define WRAP_STATIC_DATA( data_ptr )    { data_ptr, sizeof( data_ptr ) }

/**
 * \brief the function creates the data buffer with requested maximum capacity
 * \returns pointer to data_buffer_t if buffer has been created succesfully
 * null otherway
 */
static inline data_buffer_t* create_data_buffer( size_t capacity )
{
    // PRECONDITIONS
    assert( capacity > 0 && "Data buffer capacity must be greater than zero!" );

    // temporary buffer that we are going to use to allocate the 
    // space for both the data buffer and the data buffer structure
    char* tmp_buffer = ( char* ) malloc( capacity + sizeof( data_buffer_t ) );

    // if no memory
    if( tmp_buffer == 0 ) { return 0; }

    // set the content of the temporary data buffer
    data_buffer_t* tmp_data_buffer   = ( data_buffer_t* ) tmp_buffer;
    tmp_data_buffer->data_buffer     = tmp_buffer + sizeof( data_buffer_t );
    tmp_data_buffer->buffer_capacity = capacity;
    tmp_data_buffer->buffer_size     = 0;

    // POSTCONDITIONS
    assert( tmp_data_buffer != 0 && 
        "The data buffer structure pointer must not be null!" );
    assert( tmp_data_buffer->data_buffer != 0 && 
        "The data buffer pointer must not be null!" );
    assert( tmp_data_buffer->buffer_capacity == capacity && 
        "The data buffer capacity differs from desired capacity!" );

    // here we can safely return the buffer
    return tmp_data_buffer;
}

/**
 * \brief function deletes earlier created data buffer 
 * \note  because it frees single block of the memory so it's
 *        simple operation and it can be done with single free
 */
static inline void delete_data_buffer( data_buffer_t* data_buffer )
{
    // PRECONDITION
    assert( data_buffer != 0 && "Null pointer passed!" );

    // let it free
    free( data_buffer );
}

 
#endif
