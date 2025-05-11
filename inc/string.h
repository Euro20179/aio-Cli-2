#pragma once

#include <stdbool.h>
#include <stddef.h>

//creates a stack allocated char buffer for string that is the correct size for string_to_cstr
#define string_to_cstr_buf_create(name, str) char name[str.len + 1]

typedef struct{
    size_t len;
    char* data;
    size_t allocated;
    bool initialized;
} string;

///creates a new string
///set len to 0 or NULL, if you are unsure of the strings length
///`string_del` must be called to free the string's memory
void string_new(string* strout, size_t len);

///Deletes string
void string_del(string* str);

///sets the string to text
///*DOES NOT* allocate more memory if text is larger than str's remaining space
///to get more space use `string_extend`
void string_set(string* str, const char* text, size_t n);

void string_cpy(string* to, string* from);

void string_concat_char(string* str, char c);

//allocates amount more bytes of memory
void string_extend(string* str, size_t amount);

//appends text of textlen length to str (allocating more memory if needed)
void string_concat(string* str, const char* text, size_t textlen);

//concatinates 2 strings
void string2_concat(string*, string*);

void string_to_cstr(string* str, char* out);

void string_set_char_at(string*, char, int);

///Sets the string to an empty string
///DOES NOT FREE memory
void string_clear(string* str);

void string_copy(string* str, string* ostring);

//Gets a character at pos
//Returns 0 if pos >= len or the string is not initialized
char string_at(string* str, size_t pos);

//splits a string by sep, every time sep is found, cb is called with the current string
//the current string is one of:
//* the string found between the beginning and sep,
//* the string between 2 seps,
//* the string between sep and the end
//the cb will receive the current string, and the count
//count is the number of seps found (starting at 0)
//userdata will be passed as the last arg to cb
void string_split(string*, char sep, void* userdata, void(cb)(string*, size_t count, void*));

void string_slice_suffix(string*, size_t amount);

//adds a NULL terminator to the end of str, then returns the raw data, effectively this makes a cstring
char* string_mkcstr(string* str);

//translate any occurance of from to to
void string_tr(string*, char from, char to);
