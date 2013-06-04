#include <stdio.h>

#include "layer_api.h"
#include "layer_interface.h"
#include "layer_connection.h"
#include "layer_types_conf.h"
#include "layer_factory.h"
#include "layer_factory_conf.h"
#include "layer_default_allocators.h"

enum LAYERS_ID
{
      DUMMY_LAYER_TYPE_1 = 0
    , DUMMY_LAYER_TYPE_2
    , DUMMY_LAYER_TYPE_3
    , DUMMY_LAYER_TYPE_4
};

layer_state_t dummy_layer1_on_demand( layer_connectivity_t* context, char* buffer, size_t size )
{
    ( void ) size;
    ( void ) buffer;

	printf( "dummy_layer1_on_demand %p\n", context->self->user_data );
	return LAYER_STATE_OK;
}

layer_state_t dummy_layer1_on_data_ready( layer_connectivity_t* context, const char* buffer, size_t size )
{
    ( void ) size;
    ( void ) buffer;

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

layer_state_t dummy_layer2_on_demand( layer_connectivity_t* context, char* buffer, size_t size )
{
    ( void ) size;
    ( void ) buffer;

	printf( "dummy_layer2_on_demand %p\n", context->self->user_data );
	return LAYER_STATE_OK;
}

layer_state_t dummy_layer2_on_data_ready( layer_connectivity_t* context, const char* buffer, size_t size )
{
    ( void ) size;
    ( void ) buffer;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CONNECTION_SCHEME_1_DATA DUMMY_LAYER_TYPE_1, DUMMY_LAYER_TYPE_2
DEFINE_CONNECTION_SCHEME( CONNECTION_SCHEME_1, CONNECTION_SCHEME_1_DATA );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_LAYER_TYPES_CONF()
      LAYER_TYPE( DUMMY_LAYER_TYPE_1, &dummy_layer1_on_demand, &dummy_layer1_on_data_ready, &dummy_layer1_close, &dummy_layer1_on_close )
    , LAYER_TYPE( DUMMY_LAYER_TYPE_2, &dummy_layer2_on_demand, &dummy_layer2_on_data_ready, &dummy_layer2_close, &dummy_layer2_on_close )
END_LAYER_TYPES_CONF()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_FACTORY_CONF()
      FACTORY_ENTRY( DUMMY_LAYER_TYPE_1, &placement_layer_pass_create, &placement_layer_pass_delete, &default_layer_heap_alloc, &default_layer_heap_free )
    , FACTORY_ENTRY( DUMMY_LAYER_TYPE_2, &placement_layer_pass_create, &placement_layer_pass_delete, &default_layer_heap_alloc, &default_layer_heap_free )
END_FACTORY_CONF()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main( int argc, const char* argv[] )
{
    ( void ) argc;
    ( void ) argv;

    int user_data  = 0;
    int user_data2 = 0;

    void* user_datas[] = { &user_data, &user_data2 };

    layer_chain_t layer_chain = create_and_connect_layers( CONNECTION_SCHEME_1, user_datas, CONNECTION_SCHEME_LENGTH( CONNECTION_SCHEME_1 ) );

    layer_t* dummy_layer1_instance = layer_chain.bottom;
    layer_t* dummy_layer2_instance = layer_chain.top;

    CALL_ON_NEXT_ON_DEMAND( dummy_layer1_instance, 0, 0 );
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
    CALL_ON_SELF_ON_CLOSE( dummy_layer2_instance );

	return 0;
}
