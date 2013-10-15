// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file    xively.c
 * \brief   Xively C library [see xively.h]
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "xi_allocator.h"
#include "xively.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_helpers.h"
#include "xi_err.h"
#include "xi_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------
// HELPER MACROS
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// HELPER FUNCTIONS
//-----------------------------------------------------------------------

xi_value_type_t xi_get_value_type( xi_datapoint_t* p )
{
    // PRECONDITION
    assert( p != 0 );

    return p->value_type;
}

xi_datapoint_t* xi_set_value_i32( xi_datapoint_t* p, int32_t value )
{
    // PRECONDITION
    assert( p != 0 );

    p->value.i32_value  = value;
    p->value_type       = XI_VALUE_TYPE_I32;

    return p;
}

int32_t xi_get_value_i32( xi_datapoint_t* p )
{
    // PRECONDITION
    assert( p != 0 );
    assert( p->value_type == XI_VALUE_TYPE_I32 );

    return p->value.i32_value;
}

int32_t* xi_value_pointer_i32( xi_datapoint_t* p )
{
    // PRECONDITION
    assert( p != 0 );

    if( p->value_type == XI_VALUE_TYPE_I32 )
    {
      return &p->value.i32_value;
    }
    else
    {
      return NULL;
    }
}

xi_datapoint_t* xi_set_value_f32( xi_datapoint_t* p, float value )
{
    // PRECONDITION
    assert( p != 0 );

    p->value.f32_value  = value;
    p->value_type       = XI_VALUE_TYPE_F32;

    return p;
}

float xi_get_value_f32( xi_datapoint_t* p )
{
    // PRECONDITION
    assert( p != 0 );
    assert( p->value_type == XI_VALUE_TYPE_F32 );

    if( p->value_type == XI_VALUE_TYPE_F32 )
    {
      return p->value.f32_value;
    }
    else
    {
      return 0.;
    }
}

float* xi_value_pointer_f32( xi_datapoint_t* p )
{
    // PRECONDITION
    assert( p != 0 );

    if( p->value_type == XI_VALUE_TYPE_F32 )
    {
      return &p->value.f32_value;
    }
    else
    {
      return NULL;
    }
}

xi_datapoint_t* xi_set_value_str( xi_datapoint_t* p, const char* value )
{
    // PRECONDITION
    assert( p != 0 );

    int s = xi_str_copy_untiln( p->value.str_value
        , XI_VALUE_STRING_MAX_SIZE, value, '\0' );

    XI_CHECK_SIZE( s, XI_VALUE_STRING_MAX_SIZE, XI_DATAPOINT_VALUE_BUFFER_OVERFLOW );

    p->value_type = XI_VALUE_TYPE_STR;

    return p;

err_handling:
    return 0;
}

char* xi_value_pointer_str( xi_datapoint_t* p )
{
    // PRECONDITION
    assert( p != 0 );

    if( p->value_type == XI_VALUE_TYPE_STR )
    {
      return p->value.str_value;
    }
    else
    {
      return NULL;
    }
}

void xi_set_network_timeout( uint32_t timeout )
{
    xi_globals.network_timeout = timeout;
}

uint32_t xi_get_network_timeout( void )
{
    return xi_globals.network_timeout;
}

//-----------------------------------------------------------------------
// MAIN LIBRARY FUNCTIONS
//-----------------------------------------------------------------------

xi_context_t* xi_create_context(
      xi_protocol_t protocol, const char* api_key
    , xi_feed_id_t feed_id )
{
    // allocate the structure to store new context
    xi_context_t* ret = ( xi_context_t* ) xi_alloc( sizeof( xi_context_t ) );

    XI_CHECK_MEMORY( ret );

    // copy given numeric parameters as is
    ret->protocol       = protocol;
    ret->feed_id        = feed_id;

    // copy string parameters carefully
    if( api_key )
    {
        // duplicate the string
        ret->api_key  = xi_str_dup( api_key );

        XI_CHECK_MEMORY( ret->api_key );
    }
    else
    {
        ret->api_key  = 0;
    }

    return ret;

err_handling:
    if( ret )
    {
        XI_SAFE_FREE( ret );
    }

    return 0;
}

void xi_delete_context( xi_context_t* context )
{
    if( context )
    {
        XI_SAFE_FREE( context->api_key );
    }
    XI_SAFE_FREE( context );
}

const xi_response_t* xi_feed_get(
          xi_context_t* xi
        , xi_feed_t* feed )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed );

    return 0;
}

const xi_response_t* xi_feed_update(
          xi_context_t* xi
        , const xi_feed_t* feed )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed );

    return 0;
}

const xi_response_t* xi_datastream_get(
            xi_context_t* xi, xi_feed_id_t feed_id
          , const char * datastream_id, xi_datapoint_t* o )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed_id );
    XI_UNUSED( datastream_id );
    XI_UNUSED( o );

    return 0;
}


const xi_response_t* xi_datastream_create(
            xi_context_t* xi, xi_feed_id_t feed_id
          , const char * datastream_id
          , const xi_datapoint_t* datapoint )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed_id );
    XI_UNUSED( datastream_id );
    XI_UNUSED( datapoint );
}

const xi_response_t* xi_datastream_update(
          xi_context_t* xi, xi_feed_id_t feed_id
        , const char * datastream_id
        , const xi_datapoint_t* datapoint )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed_id );
    XI_UNUSED( datastream_id );
    XI_UNUSED( datapoint );
}

const xi_response_t* xi_datastream_delete(
            xi_context_t* xi, xi_feed_id_t feed_id
          , const char * datastream_id )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed_id );
    XI_UNUSED( datastream_id );
}

const xi_response_t* xi_datapoint_delete(
          const xi_context_t* xi, xi_feed_id_t feed_id
        , const char * datastream_id
        , const xi_datapoint_t* o )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed_id );
    XI_UNUSED( datastream_id );
    XI_UNUSED( o );
}

extern const xi_response_t* xi_datapoint_delete_range(
            const xi_context_t* xi, xi_feed_id_t feed_id
          , const char * datastream_id
          , const xi_timestamp_t* start
          , const xi_timestamp_t* end )
{
    XI_UNUSED( xi );
    XI_UNUSED( feed_id );
    XI_UNUSED( datastream_id );
    XI_UNUSED( start );
    XI_UNUSED( end );
}


#ifdef __cplusplus
}
#endif
