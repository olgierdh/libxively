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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, const char* argv[] )
{
    ( void ) argc;
    ( void ) argv;

    ///

    xi_datapoint_t dp;
    memset( &dp, 0, sizeof( xi_datapoint_t ) );

    xi_context_t* context = xi_create_context( XI_HTTP, argv[ 1 ], atoi( argv[ 2 ] ) );
    xi_datastream_get( context, 11707, argv[ 3 ], &dp );

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
