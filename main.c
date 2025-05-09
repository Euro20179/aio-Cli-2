#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <json-c/json.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <sixel.h>

#include "aio/aio.h"
#include "globals.h"
#include "url.h"
#include "inc/string.h"
#include "aio/aio.h"

void printSixel(const char* path) {
    sixel_encoder_t* enc;
    SIXELSTATUS status = sixel_encoder_new(&enc, NULL);
    if(SIXEL_FAILED(status)) {
        goto error;
    }

    const char* width = "300";

    sixel_encoder_setopt(enc, 'w', width);

    status = sixel_encoder_encode(enc, path);
    if(SIXEL_FAILED(status)) {
        goto error;
    }

    goto success;

error:
    fprintf(stderr, "%s\n%s\n", sixel_helper_format_error(status), sixel_helper_get_additional_message());

success:
    sixel_encoder_unref(enc);
}

void readjsonL(string* line, size_t count) {
    size_t len = line->len;
    if(len == 0) {
        return;
    }

    char buf[line->len + 1];
    string_to_cstr(line, buf);

    struct aio_entryi entry;
    aio_entryi_parse(buf, &entry);

    string out;
    string_new(&out, 0);

    aio_entryi_to_human_str(&entry, &out);

    string_to_cstr_buf_create(ob, out);
    string_to_cstr(&out, ob);
    printf("%s\n", ob);

    string_del(&out);


    // string as;
    // string_new(&as, 0);
    // aio_artstyle_to_string(entry.art_style, &as);
    //
    // char asbuf[as.len + 1];
    // string_to_cstr(&as, asbuf);

    // printf("%s\n", entry.en_title, entry.itemid, asbuf);
    //
    // string_del(&as);

    // char mt[50];
    // memset(mt, 0, 50);
    // aio_entryi_get_key(entry, "MediaType", &mt);
    // for(int i = 0; i < 50; i++) {
    //     printf("%c", mt[i]);
    // }
    // uint64_t id = aio_entryi_get_item_id(entry);
    //
    // printf("%zu\n", id);
}

int main(const int argc, char* argv[]) {
    errf = fopen("./log", "w");

    string out;
    string_new(&out, 0);

    mkapireq(&out, "/api/v1/list-entries?uid=1");

    string_split(&out, '\n', readjsonL);

    string_del(&out);

    close(errf->_fileno);
}
