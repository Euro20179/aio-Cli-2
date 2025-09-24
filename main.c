#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <json-c/arraylist.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <sixel.h>
#include <vips/vips.h>

#include "aio/aio.h"
#include "globals.h"
#include "inc/hashmap.h"
#include "inc/llist.h"
#include "inc/string.h"
#include "inc/mem.h"
#include "url.h"

#include "selector/selector.h"

#define log(...)          \
    fprintf(errf, __VA_ARGS__); \
    fflush(errf)


// initizlied in main
static bool items_allocated = false;

int sixel_write(char* data, int size, void* priv)
{
    string* out = priv;
    string_concat(out, data, size);
    return 0;
}

int printSixel(const char* path, string* sixelOut)
{
    int res = 0;

    VipsImage *x = NULL, *conv = NULL, *flattened = NULL;
    const char* vipserr;

    bool has_alpha = 0;

    sixel_output_t* out = NULL;
    SIXELSTATUS status = sixel_output_new(&out, sixel_write, sixelOut, NULL);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    if (!(x = vips_image_new_from_file(path, NULL))) {
        string_concat(sixelOut, "could not load image\n", sizeof("could not load image\n"));
        goto error;
    }

    if (vips_colourspace(x, &conv, VIPS_INTERPRETATION_sRGB, NULL) == -1) {
        string_concat(sixelOut, "could not convert colorspace\n", sizeof("could not convert colorspace\n"));
        goto error;
    };

    if (vips_image_hasalpha(conv)) {
        if (vips_flatten(conv, &flattened, NULL) == -1) {
            string_concat(sixelOut, "could not flatten\n", sizeof("could not flatten\n"));
            goto error;
        }

        has_alpha = 1;
    } else {
        flattened = conv;
    }

    uint8_t* buf = NULL;
    size_t len;
    if (!(buf = vips_image_write_to_memory(flattened, &len))) {
        string_concat(sixelOut, "coult not write to memory\n", sizeof("coult not write to memory\n"));
        goto error;
    }

    int height = vips_image_get_height(flattened);
    int width = vips_image_get_width(flattened);

    sixel_dither_t* dither;

    status = sixel_dither_new(&dither, -1, NULL);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    status = sixel_dither_initialize(dither, buf, width, height, SIXEL_PIXELFORMAT_RGB888, SIXEL_LARGE_LUM, SIXEL_REP_CENTER_BOX, SIXEL_QUALITY_HIGH);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    status = sixel_encode(buf, width, height, PIXELFORMAT_RGB888, dither, out);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    goto success;

error:
    vipserr = vips_error_buffer();
    fprintf(errf, "ERROR: %s\n%s\nvips error: %s\n", sixel_helper_format_error(status), sixel_helper_get_additional_message(), vipserr);
    fflush(errf);
    vips_error_clear();
    res = -1;

success:
    if (out != NULL) {
        sixel_output_unref(out);
    }
    if (x != NULL) {
        g_object_unref(x);
    }
    if (conv != NULL) {
        g_object_unref(conv);
    }
    if (has_alpha) {
        // only free this if the image has alpha otherwise we get double free
        g_object_unref(flattened);
    }
    return res;
    // sixel_encoder_unref(enc);
}

int add(int a, int b)
{
    return a + b;
}

void print_item(void* item)
{
    struct aio_entryi* a = item;
    string str;
    string_new(&str, 256);
    aio_entryi_to_human_str(a, &str);
    printf("%s\n", string_mkcstr(&str));
    string_del(&str);
}

void user_print_all()
{
    hashmap_foreach(aio_get_entryi(), print_item);
}

void del_item(void* item)
{
    free(item);
}

void user_search(const char* search)
{
    if (search == 0) {
        char buf[48];
        fprintf(stderr, "\x1b[1mSearch > \x1b[0m");
        fflush(stderr);
        ssize_t bytes_read = read(STDIN_FILENO, buf, 47);
        if (buf[bytes_read - 1] == '\n') {
            // we want to override the new line
            bytes_read -= 1;
        }
        buf[bytes_read] = 0;

        string* search = string_new2(bytes_read);
        string_set(search, buf, bytes_read);

        aio_search(search);

        string_del2(search);
    } else {
        string* s = string_new2(strlen(search));
        string_set(s, search, strlen(search));
        aio_search(s);
        string_del2(s);
    }

    aio_load_metadata();
}

