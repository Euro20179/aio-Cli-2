#include <string.h>
#include <curl/curl.h>

#include "globals.h"
#include "inc/string.h"
#include "url.h"

size_t curlWriteCB(char* ptr, size_t size, size_t nmemb, void* userdata) {
    for(int i = 0; i < nmemb; i++) {
        string_concat_char((string*) userdata, ptr[i]);
    }
    return nmemb;
}

CURLcode mkapireq(string* out, const char* endpoint, char* error) {
    char path[strlen(endpoint) + APILEN + 1];
    //set first byte to 0, so that strcat determines the length to be 0
    path[0] = 0;

    mkapipath(path, endpoint);

    CURLcode res =mkreq(out, path, error);

    return res;
}

CURLcode mkreq(string* out, char* path, char* error) {
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

void mkapipath(char* out, const char* endpoint) {
    size_t len = strlen(endpoint);

    strcat(out, api);
    strcat(out, endpoint);
    *(out + len + APILEN + 1) = '\0';
}
