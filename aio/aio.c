#include "aio.h"
#include "../url.h"

#include <curl/curl.h>
#include <json-c/arraylist.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <json-c/linkhash.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define key(json, name) json_object_object_get(json, name)

void _aio_clear_items() {
    hashmap_del_each(&user, aio_free);
}

void aio_format_to_string(enum aio_entryformat format, string* out)
{
    bool isdigital = false;
    // check if digital
    if ((format & F_MOD_DIGITAL) == F_MOD_DIGITAL) {
        format -= F_MOD_DIGITAL;
        isdigital = true;
    }
    // guaranteed not to have F_MOD_DIGITAL since we took it out above
    switch (format) {
    case F_VHS:
        string_concat(out, "VHS", strlen("VHS"));
        break;
    case F_CD:
        string_concat(out, "CD", strlen("CD"));
        break;
    case F_DVD:
        string_concat(out, "DVD", strlen("DVD"));
        break;
    case F_BLURAY:
        string_concat(out, "BLURAY", strlen("BLURAY"));
        break;
    case F_4KBLURAY:
        string_concat(out, "4KBLURAY", strlen("4KBLURAY"));
        break;
    case F_MANGA:
        string_concat(out, "MANGA", strlen("MANGA"));
        break;
    case F_BOOK:
        string_concat(out, "BOOK", strlen("BOOK"));
        break;
    case F_DIGITAL:
        string_concat(out, "DIGITAL", strlen("DIGITAL"));
        break;
    case F_BOARDGAME:
        string_concat(out, "BOARDGAME", strlen("BOARDGAME"));
        break;
    case F_STEAM:
        string_concat(out, "STEAM", strlen("STEAM"));
        break;
    case F_NIN_SWITCH:
        string_concat(out, "NIN_SWITCH", strlen("NIN_SWITCH"));
        break;
    case F_XBOXONE:
        string_concat(out, "XBOXONE", strlen("XBOXONE"));
        break;
    case F_XBOX360:
        string_concat(out, "XBOX360", strlen("XBOX360"));
        break;
    case F_OTHER:
        string_concat(out, "OTHER", strlen("OTHER"));
        break;
    case F_VINYL:
        string_concat(out, "VINYL", strlen("VINYL"));
        break;
    case F_IMAGE:
        string_concat(out, "IMAGE", strlen("IMAGE"));
        break;
    case F_UNOWNED:
        string_concat(out, "UNOWNED", strlen("UNOWNED"));
        break;
    }

    if (isdigital) {
        string_concat(out, "+D", 2);
    }
}

void aio_artstyle_to_string(const enum aio_artstyle as, string* out)
{
    if ((as & AS_ANIME) == AS_ANIME) {
        string_concat(out, "Anime + ", 8);
    }
    if ((as & AS_CARTOON) == AS_CARTOON) {
        string_concat(out, "Cartoon + ", 10);
    }
    if ((as & AS_CGI) == AS_CGI) {
        string_concat(out, "CGI + ", 6);
    }
    if ((as & AS_DIGITAL) == AS_DIGITAL) {
        string_concat(out, "Digital + ", 10);
    }
    if ((as & AS_HANDRAWN) == AS_HANDRAWN) {
        string_concat(out, "Handrawn + ", 11);
    }
    if ((as & AS_LIVE_ACTION) == AS_LIVE_ACTION) {
        string_concat(out, "Live action + ", 14);
    }

    if (out->len == 0) {
        return;
    }

    // remove the " +" at the end
    string_slice_suffix(out, 2);
}

void pretty_tags_handler(string* name, size_t count, void* tagsList)
{
    if (name->len != 0) {
        string2_concat(tagsList, name);
        string_concat_char(tagsList, ' ');
    }
}

