#pragma once

#include <curl/curl.h>
#include <stddef.h>

#include "c-stdlib/string.h"

///makes a request to path, putting the body into out, and curl error into error
CURLcode mkreq(string* out, const char* path, char* error);
