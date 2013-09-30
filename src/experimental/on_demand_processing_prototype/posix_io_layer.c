// c
#include <assert.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

// local
#include "posix_io_layer.h"
#include "posix_data.h"
#include "xi_allocator.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_debug.h"

#include "layer_api.h"
#include "common.h"

layer_state_t posix_io_layer_on_demand(
      layer_connectivity_t* context
    , void* data
    , const char impulse )
{
    xi_debug_logger( "[posix_io_layer_on_demand]" );

    posix_data_t* posix_data = ( posix_data_t* ) context->self->user_data;

    XI_UNUSED( impulse );

    data_descriptor_t* buffer = ( data_descriptor_t* ) data;
    int len = read( posix_data->socket_fd, buffer->data_ptr, buffer->data_size );

    if( len == buffer->data_size )
    {
        return LAYER_STATE_FULL;
    }

    return LAYER_STATE_OK;
}

layer_state_t posix_io_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const char impulse )
{
    xi_debug_logger( "[posix_io_layer_on_data_ready]" );

    posix_data_t* posix_data                = ( posix_data_t* ) context->self->user_data;
    const const_data_descriptor_t* buffer   = ( const const_data_descriptor_t* ) data;

    XI_UNUSED( impulse );

    if( buffer != 0 && buffer->data_size > 0 )
    {
        int len = write( posix_data->socket_fd, buffer->data_ptr, buffer->data_size );

        if( len < buffer->data_size )
        {
            return LAYER_STATE_OK;
        }
    }

    return LAYER_STATE_FULL;
}

layer_state_t posix_io_layer_close( layer_connectivity_t* context )
{
    posix_data_t* posix_data = ( posix_data_t* ) context->self->user_data;

    XI_UNUSED( posix_data );

    return LAYER_STATE_OK;
}

layer_state_t posix_io_layer_on_close( layer_connectivity_t* context )
{
    posix_data_t* posix_data = ( posix_data_t* ) context->self->user_data;

    XI_UNUSED( posix_data );

    return LAYER_STATE_OK;
}

layer_t* connect_to_endpoint(
      layer_t* layer
    , const char* address
    , const int port )
{

    {
        char msg[ 32 ];
        sprintf( msg, "Connecting layer [%d] to the endpoint", layer->layer_type_id );
        xi_debug_logger( msg );
    }

    posix_data_t* posix_data                    = xi_alloc( sizeof( posix_data_t ) );

    XI_CHECK_MEMORY( posix_data );

    layer->user_data                            = ( void* ) posix_data;

    xi_debug_logger( "Creating socket..." );
    posix_data->socket_fd                       = socket( AF_INET, SOCK_STREAM, 0 );

    if( posix_data->socket_fd == -1 )
    {
        xi_debug_logger( "Socket creation [failed]" );
        xi_set_err( XI_SOCKET_INITIALIZATION_ERROR );
        return 0;
    }

    xi_debug_logger( "Socket creation [ok]" );

     // socket specific data
    struct sockaddr_in name;
    struct hostent* hostinfo;

    xi_debug_logger( "Getting host by name..." );

    // get the hostaddress
    hostinfo = gethostbyname( address );

    // if null it means that the address has not been founded
    if( hostinfo == NULL )
    {
        xi_debug_logger( "Getting Host by name [failed]" );
        xi_set_err( XI_SOCKET_GETHOSTBYNAME_ERROR );
        goto err_handling;
    }

    xi_debug_logger( "Getting Host by name [ok]" );

    // set the address and the port for further connection attempt
    memset( &name, 0, sizeof( struct sockaddr_in ) );
    name.sin_family     = AF_INET;
    name.sin_addr       = *( ( struct in_addr* ) hostinfo->h_addr );
    name.sin_port       = htons( port );

    xi_debug_logger( "Connecting to the endpoint..." );

    if( connect( posix_data->socket_fd, ( struct sockaddr* ) &name, sizeof( struct sockaddr ) ) == -1 )
    {
        xi_debug_logger( "Connecting to the endpoint [failed]" );
        xi_set_err( XI_SOCKET_CONNECTION_ERROR );
        goto err_handling;
    }

    xi_debug_logger( "Connecting to the endpoint [ok]" );

    // POSTCONDITIONS
    assert( layer != 0 );
    assert( posix_data->socket_fd != -1 );

    return layer;

err_handling:
    // cleanup the memory
    if( posix_data ) { XI_SAFE_FREE( posix_data ); }

    return 0;
}
