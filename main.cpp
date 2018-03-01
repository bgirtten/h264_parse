#include "mpeg4ip.h"
#include <getopt.h>
#include "mpeg4ip_bitstream.h"
#include <math.h>
#include "mp4av_h264.h"
#include "h264_parser.h"


//Example main app
int main (int argc, char **argv)
{
#define MAX_BUFFER 65536 * 8
  const char* usageString = 
    "[-version] <file-name>\n";
  const char *ProgName = argv[0];
  while (true) {
    int c = -1;
    int option_index = 0;
    static struct option long_options[] = {
      { "version", 0, 0, 'v' },
      { NULL, 0, 0, 0 }
    };

    c = getopt_long(argc, argv, "v",
        long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
      case '?':
        fprintf(stderr, "usage: %s %s", ProgName, usageString);
        exit(0);
      case 'v':
        fprintf(stderr, "%s - %s version %s\n", 
            ProgName, MPEG4IP_PACKAGE, MPEG4IP_VERSION);
        exit(0);
      default:
        fprintf(stderr, "%s: unknown option specified, ignoring: %c\n", 
            ProgName, c);
    }
  }

  /* check that we have at least one non-option argument */
  if ((argc - optind) < 1) {
    fprintf(stderr, "usage: %s %s", ProgName, usageString);
    exit(1);
  }

  uint8_t buffer[MAX_BUFFER];
  uint32_t buffer_on, buffer_size;
  uint64_t bytes = 0;
  FILE *m_file;
  h264_decode_t dec, prevdec;
  bool have_prevdec = false;
  memset(&dec, 0, sizeof(dec));
#if 0
  uint8_t count = 0;
  // this prints out the 8-bit to # of zero bit array that we use
  // to decode ue(v)
  for (uint32_t ix = 0; ix <= 255; ix++) {
    uint8_t ij;
    uint8_t bit = 0x80;
    for (ij = 0;
        (bit & ix) == 0 && ij < 8; 
        ij++, bit >>= 1);
    printf("%d, ", ij);
    count++;
    if (count > 16) {
      printf("\n");
      count = 0;
    }
  }
  printf("\n");
#endif

  fprintf(stdout, "%s - %s version %s\n", 
      ProgName, MPEG4IP_PACKAGE, MPEG4IP_VERSION);
  m_file = fopen(argv[optind], FOPEN_READ_BINARY);

  if (m_file == NULL) {
    fprintf(stderr, "file %s not found\n", *argv);
    exit(-1);
  }

  buffer_on = buffer_size = 0;
  while (!feof(m_file)) {
    bytes += buffer_on;
    if (buffer_on != 0) {
      buffer_on = buffer_size - buffer_on;
      memmove(buffer, &buffer[buffer_size - buffer_on], buffer_on);
    }
    buffer_size = fread(buffer + buffer_on, 
        1, 
        MAX_BUFFER - buffer_on, 
        m_file);
    buffer_size += buffer_on;
    buffer_on = 0;

    bool done = false;
    CBitstream ourbs;
    do {
      uint32_t ret;
      ret = h264_find_next_start_code(buffer + buffer_on, 
          buffer_size - buffer_on);
      if (ret == 0) {
        done = true;
        if (buffer_on == 0) {
          fprintf(stderr, "couldn't find start code in buffer from 0\n");
          exit(-1);
        }
      } else {
        // have a complete NAL from buffer_on to end
        if (ret > 3) {
          uint32_t nal_len;

          nal_len = remove_03(buffer + buffer_on, ret);

#if 0
          printf("Nal length %u start code %u bytes "U64"\n", nal_len, 
              buffer[buffer_on + 2] == 1 ? 3 : 4, bytes + buffer_on);
#else
          printf("Nal length %u start code %u bytes \n", nal_len, 
              buffer[buffer_on + 2] == 1 ? 3 : 4);
#endif
          ourbs.init(buffer + buffer_on, nal_len * 8);
          uint8_t type;
          type = h264_parse_nal(&dec, &ourbs);
          if (type >= 1 && type <= 5) {
            if (have_prevdec) {
              // compare the 2
              bool bound;
              bound = compare_boundary(&prevdec, &dec);
              printf("Nal is %s\n", bound ? "part of last picture" : "new picture");
            }
            memcpy(&prevdec, &dec, sizeof(dec));
            have_prevdec = true;
          } else if (type >= 9 && type <= 11) {
            have_prevdec = false; // don't need to check
          }
        }
#if 0
        printf("buffer on "X64" "X64" %u len %u %02x %02x %02x %02x\n",
            bytes + buffer_on, 
            bytes + buffer_on + ret,
            buffer_on, 
            ret,
            buffer[buffer_on],
            buffer[buffer_on+1],
            buffer[buffer_on+2],
            buffer[buffer_on+3]);
#endif
        buffer_on += ret; // buffer_on points to next code
      }
    } while (done == false);
  }
  fclose(m_file);
  return 0;
}
