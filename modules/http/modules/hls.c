/*
 * Astra Module: HTTP HLS Output
 * Generates HLS segments from an upstream MPEG-TS stream
 */
#include <astra.h>
#include "../http.h"
#include <sys/stat.h>

struct module_data_t
{
    MODULE_STREAM_DATA();

    const char *path;
    int segment_length; // seconds
    int playlist_size;

    FILE *segment;
    asc_timer_t *timer;

    char **segments;
    int seg_index;
    int seg_total;
    int seq;
};

static void write_playlist(module_data_t *mod)
{
    char file[512];
    snprintf(file, sizeof(file), "%s/playlist.m3u8", mod->path);
    FILE *f = fopen(file, "w");
    if(!f) return;

    fprintf(f, "#EXTM3U\n#EXT-X-VERSION:3\n");
    fprintf(f, "#EXT-X-TARGETDURATION:%d\n", mod->segment_length);
    fprintf(f, "#EXT-X-MEDIA-SEQUENCE:%d\n", mod->seq - mod->seg_total);

    for(int i = 0; i < mod->seg_total; ++i)
    {
        int idx = (mod->seg_index + i) % mod->playlist_size;
        fprintf(f, "#EXTINF:%d,\n%s\n", mod->segment_length, mod->segments[idx]);
    }
    fclose(f);
}

static void rotate_segment(void *arg)
{
    module_data_t *mod = (module_data_t *)arg;

    if(mod->segment)
    {
        fclose(mod->segment);
        mod->segment = NULL;
    }

    char name[64];
    snprintf(name, sizeof(name), "segment%05d.ts", mod->seq);

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", mod->path, name);

    if(mod->segments[mod->seg_index])
    {
        char old[512];
        snprintf(old, sizeof(old), "%s/%s", mod->path, mod->segments[mod->seg_index]);
        unlink(old);
        free(mod->segments[mod->seg_index]);
    }

    mod->segments[mod->seg_index] = strdup(name);
    mod->seg_index = (mod->seg_index + 1) % mod->playlist_size;
    if(mod->seg_total < mod->playlist_size)
        mod->seg_total++;

    mod->segment = fopen(path, "wb");
    mod->seq++;

    write_playlist(mod);
}

static void on_ts(module_data_t *mod, const uint8_t *ts)
{
    if(mod->segment)
        fwrite(ts, TS_PACKET_SIZE, 1, mod->segment);
}

static void module_init(module_data_t *mod)
{
    mod->segment_length = 10;
    mod->playlist_size = 5;
    mod->path = NULL;

    module_option_number("segment", &mod->segment_length);
    module_option_number("playlist", &mod->playlist_size);
    module_option_string("path", &mod->path, NULL);

    lua_getfield(lua, MODULE_OPTIONS_IDX, "upstream");
    module_stream_t *upstream = NULL;
    if(lua_islightuserdata(lua, -1))
        upstream = (module_stream_t *)lua_touserdata(lua, -1);
    lua_pop(lua, 1);

    asc_assert(mod->path != NULL, "[http_hls] option 'path' is required");
    asc_assert(upstream != NULL, "[http_hls] option 'upstream' is required");

    mkdir(mod->path, 0755);

    mod->segments = calloc(mod->playlist_size, sizeof(char*));
    mod->seg_index = 0;
    mod->seg_total = 0;
    mod->seq = 0;

    module_stream_init(mod, on_ts);
    __module_stream_attach(upstream, &mod->__stream);

    rotate_segment(mod);
    mod->timer = asc_timer_init(mod->segment_length * 1000, rotate_segment, mod);
}

static void module_destroy(module_data_t *mod)
{
    module_stream_destroy(mod);

    if(mod->timer)
        asc_timer_destroy(mod->timer);

    if(mod->segment)
    {
        fclose(mod->segment);
        mod->segment = NULL;
    }
    for(int i = 0; i < mod->playlist_size; ++i)
    {
        if(mod->segments[i])
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", mod->path, mod->segments[i]);
            unlink(path);
            free(mod->segments[i]);
        }
    }
    free(mod->segments);
}

MODULE_STREAM_METHODS()
MODULE_LUA_METHODS()
{
    MODULE_STREAM_METHODS_REF(),
    { NULL, NULL }
};
MODULE_LUA_REGISTER(http_hls)

