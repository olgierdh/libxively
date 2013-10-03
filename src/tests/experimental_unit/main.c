// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.
#include "tinytest.h"
#include "tinytest_macros.h"
#include "layer_api.h"
#include "layer_connectivity.h"
#include "layer_interface.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////////
layer_state_t dummy_layer1_data_ready( layer_connectivity_t* context, void* data, const layer_hint_t hint )
{
    ( void ) data;
    ( void ) hint;

    printf( "dummy_layer1_data_ready %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer1_on_data_ready( layer_connectivity_t* context, const void* data, const layer_hint_t hint )
{
    ( void ) data;
    ( void ) hint;

    printf( "dummy_layer1_on_data_ready %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer1_close( layer_connectivity_t* context )
{
    printf( "dummy_layer1_close %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer1_on_close( layer_connectivity_t* context )
{
    printf( "dummy_layer1_on_close %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer2_data_ready( layer_connectivity_t* context, char* data, const layer_hint_t hint )
{
    ( void ) data;
    ( void ) hint;

    printf( "dummy_layer2_data_ready %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer2_on_data_ready( layer_connectivity_t* context, const void* data, const layer_hint_t hint )
{
    ( void ) data;
    ( void ) hint;

    printf( "dummy_layer2_on_data_ready %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer2_close( layer_connectivity_t* context )
{
    printf( "dummy_layer2_close %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}

layer_state_t dummy_layer2_on_close( layer_connectivity_t* context )
{
    printf( "dummy_layer2_on_close %p\n", context->self->user_data );
    return LAYER_STATE_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// EXPERIMENTAL LAYER TESTS
///////////////////////////////////////////////////////////////////////////////

void test_layer_initialization(void* data)
{
    (void)(data);
    int user_data = 0;

    LAYER_LOCAL_TYPE( DLT1, 0, &dummy_layer1_data_ready
                      , &dummy_layer1_on_data_ready
                      , &dummy_layer1_close
                      , &dummy_layer1_on_close );

    LAYER_LOCAL_INSTANCE(
                  dummy_layer1_instance
                , &DLT1.layer_interface
                , DLT1.layer_type_id
                , &user_data );

    tt_assert( ( intptr_t ) dummy_layer1_instance.layer_connection.self == ( intptr_t ) &dummy_layer1_instance );
 end:
 ;
}


struct testcase_t demo_tests[] = {
    /* Here's a really simple test: it has a name you can refer to it
       with, and a function to invoke it. */
    { "test_layer_initialization", test_layer_initialization, TT_ENABLED_, 0, 0 },
    /* The array has to end with END_OF_TESTCASES. */
    END_OF_TESTCASES
};

/* Next, we make an array of testgroups.  This is mandatory.  Unlike more
   heavy-duty testing frameworks, groups can't nest. */
struct testgroup_t groups[] = {

    /* Every group has a 'prefix', and an array of tests.  That's it. */
    { "experimental_layers/", demo_tests },

    END_OF_GROUPS
};

int main( int argc, char const *argv[] )
{
    /* code */
    return tinytest_main( argc, argv, groups );
}
