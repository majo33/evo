#include "evolis.h"

void debug(const char *fmt, ...)
{

}

void info(const char *fmt, ...)
{
  fprintf(stdout, fmt);
  fputc('\n', stdout);
}

void fatal(const char *fmt, ...)
{
  fprintf(stderr, fmt);
  fputc('\n', stderr);
}

#define RgbToGray(r, g, b) (((long) (r)*74L + (long)(g)*155L +(long)(b)*27L) >> 8)

//dwSizeNeeded = header.cupsHeight * header.cupsWidth * 5;
long           dwSizeNeeded = 1016 * 648 * 5;     // size of lpMem in bytes

char
OverlayPannel[10],
OverlayBackPannel[10],
TreatementK[2];

int TB,LB,BB,RB,TW,LW,BW,RW;  //QQQ kde sa nastavuju???


// Prototypes...

//int CutPage(long *stop);



// Convert RVB to k functions
void GrayToFloyd(unsigned char *lpMemIn, unsigned char *lpMemOut, long Width, long Height);
void GrayToThreshold(unsigned char *lpMemIn, unsigned char *lpMemOut, long lNbrByte);


//===============================================================================//
// Reduce color data from 8 bits to uiBitComp.

// Entree : 1 color panel (1016 * 648 octets)
// Sortie : 1 color panel with usable data ((1016*648)*uiBitComp)/8
//===============================================================================//


long ReduceColor(unsigned char *lpMemIn, unsigned char *lpMemOut, int uiBitComp)//,int nbrline)
{
    long lIndex = 0, lIndex1 = 0;
    unsigned char *lpbBuf, *lpbData, *lpbDataOut;
    long lComp = 0;
    long lNbrByte = (1016*648);

    //Reservation m√î√∏Œ©oire pour traitement d'une ligne

    lpbBuf = malloc(8);

    if (!lpbBuf)
    {

        //fprintf(stderr, "**************** EVOLIS ReduceColor manque de memoire pour lpBuf: taille demande 8 octets... \n");
        fatal("ReduceColor Fails memory lpBuf: requested size 8 bytes...");

        /// sortir de l'impression....
        return (0);
    }

    //lpbData ppV mem bmp brute
    lpbData = &lpMemIn[0];

    lpbDataOut = &lpMemOut[0];
    // pour toutes les donnees
    while (lIndex < lNbrByte)
    {
        //Traite byte par 8
        for (lIndex1 = 0; lIndex1 < 8; lIndex1++)
        {
            if (lIndex < lNbrByte)
            {
                //decalage donnees utiles
                lpbBuf[lIndex1] = *(lpbData++) >> (8 - uiBitComp);
                lIndex++;
            }
            else
            {

                lpbBuf[lIndex1] = 0x00;
            }
        }

        switch (uiBitComp)
        {
            case 6:
                *(lpbDataOut++) = (lpbBuf[0] << 2) | (lpbBuf[1] >> 4);
                *(lpbDataOut++) = (lpbBuf[1] << 4) | (lpbBuf[2] >> 2);
                *(lpbDataOut++) = (lpbBuf[2] << 6) | (lpbBuf[3]);
                *(lpbDataOut++) = (lpbBuf[4] << 2) | (lpbBuf[5] >> 4);
                *(lpbDataOut++) = (lpbBuf[5] << 4) | (lpbBuf[6] >> 2);
                *(lpbDataOut++) = (lpbBuf[6] << 6) | (lpbBuf[7]);
                break;
            case 7:
                *(lpbDataOut++) = (lpbBuf[0] << 1) | (lpbBuf[1] >> 6);
                *(lpbDataOut++) = (lpbBuf[1] << 2) | (lpbBuf[2] >> 5);
                *(lpbDataOut++) = (lpbBuf[2] << 3) | (lpbBuf[3] >> 4);
                *(lpbDataOut++) = (lpbBuf[3] << 4) | (lpbBuf[4] >> 3);

                *(lpbDataOut++) = (lpbBuf[4] << 5) | (lpbBuf[5] >> 2);
                *(lpbDataOut++) = (lpbBuf[5] << 6) | (lpbBuf[6] >> 1);
                *(lpbDataOut++) = (lpbBuf[6] << 7) | (lpbBuf[7]);
                break;

            default:    //5 bits
                *(lpbDataOut++) = (lpbBuf[0] << 3) | (lpbBuf[1] >> 2);
                *(lpbDataOut++) = (lpbBuf[1] << 6) | (lpbBuf[2] << 1) | (lpbBuf[3] >> 4);
                *(lpbDataOut++) = (lpbBuf[3] << 4) | (lpbBuf[4] >> 1);

                *(lpbDataOut++) = (lpbBuf[4] << 7) | (lpbBuf[5] << 2) | (lpbBuf[6] >> 3);
                *(lpbDataOut++) = (lpbBuf[6] << 5) | (lpbBuf[7]);
                //break;
        }
    }
    free(lpbBuf);
    lComp = (((lNbrByte * 10) / 8) * uiBitComp);
    lComp /= 10;
    if (lNbrByte % 8)
        lComp++;


    return (lComp);
}

