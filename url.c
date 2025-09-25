#include <string.h>
#include <curl/curl.h>

#include "globals.h"
#include "c-stdlib/string.h"
#include "url.h"

size_t curlWriteCB(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    for (int i = 0; i < nmemb; i++) {
        string_concat_char((string*)userdata, ptr[i]);
    }
    return nmemb;
}

CURLcode mkreq(string* out, const char* path, char* error)
{
    CURL* curl = curl_easy_init();
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, path);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCB);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return res;
}
