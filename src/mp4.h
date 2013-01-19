#ifndef MEDIAEXTRACT_MP4_H__
#define MEDIAEXTRACT_MP4_H__

#include "mediaextract.h"

#define MP4_MAGIC MAGIC("ftyp")
#define MP4_MAGIC_OFFSET  4
#define MP4_HEADER_SIZE  16

int mp4_isfile(const uint8_t *data, size_t input_len, struct file_info *info);

#endif /* MEDIAEXTRACT_MP4_H__ */