//===================================================//
// fonction pour le panneau noir & overlay uniquement
// entree : 1 octect = 1 points
// sortie : 1 octect = 8 points



//===================================================//
long ReduceBlack(unsigned char *lpMemIn, unsigned char *lpMemOut,int nbrline)
{
    long lIndex = 0, lIndex1 = 0;
    unsigned char *lpbData, *lpbDataOut;
    long lComp = 0;
    unsigned char bBuf = 0x00;

    unsigned char Mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    long lNbrByte = nbrline * 648;


    //lpbData ppV mem bmp brute
    lpbData = &lpMemIn[0];
    lpbDataOut = &lpMemOut[0];
    // pour toutes les donnees

    while (lIndex < lNbrByte)
    {


        //Traite byte par 8
        for (lIndex1 = 0; lIndex1 < 8; lIndex1++)

        {
            if (lIndex < lNbrByte)
            {
                if (*(lpbData++) == 0xFF)
                {
                    bBuf |= Mask[lIndex1];
                }
                lIndex++;
            }
        }
        *(lpbDataOut++) = bBuf;
        bBuf = 0x00;
        lComp++;
    }
    return (lComp);
}

//=====================================================================
//   Convert bitmap RVB data to evolis data format
//=====================================================================

//QQQ
//int DBNC(int col,int bl,int ov)//,int line)
//{
//    //output("\033Ss\015"); // debut sequence + CR

//    if(col == 1) {
//      //QQQ
//    }
//    else if (col == 2) {
//      //QQQ
//    }
//    else if (col == 3) {
//        DB128NC(0 * (dwSizeNeeded / 5), 'y');
//        DB128NC(1 * (dwSizeNeeded / 5), 'm');
//        DB128NC(2 * (dwSizeNeeded / 5), 'c');
//    }

//    if(bl){
//        DB2NC(3 * (dwSizeNeeded / 5), "ABP");
//    }

//    if(ov){
//        if(ov == 1){//overlay at front
//            DB2NC(0, OverlayPannel);
//        }
//        else {
//            DB2NC(0, OverlayBackPannel);
//        }
//    }

//    //output("\033Se\015.....................................");    // fin sequence + CR
//    return 0; // to return something
//}

// Telechargement d'un panneau non compresse 7 bits
// Input : ucData / raw data
//         color /y,m or c
// Return:      0 si OK
//              -1 si erreur ecriture
//===============================================================//
int DB128NC(unsigned char *lpMem, long lPos, char color)

{
    unsigned char *ucCommand, *ucCol;
    long lCommandSize = ((1016 * 648) * 7) / 8;
    long lComp;
    int numwritten = 0;

    //Reservation memoire pour une commande de telechargement
    ucCommand = malloc(lCommandSize + 11);
    if (!ucCommand)
    {
        fatal("DP128NC Error: Fails malloc... ");
        free(ucCommand);
        return (-1);
    }
    ucCol = &lpMem[lPos];
    strcpy((char*) ucCommand, "\033Db;y;128;");
    strncpy((char*) &ucCommand[4], &color, 1);
    lComp = ReduceColor(ucCol, &ucCommand[10], 7);//,1016);
    strncpy((char*) &ucCommand[lCommandSize + 10], "\015", 1);
    lCommandSize += 11;
    numwritten = fwrite(ucCommand, sizeof(unsigned char), lCommandSize, stdout);
    if (numwritten != lCommandSize)
    {
        fatal("DP128NC Error: Fails fwrite %d... \n", numwritten);

        free(ucCommand);
        return (-1);
    }
    free(ucCommand);
    return (0);
}

//===============================================================//
// Telechargement d'un panneau non compresse overlay noir
// Input : ucData / raw datal978
//         color /      k: noir
//                      f: full varnish
// Return:      0 si OK
//              -1 si erreur ecriture
//===============================================================//

