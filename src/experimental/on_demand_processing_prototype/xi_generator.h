#ifndef __XI_GENERATOR_H__
#define __XI_GENERATOR_H__

#include <string.h>

#include "xi_coroutine.h"
#include "common.h"

/**
 * \brief  this is the generator abstract interface definition
 *
 * \param   input the input data, the generator will know how to interpret that
 * \param   curr_state returned through the pointer, this is where generator will return and save it's state
 *              so that the communication can be done throught that short value
 * \warning do not change the curr_state from outside, unless you know what youre doing!
 * \return  abstract value that can be interpret via the generator caller
 */
typedef const void* ( xi_generator_t )( const void* input, short* curr_state );

#define ENABLE_GENERATOR() \
    static const_data_descriptor_t __tmp = { 0, 0, 0 }; \

// couple of macros
#define gen_ptr_text( state, ptr_text ) \
{ \
    size_t len = strlen( ptr_text ); \
    __tmp.data_ptr = ptr_text; \
    __tmp.data_size = len; \
    __tmp.hint_size = len; \
    YIELD( state, ( void* ) &__tmp ); \
}

#define gen_ptr_text_and_exit( state, ptr_text ) \
{ \
    size_t len = strlen( ptr_text ); \
    __tmp.data_ptr  = ptr_text; \
    __tmp.data_size = len; \
    __tmp.hint_size = len; \
    EXIT( state, ( void* ) &__tmp ); \
}

// couple of macros
#define gen_static_text( state, text ) \
{ \
    static const char* const tmp_str = text; \
    __tmp.data_ptr = tmp_str; \
    __tmp.hint_size = __tmp.data_size = sizeof( text ) - 1; \
    YIELD( state, ( void* ) &__tmp ); \
}

#define gen_static_text_and_exit( state, text ) \
{ \
    static const char* const tmp_str = text; \
    __tmp.data_ptr = tmp_str; \
    __tmp.hint_size = __tmp.data_size = sizeof( text ) - 1; \
    EXIT( state, ( void* ) &__tmp ); \
}


#endif // __XI_GENERATOR_H__
