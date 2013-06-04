#ifndef __LAYER_DEFAULT_ALLOCATORS_H__
#define __LAYER_DEFAULT_ALLOCATORS_H__

/**
 * \file    layer_default_allocators.h
 * \author  Olgierd Humenczuk
 * \brief   containes the basic set of allocators that can be used across the whole layer system
 *          to allocate/free the memory for the new layers, this set containes different allocation / free
 *          strategies to adjust to fulfill different requirements
 */

#include "layer.h"
#include "layer_type.h"
#include "xi_allocator.h"
#include "xi_macros.h"

/**
 * \brief   default_layer_heap_alloc
 * \param   type simple pointer to type structure
 * \return  pointer to allocated layer_t structure
 */
static inline layer_t* default_layer_heap_alloc( const layer_type_t* type )
{
    layer_t* ret = ( layer_t* ) xi_alloc( sizeof( layer_t ) );

    XI_CHECK_MEMORY( ret );

    memset( ret, 0, sizeof( layer_t ) );

    ret->layer_functions                = &type->layer_interface;
    ret->layer_type_id                  = type->layer_type_id;
    ret->layer_connection.self          = ret;

    return ret;

err_handling:
    return 0;
}

/**
 * \brief default_layer_heap_free
 * \param type
 * \param layer
 */
static inline void default_layer_heap_free( layer_type_t* type, layer_t* layer )
{
    XI_UNUSED( type );
    xi_free( layer );
}

#endif // __LAYER_DEFAULT_ALLOCATORS_H__
