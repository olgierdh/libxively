#include <stdio.h>

#include "xi_consts.h"
#include "xively.h"

#include "common.h"
#include "layer_api.h"
#include "layer_interface.h"
#include "layer_connection.h"
#include "layer_types_conf.h"
#include "layer_factory.h"
#include "layer_factory_conf.h"
#include "layer_default_allocators.h"

#include "csv_layer.h"

#include "xi_stated_sscanf.h"

#include "posix_io_layer.h"
#include "http_layer.h"
#include "http_layer_data.h"

enum LAYERS_ID
{
      IO_LAYER = 0
    , HTTP_LAYER
    , CSV_LAYER
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CONNECTION_SCHEME_1_DATA IO_LAYER, HTTP_LAYER, CSV_LAYER
DEFINE_CONNECTION_SCHEME( CONNECTION_SCHEME_1, CONNECTION_SCHEME_1_DATA );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_LAYER_TYPES_CONF()
      LAYER_TYPE( IO_LAYER, &posix_io_layer_data_ready, &posix_io_layer_on_data_ready
                          , &posix_io_layer_close, &posix_io_layer_on_close )
    , LAYER_TYPE( HTTP_LAYER, &http_layer_data_ready, &http_layer_on_data_ready
                            , &http_layer_close, &http_layer_on_close )
    , LAYER_TYPE( CSV_LAYER, &csv_layer_data_ready, &csv_layer_on_data_ready
                        , &csv_layer_close, &csv_layer_on_close )
END_LAYER_TYPES_CONF()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_FACTORY_CONF()
      FACTORY_ENTRY( IO_LAYER, &placement_layer_pass_create, &placement_layer_pass_delete
                             , &default_layer_stack_alloc, &default_layer_stack_free )
    , FACTORY_ENTRY( HTTP_LAYER, &placement_layer_pass_create, &placement_layer_pass_delete
                               , &default_layer_stack_alloc, &default_layer_stack_free )
    , FACTORY_ENTRY( CSV_LAYER, &placement_layer_pass_create, &placement_layer_pass_delete
                           , &default_layer_stack_alloc, &default_layer_stack_free )
END_FACTORY_CONF()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
    ( void ) argc;
    ( void ) argv;

    ///
    http_layer_data_t   http_layer_data;
    csv_layer_data_t    csv_layer_data;
    xi_datapoint_t      datapoint;


    memset( &http_layer_data, 0, sizeof( http_layer_data_t ) );
    memset( &csv_layer_data, 0, sizeof( csv_layer_data_t ) );

    /// context and response
    xi_context_t* context = xi_create_context( XI_HTTP, argv[ 1 ], atoi( argv[ 2 ] ) );
    xi_response_t xi_response;

    memset( &xi_response, 0, sizeof( xi_response_t ) );
    http_layer_data.response = &xi_response;

    ///
    void* user_datas[] = { 0, ( void* ) &http_layer_data, ( void* ) &csv_layer_data };

    layer_chain_t layer_chain = create_and_connect_layers( CONNECTION_SCHEME_1, user_datas, CONNECTION_SCHEME_LENGTH( CONNECTION_SCHEME_1 ) );

    layer_t* io_layer = connect_to_endpoint( layer_chain.bottom, XI_HOST, XI_PORT );
    layer_t* csv_layer = layer_chain.top;

    ///

#if 0
    datapoint.value.i32_value   = 13423;
    datapoint.value_type        = XI_VALUE_TYPE_I32;
#endif

#if 1
    memcpy( datapoint.value.str_value, "test the text", sizeof( "test the text" ) );
    datapoint.value_type            = XI_VALUE_TYPE_STR;
    datapoint.timestamp.timestamp   = 0;
    datapoint.timestamp.micro       = 0;
#endif

    // create the input parameter
    http_layer_input_t http_layer_input =
    {
          HTTP_LAYER_INPUT_DATASTREAM_GET
        , context
        , 0
        , { { argv[ 3 ], &datapoint } }
    };


    if( io_layer == 0 )
    {
        printf( "Could not connect to the endpoint\n" ); exit( 1 );
    }

    // prepare the xi data
    CALL_ON_SELF_DATA_READY( csv_layer, ( void *) &http_layer_input, LAYER_HINT_NONE );

    /*short state = 0;
    xi_datapoint_t dp = { { 15 }, XI_VALUE_TYPE_I32, { 123233, 0 } };

    while( state != 1 )
    {
        const const_data_descriptor_t* dd = csv_layer_data_generator_datapoint( ( void* ) &dp, &state );
        printf( "%s", dd->data_ptr );
    }

    state = 0;

    while( state != 1 )
    {
        const const_data_descriptor_t* dd = csv_layer_data_generator_datapoint( ( void* ) &dp, &state );
        printf( "%s", dd->data_ptr );
    }*/

#if 0
    {
        xi_datapoint_t   dp;
        csv_layer_data_t ld;

        memset( &ld, 0, sizeof( csv_layer_data_t ) );

        const char source[] = "2013-01-01T22:22:22.000000Z,123\0";

        const_data_descriptor_t dd = { source, sizeof( source ), sizeof( source ), 0 };

        csv_layer_parse_datastream( &ld, &dd, LAYER_HINT_NONE, &dp );
    }
#endif

#if 0
    {l
        xi_feed_t         f;
        csv_layer_data_t ld;

        memset( &ld, 0, sizeof( csv_layer_data_t ) );

        const char source1[] = "datastr";
        const char source2[] = "eam_id,2013-01";
        const char source3[] = "-01T22:22:22.000000Z,123\0";

        const_data_descriptor_t dd1 = { source1, sizeof( source1 ) - 1, sizeof( source1 ) - 1, 0 };
        const_data_descriptor_t dd2 = { source2, sizeof( source2 ) - 1, sizeof( source2 ) - 1, 0 };
        const_data_descriptor_t dd3 = { source3, sizeof( source3 ) - 1, sizeof( source3 ) - 1, 0 };

        layer_state_t state1 = csv_layer_parse_feed( &ld, &dd1, LAYER_HINT_MORE_DATA, &f );
        layer_state_t state2 = csv_layer_parse_feed( &ld, &dd2, LAYER_HINT_MORE_DATA, &f );
        layer_state_t state3 = csv_layer_parse_feed( &ld, &dd3, LAYER_HINT_NONE, &f );

        if( state1 == LAYER_STATE_ERROR )
        {
            int i = 0;
        }
    }
#endif

	return 0;
}
