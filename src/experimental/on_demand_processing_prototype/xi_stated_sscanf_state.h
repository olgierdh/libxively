#ifndef __XI_STATED_SSCANF_STATE_H__
#define __XI_STATED_SSCANF_STATE_H__

typedef struct
{
    short state;
    char buffer[ 8 ];
    unsigned char p;
    unsigned char i;
    unsigned char buff_len;
    unsigned char tmp_i;
    unsigned char vi;
} xi_stated_sscanf_state_t;


#endif // __XI_STATED_SSCANF_STATE_H__
