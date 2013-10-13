#ifndef __XI_COROUTINE_H__
#define __XI_COROUTINE_H__

// COROUTINE API
#define BEGIN_CORO( state )\
    switch( state )\
    { \
        default:

#define YIELD( state, ret )\
    state = __LINE__; return ret; case __LINE__:

#define EXIT( state, ret )\
    state = 1; return ret;

#define RESTART( state, ret )\
    state = 0; return ret;

#define END_CORO()\
    };

#endif // __XI_COROUTINE_H__
