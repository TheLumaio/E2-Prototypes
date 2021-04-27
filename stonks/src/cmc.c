#include "cmc.h"

#define CURL_STATICLIB
#include <curl/curl.h>

#include <raylib.h>

bool cmc_init(const char* api_key)
{
    _api_key = api_key;

    curl_global_init(CURL_GLOBAL_ALL);
    _curl = curl_easy_init();

    // _headers = curl_slist_append(_headers, FormatText("X-CMC_PRO_API_KEY: %s", api_key));

    return _curl!=NULL;
}

coin_t* cmc_get_coin(const char* slug)
{
    curl_easy_setopt(_curl, CURLOPT_URL, "");
}

char* cmc_remote(const char* uri)
{
    curl_easy_setopt(_curl, CURLOPT_URL, FormatText("url%s", uri));
    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(_curl, CURLOPT_USERAGENT, "curl/7.42.0");
    curl_easy_setopt(_curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(_curl, CURLOPT_TCP_KEEPALIVE, 1L);
    
    curl_easy_perform(_curl);
}
