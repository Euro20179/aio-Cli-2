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
    if(aio_entryi_parse(buf, entry) == -1) {
        fprintf(stderr, "%s\n", buf);
        return;
    }

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

void user_search(const char* search) {
    string out;
    string_new(&out, 0);

    hashmap_new(&items);

    char pathbuf[48 + 32];

    if(search == 0) {
        char buf[48];
        printf("\x1b[1mSearch > \x1b[0m");
        fflush(stdout);
        ssize_t bytes_read = read(STDIN_FILENO, buf, 47);
        if(buf[bytes_read - 1] == '\n') {
            //we want to override the new line
            bytes_read -= 1;
        }
        buf[bytes_read] = 0;
        snprintf(pathbuf, 48 + 32, "/api/v1/query-v3?uid=1&search=%s", buf);
    } else {
        snprintf(pathbuf, 48 + 32, "/api/v1/query-v3?uid=1&search=%s", search);
    }

    char buf[CURL_ERROR_SIZE];

    CURLcode res = mkapireq(&out,pathbuf, buf);
    if(res != 0) {
        printf("%s\n", buf);
    }

    string_split(&out, '\n', NULL, create_entry_items);

    string_del(&out);
    hashmap_del(&items);
}

void action_search(char* search) {
    user_search(search);
}

void handle_action(string* action, size_t action_no, void* userdata) {
    char* act = string_mkcstr(action);

    if(strncmp(act, "s", 1) == 0) {
        action_search(NULL);
    }
}

void handle_argv_actions(char* raw_actions[], size_t total_len) {
    string actions;
    string_new(&actions, total_len);
    string_set(&actions, raw_actions[0], total_len);
    string_split(&actions, '\0', NULL, handle_action);
    string_del(&actions);
}

int main(const int argc, char* argv[]) {
    errf = fopen("./log", "w");

    if(argc > 1) {
        //this assumes the strings are stored next to each other in an array which i dont think is necessarily true
        size_t total_argv_len = (argv[argc-1] + strlen(argv[argc - 1])) - argv[1];
        handle_argv_actions(&(argv[1]), total_argv_len);
        return 0;
    }

    char* search = NULL;
    if(argc > 1) {
        search = argv[1];
    }

    action_search(search);

    close(errf->_fileno);
}
