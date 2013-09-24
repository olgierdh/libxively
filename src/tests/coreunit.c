// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.
#include "tinytest.h"
#include "tinytest_macros.h"

#include "xively.h"
#include "csv_data.h"
#include "xi_err.h"
#include "http_layer_parser.h"
#include "http_layer_queries.h"
#include "xi_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
// HTTP PARSER TESTS
///////////////////////////////////////////////////////////////////////////////

// decl
const char* parse_http_status( http_response_t* response, const char* content );

void test_parse_http_status(void* data)
{
    (void)(data);

    // prepare structure
    http_response_t response;
    memset( &response, 0, sizeof( http_response_t ) );

    // simplest
    {
        const char test_status1[] =
            "HTTP/1.1 200 OK\r\n"
            "Date: Sun, 14 Apr 2013 19:32:40 GMT\r\n";

        const char* ret = parse_http_status( &response, test_status1 );

        tt_assert( ret != 0 );
        tt_assert( response.http_version1 == 1 );
        tt_assert( response.http_version2 == 1 );
        tt_assert( response.http_status == 200 );
        tt_assert( strcmp( response.http_status_string, "OK" ) == 0 );
        tt_assert( ret == test_status1 + 17 );

    }

    // simplest2
    {
        const char test_status1[] =
            "HTTP/1.1 404 Not Found\r\n"
            "Date: Sun, 14 Apr 2013 19:32:40 GMT\r\n";

        const char* ret = parse_http_status( &response, test_status1 );

        tt_assert( ret != 0 );
        tt_assert( response.http_version1 == 1 );
        tt_assert( response.http_version2 == 1 );
        tt_assert( response.http_status == 404 );
        tt_assert( strcmp( response.http_status_string, "Not Found" ) == 0 );
        tt_assert( ret == test_status1 + 24 );
    }


    // clean
    memset( &response, 0, sizeof( http_response_t ) );

    // malformed
    {
        const char test_status1[] =
            "HTTP1.1 200 OK\r\n"
            "Date: Sun, 14 Apr 2013 19:32:40 GMT\r\n";

        const char* ret = parse_http_status( &response, test_status1 );

        tt_assert( ret == 0 );
        tt_assert( xi_get_last_error() == XI_HTTP_STATUS_PARSE_ERROR );
    }

    // clean
    xi_set_err( XI_NO_ERR );
    memset( &response, 0, sizeof( http_response_t ) );

    // malformed2
    {
        const char test_status1[] =
            "HTTP1.1 200 OK";

        const char* ret = parse_http_status( &response, test_status1 );

        tt_assert( ret == 0 );
        tt_assert( xi_get_last_error() == XI_HTTP_STATUS_PARSE_ERROR );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

// decl
const char* parse_http_header( http_response_t* response
    , const char* content );

void test_parse_http_header(void *data)
{
    (void)(data);

    // prepare structure
    http_response_t response;
    memset( &response, 0, sizeof( http_response_t ) );

    // simplest
    {
        const char test_header1[] =
            "Date: Sun, 14 Apr 2013 19:32:40 GMT\r\n";

        const char* ret = parse_http_header( &response, test_header1 );

        tt_assert( ret != 0 );
        tt_assert( response.http_headers_size == 1 );
        tt_assert( response.http_headers[ 0 ].header_type == XI_HTTP_HEADER_DATE );
        tt_assert( strcmp( response.http_headers[ 0 ].name, "Date" ) == 0 );
        tt_assert( strcmp( response.http_headers[ 0 ].value, "Sun, 14 Apr 2013 19:32:40 GMT" ) == 0 );
        tt_assert( response.http_headers_checklist[ ( size_t ) XI_HTTP_HEADER_DATE ] == &response.http_headers[ 0 ] );
    }

    memset( &response, 0, sizeof( http_response_t ) );
    xi_set_err( XI_NO_ERR );

    // simplest2
    {
        const char test_header1[] =
            "Content-Length: 0\r\n";

        const char* ret = parse_http_header( &response, test_header1 );

        tt_assert( ret != 0 );
        tt_assert( response.http_headers_size == 1 );
        tt_assert( response.http_headers[ 0 ].header_type == XI_HTTP_HEADER_CONTENT_LENGTH );
        tt_assert( strcmp( response.http_headers[ 0 ].name, "Content-Length" ) == 0 );
        tt_assert( strcmp( response.http_headers[ 0 ].value, "0" ) == 0 );
        tt_assert( response.http_headers_checklist[ ( size_t ) XI_HTTP_HEADER_CONTENT_LENGTH ] == &response.http_headers[ 0 ] );
    }

    memset( &response, 0, sizeof( http_response_t ) );
    xi_set_err( XI_NO_ERR );

    // simplest3
    {
        const char test_header1[] =
            "Imaginary-Header: 0\r\n";

        const char* ret = parse_http_header( &response, test_header1 );

        tt_assert( ret != 0 );
        tt_assert( response.http_headers_size == 1 );
        tt_assert( response.http_headers[ 0 ].header_type == XI_HTTP_HEADER_UNKNOWN );
        tt_assert( strcmp( response.http_headers[ 0 ].name, "Imaginary-Header" ) == 0 );
        tt_assert( strcmp( response.http_headers[ 0 ].value, "0" ) == 0 );
    }

    memset( &response, 0, sizeof( http_response_t ) );
    xi_set_err( XI_NO_ERR );

    // malformed
    {
        const char test_header1[] =
            "Date Sun, 14 Apr 2013 19-32-40 GMT\r\n";

        const char* ret = parse_http_header( &response, test_header1 );

        tt_assert( ret == 0 );
        tt_assert( xi_get_last_error() == XI_HTTP_HEADER_PARSE_ERROR );
    }

    memset( &response, 0, sizeof( http_response_t ) );
    xi_set_err( XI_NO_ERR );

    // malformed
    {
        const char test_header1[] =
            "Date: Sun, 14 Apr 2013 19:32:40 GMT";

        const char* ret = parse_http_header( &response, test_header1 );

        tt_assert( ret == 0 );
        tt_assert( xi_get_last_error() == XI_HTTP_HEADER_PARSE_ERROR );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_parse_http(void* data)
{
    (void)(data);

    // prepare structure
    http_response_t response;
    memset( &response, 0, sizeof( http_response_t ) );

    // simplest
    {
        const char test_response[] =
            "HTTP/1.1 200 OK\r\n"
            "Date: Sun, 14 Apr 2013 20:20:01 GMT\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Content-Length: 0\r\n"
            "Connection: keep-alive\r\n"
            "X-Request-Id: 9de9ac7071bdbf105804ce0eaf7979adc77f6e25\r\n"
            "Cache-Control: max-age=0\r\n"
            "Vary: Accept-Encoding\r\n\r\n";

        http_response_t* ret = parse_http( &response, test_response );

        tt_assert( ret != 0 );
        tt_assert( ret->http_headers_size == 7 );
    }

    memset( &response, 0, sizeof( http_response_t ) );
    xi_set_err( XI_NO_ERR );

    // with content
    {
        const char test_response[] =
            "HTTP/1.1 404 Not Found\r\n"
            "Date: Sun, 14 Apr 2013 20:20:01 GMT\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Content-Length: 9\r\n"
            "Connection: keep-alive\r\n"
            "Vary: Accept-Encoding\r\n\r\n"
            "Not Found";

        http_response_t* ret = parse_http( &response, test_response );

        tt_assert( ret != 0 );
        tt_assert( ret->http_headers_size == 5 );
        tt_assert( strcmp( ret->http_content, "Not Found" ) == 0 );
    }

    memset( &response, 0, sizeof( http_response_t ) );
    xi_set_err( XI_NO_ERR );

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

///////////////////////////////////////////////////////////////////////////////
// HTTP CONSTRUCT TEST
///////////////////////////////////////////////////////////////////////////////

void test_http_construct_request(void *data)
{
    (void)(data);

    // simple test
    {
        const char expected[] =
            "GET /v2/feeds/128/datastreams/test.csv HTTP/1.1\r\n"
            "Host: " XI_HOST "\r\n"
            "User-Agent: " XI_USER_AGENT "\r\n"
            "Accept: */*\r\n"
            "X-ApiKey: apikey\r\n";

        xi_feed_id_t feed_id = 128;
        const char* ret = http_construct_request_datastream( "GET", &feed_id, "test", "apikey" );
        tt_assert( strcmp( expected, ret ) == 0 );
    }

    // simple test2
    {
        const char expected[] =
            "GET /v2/feeds/128/datastreams.csv HTTP/1.1\r\n"
            "Host: " XI_HOST "\r\n"
            "User-Agent: " XI_USER_AGENT "\r\n"
            "Accept: */*\r\n"
            "X-ApiKey: apikey\r\n";

        xi_feed_id_t feed_id = 128;
        const char* ret = http_construct_request_datastream( "GET", &feed_id, 0, "apikey" );
        tt_assert( strcmp( expected, ret ) == 0 );
    }

 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_http_construct_content(void *data)
{
    (void)(data);

    // simple test
    {
        const char expected[] =
            "Content-Type: text/plain\r\n"
            "Content-Length: 128\r\n";

        const char* ret = http_construct_content( 128 );
        tt_assert( strcmp( expected, ret ) == 0 );
    }

 end:
    xi_set_err( XI_NO_ERR );
    ;
}

///////////////////////////////////////////////////////////////////////////////
// CSV TESTS
///////////////////////////////////////////////////////////////////////////////

void test_csv_decode_datapoint(void *data)
{
    (void)(data);

    // prepare the data
    xi_datapoint_t data_point;
    memset( &data_point, 0, sizeof( xi_datapoint_t ) );

    // data
    const char test_data[] = "2013-01-01T18:44:21.423452Z,216";

    // test
    const xi_datapoint_t* o = csv_decode_datapoint( test_data, &data_point );

    // test
    tt_assert( o != 0 );
    tt_assert( data_point.value.i32_value == 216 );
    tt_assert( data_point.timestamp.micro == 423452 );


    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_csv_decode_datapoint_error( void * data )
{
    (void)(data);

    // prepare the data
    xi_datapoint_t data_point;
    memset( &data_point, 0, sizeof( xi_datapoint_t ) );

    { // test case broken data
        // prepare data
        const char test_data[] = "213-01-01T18:44:21.423452Z,216";

        // call function
        const xi_datapoint_t* o = csv_decode_datapoint( test_data, &data_point );

        // test values
        tt_assert( o == 0 );
        tt_assert( XI_CSV_TIME_CONVERTION_ERROR == xi_get_last_error() );
    }

    { // test case wrong content
        // prepare data
        const char test_data[] = "2013-01-01T18:44:21,216";

        // call function
        const xi_datapoint_t* o = csv_decode_datapoint( test_data, &data_point );

        // test values
        tt_assert( o == 0 );
        tt_assert( XI_CSV_DECODE_DATAPOINT_PARSER_ERROR == xi_get_last_error() );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_csv_encode_create_datastream( void* data )
{
    (void)(data);

    // prepare the data
    xi_datapoint_t data_point;
    memset( &data_point, 0, sizeof( xi_datapoint_t ) );

    // data
    const char datastream_id[] = "test_datastream";

    // test
    {
        data_point.value.i32_value = 216;
        const char* o = csv_encode_create_datastream(
            datastream_id, &data_point );

        // test values
        tt_assert( o != 0 );
        tt_assert( strcmp( o, "test_datastream,216\n" ) == 0 );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_csv_encode_create_datastream_error( void* data )
{
    (void)(data);

    // prepare the data
    xi_datapoint_t data_point;
    memset( &data_point, 0, sizeof( xi_datapoint_t ) );

    // data
    const char datastream_id[] =
        "abcdefghabcdefghabcdefghabcdefgh" // 32 * 8 + 1 = 257 should exceed the capacity
        "abcdefghabcdefghabcdefghabcdefgh"
        "abcdefghabcdefghabcdefghabcdefgh"
        "abcdefghabcdefghabcdefghabcdefgh"
        "abcdefghabcdefghabcdefghabcdefgh"
        "abcdefghabcdefghabcdefghabcdefgh"
        "abcdefghabcdefghabcdefghabcdefgh"
        "abcdefghabcdefghabcdefghabcdefgh"
        "a"; // 1 additional

    // test
    {
        data_point.value.i32_value = 216;
        const char* o = csv_encode_create_datastream(
            datastream_id, &data_point );

        // test values
        tt_assert( o == 0 );
        tt_assert( XI_CSV_ENCODE_DATASTREAM_BUFFER_OVERRUN == xi_get_last_error() );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_csv_encode_datapoint( void* data )
{
    (void)(data);

    // prepare the data
    xi_datapoint_t data_point;
    memset( &data_point, 0, sizeof( xi_datapoint_t ) );

    // data
    // const char datastream_id[] = "test_datastream";

    // test
    {
        data_point.value.i32_value = 216;
        const char* o = csv_encode_datapoint( &data_point );

        // test values
        tt_assert( o != 0 );
        tt_assert( strcmp( o, "216\n" ) == 0 );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_set_err( XI_NO_ERR );
    ;
}

void test_helpers_copy_until( void* data )
{
    (void)(data);

    const int buffer_size         = 64;
    const int buffer_size_small     = 2;

    // simple test
    {
        const char src[] = "test of some string";

        char dst[ buffer_size ];
        memset( dst, 0, sizeof( dst ) );

        int s = xi_str_copy_untiln( dst, buffer_size, src, ' ' );

        tt_assert( strcmp( dst, "test" ) == 0 );
        tt_assert( s == 4 );
    }

    // test not found
    {
        const char src[] = "test of some string";

        char dst[ buffer_size ];
        memset( dst, 0, sizeof( dst ) );

        int s = xi_str_copy_untiln( dst, buffer_size, src, '0' );

        tt_assert( strcmp( dst, "test of some string" ) == 0 );
        tt_assert( s == sizeof( src ) - 1 );
    }

    // test capacity exceeded
    {
        const char src[] = "test of some string";

        char dst[ buffer_size_small ];
        memset( dst, 0, sizeof( dst ) );

        int s = xi_str_copy_untiln( dst, buffer_size_small, src, '0' );

        tt_assert( strcmp( dst, "t" ) == 0 );
        tt_assert( s == buffer_size_small - 1 );
    }

 end:
    ;
}

//decl

extern xi_datapoint_t* csv_decode_value(
    const char* buffer, xi_datapoint_t* p );

void test_helpers_decode_value( void* data )
{
    (void)(data);

    const char test_string[] = "test string just to test";
    const char test_string2[] = "123123";
    const char test_string3[] = "-321123";
    const char test_string4[] = "123.123";
    const char test_string5[] = ".123";
    const char test_string6[] = "-.123";
    xi_datapoint_t p;

    csv_decode_value( test_string, &p );
    tt_assert( p.value_type = XI_VALUE_TYPE_STR );
    tt_assert( strcmp( p.value.str_value, test_string ) == 0 );

    csv_decode_value( test_string2, &p );
    tt_assert( p.value_type         == XI_VALUE_TYPE_I32 );
    tt_assert( p.value.i32_value     == 123123 );

    csv_decode_value( test_string3, &p );
    tt_assert( p.value_type         == XI_VALUE_TYPE_I32 );
    tt_assert( p.value.i32_value     == -321123 );

    csv_decode_value( test_string4, &p );
    tt_assert( p.value_type         == XI_VALUE_TYPE_F32 );
    tt_assert( p.value.f32_value     == 123.123f );

    csv_decode_value( test_string5, &p );
    tt_assert( p.value_type         == XI_VALUE_TYPE_F32 );
    tt_assert( p.value.f32_value     == .123f );

    csv_decode_value( test_string6, &p );
    tt_assert( p.value_type         == XI_VALUE_TYPE_F32 );
    tt_assert( p.value.f32_value     == -.123f );

 end:
    ;
}

void test_create_and_delete_context(void* data)
{
  (void)(data);

#define TEST_API_KEY_STRING "abcXYZ2398138172jckajf39djas_dasd-TyX"
#define TEST_FEED_ID_NUMBER 123456

  xi_context_t* xi_context
      = xi_create_context( XI_HTTP
              , TEST_API_KEY_STRING
              , TEST_FEED_ID_NUMBER );

  tt_assert( xi_context != 0 );
  tt_assert( strcmp( xi_context->api_key, TEST_API_KEY_STRING ) == 0 );
  tt_assert( xi_context->feed_id == TEST_FEED_ID_NUMBER );

  xi_delete_context( xi_context );

end:
   xi_set_err( XI_NO_ERR );
   ;
}

void test_datapoint_value_setters_and_getters(void* data)
{
  (void)(data);

  void * ret;

  xi_datapoint_t dp;

  ret = xi_set_value_i32( &dp, 320 );

  tt_assert( ret != 0 );

  tt_assert( xi_get_value_i32( &dp ) == 320 );

  tt_assert( xi_get_value_type( &dp ) == XI_VALUE_TYPE_I32 );

  tt_assert( xi_value_pointer_i32( &dp ) != 0 &&
             xi_value_pointer_f32( &dp ) == 0 &&
             xi_value_pointer_str( &dp ) == 0 );

  ret = xi_set_value_f32( &dp, (320.0/10) );

  tt_assert( ret != 0 );

  tt_assert( xi_get_value_f32( &dp ) == (320.0/10) );

  tt_assert( xi_get_value_type( &dp ) == XI_VALUE_TYPE_F32 );

  tt_assert( xi_value_pointer_i32( &dp ) == 0 &&
             xi_value_pointer_f32( &dp ) != 0 &&
             xi_value_pointer_str( &dp ) == 0 );

  ret = xi_set_value_str( &dp, "Let's test everything!" );

  tt_assert( ret != 0 );

  char s[XI_VALUE_STRING_MAX_SIZE+100];
  for (size_t i = 0; i < sizeof(s); i++) {
    s[i] = 'a';
  }

  ret = xi_set_value_str( &dp, s );

  tt_assert( ret != 0 )

  tt_assert( xi_get_value_type( &dp ) == XI_VALUE_TYPE_STR );

  tt_assert( xi_value_pointer_i32( &dp ) == 0 &&
             xi_value_pointer_f32( &dp ) == 0 &&
             xi_value_pointer_str( &dp ) != 0 );

  char * x = xi_value_pointer_str( &dp );

  tt_assert( strlen(s) > strlen(x) );
  tt_assert( strlen(x) == XI_VALUE_STRING_MAX_SIZE-1 );

  s[XI_VALUE_STRING_MAX_SIZE-5] = '\0';

  ret = xi_set_value_str( &dp, s );

  tt_assert( ret != 0 );

  tt_assert( strlen(s) == strlen(x) );
  tt_assert( strlen(x) == XI_VALUE_STRING_MAX_SIZE-5 );

end:
   ;
}


struct testcase_t demo_tests[] = {
    /* Here's a really simple test: it has a name you can refer to it
       with, and a function to invoke it. */
    { "test_parse_http_status", test_parse_http_status, TT_ENABLED_, 0, 0 },
    { "test_parse_http_header", test_parse_http_header, TT_ENABLED_, 0, 0 },
    { "test_parse_http", test_parse_http, TT_ENABLED_, 0, 0 },

    { "test_http_construct_request", test_http_construct_request, TT_ENABLED_, 0, 0 },
    { "test_http_construct_content", test_http_construct_content, TT_ENABLED_, 0, 0 },

    { "test_csv_decode_datapoint", test_csv_decode_datapoint, TT_ENABLED_, 0, 0 },
    { "test_csv_decode_datapoint_error", test_csv_decode_datapoint_error, TT_ENABLED_, 0, 0 },
    { "test_csv_encode_create_datastream", test_csv_encode_create_datastream, TT_ENABLED_, 0, 0 },
    { "test_csv_encode_create_datastream_error", test_csv_encode_create_datastream_error, TT_ENABLED_, 0, 0 },
    { "test_csv_encode_datapoint", test_csv_encode_datapoint, TT_ENABLED_, 0, 0 },

    { "test_helpers_copy_until", test_helpers_copy_until, TT_ENABLED_, 0, 0 },
    { "test_helpers_decode_value", test_helpers_decode_value, TT_ENABLED_, 0, 0 },

    { "test_create_and_delete_context", test_create_and_delete_context, TT_ENABLED_, 0, 0 },
    { "test_datapoint_value_setters_and_getters", test_datapoint_value_setters_and_getters, TT_ENABLED_, 0, 0 },
    /* The array has to end with END_OF_TESTCASES. */
    END_OF_TESTCASES
};

/* Next, we make an array of testgroups.  This is mandatory.  Unlike more
   heavy-duty testing frameworks, groups can't nest. */
struct testgroup_t groups[] = {

    /* Every group has a 'prefix', and an array of tests.  That's it. */
    { "csv/", demo_tests },

    END_OF_GROUPS
};

#if XI_UNIT_TEST_NATIVE
int main( int argc, char const *argv[] )
{
  return tinytest_main( argc, argv, groups );
}
#else
int main() 
{
  const char a[] = {"avr"};
  return tinytest_main(1, (const char **)a, groups);
}
#endif
