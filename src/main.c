#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "evolis.h"

#define PORTRAIT 1

void writeToFile(const char *filename, const unsigned char *data, unsigned len)
{
  FILE *file = fopen(filename, "wb");

  if (file == NULL) {
    fatal("Can't create file");
  }

  if (fwrite(data, len, 1, file) != 1) {
    fatal("Error white writting data");
  }

  fclose(file);
}

int main(int argc, char *argv[])
{
  unsigned char *mem = malloc(WIDTH * HEIGHT * LAYERS);
  //QQQ check allocation

  int fd = 0;
  cups_raster_t *ras;
  ras = cupsRasterOpen(fd, CUPS_RASTER_READ);

  if (ras == NULL) {
    fatal("Can't open raster data!");
    return -1;
  }

  info("Raster loaded");

  cups_page_header_t header;

  if (cupsRasterReadHeader(ras, &header)) {
    info("Header loaded");
    PidaOption_t info;

    int cOrientation = 0; //QQQ nacitat z cups
    int isPortrait = ( header.Orientation == 1 || header.Orientation == 3 || cOrientation == PORTRAIT );


    if (header.cupsHeight == WIDTH && header.cupsWidth == WIDTH) {
      ShareRVBtoMem(&info, header.cupsHeight, header.cupsWidth,  header.cupsBytesPerLine, ras, mem);
    } else {
      ShareRVBtoMemWindow(&info, header.cupsHeight, header.cupsWidth,  header.cupsBytesPerLine, ras, isPortrait, mem);
    }

    int col = 3;
    int layerSize = WIDTH * HEIGHT;

    if (col == 3) {
      unsigned char *yellow  = &mem[0 * layerSize];
      unsigned char *magenta = &mem[1 * layerSize];
      unsigned char *cyan    = &mem[2 * layerSize];

      const int bufferSize = (WIDTH * HEIGHT * 7) / 8;
      unsigned char *buffer = malloc(bufferSize);

      ReduceColor(yellow, buffer, 7);
      writeToFile("yellow", buffer, bufferSize);
      ReduceColor(magenta, buffer, 7);
      writeToFile("magenta", buffer, bufferSize);
      ReduceColor(cyan, buffer, 7);
      writeToFile("cyan", buffer, bufferSize);
      free(buffer);
    }

    int bl = 1;

    if (bl) {
      const int bufferSize = WIDTH * HEIGHT / 8;
      unsigned char *buffer = malloc(bufferSize);
      unsigned char *black = &mem[3 * layerSize];
//      DB2NC(mem, 3 * layerSize, "ABP");
      ReduceBlack(black, buffer, WIDTH);
      writeToFile("black", buffer, bufferSize);
      free(buffer);
    }
    cupsRasterClose(ras); //QQQ nezabudnut zatvorit pri chybe
  } else {
    fatal("Can't read cups raster header");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
