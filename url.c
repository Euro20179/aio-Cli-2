#include <string.h>
#include <curl/curl.h>

#include "globals.h"
#include "inc/string.h"
#include "aio/aio.h"
#include "url.h"

CURLcode mkapireq(string* out, const char* endpoint, char* error) {
    char path[strlen(endpoint) + APILEN + 1];
    //set first byte to 0, so that strcat determines the length to be 0
    path[0] = 0;

    mkapipath(path, endpoint);

    CURLcode res =mkreq(out, path, error);

    return res;
}

void mkapipath(char* out, const char* endpoint) {
    size_t len = strlen(endpoint);

    strcat(out, api);
    strcat(out, endpoint);
    *(out + len + APILEN + 1) = '\0';
}
