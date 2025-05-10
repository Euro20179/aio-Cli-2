#include <json-c/arraylist.h>
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
#include "inc/hashmap.h"
#include "url.h"
#include "inc/string.h"
#include "aio/aio.h"

//initizlied in main
static hashmap items;

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


void create_entry_items(string* line, size_t count, void* userdata) {
    size_t len = line->len;
    if(len == 0) {
        return;
    }

    char buf[line->len + 1];
    string_to_cstr(line, buf);

    struct aio_entryi* entry = malloc(sizeof(struct aio_entryi));
    aio_entryi_parse(buf, entry);

    char idbuf[32];
    idbuf[0] = 0;
    snprintf(idbuf, 32, "%ld", entry->itemid);

    string human;
    string_new(&human, 256);
    aio_entryi_to_human_str(entry, &human);
    printf("%s\n", string_mkcstr(&human));

    string_del(&human);

    hashmap_set(&items, idbuf, entry);
}

int add(int a, int b) {
    return a + b;
}

void user_search() {
    string out;
    string_new(&out, 0);

    hashmap_new(&items);

    printf("\x1b[1mSearch > \x1b[0m");
    fflush(stdout);
    char buf[48];
    ssize_t bytes_read = read(STDIN_FILENO, buf, 47);
    if(buf[bytes_read - 1] == '\n') {
        //we want to override the new line
        bytes_read -= 1;
    }
    buf[bytes_read] = 0;

    char pathbuf[48 + 32];
    snprintf(pathbuf, 48 + 32, "/api/v1/query-v3?uid=1&search=%s", buf);

    mkapireq(&out,pathbuf);

    string_split(&out, '\n', NULL, create_entry_items);

    string_del(&out);
    hashmap_del(&items);
}

int main(const int argc, char* argv[]) {
    errf = fopen("./log", "w");

    user_search();

    close(errf->_fileno);
}
