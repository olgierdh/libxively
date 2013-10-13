#include "xi_stated_sscanf.h"
#include "xi_coroutine.h"

short xi_stated_sscanf(
          xi_stated_sscanf_state_t* s
        , const const_data_descriptor_t* pattern
        , const const_data_descriptor_t* source
        , void** variables )
{
    // internal tmp variables
    int*  value  = 0;
    char* svalue = 0;
    int   base   = 0;

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
            s->p++; // let's move the marker to the type

            // @TODO think of this implementation and take into concideration
            // an idea of putting more of each case implementation into the common function
            // that would than be reused and make the code smaller
            if( pattern->data_ptr[ s->p ] == 'd' )
            {
                s->buff_len = 0;

                while( source->data_ptr[ s->i ] >= 48 && source->data_ptr[ s->i ] <= 57 )
                {
                    s->buffer[ s->buff_len++ ] = source->data_ptr[ s->i++ ];

                    if( s->i == source->hint_size )
                    {
                        YIELD( s->state, 0 )
                        s->i = 0;
                    }
                }

                base      = 10;
                value     = ( int* ) variables[ s->vi ];
                *value    = ( s->buffer[ s->buff_len - 1 ] - 48 );

                for( unsigned char j = 1; j < s->buff_len; ++j, base *= 10 )
                { *value += base * ( s->buffer[ s->buff_len - j - 1 ] - 48 ); }

                s->p++;     // move on, finished with parsing
                s->vi++;    // switch to the next variable
            }
            else if( pattern->data_ptr[ s->p ] == 's' )
            {
                s->tmp_i = 0;

                svalue = ( char* ) variables[ s->vi ];

                while( ( source->data_ptr[ s->i ] >= 65 && source->data_ptr[ s->i ] <= 122 ) || source->data_ptr[ s->i ] == 45 )
                {
                    svalue[ s->tmp_i++ ] = source->data_ptr[ s->i++ ];

                    if( s->i == source->hint_size )
                    {
                        YIELD( s->state, 0 )
                        svalue = ( char* ) variables[ s->vi ];
                        s->i = 0;
                    }
                }

                svalue[ s->tmp_i ] = '\0'; // put guard

                s->p++;     // move on, finished with parsing
                s->vi++;    // switch to the next variable
            }
            else if( pattern->data_ptr[ s->p ] == '.' )
            {
                s->tmp_i = 0;

                svalue = ( char* ) variables[ s->vi ];

                while( source->data_ptr[ s->i ] >= 32 && source->data_ptr[ s->i ] <= 122 )
                {
                    svalue[ s->tmp_i++ ] = source->data_ptr[ s->i++ ];

                    if( s->i == source->hint_size )
                    {
                        YIELD( s->state, 0 )
                        svalue = ( char* ) variables[ s->vi ];
                        s->i = 0;
                    }
                }

                svalue[ s->tmp_i ] = '\0'; // put guard

                s->p++;     // move on, finished with parsing
                s->vi++;    // switch to the next variable
            }
            else if( pattern->data_ptr[ s->p ] == 'B' )
            {
                s->tmp_i = 0;

                svalue = ( char* ) variables[ s->vi ];

                while( source->data_ptr[ s->i ] >= 0 && source->data_ptr[ s->i ] <= 127 )
                {
                    svalue[ s->tmp_i++ ] = source->data_ptr[ s->i++ ];

                    if( s->i == source->hint_size )
                    {
                        YIELD( s->state, 0 )
                        svalue = ( char* ) variables[ s->vi ];
                        s->i = 0;
                    }
                }

                svalue[ s->tmp_i ] = '\0'; // put guard

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


