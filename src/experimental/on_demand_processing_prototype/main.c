#include <stdio.h>

#include "xi_consts.h"

#include "common.h"
#include "layer_api.h"
#include "layer_interface.h"
#include "layer_connection.h"
#include "layer_types_conf.h"
#include "layer_factory.h"
#include "layer_factory_conf.h"
#include "layer_default_allocators.h"

#include "posix_io_layer.h"

enum LAYERS_ID
{
      IO_LAYER = 0
    , DUMMY_LAYER_TYPE_1
};

layer_state_t dummy_layer1_on_demand( layer_connectivity_t* context, void* data, const char impulse )
{
    ( void ) impulse;
    ( void ) data;

    return CALL_ON_PREV_ON_DEMAND( context->self, data, 0 );
}

layer_state_t dummy_layer1_on_data_ready( layer_connectivity_t* context, const void* data, const char impulse  )
{
    ( void ) impulse;
    ( void ) data;

    return CALL_ON_PREV_ON_DATA_READY( context->self, data, 0 );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CONNECTION_SCHEME_1_DATA IO_LAYER, DUMMY_LAYER_TYPE_1
DEFINE_CONNECTION_SCHEME( CONNECTION_SCHEME_1, CONNECTION_SCHEME_1_DATA );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_LAYER_TYPES_CONF()
      LAYER_TYPE( IO_LAYER, &posix_io_layer_on_demand, &posix_io_layer_on_data_ready
                          , &posix_io_layer_close, &posix_io_layer_on_close )
    , LAYER_TYPE( DUMMY_LAYER_TYPE_1, &dummy_layer1_on_demand, &dummy_layer1_on_data_ready
                                    , &dummy_layer1_close, &dummy_layer1_on_close )
END_LAYER_TYPES_CONF()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_FACTORY_CONF()
      FACTORY_ENTRY( IO_LAYER, &placement_layer_pass_create, &placement_layer_pass_delete
                             , &default_layer_heap_alloc, &default_layer_heap_free )
    , FACTORY_ENTRY( DUMMY_LAYER_TYPE_1, &placement_layer_pass_create, &placement_layer_pass_delete
                                       , &default_layer_heap_alloc, &default_layer_heap_free )
END_FACTORY_CONF()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char test_msg[]        = "This is test msg\n\n";
static const size_t test_msg_length = sizeof( test_msg );

int main( int argc, const char* argv[] )
{
    ( void ) argc;
    ( void ) argv;

    void* user_datas[] = { 0, 0 };

    layer_chain_t layer_chain = create_and_connect_layers( CONNECTION_SCHEME_1, user_datas, CONNECTION_SCHEME_LENGTH( CONNECTION_SCHEME_1 ) );

    layer_t* io_layer = connect_to_endpoint( layer_chain.bottom, XI_HOST, XI_PORT );
    layer_t* dummy_layer = layer_chain.top;

    printf( "%d\n", dummy_layer->layer_type_id );

    if( io_layer == 0 )
    {
        printf( "Could not connect to the endpoint\n" ); exit( 1 );
    }

    const const_data_descriptor_t test_msg_data = { test_msg, test_msg_length };
    CALL_ON_SELF_ON_DATA_READY( dummy_layer, ( const void* ) &test_msg_data, 0 );


    char buff[ 32 ];
    memset( buff, 0, sizeof( buff ) );

    data_descriptor_t tmp_data = { buff, 31 };


    layer_state_t layer_state = CALL_ON_SELF_ON_DEMAND( dummy_layer, ( void *) &tmp_data, 0 );

    printf( "Buffer \n[" );
    printf( "%s", buff );

    while( layer_state == LAYER_STATE_FULL )
    {
        layer_state = CALL_ON_SELF_ON_DEMAND( dummy_layer, ( void* ) &tmp_data, 0 );
        printf( "%s", buff );
        memset( buff, 0, sizeof( buff ) );
    }

    printf( "]\n" );

    /*CALL_ON_NEXT_ON_DEMAND( dummy_layer1_instance, 0, 0 );
    CALL_ON_PREV_ON_DEMAND( dummy_layer2_instance, 0, 0 );
    CALL_ON_SELF_ON_DEMAND( dummy_layer1_instance, 0, 0 );
    CALL_ON_SELF_ON_DEMAND( dummy_layer2_instance, 0, 0 );

    CALL_ON_NEXT_ON_DATA_READY( dummy_layer1_instance, 0, 0 );
    CALL_ON_PREV_ON_DATA_READY( dummy_layer2_instance, 0, 0 );
    CALL_ON_SELF_ON_DATA_READY( dummy_layer1_instance, 0, 0 );
    CALL_ON_SELF_ON_DATA_READY( dummy_layer2_instance, 0, 0 );

    CALL_ON_NEXT_CLOSE( dummy_layer1_instance );
    CALL_ON_PREV_CLOSE( dummy_layer2_instance );
    CALL_ON_SELF_CLOSE( dummy_layer1_instance );
    CALL_ON_SELF_CLOSE( dummy_layer2_instance );

    CALL_ON_NEXT_ON_CLOSE( dummy_layer1_instance );
    CALL_ON_PREV_ON_CLOSE( dummy_layer2_instance );
    CALL_ON_SELF_ON_CLOSE( dummy_layer1_instance );
    CALL_ON_SELF_ON_CLOSE( dummy_layer2_instance );*/

	return 0;
}
