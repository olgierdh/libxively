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

layer_state_t posix_io_layer_on_demand(
      layer_connectivity_t* context
    , char* buffer
    , size_t size )
{
    posix_data_t* posix_data = ( posix_data_t* ) context->self->user_data;

    XI_UNUSED( posix_data );
    XI_UNUSED( buffer );
    XI_UNUSED( size );

    return LAYER_STATE_OK;
}

layer_state_t posix_io_layer_on_data_ready(
      layer_connectivity_t* context
    , const char* buffer
    , size_t size )
{
    posix_data_t* posix_data = ( posix_data_t* ) context->self->user_data;

    XI_UNUSED( posix_data );
    XI_UNUSED( buffer );
    XI_UNUSED( size );

    return LAYER_STATE_OK;
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

/**
 * \struct  posix_io_layer_init_data_t
 * \brief   simple configuration of the initialization of the posix connection endpoint
 */
typedef struct
{
    const char* address;
    const int   port;
} posix_io_layer_init_data_t;

/**
 * \brief connect_to_endpoint
 * \param layer
 * \param init_data
 * \return
 */
layer_t* connect_to_endpoint(
      layer_t* layer
    , void* init_data )
{
    posix_io_layer_init_data_t* the_init_data   = ( posix_io_layer_init_data_t* ) init_data;
    posix_data_t* posix_data                    = xi_alloc( sizeof( posix_data_t ) );

    XI_CHECK_MEMORY( posix_data );

    layer->user_data                            = ( void* ) posix_data;

    posix_data->socket_fd                       = socket( AF_INET, SOCK_STREAM, 0 );

    if( posix_data->socket_fd == -1 )
    {
        xi_set_err( XI_SOCKET_INITIALIZATION_ERROR );
        return 0;
    }

     // socket specific data
    struct sockaddr_in name;
    struct hostent* hostinfo;

    // get the hostaddress
    hostinfo = gethostbyname( the_init_data->address );

    // if null it means that the address has not been founded
    if( hostinfo == NULL )
    {
        xi_set_err( XI_SOCKET_GETHOSTBYNAME_ERROR );
        goto err_handling;
    }

    // set the address and the port for further connection attempt
    memset( &name, 0, sizeof( struct sockaddr_in ) );
    name.sin_family     = AF_INET;
    name.sin_addr       = *( ( struct in_addr* ) hostinfo->h_addr );
    name.sin_port       = htons( the_init_data->port );

    if( connect( posix_data->socket_fd, ( struct sockaddr* ) &name, sizeof( struct sockaddr ) ) == -1 )
    {
        xi_set_err( XI_SOCKET_CONNECTION_ERROR );
        goto err_handling;
    }

    // POSTCONDITIONS
    assert( layer != 0 );
    assert( posix_data->socket_fd != -1 );

    return layer;

err_handling:
    // cleanup the memory
    if( posix_data ) { XI_SAFE_FREE( posix_data ); }

    return 0;
}