void aio_entryi_to_human_str(const struct aio_entryi* entry, string* out)
{
    // create a line from title and value
#define line(title, value)                             \
    string_concat(out, title ": ", strlen(title) + 2); \
    string_concat(out, value, strlen(value));          \
    string_concat_char(out, '\n')

    // create a line from title, an sprintf format, and value
#define linef(title, fmt, value)        \
    {                                   \
        char buf[100];                  \
        buf[0] = 0;                     \
        snprintf(buf, 100, fmt, value); \
        line(title, buf);               \
    }

    // create a line by first converting the value to a string via a conversion func
#define line_from_string(title, value, convert)        \
    {                                                  \
        string line_from_string_buf;                   \
        string_new(&line_from_string_buf, 128);        \
        const char* c_sbuf;                            \
        convert(value, &line_from_string_buf);         \
        c_sbuf = string_mkcstr(&line_from_string_buf); \
        line(title, c_sbuf);                           \
        string_del(&line_from_string_buf);             \
    }

    line("Title", entry->en_title);
    line("Native title", entry->native_title);
    line("Type", entry->type);
    //
    line_from_string("Format", entry->format, aio_format_to_string);
    line_from_string("Art style", entry->art_style, aio_artstyle_to_string);
    //
    //
    linef("Item id", "%ld", entry->itemid);
    linef("Parent id", "%ld", entry->parentid);
    linef("Copy of", "%ld", entry->copyof);
    linef("In library", "%ld", entry->library);
    line("Location", entry->location);
    linef("Purchase price", "%0.2lf", entry->purchase_price);

    if (entry->collection == 0 || strlen(entry->collection) == 0) {
        line("Tags", entry->collection);
    } else {
        string tagsList;
        string_new(&tagsList, 32);

        string collection;
        size_t collection_len = strlen(entry->collection);
        string_new(&collection, collection_len);
        string_set(&collection, entry->collection, collection_len);

        string_split(&collection, '\x1F', &tagsList, pretty_tags_handler);

        line("Tags", string_mkcstr(&tagsList));

        string_del(&collection);
        string_del(&tagsList);
    }

#undef line
#undef linef
#undef line_from_string
}

int aio_entryi_parse(const char* json, struct aio_entryi* out)
{
    EntryI_json data = json_tokener_parse(json);
    if (data == NULL) {
        return -1;
    }

#define set(key, field) aio_entry_get_key(data, key, &out->field)
    set("ItemId", itemid);
    set("Uid", uid);
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
    return 0;
}

hashmap* aio_get_entryi() {
    return &info;
}

int aio_entrym_parse(const char* json, struct aio_entrym* out)
{
    MetaInfo_json data = json_tokener_parse(json);
    if (data == NULL) {
        return -1;
    }

#define set(key, field) aio_entry_get_key(data, key, &out->field)
    set("ItemId", itemid);
    set("Uid", uid);
    set("Title", title);
    set("Native_Title", native_title);
    // set("Rating", rating);
    set("RatingMax", rating_max);
    set("Description", description);
    set("ReleaseYear", release_year);
    set("Thumbnail", thumbnail);
    set("MediaDependant", media_dependant);
    set("Datapoints", datapoints);
    set("Provider", provider);
    set("ProviderID", provider_id);
    set("Genres", genres);
#undef set

    json_object* temp = key(data, "Rating");
    out->rating = json_object_get_double(temp);
    temp = key(data, "RatingMax");
    out->rating_max = json_object_get_double(temp);

    return 0;
}

hashmap* aio_get_entrym() {
    return &meta;
}

int aio_entry_get_key(EntryI_json info, const char* key, void* out)
{
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
        *(double*)out = json_object_get_double(data);
        return 0;
    case json_type_string:
        *(const char**)out = json_object_get_string(data);
        return 0;
    }
    return json_object_get_uint64(key(info, "ItemId"));
}

void aio_init() {
    hashmap_new(&info);
    hashmap_new(&meta);
    hashmap_new(&user);

    itemids = array_new2(0, sizeof(aioid_t));
}

void aio_shutdown() {
    hashmap_del_each(&info, aio_free);
    hashmap_del_each(&user, aio_free);
    hashmap_del_each(&meta, aio_free);

    array_del2(itemids);
}

void aio_id_to_string(const aioid_t id, string* out)
{
    char idbuf[32];
    idbuf[0] = 0;
    snprintf(idbuf, 32, "%ld", id);
    int len = strlen(idbuf);
    string_concat(out, idbuf, len);
}

#undef key

struct aio_entryi* aio_entryi_new()
{
    struct aio_entryi* e = malloc(sizeof(struct aio_entryi));
    e->itemid = 0;
    e->uid = 0;
    e->en_title = "";
    e->native_title = "";
    e->location = "";
    e->purchase_price = 0;
    e->parentid = 0;
    e->type = "";
    e->art_style = 0;
    e->copyof = 0;
    e->library = 0;
    e->format = 0;
    e->collection = "";
    return e;
}

