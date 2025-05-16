#include "fuzz_data.h"

#include <libconfig.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONFIG_SIZE 4096
#define MAX_PATH_SIZE 256
#define MIN_BUFF_SIZE sizeof(fuzz_data_t) + 2 // Room for fixed-width data and two null terminators
#define MAX_BUFF_SIZE sizeof(fuzz_data_t) + MAX_CONFIG_SIZE + MAX_PATH_SIZE + 1

// Forward-declare the libFuzzer's mutator callback.
extern size_t LLVMFuzzerMutate(uint8_t *Data, size_t Size, size_t MaxSize);

static uint8_t dummy_input[] = {
    0x0, // Lookup type
    0x17, 0x0, 0x0, 0x0, // Content size
    0x3, 0x0, 0x0, 0x0, // Path size
    '{', '\n', '\t', 'a', ':', '\n', '{', '\n', '\t', '\t', 'b', ' ', '=', ' ', '1', ';', '\n', '\t', '}', ';', '\n', '}', ';', '\0', // config
    'a', '.', 'b', '\0' // Path
};

/**
 * To be called statically from the harness, this function opens /dev/null (Only
 * Linux compatible)
 * @return: FILE pointer to /dev/null
 */
FILE *open_null_file()
{
    FILE *dev_null = fopen("/dev/null", "w");

    if (dev_null == NULL)
    {
        abort();
    }

    return dev_null;
}

// Recursive function to search for a setting by name
config_setting_t *find_setting_recursive(
    config_setting_t *setting,
    const char *name,
    const size_t name_len)
{
    if (NULL == setting || NULL == name)
    {
        return NULL;
    }

    // Check if the current setting's name matches
    const char *setting_name = config_setting_name(setting);
    if (NULL != setting_name && strlen(setting_name) == name_len) {
        if (strncmp(config_setting_name(setting), name, name_len) == 0) {
            return setting;
        }
    }

    // If it's a group, iterate over its children recursively
    if (config_setting_is_group(setting) || config_setting_is_array(setting)
        || config_setting_is_list(setting))
    {
        int count = config_setting_length(setting);
        for (int i = 0; i < count; ++i)
        {
            config_setting_t *child = config_setting_get_elem(setting, i);
            config_setting_t *result = find_setting_recursive(child, name, name_len);
            if (result != NULL)
            {
                return result; // Return the found setting
            }
        }
    }

    // Return NULL if not found
    return NULL;
}

size_t min_size(size_t a, size_t b)
{
    return (a <= b) ? a : b;
}

