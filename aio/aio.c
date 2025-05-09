#include "aio.h"
#include <stdio.h>
#include <json-c/arraylist.h>
#include <json-c/json_object.h>
#include <json-c/json_types.h>
#include <json-c/linkhash.h>
#include <stdint.h>
#include <string.h>

#define key(json, name) json_object_object_get(json, name)

void aio_artstyle_to_string(const enum aio_artstyle as, string *out) {
    if ((as & AS_ANIME) == AS_ANIME) {
        string_concat(out, "Anime + ", 8);
    }
    if((as & AS_CARTOON) == AS_CARTOON) {
        string_concat(out, "Cartoon + ", 10);
    }
    if((as & AS_CGI) == AS_CGI) {
        string_concat(out, "CGI + ", 6);
    }
    if((as & AS_DIGITAL) == AS_DIGITAL) {
        string_concat(out, "Digital + ", 10);
    }
    if((as & AS_HANDRAWN) == AS_HANDRAWN) {
        string_concat(out, "Handrawn + ", 11);
    }
    if((as & AS_LIVE_ACTION) == AS_LIVE_ACTION) {
        string_concat(out, "Live action + ", 14);
    }

    if(out->len == 0) {
        return;
    }

    //remove the " +" at the end
    string_slice_suffix(out, 2);
}

void aio_entryi_to_human_str(const struct aio_entryi* entry, string* out) {
    const size_t en_title_len = strlen(entry->en_title);

    #define line(title, value) \
        string_concat(out, title ": ", strlen(value) + 1); \
        string_concat(out, value, strlen(value)); \
        string_concat_char(out, '\n')

    line("Title", entry->en_title);
    line("Native title", entry->native_title);

    #undef line
}

void aio_entryi_parse(const char* json, struct aio_entryi* out) {
    EntryI_json data = json_tokener_parse(json);
    #define set(key, field) aio_entryi_get_key(data, key, &out->field)
    set("ItemId", itemid);
    set("ItemId", itemid);
    set("ArtStyle", art_style);
    set("En_Title", en_title);
    set("Native_Title", native_title);
    set("Format", format);
    set("Location", location);
    set("PurchasePrice", purchase_price);
    set("Collection", collection);
    set("ParentId", parentid);
    set("Type", type);
    set("CopyOf", copyof);
    set("Library", library);
    #undef set
}

int aio_entryi_get_key(EntryI_json info, const char* key, void* out){
    json_object* data = key(info, key);
    switch (json_object_get_type(data)) {
        case json_type_object:
            *(lh_table**)out = json_object_get_object(data);
            return 0;
        case json_type_null:
            *(EntryI_json*)out = NULL;
            return -1;
        case json_type_array:
            *(array_list**)out = json_object_get_array(data);
            return 0;
        case json_type_boolean:
            *(_Bool*)out = json_object_get_boolean(data);
            return 0;
        case json_type_int:
            *(int64_t*)out = json_object_get_int64(data);
            return 0;
        case json_type_double:
            *(double*)out = json_object_get_boolean(data);
            return 0;
        case json_type_string:
            *(const char**)out = json_object_get_string(data);
            return 0;
    }
    return json_object_get_uint64(key(info, "ItemId"));
}


#undef key
