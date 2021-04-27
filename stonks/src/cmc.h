#ifndef CMC_H
#define CMC_H

// coinmarketcap helper api
#include <stdio.h>
#include <stdbool.h>

    #define CloseWindow CloseWindow_orig
    #define ShowWindow ShowWindow_orig
    #define Rectangle Rectangle_orig
    #define ShowCursor ShowCursor_orig
    #define LoadImage LoadImage_orig
    #define DrawText DrawText_orig
    #define DrawTextEx DrawTextEx_orig
    #define PlaySound PlaySound_orig
#include <curl/curl.h>
    #undef CloseWindow
    #undef ShowWindow
    #undef Rectangle
    #undef ShowCursor
    #undef LoadImage
    #undef DrawText
    #undef DrawTextEx
    #undef PlaySound

typedef struct coin_t
{

} coin_t;

static char* _api_key = NULL;
static CURL* _curl = NULL;
static CURLcode _res;
static struct curl_slist* _headers = NULL;

bool cmc_init(const char* api_key);
void cmc_cleanup();
coin_t* cmc_get_coin(const char* slug);

char* cmc_remote(const char* uri);

#endif
