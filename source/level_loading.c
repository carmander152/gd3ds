#include <stdlib.h>
#include <stdbool.h>
#include "level_loading.h"
#include "defines.h"
#include <zlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "color_channels.h"
#include "objects.h"
#include "objects_array.h"
#include "mp3_player.h"
#include "graphics.h"
#include "math_helpers.h"
#include "utils/json_config.h"
#include "state.h"
#include "player/collision.h"

ObjectsArray objects = { 0 };
Section empty_section = { 0 };
Section *section_hash[SECTION_HASH_SIZE] = {0};
int channelCount = 0;
GDColorChannel *colorChannels = NULL;
LoadedLevelInfo level_info;

static inline unsigned int section_hash_func(unsigned int x, unsigned int y) {
    return ((unsigned int)x * 73856093u ^ (unsigned int)y * 19349663u) & (SECTION_HASH_SIZE - 1);
}

Section *get_section(int x, int y) {
    unsigned int h = section_hash_func(x, y);
    Section *sec = section_hash[h];
    while (sec) {
        if (sec->x == x && sec->y == y) return sec;
        sec = sec->next;
    }
    return &empty_section;
}

Section *get_or_create_section(int x, int y) {
    unsigned int h = section_hash_func(x, y);
    Section *sec = section_hash[h];
    while (sec) {
        if (sec->x == x && sec->y == y) return sec;
        sec = sec->next;
    }
    sec = malloc(sizeof(Section));
    sec->objects = malloc(sizeof(int) * 8);
    sec->object_count = 0;
    sec->object_capacity = 8;
    sec->x = x;
    sec->y = y;
    sec->next = section_hash[h];
    section_hash[h] = sec;
    return sec;
}

void free_sections(void) {
    for (int i = 0; i < SECTION_HASH_SIZE; i++) {
        Section *sec = section_hash[i];
        while (sec) {
            Section *next = sec->next;
            free(sec->objects);
            free(sec);
            sec = next;
        }
        section_hash[i] = NULL;
    }
}

void assign_object_to_section(int obj) {
    int sx = (int)(objects.x[obj] / SECTION_SIZE);
    int sy = (int)(objects.y[obj] / SECTION_SIZE);
    Section *sec = get_or_create_section(sx, sy);
    if (sec->object_count >= sec->object_capacity) {
        sec->object_capacity *= 2;
        sec->objects = realloc(sec->objects, sizeof(int) * sec->object_capacity);
    }
    sec->objects[sec->object_count++] = obj;
}

char *read_file(const char *filepath, size_t *out_size) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    char *buffer = malloc(size + 1);
    if (!buffer) { fclose(f); return NULL; }
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    if (out_size) *out_size = size;
    return buffer;
}

char *extract_gmd_key(const char *data, const char *key, const char *type) {
    char key_tag[32];
    snprintf(key_tag, sizeof(key_tag), "<k>%s</k>", key);
    char *key_pos = strstr(data, key_tag);
    if (!key_pos) return NULL;
    char *start = key_pos + strlen(key_tag);
    while (*start && isspace((unsigned char)*start)) start++;
    char type_start_tag[16];
    snprintf(type_start_tag, sizeof(type_start_tag), "<%s>", type);
    if (strncmp(start, type_start_tag, strlen(type_start_tag)) != 0) return NULL;
    start += strlen(type_start_tag);
    char type_end_tag[16];
    snprintf(type_end_tag, sizeof(type_end_tag), "</%s>", type);
    char *end = strstr(start, type_end_tag);
    if (!end) return NULL;
    int len = end - start;
    char *value = malloc(len + 1);
    strncpy(value, start, len);
    value[len] = '\0';
    return value;
}

int b64_char(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a' + 26;
    if ('0' <= c && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

void fix_base64_url(char *b64) {
    for (int i = 0; b64[i]; i++) {
        if (b64[i] == '-') b64[i] = '+';
        else if (b64[i] == '_') b64[i] = '/';
    }
}

int base64_decode(const char *in, unsigned char *out) {
    int len = 0;
    for (int i = 0; in[i] && in[i+1] && in[i+2] && in[i+3]; i += 4) {
        int a = b64_char(in[i]);
        int b = b64_char(in[i+1]);
        int c = in[i+2] == '=' ? 0 : b64_char(in[i+2]);
        int d = in[i+3] == '=' ? 0 : b64_char(in[i+3]);
        if (a == -1 || b == -1 || c == -1 || d == -1) return -1;
        out[len++] = (a << 2) | (b >> 4);
        if (in[i+2] != '=') out[len++] = (b << 4) | (c >> 2);
        if (in[i+3] != '=') out[len++] = (c << 6) | d;
    }
    return len;
}

uLongf get_uncompressed_size(unsigned char *data, int data_len) {
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    strm.next_in = data;
    strm.avail_in = data_len;
    if (inflateInit2(&strm, 15 | 32) != Z_OK) return 0;
    uLongf total_out = 0;
    unsigned char buf[4096];
    do {
        strm.next_out = buf;
        strm.avail_out = sizeof(buf);
        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            inflateEnd(&strm);
            return 0;
        }
        total_out += sizeof(buf) - strm.avail_out;
        if (ret == Z_STREAM_END) break;
    } while (strm.avail_in > 0);
    inflateEnd(&strm);
    return total_out;
}

char *decompress_data(unsigned char *data, int data_len, uLongf *out_len) {
    uLongf final_size = get_uncompressed_size(data, data_len);
    z_stream strm = {0};
    strm.next_in = data;
    strm.avail_in = data_len;
    if (inflateInit2(&strm, 15 | 32) != Z_OK) return NULL;
    char *out = malloc(final_size + 1);
    if (!out) { inflateEnd(&strm); return NULL; }
    strm.next_out = (Bytef *)out;
    strm.avail_out = final_size;
    int ret = inflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) { free(out); inflateEnd(&strm); return NULL; }
    *out_len = strm.total_out;
    out[*out_len] = '\0';
    inflateEnd(&strm);
    return out;
}

