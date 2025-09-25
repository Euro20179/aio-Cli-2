#pragma once

#include <curl/curl.h>
#include <stddef.h>

#include "c-stdlib/string.h"

//Create a full api path using the global api variable as a base, and puts the result in out
//WARNING: This function **SHOULD NOT** be given ANY input that was not generated at compile time.
void mkapipath(char* out, const char* endpoint);


//error must be CURL_ERROR_SIZE large
CURLcode mkapireq(string* out, const char* endpoint, char* error);
