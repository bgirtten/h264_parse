#include <math.h>
#include "mpeg4ip.h"
#include "mpeg4ip_bitstream.h"
#include "mp4av_h264.h"

uint32_t h264_find_next_start_code (uint8_t *pBuf, uint32_t bufLen);
uint8_t h264_parse_nal (h264_decode_t *dec, CBitstream *bs);
bool compare_boundary (h264_decode_t *prev, h264_decode_t *on);
uint32_t remove_03 (uint8_t *bptr, uint32_t len);

