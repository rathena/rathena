#ifndef FUZZ_DATA_H
#define FUZZ_DATA_H
#include <stdint.h>
#include <stddef.h>


typedef struct
{
    uint8_t lookup_type;
    uint32_t content_size;
    uint32_t path_size;
    uint8_t data[];
} __attribute__((packed)) fuzz_data_t;

/**
* @brief Retrieve the content from the fuzz_data object
* @param fuzz_data The fuzz_data object
* @param[out] buff: The buffer to point towards the configuration data
*/
void fuzz_data_content(fuzz_data_t *fuzz_data, uint8_t **buff);

/**
* @brief Retrieve the path from the fuzz_data object
* @param fuzz_data The fuzz_data object
* @param[out] buff: The buffer to point towards the path
*/
void fuzz_data_path(fuzz_data_t *fuzz_data, uint8_t **buff);

#endif //FUZZ_DATA_H
