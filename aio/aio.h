#pragma once
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <stdint.h>

#include "../inc/string.h"

//general info for an entry
typedef json_object* EntryI_json;
//user viewing info for an entry
typedef json_object* UserInfo_json;
//metadata info for an entry
typedef json_object* MetaInfo_json;
//an array of aio events
typedef json_object* Events_json;

typedef char* MediaType;

typedef int64_t aioid_t;


enum aio_artstyle : int64_t{
    AS_ANIME = 1,
    AS_CARTOON = 2,
    AS_HANDRAWN = 4,
    AS_DIGITAL = 8,
    AS_CGI = 16,
    AS_LIVE_ACTION = 32,
};

enum aio_entryformat : uint32_t {
    F_VHS,
    F_CD,
    F_DVD,
    F_BLURAY,
    F_4KBLURAY,
    F_MANGA,
    F_BOOK,
    F_DIGITAL,
    F_BOARDGAME,
    F_STEAM,
    F_NIN_SWITCH,
    F_XBOXONE,
    F_XBOX360,
    F_OTHER,
    F_VINYL,
    F_IMAGE,
    F_UNOWNED,

    //bit modifier
    F_MOD_DIGITAL = 0x1000
};

struct aio_entryi{
    aioid_t itemid;
    const char* en_title;
    const char* native_title;
    const char* location;
    double purchase_price;
    aioid_t parentid;
    const char* type;
    enum aio_artstyle art_style;
    aioid_t copyof;
    aioid_t library;
    enum aio_entryformat format;
    const char* collection;
};

void aio_artstyle_to_string(const enum aio_artstyle, string* out);

//returns -1 if json is not parsable
int aio_entryi_parse(const char* json, struct aio_entryi* out);

int aio_entryi_get_key(EntryI_json info, const char* key, void* out);

void aio_entryi_to_human_str(const struct aio_entryi* entry, string* out);

void aio_id_to_string(const aioid_t, string*);