int DB2NC(unsigned char *lpMem, long lPos, char pannel[10])
{
    unsigned char *ucCommand, *ucCol;
    long lComp;

    long lCommandSize = ((1016 * 648)) / 8;
    int numwritten = 0;
    int B, T, L, R, i = 0, j = 0;
    int pB, pT, ModuloB, ModuloT;

    //unsigned char        MaskB[8] = { 0x00,0x80,0xC0,0xE0,0xF0,0x7F,0x3F,0x1F};
    //unsigned char        MaskT[8] = { 0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F};
    unsigned char MaskB[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    unsigned char MaskT[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };


    //Reservation memoire pour une commande de telechargement

    ucCommand = malloc(lCommandSize + 9);

    //debug("DP2NC Size for one panel: %d... ", (lCommandSize + 9));


    if (!ucCommand)
    {
        fatal("DP2NC Error: Fails malloc... ");
        free(ucCommand);
        return (-1);
    }

    if (!(strcmp(pannel, "ABP")))// == NULL)
    {           // panneau noir
        ucCol = &lpMem[lPos];
        strcpy((char*) ucCommand, "\033Db;k;2;");
        lComp = ReduceBlack(ucCol, &ucCommand[8],1016);
        strncpy((char*) &ucCommand[lCommandSize + 8], "\015", 1);
        lCommandSize += 9;
        numwritten = fwrite(ucCommand, sizeof(unsigned char), lCommandSize, stdout);
        if (numwritten != lCommandSize)
        {
            fatal("DP2NC Error: Fails fprintf %d... ", numwritten);

            free(ucCommand);
            return (-1);
        }
    }
    else if (!(strcmp(pannel, "NO")))// == NULL)
    {       //No Varnish
        memset(ucCommand, 0x00, lCommandSize);
        strcpy((char*)ucCommand, "\033Db;o;2;");
        strncpy((char*) &ucCommand[lCommandSize + 8], "\015", 1);
        lCommandSize += 9;
        numwritten = fwrite(ucCommand, sizeof(unsigned char), lCommandSize, stdout);
        if (numwritten != lCommandSize)
        {
            fatal("DP2NC Error: Fails fwrite %d... ", numwritten);
            free(ucCommand);
            return (-1);
        }
    }
    else if (!(strcmp(pannel, "FO")))// == NULL)
    {   //Full Varnish
        memset(ucCommand, 0xFF, lCommandSize);
        strcpy((char*) ucCommand, "\033Db;o;2;");
        strncpy((char*) &ucCommand[lCommandSize + 8], "\015", 1);
        lCommandSize += 9;
        numwritten = fwrite(ucCommand, sizeof(unsigned char), lCommandSize, stdout);
        if (numwritten != lCommandSize)
        {
            fatal("DP2NC Error: Fails fwrite %d... ", numwritten);
            free(ucCommand);
            return (-1);
        }
    }
    else
    {

        if (!(strcmp(pannel, "OA")))// == NULL)
        {
            //Cover Overlay
            B = BB * 12;
            T = TB * 12;
            L = LB * 12;
            R = RB * 12;
            debug("DP2NC OA... ");
            memset(ucCommand, 0x00, lCommandSize);


            for (j = L; j <= R; j++)

            {
                pB = (j * 648) + (648 - B);
                ModuloB = pB % 8;
                pB /= 8;
                if (ModuloB)
                    pB++;
                ucCommand[pB + 7] = MaskB[ModuloB];
                pB++;
                pT = (j * 648) + (648 - T);
                ModuloT = pT % 8;
                pT /= 8;
                if (ModuloT)
                    pT++;
                ucCommand[pT + 7] = MaskT[ModuloT];
                pT--;
                for (i = (pB); i <= (pT); i++)
                {
                    ucCommand[i + 7] = 0xFF;
                }
            }
            B = BW * 12;
            T = TW * 12;
            L = LW * 12;
            R = RW * 12;
        }
        else if (!(strcmp(pannel, "SCI")))// == NULL)
        {
            B = 319;
            T = 106;
            L = 71;
            R = 307;

            memset(ucCommand, 0xFF, lCommandSize);
            debug("DP2NC SCI... ");
        }
        else if (!(strcmp(pannel, "SCA")))// == NULL)
        {
            B = 354;
            T = 177;
            L = 71;
            R = 283;

            memset(ucCommand, 0xFF, lCommandSize);
            debug("DP2NC SCA... ");
        }
        else if (!(strcmp(pannel, "MS")))// == NULL)
        {
            B = 216;
            T = 60;
            L = 0;
            R = 1016;

            memset(ucCommand, 0xFF, lCommandSize);
            debug("DP2NC MS... ");
        }
        //White part
        for (j = L; j <= R; j++)
        {
            pB = (j * 648) + (648 - B);
            ModuloB = pB % 8;
            pB /= 8;
            if (ModuloB)
                pB++;
            ucCommand[pB + 7] = MaskB[ModuloB];
            pB++;
            pT = (j * 648) + (648 - T);
            ModuloT = pT % 8;
            pT /= 8;
            if (ModuloT)
                pT++;
            ucCommand[pT + 7] = MaskT[ModuloT];
            pT--;
            for (i = (pB); i <= (pT); i++)
            {
                ucCommand[i + 7] = 0x00;
            }
        }//end of white part

        strcpy((char*) ucCommand, "\033Db;o;2;");

        strncpy((char*) &ucCommand[lCommandSize + 8], "\015", 1);
        lCommandSize += 9;
        numwritten = fwrite(ucCommand, sizeof(unsigned char), lCommandSize, stdout);
        if (numwritten != lCommandSize)
        {
            fatal("DP2NC Error: Fails fwrite %d... ", numwritten);
            free(ucCommand);
            return (-1);
        }


    }

    free(ucCommand);
    return (0);
}

//=====================================================================
//   Convert color bitmap to monochrome bitmap
//=====================================================================

void GrayToThreshold(unsigned char *lpMemIn, unsigned char *lpMemOut, long lNbrByte)
{
    long lIndex = 0;
    unsigned char *lpbData, *lpbDataOut;
    int gray, k;

    //lpbData ppV mem bmp brute
    lpbData = &lpMemIn[0];
    lpbDataOut = &lpMemOut[0];
    // pour toutes les donnees
    while (lIndex < lNbrByte)


    {
        gray = lpbData[lIndex];
        k = 0xff - gray;
        if (k > 128)
            gray = 0xFF;
        else
            gray = 0x00;
        lpbDataOut[lIndex] = (unsigned char) gray;
        lIndex++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////
///////                                                         GRAY  FLOYD
/////// Input parameters
///////                         lpMemIn : gray data (1 byte by dot)
///////                         Height  : height of the BMP image (648)
///////                         Width   : width of the BMP image (1016)
/////// Output parameter
///////                         lpMemOut: pointer on the monochrome data, result of the gray conversion
//////////////////////////////////////////////////////////////////////////////////////

void GrayToFloyd(unsigned char *lpMemIn, unsigned char *lpMemOut, long Width, long Height)
{
    long index_line = 0, index_dot = 0;
    unsigned char *lpbData, *lpbDataOut;
    int gray, k, l;
    int Error[1032];
    int NextError[1032];
    int Cerror;
    int CE3, CE5, CE7, CE1;

    if (Height < Width)
    {
        index_line = Width;

        Width = Height;
        Height = index_line;

    }

    for (index_line = 0; index_line < 1032; index_line++)
    {
        Error[index_line] = NextError[index_line] = 0;
    }
    //lpbData ppV mem bmp brute


    lpbData = &lpMemIn[0];
    lpbDataOut = &lpMemOut[0];
    // pour toutes les donnees
    for (index_line = 0; index_line < Height; index_line++)

    {
        //for(index_dot = Width ; index_dot > 0; index_dot--)
        for (index_dot = 0; index_dot < Width; index_dot++)
        {
            gray = lpbData[index_dot + (index_line * Width)];

            k = (gray + (int) Error[index_dot]);
            if (k > -128 && k < 128)
                l = 0x00;

            else
                l = 0xFF;


            lpbDataOut[index_dot + (index_line * Width)] = 0xFF - (unsigned char) l;



            Cerror = k - l;
            CE3 = (Cerror * 3 / 16);
            CE5 = (Cerror * 5 / 16);
            CE7 = (Cerror * 7 / 16);
            CE1 = Cerror - CE3 - CE5 - CE7;

            NextError[index_dot] += CE5;
            if (index_dot < 1031)
            {
                Error[index_dot + 1] += CE7;
                NextError[index_dot + 1] += CE1;
            }
            if (index_dot > 0)
                NextError[index_dot - 1] += CE3;
        }
        for (index_dot = 0; index_dot < 1032; index_dot++)
        {
            Error[index_dot] = NextError[index_dot];
            NextError[index_dot] = 0;
        }
    }
}

void ShareRVBtoMem(PidaOption_t *pida_opt, long Height, long Width, long WidthLine, cups_raster_t * ras, unsigned char *lpMem)
{
    //Pointeur sur image buffer
    unsigned char *lpImgBuf, *lpYellow, *lpMagenta, *lpCyan, *lpBlack, *lpGray;
    //valeur RVB courante & pr
    unsigned char red, green, blue, r, g, b;
    //index pour scruter le fichier bitmap
    long index_line, index_dot;
    long  stpt = 0;
   /* long  offx = 0, offy = 0; */
    pida_opt->yellowMeanValue =0;
    pida_opt->magentaMeanValue =0;
    pida_opt->cyanMeanValue =0;
    pida_opt->whitePixelCount = 0;
    pida_opt->pixelCount = 0;
    pida_opt->whiteThreshold = 255;

    lpImgBuf = malloc(WidthLine);
    if (!lpImgBuf)
    {
        fatal("ShareRVBtoMem fail to book memory: size requested %d bytes... \n", WidthLine);
        return;
    }
    lpYellow = &lpMem[0];
    //memset(lpYellow,0x00,Width * Height);
    memset(lpYellow,0x00,1016*648);
    lpMagenta = &lpMem[Width * Height];
    //memset(lpMagenta,0x00,Width * Height);
    memset(lpMagenta,0x00,1016*648);
    lpCyan = &lpMem[2 * Width * Height];
    //memset(lpCyan,0x00,Width * Height);
    memset(lpCyan,0x00,1016*648);
    lpBlack = &lpMem[3 * Width * Height];
    //memset(lpBlack,0x00,Width * Height);
    memset(lpBlack,0x00,1016*648);
    lpGray = &lpMem[4 * Width * Height];
    //memset(lpGray,0xFF,Width * Height);
    memset(lpGray,0xFF,1016*648);

    ////////////////
    //CAS PORTRAIT//
    ////////////////
    if (Height >= Width)
    {
        info("Portrait");
        //Pour toutes les lignes de l'image
        for (index_line = 0; index_line < Height; index_line++)
        {
            if (cupsRasterReadPixels(ras, lpImgBuf, WidthLine) == WidthLine)
            {
                r = 0xFF;
                g = 0xFF;
                b = 0xFF;
                stpt = 0;
                //Pour tous les points de la ligne
                for (index_dot = Width; index_dot > 0; index_dot--)
                {
                    pida_opt->pixelCount++;
                    red = lpImgBuf[(index_dot * 3)];
                    green = lpImgBuf[(index_dot * 3) + 1];
                    blue = lpImgBuf[(index_dot * 3) + 2];
                    lpGray[(index_line * 648) + index_dot] = (unsigned char) RgbToGray(red, green, blue);
                    if (blue == 0x00 && green == 0x00 && red == 0x00)
                    {
                        if (stpt == 0)
                        {
                            //Erreur sur le premier point blanc par defaut
                            lpYellow[(index_line * 648) + index_dot] = 0x00;
                            lpMagenta[(index_line * 648) + index_dot] = 0x00;
                            lpCyan[(index_line * 648) + index_dot] = 0x00;
                            lpBlack[(index_line * 648) + index_dot] = 0xFF;
                            //premier point trait...
                            stpt = 1;
                        }
                        else
                        {
                            //Cas point noir on met la couleur du point precedent
                            lpYellow[(index_line * 648) + index_dot] = ((0xFF - b));
                            lpMagenta[(index_line * 648) + index_dot] = ((0xFF - g));
                            lpCyan[(index_line * 648) + index_dot] = ((0xFF - r));
                            lpBlack[(index_line * 648) + index_dot] = 0xFF;

                        }
                    }
                    else
                    {
                        if ((r ==0xFF) && (g==0xFF) && (b==0xFF))
                        {
                                pida_opt->whitePixelCount++;
                        }
                        //Sauvegarde de la valeur precedente
                        r = red;
                        g = green;
                        b = blue;
                        //Traitement particulier pour le premier point
                        lpYellow[(index_line * 648) + index_dot] = ((0xFF - b));
                        lpMagenta[(index_line * 648) + index_dot] = ((0xFF - g));
                        lpCyan[(index_line * 648) + index_dot] = ((0xFF - r));
                        lpBlack[(index_line * 648) + index_dot] = 0x00;
                        //premier point traite
                        stpt = 1;
                        pida_opt->yellowMeanValue = pida_opt->yellowMeanValue + (double)lpYellow[(index_line * 648) + index_dot];
                        pida_opt->magentaMeanValue = pida_opt->magentaMeanValue + (double)lpMagenta[(index_line * 648) + index_dot];
                        pida_opt->cyanMeanValue = pida_opt->cyanMeanValue + (double)lpCyan[(index_line * 648) + index_dot];

                    }
                }   // for point de la line
            }   // test read line = cupsRasterReadPixels
        }   //for ligne
    }   //end if H>W
    /////////////////
    //CAS LANDSCAPE//
    /////////////////
    else
    {
        info("Landscape");
        for (index_line = (Height); index_line > 0; index_line--)
        {
            r = 0xFF;
            g = 0xFF;
            b = 0xFF;
            stpt = 0;
            if (cupsRasterReadPixels(ras, lpImgBuf, WidthLine) == WidthLine)
            {
                for (index_dot = 0; index_dot < Width; index_dot++)
                {
                    pida_opt->pixelCount++;
                    red = lpImgBuf[(index_dot * 3)];
                    green = lpImgBuf[(index_dot * 3) + 1];
                    blue = lpImgBuf[(index_dot * 3) + 2];
                    lpGray[(index_line) + (index_dot * Height)] = (unsigned char) RgbToGray(red, green, blue);

                    if (blue == 0x00 && green == 0x00 && red == 0x00)
                    {
                        if (stpt == 0)
                        {
                            //Erreur sur le premier point blanc par defaut
                            lpYellow[(index_line) + (index_dot * Height)] = 0x00;
                            lpMagenta[(index_line) + (index_dot * Height)] = 0x00;
                            lpCyan[(index_line) + (index_dot * Height)] = 0x00;
                            lpBlack[(index_line) + (index_dot * Height)] = 0xFF;
                            //premier point traite
                            stpt = 1;
                        }
                        else
                        {
                            //Cas point noir on met la couleur du point precedent
                            lpYellow[(index_line) + (index_dot * Height)] = ((0xFF - b));
                            lpMagenta[(index_line) + (index_dot * Height)] = ((0xFF - g));
                            lpCyan[(index_line) + (index_dot * Height)] = ((0xFF - r));
                            lpBlack[(index_line) + (index_dot * Height)] = (0xFF);
                        }
                    }
                    else
                    {
                        //ManageWhiteDots(pida_opt, r, g, b);
                        if ((r ==0xFF) && (g==0xFF) && (b==0xFF))
                        {
                                pida_opt->whitePixelCount++;
                        }
                        //Sauvegarde de la valeur precedente
                        r = red;
                        g = green;
                        b = blue;
                        //Traitement particulier pour le premier point
                        lpYellow[(index_line) + (index_dot * Height)] = ((0xFF - b));
                        lpMagenta[(index_line) + (index_dot * Height)] = ((0xFF - g));
                        lpCyan[(index_line) + (index_dot * Height)] = ((0xFF - r));
                        lpBlack[(index_line) + (index_dot * Height)] = 0x00;
                        //premier point traite
                        stpt = 1;
                        pida_opt->yellowMeanValue = pida_opt->yellowMeanValue + (double)lpYellow[(index_line) + (index_dot * Height)];
                        pida_opt->magentaMeanValue = pida_opt->magentaMeanValue + (double)lpMagenta[(index_line) + (index_dot * Height)];
                        pida_opt->cyanMeanValue = pida_opt->cyanMeanValue + (double)lpCyan[(index_line) + (index_dot * Height)];

                    }
                }   // for point de la line
            }   // test read line
        }       //for ligne
    }           //end else if H>W
    free(lpImgBuf);
}

// ----------------------------------------------------
//  static void ShareRVBtoMemWindow(long Height, long Width, long WidthLine, cups_raster_t * ras, int isPortrait )
//
// @author sa
// ----------------------------------------------------

void ShareRVBtoMemWindow(PidaOption_t *pida_opt, long Height, long Width, long WidthLine, cups_raster_t * ras, int isPortrait, unsigned char *lpMem)
{
    unsigned char *lpImgBuf, *lpYellow, *lpMagenta, *lpCyan, *lpBlack, *lpGray;         // Pointeur sur image buffer
    unsigned char red, green, blue, r, g, b;                                            // valeur RVB courante & presente
    long index_line, index_dot;                                                         // index pour scruter le fichier bitmap
    long  stpt = 0;                                                                     // Booleen indiquant le premier bit de la chaine
    long  offx = 0, offy = 0;
    long  Hfix = 1016, Wfix = 648;
    long  dot = 0, line =0;
    pida_opt->yellowMeanValue =0;
    pida_opt->magentaMeanValue =0;
    pida_opt->cyanMeanValue =0;
    pida_opt->whitePixelCount = 0;
    pida_opt->pixelCount = 0;
    pida_opt->whiteThreshold = 255;

    lpImgBuf = malloc(WidthLine);
    if (!lpImgBuf) {
        fatal("ShareRVBtoMem fail to book memory: size requested %d bytes... \n", WidthLine);
        return;
    }

    lpYellow = &lpMem[0];
    memset(lpYellow,0x00,1016*648);
    lpMagenta = &lpMem[1016*648];
    memset(lpMagenta,0x00,1016*648);
    lpCyan = &lpMem[2 * 1016*648];
    memset(lpCyan,0x00,1016*648);
    lpBlack = &lpMem[3 * 1016*648];
    memset(lpBlack,0x00,1016*648);
    lpGray = &lpMem[4 * 1016*648];
    memset(lpGray,0xFF,1016*648);

    if ( isPortrait )
    {
        if(Width%2)
        {
            cupsRasterReadPixels(ras, lpImgBuf, WidthLine);
            //info("Read odd line ");
            Width--;
        }
        if(Width > 648)
            offx = (Width - 648)/2;
        else
            offx = 0;

        if(Height > 1016)
            offy = (Height - 1016)/2;
        else
            offy = 0;

        Hfix = 1016;
        Wfix = 648;
    }
    else
    {
        if (Height%2)
        {
            cupsRasterReadPixels(ras, lpImgBuf, WidthLine);
            //info("Read odd line ");
            Height--;
        }
        if(Width > 1016)
            offx = (Width - 1016)/2;
        else
            offx = 0;

        if(Height > 648)
            offy = (Height - 648)/2;
        else
            offy = 0;
        Hfix = 648;
        Wfix = 1016;
    }

    //info(" offx : %d offy : %d", offx, offy);
    ////////////////
    //CAS PORTRAIT//
    ////////////////
    //if (Height >= Width)
    if ( isPortrait )
    {
        info("Portrait");
        for (index_line = (Height); index_line > (Height - offy); index_line--)
        {
            cupsRasterReadPixels(ras, lpImgBuf, WidthLine);
        }
        //info(" H : %d  W: %d",Hfix,Wfix);
        //Pour toutes les lignes de l'image
        line = 0;
        for (index_line; index_line > offy; index_line--)
        {
            if (cupsRasterReadPixels(ras, lpImgBuf, WidthLine) == WidthLine)
            {
                r = 0xFF;
                g = 0xFF;
                b = 0xFF;
                stpt = 0;
                //Pour tous les points de la ligne
                dot = 648;

                for (index_dot = (Width-offx); index_dot > offx; index_dot--)
                {
                    red = lpImgBuf[(index_dot * 3)];
                    green = lpImgBuf[(index_dot * 3) + 1];
                    blue = lpImgBuf[(index_dot * 3) + 2];
                    lpGray[(line*648) + (dot) ] = (unsigned char) RgbToGray(red, green, blue);
                    pida_opt->pixelCount++;

                    if (blue == 0x00 && green == 0x00 && red == 0x00)
                    {
                        if (stpt == 0)
                        {
                            //Erreur sur le premier point blanc par defaut
                            lpYellow[(line*648) + (dot)] = 0x00;
                            lpMagenta[(line*648) + (dot)] = 0x00;
                            lpCyan[(line*648) + (dot)] = 0x00;
                            lpBlack[(line*648) + (dot)] = 0xFF;
                            //premier point traite
                            stpt = 1;
                        }
                        else
                        {
                            //Cas point noir on met la couleur du point precedent
                            lpYellow[(line*648) + (dot)] = ((0xFF - b));
                            lpMagenta[(line*648) + (dot)] = ((0xFF - g));
                            lpCyan[(line*648) + (dot)] = ((0xFF - r));
                            lpBlack[(line*648) + (dot)] = 0xFF;
                        }
                    }
                    else
                    {
                        if ((r ==0xFF) && (g==0xFF) && (b==0xFF))
                        {
                                pida_opt->whitePixelCount++;
                        }
                        //Sauvegarde de la valeur precedente
                        r = red;
                        g = green;
                        b = blue;
                        //Traitement particulier pour le premier point
                        lpYellow[(line*648) + (dot)] = ((0xFF - b));
                        lpMagenta[(line*648) + (dot)] = ((0xFF - g));
                        lpCyan[(line*648) + (dot)] = ((0xFF - r));
                        lpBlack[(line*648) + (dot)] = 0x00;
                        //premier point traite
                        stpt = 1;
                        pida_opt->yellowMeanValue = pida_opt->yellowMeanValue + lpYellow[(line*648) + (dot)] / 1016 * 648;
                        pida_opt->magentaMeanValue = pida_opt->magentaMeanValue + lpMagenta[(line*648) + (dot)] / 1016 * 648;
                        pida_opt->cyanMeanValue = pida_opt->cyanMeanValue + lpCyan[(line*648) + (dot)] / 1016 * 648;

                    }
                    dot--;
                }   // for point de la line
            }   // test read line = cupsRasterReadPixels
            line++;
        }       //for ligne
        for (index_line ; index_line > 0; index_line--)
        {
            if(cupsRasterReadPixels(ras, lpImgBuf, WidthLine) != WidthLine)
            {
                debug("Erreur de lecture line CUPS15");
            }
        }
    }       //end if H>W
    /////////////////
    //CAS LANDSCAPE//
    /////////////////
    else
    {
        info("Landscape");
        //info("index_line: %d",Height);
        for (index_line = (Height); index_line > (Height - offy); index_line--)
        {
            if(cupsRasterReadPixels(ras, lpImgBuf, WidthLine) != WidthLine)
            {
                debug("erreur ");
            }
        }
        line = 648;//Hfix;
        for (index_line ; index_line > offy; index_line--)
        {
            r = 0xFF;
            g = 0xFF;
            b = 0xFF;
            stpt = 0;
            if (cupsRasterReadPixels(ras, lpImgBuf, WidthLine) == WidthLine)
            {
                dot = 0;
                for (index_dot = offx; index_dot < (Width - offx); index_dot++)
                {
                    pida_opt->pixelCount++;
                    red = lpImgBuf[(index_dot * 3)];
                    green = lpImgBuf[(index_dot * 3) + 1];
                    blue = lpImgBuf[(index_dot * 3) + 2];
                    lpGray[(line) + (dot * 648)] = (unsigned char) RgbToGray(red, green, blue);
                    if(blue == 0x00 && green == 0x00 && red == 0x00)
                    {
                        if (stpt == 0)
                        {
                            //Erreur sur le premier point blanc par defaut
                            lpYellow[(line) + (dot * 648)] = 0x00;
                            lpMagenta[(line) + (dot * 648)] = 0x00;
                            lpCyan[(line) + (dot * 648)] = 0x00;
                            lpBlack[(line) + (dot * 648)] = 0xFF;
                            //premier point traite
                            stpt = 1;
                        }
                        else
                        {
                            //Cas point noir on met la couleur du point precedent
                            lpYellow[(line) + (dot * 648)] = ((0xFF - b));
                            lpMagenta[(line) + (dot * 648)] = ((0xFF - g));
                            lpCyan[(line) + (dot * 648)] = ((0xFF - r));
                            lpBlack[(line) + (dot * 648)] = (0xFF);
                        }
                    }
                    else
                    {
                        if ((r ==0xFF) && (g==0xFF) && (b==0xFF))
                        {
                                pida_opt->whitePixelCount++;
                        }
                        //Sauvegarde de la valeur precedente
                        r = red;
                        g = green;
                        b = blue;
                        //Traitement particulier pour le premier point
                        lpYellow[(line) + (dot * 648)] = ((0xFF - b));
                        lpMagenta[(line) + (dot * 648)] = ((0xFF - g));
                        lpCyan[(line) + (dot * 648)] = ((0xFF - r));
                        lpBlack[(line) + (dot * 648)] = 0x00;
                        //premier point traite
                        stpt = 1;
                        pida_opt->yellowMeanValue = pida_opt->yellowMeanValue + lpYellow[(line) + (dot * 648)]  / 1016 * 648;
                        pida_opt->magentaMeanValue = pida_opt->magentaMeanValue + lpMagenta[(line) + (dot * 648)]  / 1016 * 648;
                        pida_opt->cyanMeanValue = pida_opt->cyanMeanValue + lpCyan[(line) + (dot * 648)]  / 1016 * 648;
                    }
                    dot++;
                }   // for point de la line
            }   // test read line
            line--;
        }       //for ligne
        for (index_line; index_line > 0; index_line--)
        {
            if(cupsRasterReadPixels(ras, lpImgBuf, WidthLine) != WidthLine)
            {
                debug("erreur ");
            }
        }
    }           //end else if H>W
    free(lpImgBuf);
}
