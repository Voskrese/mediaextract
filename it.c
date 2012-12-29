#include "it.h"

int it_isfile(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t input_len = (size_t)(end - start);
	if (input_len < IT_HEADER_SIZE)
		return 0;

	if (MAGIC(start) != IT_MAGIC)
		return 0;

	uint16_t orders      = le16toh(*(uint16_t *)(start + 0x20));
	uint16_t instruments = le16toh(*(uint16_t *)(start + 0x22));
	uint16_t samples     = le16toh(*(uint16_t *)(start + 0x24));
	uint16_t patterns    = le16toh(*(uint16_t *)(start + 0x26));

	size_t length = IT_HEADER_SIZE + orders + instruments * 4 + samples * 4 + patterns * 4;

	if (input_len < length)
		return 0;

#define UPDATE_LENGTH(len) \
	{ \
		size_t _len = (len); \
		if (_len > length) \
		{ \
			length = _len; \
			if (input_len < length) \
				return 0; \
		} \
	}

	/* scan instruments */
	for (const uint32_t *para = (const uint32_t *)(start + IT_HEADER_SIZE + orders),
		*para_end = para + instruments;
		para < para_end; ++ para)
	{
		size_t off = (size_t)le32toh(*para);

		UPDATE_LENGTH(off + IT_INSTRUMENT_SIZE);
	}

	/* scan samples */
	for (const uint32_t *para = (const uint32_t *)(start + IT_HEADER_SIZE + orders + instruments * 4),
		*para_end = para + samples;
		para < para_end; ++ para)
	{
		size_t off = (size_t)le32toh(*para);
		const unsigned char *ptr = start + off;

		UPDATE_LENGTH(off + IT_SAMPLE_HEADER_SIZE);
		
		if (MAGIC(ptr) != IT_SAMPLE_MAGIC)
			continue;

		uint32_t sample_length  = le32toh(*(uint32_t *)(ptr + 0x30));
		uint32_t sample_pointer = le32toh(*(uint32_t *)(ptr + 0x48));

		if (sample_length && sample_pointer)
		{
			size_t sample_end = sample_pointer + sample_length;

			// there are some IT files out there with truncated samples:
			if (sample_end > input_len) sample_end = input_len;
			if (sample_end > length)    length     = sample_end;
		}
	}
	
	/* scan patterns */
	for (const uint32_t *para = (const uint32_t *)(start + IT_HEADER_SIZE + orders + instruments * 4 + samples * 4),
		*para_end = para + patterns;
		para < para_end; ++ para)
	{
		size_t off = (size_t)le32toh(*para);
		const unsigned char *ptr = start + off;

		UPDATE_LENGTH(off + IT_PATTERN_HEADER_SIZE);

		uint16_t pattern_length = le32toh(*(uint16_t *)ptr);
		size_t   pattern_end    = off + IT_PATTERN_HEADER_SIZE + pattern_length;

		// there are some IT files out there with truncated patterns:
		if (pattern_end > input_len) pattern_end = input_len;
		if (pattern_end > length)    length      = pattern_end;
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}