#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void string_new(string* strout, size_t len) {
    strout->allocated = len;
    strout->len = 0;

    strout->data = calloc(len, 1);

    strout->initialized = true;
}

void string_del(string* str) {
    if(str->initialized != true) return;
    str->initialized = 0;
    str->len = 0;
    str->allocated = 0;
    free(str->data);
}

void string_concat(string* str, const char* text, size_t textlen) {
    if(str->initialized != true) {
        return;
    }
    if(textlen + str->len > str->allocated) {
        string_extend(str, textlen);
    }

    memcpy(str->data + str->len, text, textlen);
    str->len += textlen;
}

void string2_concat(string* to, string* from) {
    if(from->len + to->len > to->allocated) {
        string_extend(to, from->len);
    }

    memcpy(to->data + to->len, from->data, from->len);

    to->len += from->len;
}

void string_set(string* str, const char* text, size_t n) {
    if(str->initialized != true) return;

    if(n > str->allocated) {
        n = str->allocated;
    }

    str->len = n;

    for(int i = 0; i < n; i++) {
        str->data[i] = text[i];
    }
}

void string_set_char_at(string* str, char c, int idx) {
    if(idx > str->allocated) return;
    if(idx >= str->len) {
        str->len = idx + 1;
    }
    str->data[idx] = c;
}

void string_concat_char(string *str, char c) {
    if(str->len >= str->allocated) {
        string_extend(str, str->allocated || 1);
    }
    str->data[str->len] = c;
    str->len++;
}

void string_extend(string *str, size_t amount) {
    if(str->initialized != true) return;
    str->allocated += amount;
    str->data = realloc(str->data, str->allocated);
}


void string_to_cstr(string *str, char *out) {
    if(str->initialized != true || str->len < 1) {
        out[0] = 0;
        return;
    }
    memcpy(out, str->data, str->len);
    out[str->len] = 0;
}

char* string_mkcstr(string *str) {
    string_concat_char(str, '\0');
    return str->data;
}

void string_clear(string* str) {
    str->len = 0;
}

void string_copy(string* str, string* ostring) {
    ostring->allocated = str->allocated;
    ostring->initialized = str->initialized;
    ostring->len = str->len;
    if(str->initialized) {
        *(ostring->data) = *(str->data);
        // ostring->data = str->data;
        // memcpy(ostring->data, str->data, str->len);
    }
}

char string_at(string* str, size_t pos) {
    if(str->initialized != true || pos >= str->len) {
        return 0;
    }
    return str->data[pos];
}

void string_split(string* str, char sep, void* userdata, void(cb)(string*, size_t, void*)) {
    string cur;
    string_new(&cur, 0);

    size_t count = 0;

    for(size_t i = 0; i < str->len; i++) {
        char ch = string_at(str, i);
        if(ch == sep) {
            cb(&cur, count++, userdata);
            string_clear(&cur);
            cur.len = 0;
        } else {
            string_concat_char(&cur, ch);
        }
    }

    if(cur.len > 0) {
        cb(&cur, count, userdata);
    }

    string_del(&cur);
}

void string_slice_suffix(string * str, size_t amount) {
    str->len -= amount;
}

size_t string_len(string * str) {
    return str->len;
}

void string_tr(string * str, char from, char to) {
    for(size_t i = 0; i < str->len; i++) {
        if (string_at(str, i) == from) {
            string_set_char_at(str, to, i);
        }
    }
}

void string_cpy(string* to, string* from) {
    if(to->allocated < from->allocated) {
        string_extend(to, from->len);
    }
    to->len = from->len;
    memcpy(to->data, from->data, from->len);
}
