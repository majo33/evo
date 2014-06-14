#ifndef EVOLIS_H
#define EVOLIS_H

#include <cups/raster.h>

#define WIDTH 1016
#define HEIGHT 648
#define LAYERS 5

typedef struct PidaOption_tag {
   unsigned long whitePixelCount;
   double yellowMeanValue;
   double magentaMeanValue;
   double cyanMeanValue;
   unsigned char whiteThreshold;
   unsigned long pixelCount;
} PidaOption_t;

void debug(const char *fmt, ...);

void info(const char *fmt, ...);

void fatal(const char *fmt, ...);

void ShareRVBtoMem(PidaOption_t *pida_opt, long Height, long Width, long WidthLine, cups_raster_t * ras, unsigned char *lpMem);
void ShareRVBtoMemWindow(PidaOption_t *pida_opt, long Height, long Width, long WidthLine, cups_raster_t * ras, int isPortrait, unsigned char *lpMem);

int DB128NC(unsigned char *lpMem, long lPos, char color); //y,m,c pannels 7 bits per color
int DB2NC(unsigned char *lpMem, long lPos, char pannel[10]);  // k,o panel 2 levels

long ReduceBlack(unsigned char *lpMemIn, unsigned char *lpMemOut,int nbrline);
long ReduceColor(unsigned char *lpMemIn, unsigned char *lpMemOut, int uiBitComp);

#endif // EVOLIS_H
