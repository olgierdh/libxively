// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.
/**
 * \brief   Example 004 that shows the basics of the xi synchronious interface
 * \file    main.c
 * \author  Olgierd Humenczuk
 *
 * The example that shows how to:
 *
 * a) initialize xi
 * b) get the datastream
 * c) gracefully close xi library
 *
 */
#include <xively.h>
#include <xi_helpers.h>

#include <time.h>
#include <stdio.h>
#include <sys/select.h>

#include "io/posix_asynch/posix_asynch_data.h"
#include "nob/nob_runner.h"

void print_usage()
{
    static const char usage[] = "This is example_03 of xi library\n"
    "to view your datastream write: \n"
    "example_04 api_key feed_id datastream_id\n";

    printf( "%s", usage );
}

int main( int argc, const char* argv[] )
{

    XI_UNUSED( argc );
    XI_UNUSED( argv );

#ifdef XI_NOB_ENABLED
    if( argc < 4 )
    {
        print_usage();
        exit( 0 );
    }

    // create the xi library context
    xi_context_t* xi_context
        = xi_create_context(
                  XI_HTTP, argv[ 1 ]
                , atoi( argv[ 2 ] ) );

    // check if everything works
    if( xi_context == 0 )
    {
        return -1;
    }

    xi_datapoint_t ret;

    xi_nob_datastream_get( xi_context, atoi( argv[ 2 ] ), argv[ 3 ], &ret );

    posix_asynch_data_t* posix_data
            = ( posix_asynch_data_t* ) xi_context->layer_chain.bottom->user_data;

    fd_set rfds;
    struct timeval tv;
    int retval = 0;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO( &rfds );
    FD_SET( posix_data->socket_fd, &rfds );

    while( 1 )
    {
        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select( posix_data->socket_fd + 1, &rfds, &rfds, NULL, &tv );

        if ( retval == -1 )
        {
            perror("error in select()");
        }
        else if ( retval )
        {
            if( process_xively_nob_step( xi_context ) != LAYER_STATE_NOT_READY )
            {
                break;
            }
        }
        else
        {
            printf( "No data within five seconds.\n" );
            exit( 0 );
        }
    }

    printf( "timestamp = %ld.%ld, value = %d\n"
        , ret.timestamp.timestamp, ret.timestamp.micro
        , ret.value.i32_value );

    // destroy the context cause we don't need it anymore
    xi_delete_context( xi_context );
#endif
    return 0;
}