char *get_metadata_value(const char *levelString, const char *key) {
    if (!levelString || !key) return NULL;
    const char *end = strchr(levelString, ';');
    if (!end) return NULL;
    size_t metadataLen = end - levelString;
    char *metadata = malloc(metadataLen + 1);
    strncpy(metadata, levelString, metadataLen);
    metadata[metadataLen] = '\0';
    char *token = strtok(metadata, ",");
    while (token) {
        if (strcmp(token, key) == 0) {
            char *value = strtok(NULL, ",");
            if (!value) break;
            char *result = strdup(value);
            free(metadata);
            return result;
        }
        token = strtok(NULL, ",");
    }
    free(metadata);
    return NULL;
}

char *decompress_level(char *data) {
    char *b64 = extract_gmd_key((const char *) data, "k4", "s");
    if (!b64) return data;
    fix_base64_url(b64);
    unsigned char *decoded = malloc(strlen(b64));
    int decoded_len = base64_decode(b64, decoded);
    if (decoded_len <= 0) { free(b64); free(decoded); return NULL; }
    uLongf decompressed_len;
    char *decompressed = decompress_data(decoded, decoded_len, &decompressed_len);
    free(decoded);
    free(b64);
    return decompressed;
}

char **split_string(const char *str, char delimiter, int *outCount) {
    char **result = NULL;
    int count = 0;
    const char *start = str;
    const char *ptr = str;
    while (*ptr) {
        if (*ptr == delimiter) {
            int len = ptr - start;
            if (len > 0) {
                char *token = (char *)malloc(len + 1);
                strncpy(token, start, len);
                token[len] = '\0';
                result = (char **)realloc(result, sizeof(char*) * (count + 1));
                result[count++] = token;
            }
            start = ptr + 1;
        }
        ptr++;
    }
    if (ptr > start) {
        int len = ptr - start;
        char *token = (char *)malloc(len + 1);
        strncpy(token, start, len);
        token[len] = '\0';
        result = (char **)realloc(result, sizeof(char*) * (count + 1));
        result[count++] = token;
    }
    *outCount = count;
    return result;
}

void free_string_array(char **arr, int count) {
    for (int i = 0; i < count; i++) free(arr[i]);
    free(arr);
}

GDValueType get_value_type_for_key(int key) {
    switch (key) {
        case 1:  return GD_VAL_INT;
        case 2:  return GD_VAL_FLOAT;
        case 3:  return GD_VAL_FLOAT;
        case 4:  return GD_VAL_BOOL;
        case 5:  return GD_VAL_BOOL;
        case 6:  return GD_VAL_FLOAT;
        case 10: return GD_VAL_FLOAT;
        case 35: return GD_VAL_FLOAT; // Opacity (2.0)
        case 51: return GD_VAL_INT;   // Target Group (2.0)
        case 57: return GD_VAL_INT;   // Object Group (2.0)
        case 28: return GD_VAL_FLOAT; // Move X (2.0)
        case 29: return GD_VAL_FLOAT; // Move Y (2.0)
        default: return GD_VAL_INT;
    }
}

int convert_object(int id) {
    switch (id) {
        case 901: return 901; // Move Trigger
        case 1007: return 1007; // Alpha Trigger
        case 143: return 143; // Robot Portal
        case 1734: return 675;
        case 1329: return SECRET_COIN;
    }
    return id;
}

void fill_object_data(int object, int key, GDValueType type, GDValue val) {
    switch (key) {
        case 1:  if (type == GD_VAL_INT) objects.id[object] = convert_object(val.i); break;
        case 2:  if (type == GD_VAL_FLOAT) objects.x[object] = val.f; break;
        case 3:  if (type == GD_VAL_FLOAT) objects.y[object] = val.f; break;
        case 10: if (type == GD_VAL_FLOAT) objects.trig_duration[object] = val.f; break;
        case 35: if (type == GD_VAL_FLOAT) objects.opacity[object] = val.f; break;
        case 51: if (type == GD_VAL_INT) objects.target_color_id[object] = val.i; break;
        case 57: if (type == GD_VAL_INT) objects.group_id[object] = val.i; break;
        case 28: if (type == GD_VAL_FLOAT) objects.trig_colorR[object] = (unsigned char)val.f; break;
        case 29: if (type == GD_VAL_FLOAT) objects.trig_colorG[object] = (unsigned char)val.f; break;
    }
}

