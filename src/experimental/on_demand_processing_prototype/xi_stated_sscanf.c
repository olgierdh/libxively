#include "xi_stated_sscanf.h"
#include "xi_coroutine.h"


typedef unsigned char ( accept_char_t )( const char c );

static inline unsigned char is_digit( const char c )
{
    return ( c >= 48 && c <= 57 ) ? 1 : 0;
}

static inline unsigned char is_alpha( const char c )
{
    return ( c >= 65 && c <= 122 ) ? 1 : 0;
}

static inline unsigned char is_header( const char c )
{
    return ( ( c >= 65 && c <= 122 ) || c == 45 ) ? 1 : 0;
}

static inline unsigned char is_any( const char c )
{
    return ( c >= 32 && c <= 122 ) ? 1 : 0;
}

static inline void to_int( xi_stated_sscanf_state_t* s, int* out )
{
    int base = 10;
    *out     = ( s->buffer[ s->buff_len - 1 ] - 48 );

    for( unsigned char j = 1; j < s->buff_len; ++j, base *= 10 )
    {
        *out += base * ( s->buffer[ s->buff_len - j - 1 ] - 48 );
    }
}

static inline unsigned char pass_all( const char c )
{
    ( void ) c;
    return 1;
}


static inline char safe_until_copier(
          unsigned char* dst_i
        , unsigned char* src_i
        , data_descriptor_t* dst
        , const const_data_descriptor_t* src
        , accept_char_t* fun )
{
    while( fun( src->data_ptr[ *src_i ] ) )
    {
        dst->data_ptr[ (*dst_i)++ ] = src->data_ptr[ (*src_i)++ ];

        if( *dst_i == dst->hint_size )
        {
            return -1;
        }

        if( *src_i == src->hint_size )
        {
            return 0; // more data needed
        }
    }

    return 1;
}


short xi_stated_sscanf(
          xi_stated_sscanf_state_t* s
        , const const_data_descriptor_t* pattern
        , const const_data_descriptor_t* source
        , void** variables )
{
    // internal tmp variables
    char res     = 0;

    BEGIN_CORO( s->state )

    s->vi = 0;
    s->p  = 0;

    for( ; s->p < pattern->hint_size - 1; )
    {
        if( pattern->data_ptr[ s->p ] != '%' ) // check on the raw pattern basis one to one
        {
            if( pattern->data_ptr[ s->p ] != source->data_ptr[ s->i ] )
            {
                return -1;
            }
            else
            {
                s->i++;
                s->p++;

                if( s->i == source->hint_size )
                {
                    // accepting should have bigger priority than more data
                    if( s->p == pattern->hint_size - 1 )
                    {
                        goto accept;
                    }
                    else
                    {
                        YIELD( s->state, 0 )
                        s->i = 0;
                        continue;
                    }
                }
            }
        }
        else // parsing state
        {
            // simplified version so don't expect to parse %%
            s->p++;             // let's move the marker to the type or constraint
            s->max_len = 255;   // let's clean the max len

            if( is_digit( pattern->data_ptr[ s->p ] ) )
            {
                s->buff_len = 0;

                {
                    data_descriptor_t dst_desc = { s->buffer, sizeof( s->buffer ), sizeof( s->buffer ) };
                    res = safe_until_copier( &s->buff_len, &s->p, &dst_desc, pattern, &is_digit );
                }

                if( res == -1 )
                {
                    EXIT( s->state, -1 );
                }

                to_int( s, ( int* ) &( s->max_len ) );
            }


            // @TODO think of this implementation and take into concideration
            // an idea of putting more of each case implementation into the common function
            // that would than be reused and make the code smaller
            if( pattern->data_ptr[ s->p ] == 'd' )
            {
                s->buff_len = 0;

                do
                {
                    data_descriptor_t dst_desc = { s->buffer, sizeof( s->buffer ), sizeof( s->buffer ) };
                    res = safe_until_copier( &s->buff_len, &s->i, &dst_desc, source, &is_digit );
                    if( res == 0 )
                    {
                        YIELD( s->state, 0 )
                        s->i = 0;
                    }
                } while( res == 0 );

                to_int( s, variables[ s->vi ] );

                s->p++;     // move on, finished with parsing
                s->vi++;    // switch to the next variable
            }
            else if( pattern->data_ptr[ s->p ] == 's' || pattern->data_ptr[ s->p ] == '.' || pattern->data_ptr[ s->p ] == 'B' )
            {
                s->tmp_i = 0;

                do
                {
                    data_descriptor_t dst_desc = { ( char* ) variables[ s->vi ], s->max_len, s->max_len - 1 };

                    switch( pattern->data_ptr[ s->p ] )
                    {
                        case 's':
                            res = safe_until_copier( &s->tmp_i, &s->i, &dst_desc, source, &is_header );
                            break;
                        case '.':
                            res = safe_until_copier( &s->tmp_i, &s->i, &dst_desc, source, &is_any );
                            break;
                        case 'B':
                            res = safe_until_copier( &s->tmp_i, &s->i, &dst_desc, source, &pass_all );
                            break;
                    }

                    if( res == 0 )
                    {
                        YIELD( s->state, 0 )
                        s->i = 0;
                    }
                } while( res == 0 );

                ( ( char* ) variables[ s->vi ] )[ s->tmp_i ] = '\0'; // put guard

                s->p++;     // move on, finished with parsing
                s->vi++;    // switch to the next variable
            }
        }
    }

accept:
    RESTART( s->state, 1 )

    END_CORO()

    return 1;
}