struct aio_entrym* aio_entrym_new()
{
    struct aio_entrym* e = malloc(sizeof(struct aio_entrym));
    e->itemid = 0;
    e->uid = 0;
    e->media_dependant = "{}";
    e->datapoints = "{}";
    e->description = NULL;
    e->genres = NULL;
    e->native_title = NULL;
    e->title = NULL;
    e->provider = NULL;
    e->provider_id = NULL;
    e->thumbnail = NULL;
    return e;
}

void aio_free(void* entry)
{
    free(entry);
}

array* aio_get_itemids() {
    return itemids;
}

const char* aio_action_to_string(const enum aio_action action)
{
    switch (action) {
    case S_NONE:
        return "None";
    case S_VIEWING:
        return "Viewing";
    case S_FINISHED:
        return "Finished";
    case S_DROPPED:
        return "Dropped";
    case S_PAUSED:
        return "Paused";
    case S_PLANNED:
        return "Planned";
    case S_REVIEWING:
        return "ReViewing";
    case S_WAITING:
        return "Waiting";
    }
}

enum aio_action aio_action_from_string(const char* action)
{
    switch (action[0]) {
    case 'N':
        return S_NONE;
    case 'V':
        return S_VIEWING;
    case 'F':
        return S_FINISHED;
    case 'D':
        return S_DROPPED;
    case 'R':
        return S_REVIEWING;
    case 'W':
        return S_WAITING;
    default:
        switch (action[1]) {
        case 'a':
            return S_PAUSED;
        case 'l':
            return S_PLANNED;
        default:
            return S_NONE;
        }
    }
}

void create_metadata_items(string* line, size_t count, void* userdata)
{
    size_t len = line->len;
    if (len == 0) {
        return;
    }

    string_to_cstr_buf_create(buf, (*line));
    string_to_cstr(line, buf);

    struct aio_entrym* entry = aio_entrym_new();
    if (aio_entrym_parse(buf, entry) == -1) {
        return;
    }

    char idbuf[32];
    idbuf[0] = 0;
    snprintf(idbuf, 32, "%ld", entry->itemid);

    hashmap_set(&meta, idbuf, entry);
}

void create_entry_items(string* line, size_t count, void* userdata)
{
    size_t len = line->len;
    if (len == 0) {
        return;
    }

    char buf[line->len + 1];
    string_to_cstr(line, buf);

    struct aio_entryi* entry = aio_entryi_new();
    if (aio_entryi_parse(buf, entry) == -1) {
        return;
    }

    char idbuf[32];
    idbuf[0] = 0;
    snprintf(idbuf, 32, "%ld", entry->itemid);

    array_append(itemids, &entry->itemid);

    string* key = string_new2(32);
    string_set(key, idbuf, 32);
    hashmap_set_safe(&info, move(key), entry);
}

CURLcode aio_search(string* search) {
    string* uri = string_new2(string_len(search) * 3);
    string_uri_encode(search, uri);
    char* s = string_mkcstr(uri);

    char pathbuf[48 + 32];
    snprintf(pathbuf, 48 + 32, "/api/v1/query-v3?uid=1&search=%s", s);

    string out;
    string_new(&out, 0);

    char buf[CURL_ERROR_SIZE];
    CURLcode res = mkapireq(&out, pathbuf, buf);
    if (res != 0) {
        return res;
    }

    string_split(&out, '\n', NULL, create_entry_items);

    string_del2(uri);

    return 0;
}

CURLcode aio_load_metadata() {
    char buf[CURL_ERROR_SIZE];

    string out;
    string_new(&out, 0);

    const char* metadataPath = "/api/v1/metadata/list-entries?uid=1";
    CURLcode res = mkapireq(&out, metadataPath, buf);
    if (res != 0) {
        return res;
    }
    string_split(&out, '\n', NULL, create_metadata_items);
    string_del(&out);

    return 0;
}

void* aio_get_by_id(aioid_t id, void* from_map) {
    string* idstr = string_new2(32);
    aio_id_to_string(id, idstr);
    char* line = string_mkcstr(idstr);
    void* entry = hashmap_get(from_map, line);
    string_del2(idstr);
    return entry;
}