bool init_arrays(int count) {
    objects.random = malloc(sizeof(int) * count);
    objects.id = malloc(sizeof(int) * count);
    objects.x = malloc(sizeof(float) * count);
    objects.y = malloc(sizeof(float) * count);
    objects.rotation = malloc(sizeof(float) * count);
    objects.zlayer = malloc(sizeof(int) * count);
    objects.zorder = malloc(sizeof(int) * count);
    objects.trig_duration = malloc(sizeof(float) * count);
    objects.width = malloc(sizeof(float) * count);
    objects.height = malloc(sizeof(float) * count);
    objects.v1p9_col_channel = malloc(sizeof(unsigned short) * count);
    objects.col_channel = malloc(sizeof(unsigned short) * count);
    objects.detail_col_channel = malloc(sizeof(unsigned short) * count);
    objects.target_color_id = malloc(sizeof(unsigned short) * count);
    objects.hitbox_counter = malloc(sizeof(unsigned short) * count);
    objects.transition_applied = malloc(sizeof(unsigned char) * count);
    objects.trig_colorR = malloc(sizeof(unsigned char) * count);
    objects.trig_colorG = malloc(sizeof(unsigned char) * count);
    objects.trig_colorB = malloc(sizeof(unsigned char) * count);
    objects.orientation = malloc(sizeof(unsigned char) * count);
    objects.tintGround = malloc(sizeof(bool) * count);
    objects.p1_color = malloc(sizeof(bool) * count);
    objects.p2_color = malloc(sizeof(bool) * count);
    objects.blending = malloc(sizeof(bool) * count);
    objects.touch_triggered = malloc(sizeof(bool) * count);
    objects.flippedH = malloc(sizeof(bool) * count);
    objects.flippedV = malloc(sizeof(bool) * count);
    objects.toggled = malloc(sizeof(bool) * count);
    objects.activated = malloc(sizeof(u8) * count);
    objects.collided = malloc(sizeof(u8) * count);
    objects.group_id = malloc(sizeof(int) * count);
    objects.opacity = malloc(sizeof(float) * count);

    for (int i = 0; i < count; i++) {
        objects.opacity[i] = 1.0f;
        objects.group_id[i] = 0;
        objects.activated[i] = 0;
    }
    return true;
}

void free_arrays() {
    free(objects.random); free(objects.id); free(objects.x); free(objects.y);
    free(objects.rotation); free(objects.zlayer); free(objects.zorder);
    free(objects.trig_duration); free(objects.width); free(objects.height);
    free(objects.v1p9_col_channel); free(objects.col_channel);
    free(objects.detail_col_channel); free(objects.target_color_id);
    free(objects.hitbox_counter); free(objects.transition_applied);
    free(objects.trig_colorR); free(objects.trig_colorG); free(objects.trig_colorB);
    free(objects.orientation); free(objects.tintGround); free(objects.p1_color);
    free(objects.p2_color); free(objects.blending); free(objects.touch_triggered);
    free(objects.flippedH); free(objects.flippedV); free(objects.toggled);
    free(objects.activated); free(objects.collided); free(objects.group_id);
    free(objects.opacity);
}

bool parse_string(const char *levelString) {
    int sectionCount = 0;
    char **sections = split_string(levelString, ';', &sectionCount);
    if (sectionCount < 3) { free_string_array(sections, sectionCount); return false; }
    int objectCount = sectionCount - 1;
    if (!init_arrays(objectCount)) return false;
    objects.count = objectCount;
    for (int i = 0; i < objectCount; i++) {
        int tokenCount = 0;
        char **tokens = split_string(sections[i + 1], ',', &tokenCount);
        for (int j = 0; j + 1 < tokenCount; j += 2) {
            int key = atoi(tokens[j]);
            GDValueType type = get_value_type_for_key(key);
            GDValue val;
            if (type == GD_VAL_INT) val.i = atoi(tokens[j+1]);
            else if (type == GD_VAL_FLOAT) val.f = atof(tokens[j+1]);
            else val.b = (tokens[j+1][0] == '1');
            fill_object_data(i, key, type, val);
        }
        free_string_array(tokens, tokenCount);
        assign_object_to_section(i);
    }
    free_string_array(sections, sectionCount);
    return true;
}

int load_level(char *path) {
    size_t out;
    char *level = read_file(path, &out);
    if (!level) return 1;
    char *data = decompress_level(level);
    if (!data) return 2;
    parse_string(data);
    free(data);
    free(level);
    return 0;
}

void unload_level() {
    free_arrays();
    free_sections();
}