void action_search(char* search)
{
    user_search(search);
}

struct argv_actions_state {
    size_t arg_count;
    llist action_args;
    size_t waiting_on_n_more_args;

    const char* action;
};

void handle_action(string* action, size_t action_no, void* userdata)
{
    char* act = string_mkcstr(action);

    struct argv_actions_state* state = userdata;

    // keeps track of whether or not we got the final arg that we were waiting for
    bool just_hit_0 = false;

    if (state->waiting_on_n_more_args > 0) {
        state->waiting_on_n_more_args--;
        if (state->waiting_on_n_more_args == 0) {
            just_hit_0 = true;
        }

        state->arg_count++;

        string* action_cpy = string_new2(action->len);
        string_cpy(action_cpy, action);

        llist_append(&state->action_args, action_cpy);
    } else if (strncmp(act, "s", 1) == 0) {
        state->waiting_on_n_more_args = 1;
        state->action = "s";
    } else if (strncmp(act, "p", 1) == 0) {
        user_print_all();
    }

    if (just_hit_0) {
        if (state->action[0] == 's') {
            string* search = llist_at(&state->action_args, 0);
            string* uri = string_new2(string_len(search));

            string_uri_encode(search, uri);

            char* s = string_mkcstr(uri);
            action_search(s);

            string_del2(uri);
            llist_clear(&state->action_args);
            string_del2(search);
        }
    }
}

void handle_argv_actions(char* raw_actions[], size_t total_len)
{
    string actions;
    string_new(&actions, total_len);
    string_set(&actions, raw_actions[0], total_len);

    struct argv_actions_state state;
    state.arg_count = 0;
    llist_new(&state.action_args);
    state.waiting_on_n_more_args = 0;

    string_split(&actions, '\0', &state, handle_action);

    llist_del(&state.action_args);
    string_del(&actions);
}

