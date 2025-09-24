/*

                                = REQUIREMENTS =

    - json-c

    ---------------------------------------------------------------------------

                                = Description = 

    This is the aio c library.

    Generally this library should provide all functions that make interfacing
    with the aio easy and simple.

                                   = USAGE =

    == Startup ==

    call aio_init() at the start
    and aio_shutdown() whenever you would like to shutdown aio

    aio_init() and shutdown() can be called multiple times,
    but aio_init() must be called first every time


    == General usage ==

    aio contains state of which items are loaded

    there are many ways to load items.

*/

#pragma once
#include <curl/curl.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <stdint.h>

#include "../inc/string.h"
#include "../inc/hashmap.h"
#include "../inc/array.h"

// general info for an entry
typedef json_object* EntryI_json;
// user viewing info for an entry
typedef json_object* UserInfo_json;
// metadata info for an entry
typedef json_object* MetaInfo_json;
// an array of aio events
typedef json_object* Events_json;

typedef json_object* Entry_json;

typedef char* MediaType;

typedef int64_t aioid_t;


//This is where all the stuff gets stored and retrieved from
static hashmap info;
static hashmap meta;
static hashmap user;

static array* itemids;

enum aio_artstyle : uint64_t {
    AS_ANIME = 1,
    AS_CARTOON = 1 << 1,
    AS_HANDRAWN = 1 << 2,
    AS_DIGITAL = 1 << 3,
    AS_CGI = 1 << 4,
    AS_LIVE_ACTION = 1 << 5,
    AS_2D = 1 << 6,
    AS_3D = 1 << 7,
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

    // bit modifier
    F_MOD_DIGITAL = 0x1000
};

enum aio_action {
    S_NONE,
    S_VIEWING,
    S_FINISHED,
    S_DROPPED,
    S_PAUSED,
    S_PLANNED,
    S_REVIEWING,
    S_WAITING
};

struct aio_entryi {
    aioid_t itemid;
    aioid_t uid;
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
    /// Formatted as a string concatinated list of \x1F<TAG>\x1F
    /// eg: \x1Ftag1\x1F\x1Ftag2\x1F
    const char* collection;
};

struct aio_entrym {
    aioid_t itemid;
    aioid_t uid;
    const char* title;
    const char* native_title;
    double rating;
    double rating_max;
    const char* description;
    int64_t release_year;
    const char* thumbnail;
    const char* media_dependant;
    const char* datapoints;
    const char* provider;
    const char* provider_id;
    const char* genres;
};

// enum functions {{{
void aio_artstyle_to_string(const enum aio_artstyle, string* out);

const char* aio_action_to_string(const enum aio_action);
/// action must be a string that would be given from aio_action_to_string()
/// case sensitive
enum aio_action aio_action_from_string(const char* action);
// }}}



// info entry functions {{{

/// caller must call aio_free with the result
struct aio_entryi* aio_entryi_new();

// returns -1 if json is not parsable
int aio_entryi_parse(const char* json, struct aio_entryi* out);

void aio_entryi_to_human_str(const struct aio_entryi* entry, string* out);

///gets the hashmap of loaded info entries
hashmap* aio_get_entryi();
// }}}


// metadata entry functions {{{

/// Caller must call aio_free with the result
struct aio_entrym* aio_entrym_new();

/// returns -1 if json is not parsable
int aio_entrym_parse(const char* json, struct aio_entrym* out);

///gets the hashmap of loaded meta entries
hashmap* aio_get_entrym();

// }}}



// general entry functions {{{

/// gets a key from an entry (metadata/user/info)
int aio_entry_get_key(Entry_json info, const char* key, void* out);

void aio_free(void*);

array* aio_get_itemids();
//}}}


// api functions {{{

///clears all loaded items and makes a new search
///loads all resulting INFO.
///(does not load meta, events, or user)
CURLcode aio_search(string* search);

///loads metadata for all loaded info entries
CURLcode aio_load_metadata();

// }}}

// helper functions {{{

void aio_init();
void aio_shutdown();

void aio_id_to_string(const aioid_t, string*);
// }}}
