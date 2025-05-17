#include "fuzz_data.h"

void fuzz_data_content(fuzz_data_t *fuzz_data, uint8_t **buff)
{
  *buff = fuzz_data->data;

  // Ensure the buffer is null terminated
  (*buff)[fuzz_data->content_size] = '\0';
}

void fuzz_data_path(fuzz_data_t *fuzz_data, uint8_t **buff)
{
  *buff = fuzz_data->data + fuzz_data->content_size + 1;

  // Ensure the buffer is null terminated
  (*buff)[fuzz_data->path_size] = '\0';
}

