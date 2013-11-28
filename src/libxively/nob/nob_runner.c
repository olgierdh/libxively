#include "nob_runner.h"
#include "layer_api.h"
#include "xi_coroutine.h"

layer_state_t process_xively_nob_step( xi_context_t* xi )
{
    // PRECONDITION
    //assert( xi != 0 );

    static int16_t state                = 0;
    static layer_state_t layer_state    = LAYER_STATE_OK;

    //xi_debug_format("state=%d layer_state=%d", state, layer_state);

    BEGIN_CORO( state )

    //xi_debug_format("state=%d layer_state=%d", state, layer_state);
    // write data to the endpoint
    do
    {
      // context->layer_connection.layer->layer_functions->target()
      //if( xi->layer_chain.top->layer_connection.self == 0 ) xi_debug_logger("top layer not initialiesed?");
      //if( xi->layer_chain.bottom->layer_connection.self == 0 ) xi_debug_logger("bottom layer not initialiesed?");

        layer_state = CALL_ON_SELF_DATA_READY(
                      xi->layer_chain.top
                    , xi->input, LAYER_HINT_NONE );

        //xi_debug_format("state=%d layer_state=%d", state, layer_state);
        if( layer_state == LAYER_STATE_NOT_READY )
        {
            //xi_debug_format("state=%d layer_state=%d", state, layer_state);
            YIELD( state, layer_state );
            //xi_debug_format("state=%d layer_state=%d", state, layer_state);
        }

    } while( layer_state == LAYER_STATE_NOT_READY );

    //xi_debug_format("state=%d layer_state=%d", state, layer_state);
    if( layer_state == LAYER_STATE_ERROR )
    {
        //xi_debug_format("state=%d layer_state=%d", state, layer_state);
        EXIT( state, layer_state);
    }

    YIELD( state, LAYER_STATE_OK );
    //xi_debug_format("state=%d layer_state=%d", state, layer_state);
    // now read the data from the endpoint
    do
    {
        layer_state = CALL_ON_SELF_ON_DATA_READY(
                      xi->layer_chain.bottom
                    , 0, LAYER_HINT_NONE );

        if( layer_state == LAYER_STATE_NOT_READY )
        {
            //xi_debug_format("state=%d layer_state=%d", state, layer_state);
            YIELD( state, layer_state );
            //xi_debug_format("state=%d layer_state=%d", state, layer_state);
        }

    } while( layer_state == LAYER_STATE_NOT_READY );

    //xi_debug_format("state=%d layer_state=%d", state, layer_state);
    EXIT( state, layer_state );

    END_CORO()

    state = 0;
    return 0;
}
