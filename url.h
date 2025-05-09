#pragma once

#include <stddef.h>

#include "inc/string.h"

//Create a full api path using the global api variable as a base, and puts the result in out
//WARNING: This function **SHOULD NOT** be given ANY input that was not generated at compile time.
void mkapipath(char* out, const char* endpoint);


void mkapireq(string* out, const char* endpoint);

//makes a request and puts the text output into out
void mkreq(string* out, char* path);