size_t LLVMFuzzerCustomMutator(uint8_t *data, size_t size,
    size_t max_size, unsigned int seed)
{
    size_t remaining_size = max_size;
    config_t cfg = {0};
    uint8_t* config_data = NULL;
    uint8_t* path_data = NULL;

    srand(seed);
    config_init(&cfg);

    if (size < MIN_BUFF_SIZE || max_size < MIN_BUFF_SIZE || max_size > MAX_BUFF_SIZE) {
        return MIN_BUFF_SIZE;
    }

    fuzz_data_t *fuzz_data = (fuzz_data_t *) data;
    remaining_size -= sizeof(fuzz_data_t);

    // Limit sizes to avoid overflow
    size_t max_content = min_size(remaining_size, MAX_CONFIG_SIZE - 1);
    fuzz_data->content_size = rand() % max_content;

    size_t max_path = min_size(remaining_size - fuzz_data->content_size, MAX_PATH_SIZE - 1);
    fuzz_data->path_size = rand() % max_path;

    if (fuzz_data->content_size + fuzz_data->path_size + sizeof(fuzz_data_t) >= max_size) {
        fuzz_data->content_size = max_content / 2;
        fuzz_data->path_size = max_path / 2;
    }

    fuzz_data_content(fuzz_data, &config_data);
    fuzz_data_path(fuzz_data, &path_data);

    // Mutate content and path safely
    fuzz_data->content_size = LLVMFuzzerMutate(config_data, fuzz_data->content_size, max_content);
    config_data[fuzz_data->content_size < max_content ? fuzz_data->content_size : max_content - 1] = '\0';

    fuzz_data->path_size = LLVMFuzzerMutate(path_data, fuzz_data->path_size, max_path);
    path_data[fuzz_data->path_size < max_path ? fuzz_data->path_size : max_path - 1] = '\0';

    return sizeof(fuzz_data_t) + fuzz_data->content_size + fuzz_data->path_size + 2;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, const size_t size)
{
    static FILE *dev_null;
    uint8_t data_buff[MAX_BUFF_SIZE] = {0};
    uint8_t scratch_mem[sizeof(uint64_t)] = {0};
    config_t cfg = {0};
    uint8_t *config_ptr;
    uint8_t *path_ptr;
    config_setting_t *root;
    int rc = -1;

    fuzz_data_t *fuzz_data = (fuzz_data_t*) data_buff;

    if (NULL == dev_null)
    {
        // Only called once per process
        dev_null = open_null_file();
    }

    if (size < MIN_BUFF_SIZE || size > MAX_BUFF_SIZE)
    {
        // Not enough bytes to be a fuzz_data_t
        rc = 0;
        goto end;
    }

    // Copy the data to a buffer that can be mutated
    memcpy(data_buff, data, size);

    config_init(&cfg);

    if (fuzz_data->content_size > MAX_CONFIG_SIZE || 
        fuzz_data->path_size > MAX_PATH_SIZE || 
        sizeof(fuzz_data_t) + fuzz_data->content_size + fuzz_data->path_size > size) { 
        goto end; 
    } 
    fuzz_data_content(fuzz_data, &config_ptr);
    fuzz_data_path(fuzz_data, &path_ptr);
    const char *config_data = (const char *) config_ptr;
    const char *path_data = (const char *) path_ptr;

    if (CONFIG_TRUE != config_read_string(&cfg, config_data)
        || NULL == (root = config_root_setting(&cfg)))
    {
        // Parsing failed
        goto end;
    }

    config_setting_t *setting = find_setting_recursive(
        root,
        (const char*) path_ptr,
        fuzz_data->path_size
    );

    if (NULL != setting)
    {
        config_setting_get_elem(setting, config_setting_length(setting) - 1);

        switch (fuzz_data->lookup_type % (CONFIG_TYPE_LIST + 1)) {
            case CONFIG_TYPE_FLOAT:
                config_setting_lookup_float(setting, path_data,
                    (double*) scratch_mem);
                break;
            case CONFIG_TYPE_BOOL:
                config_setting_lookup_bool(setting, path_data,
                    (int*) scratch_mem);
                break;
            case CONFIG_TYPE_INT:
                config_setting_lookup_int(setting, path_data,
                    (int*) scratch_mem);
                break;
            case CONFIG_TYPE_INT64:
                config_setting_lookup_int64(setting, path_data,
                    (long long *) scratch_mem);
                break;
            case CONFIG_TYPE_STRING: {
                const char *string_ptr = NULL;
                config_setting_lookup_string(setting, path_data, &string_ptr);
                break;
            }
            default:
                config_setting_lookup_const(setting, path_data);
        }
    }

    if (NULL != (setting = config_setting_get_member(root, path_data)))
    {
        // This setting exists, let's overwrite it
        config_setting_set_float(setting, 1.234);
    }
    else {
        // This setting does not exist, create it
        setting = config_setting_add(root, path_data, CONFIG_TYPE_FLOAT);

        if (setting == NULL)
        {
            rc = -1;
            goto end;
        }
        config_setting_set_float(setting, 1.234);
    }

    config_write(&cfg, dev_null);
    config_setting_remove(root, path_data);

    rc = 0;

    end:
    config_destroy(&cfg);
    return rc;
}