string* preview(struct selector_preview_info info)
{
    selector_id_t id = info.id;
    string* out = string_new2(100);
    aioid_t i = *(aioid_t*)array_at(aio_get_itemids(), id);
    string* idstr = string_new2(0);
    aio_id_to_string(i, idstr);
    char* idline = string_mkcstr(idstr);

    struct aio_entryi* entry = (struct aio_entryi*)hashmap_get(aio_get_entryi(), idline);
    struct aio_entrym* meta = (struct aio_entrym*)hashmap_get(aio_get_entrym(), idline);

    if (entry == NULL || meta == NULL) {
        return out;
    }

    string_nconcatf(out, 1000, "Results: %d\n-----------\nId: %lu\n\x1b[34mTitle: %s\x1b[0m (%.1f/%.1f)\n", array_len(aio_get_itemids()), entry->itemid, entry->en_title, meta->rating, meta->rating_max);
    if (entry->native_title[0] != 0) {
        string_nconcatf(out, 1000, "\x1b[34mNative Title: %s\x1b[0m\n", entry->native_title);
    }
    if (entry->collection[0] != 0) {
        for (int i = 0; i < strlen(entry->collection); i++) {
            if (entry->collection[i] == '\x1F') {
                ((char*)entry->collection)[i] = ' ';
            }
        }
        string_nconcatf(out, 1000, "\x1b[35mTags: %s\x1b[0m\n", entry->collection);
    }
    string_nconcatf(out, 1000, "\x1b[36mType: %s\x1b[0m\n---------------\n", entry->type);

    string* desc = string_new2(0);

    for (int i = 0; i < strlen(meta->description); i++) {
        if (i != 0 && i % info.width == 0) {
            string_concat_char(desc, '\n');
        } else {
            string_concat_char(desc, meta->description[i]);
        }
    }

    if (meta->thumbnail[0] != 0) {
        char image_path[sizeof("./test/") + idstr->len + 1];
        char sixel_path[sizeof("./test/") + idstr->len + 1 + sizeof(".sixel")];
        snprintf(image_path, sizeof("./test/") + idstr->len + 1, "./test/%s", idline);
        snprintf(sixel_path, sizeof("./test/") + idstr->len + 1 + sizeof(".sixel"), "./test/%s.sixel", idline);

        struct stat st;
        if (stat(image_path, &st) != 0) {
            string thumbnail;
            // reserve 10 megs to hopefully avoid massive reallocs
            string_new(&thumbnail, 1024 * 1024 * 10);

            char error[CURL_ERROR_SIZE];
            CURLcode res = mkreq(&thumbnail, (char*)meta->thumbnail, error);
            if (res != 0) {
                fprintf(errf, "%s\n", error);
            } else if (string_len(&thumbnail) != 0) {
                int f = open(image_path, O_CREAT | O_RDWR, 0644);
                write(f, thumbnail.data, thumbnail.len);
                close(f);
            }
            string_del(&thumbnail);
        }

        if (stat(sixel_path, &st) != 0) {
            string* sixel = string_new2(1024 * 1024 * 10);

            log("Creating sixel for %s (%ld)\n", entry->en_title, entry->itemid);

            int res = printSixel(image_path, sixel);
            if (res == 0) {
                string_nconcatf(out, string_len(sixel), "%s\n", string_mkcstr(sixel));
                int f = open(sixel_path, O_RDWR | O_CREAT, 0644);
                write(f, sixel->data, sixel->len);
                close(f);
            }
            string_del2(sixel);
        } else {
            int f = open(sixel_path, O_RDONLY);
            char* buf = malloc(st.st_size + 1);
            if (buf == NULL) {
                close(f);
            } else {
                read(f, buf, st.st_size);
                buf[st.st_size] = 0;
                string_nconcatf(out, st.st_size, "%s\n", buf);
                free(buf);
                close(f);
            }
        }
    }

    string_nconcatf(out, 3000, "%s\n", string_mkcstr(desc));
    string_del2(desc);
    string_del2(idstr);
    return out;
}

int main(const int argc, char* argv[])
{
    VIPS_INIT(argv[0]);

    aio_init();

    errf = fopen("./log", "w");

    if (argc > 1) {
        string* raw_actions_char_buf = string_new2(0);
        for(int i = 1; i < argc; i++) {
            string_concat(raw_actions_char_buf, argv[i], strlen(argv[i]) + 1);
        }
        handle_argv_actions(&(argv[1]), string_len(raw_actions_char_buf));
        return 0;
    }

    char* search = NULL;
    if (argc > 1) {
        search = argv[1];
    }

    action_search(search);

    hashmap* info = aio_get_entryi();

    if(hashmap_item_count(aio_get_entryi()) == 0) {
        printf("No results\n");
        return 1;
    }

    struct selector_action_handlers actions = {
        .on_hover = NULL,
        .preview_gen = preview,
    };
    array* lines = array_new2(0, sizeof(const char**));
    for (size_t i = 0; i < array_len(aio_get_itemids()); i++) {
        aioid_t idint = *(aioid_t*)array_at(aio_get_itemids(), i);
        struct aio_entryi* entry = aio_get_by_id(idint, aio_get_entryi());
        if (entry == NULL)
            continue;
        array_append(lines, &entry->en_title);
    }

    selector* s = selector_new2(actions, lines);
    selector_id_t row = selector_select(s);
    const char* z = selector_get_by_id(s, row);
    selector_del2(s);

    // const char* action_list[] = {
    //     "finish",
    //     "start",
    // };
    //
    // array* action_arr = array_new2(2, sizeof(const char**));
    // for(int i = 0; i < 2; i++) {
    //     array_append(action_arr, &action_list[i]);
    // }

    // actions.preview_gen = NULL;
    // s = selector_new2(actions, action_arr);
    // selector_select(s);
    // selector_del2(s);


    printf("You selected: %s\n", z);

    fclose(errf);
}
