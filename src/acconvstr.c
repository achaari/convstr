
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef unsigned char acbyte;

static void actrns_encode_bytes(const acbyte *valarrp, int mlen, int *idvxp, int *valxp, int *idvlxp)
{
    unsigned int valx = 0, pwd = 0;

    for (int idx = 0; idx < 4; idx++) {
	if ((*idvxp) < mlen) {
	    valx += (idx ? (1 << (idx * 8)) : 1) * valarrp[(*idvxp)++];
	}
    }

    for (int idx = 0; idx < 5; idx++) {
	if (idx == 0) {
	    pwd = valx & 63;
	}
	else if (idx == 4) {
	    pwd = valx >> 24;
	}
	else {
	    pwd = (valx & (63 << (idx * 6))) >> (idx * 6);
	}
	valxp[(*idvlxp)++] = pwd;
    }

}

static void actrns_decode_bytes(const int *valxp, int mlen, int *idvxp, acbyte *valarrp, int *idarrxp)
{
    unsigned int valx = 0, pwd = 0;

    for (int idx = 0; idx < 5; idx++) {
	valx += (idx ? (1 << (idx * 6)) : 1) * valxp[(*idvxp)++];
    }

    for (int idx = 0; idx < 4; idx++) {
	pwd = (valx & (255 << (idx * 8)));
	if (pwd) {
	    if (idx) {
		pwd = pwd >> (idx * 8);
	    }
	    valarrp[(*idarrxp)++] = (acbyte)pwd;
	}
    }

}

static void actrns_encode_strtab(const char *valstrp, int stlen, int **arrvalxp, int *idvalp)
{
    *arrvalxp = calloc(2 * stlen, sizeof(int));

    int idx = 0;

    while (idx < stlen) {
	actrns_encode_bytes((const acbyte *)valstrp, stlen, &idx, *arrvalxp, idvalp);
    }
}

static void actrns_decode_strtab(int *arrvalxp, int tablen, char *valstrp, int *idstrp)
{
    int idx = 0;
    while (idx < tablen) {
	actrns_decode_bytes(arrvalxp, tablen, &idx, valstrp, idstrp);
    }

    valstrp[*idstrp] = '\0';
}

static const char *vrxtab = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char accmpr_get_randc()
{
    srand(time(0));

    return vrxtab[rand() % 62];
}

static void actrns_encode_strval(const char *str, int **idxarp, int *maxlvp)
{
    char *codes;

    codes = calloc(strlen(str) + 30, sizeof(char));

    char crnd = accmpr_get_randc();

    sprintf(codes, "%c:%d:%s:%c", crnd, strlen(str), str,  crnd);

    actrns_encode_strtab((const acbyte*)codes, strlen(codes), idxarp, maxlvp);

    free(codes);
    codes = NULL;
}

static int actrns_count_digit(int intref)
{
    char idxs[30];
    sprintf(idxs, "%d", intref);
    return strlen(idxs);
}


static void actrns_decode_strval(int *arrvalxp, int tablen, char *valstrp)
{
    char *codes = calloc(tablen * 2, sizeof(char));

    int idstdx = 0;

    *valstrp = '\0';

    actrns_decode_strtab(arrvalxp, tablen, codes, &idstdx);

    char cmdchr = codes[0];
    if (cmdchr != codes[strlen(codes) - 1] || codes[1] != ':' || codes[strlen(codes) - 2] != ':') {
	return;
    }

    int cdlen = strlen(codes);
    int stlen = atoi(codes + 2);
    int nbdgt = actrns_count_digit(stlen);

    char *codecs = codes + 3 + nbdgt;

    if (stlen != (cdlen - nbdgt - 5)) {
	return;
    }
    else if (codecs[stlen] != ':' || *(codecs - 1) != ':' || codecs[stlen + 1] != cmdchr || codecs[stlen + 2] != '\0') {
	return;
    }

    codecs[stlen] = '\0';

    strcpy(valstrp, codecs);

    free(codes);
    codes = NULL;    
}

#define posex  33 
#define posvx  (posex + 11)	
#define lista  200

static void actrns_encode_inttab(int *lnarr, int lnx, char *txt)
{
    int idx = 0, lsd, lvd;
    char buff[10];

    *txt = '\0';

    while (idx <= lnx) {
	if ((char)lnarr[idx] == '\0') {
	    sprintf(buff, "%c", (acbyte)posex);
	}
	else {
	    lsd = lnarr[idx] / lista;
	    lvd = lnarr[idx] % lista + posvx;

	    if (lsd) {
		sprintf(buff, "%c%c", (acbyte)(lsd + posex), (acbyte)lvd);
	    }
	    else {
		sprintf(buff, "%c", (acbyte)lvd);
	    }
	}

	strcat(txt, buff);

	idx++;
    }
}

static void actrns_decode_inttab(const char *str, int *maxlvp, int **idxarp)
{
    int idx = 0, lvd = 0, lnx = strlen(str);
    acbyte lvx;

    *idxarp = calloc(lnx, sizeof(int));

    while (idx < lnx) {
	lvx = str[idx];

	if (lvx < posex) {
	    /* Sould not happen */
	    free(*idxarp);
	    *idxarp = NULL;
	    *maxlvp = 0;
	    break;
	}
	if (lvx == posex) {
	    lvd = (int)'\0';
	}
	else if (lvx < posvx) {
	    lvd = ((int)lvx - posex) * lista + (int)(acbyte)str[++idx] - posvx;
	}
	else {
	    lvd = (int)(acbyte)lvx - posvx;
	}

	(*idxarp)[(*maxlvp)++] = lvd;
	++idx;
    }
}

static void actrns_encode_string(const char *valstrp, char *outstrp)
{
    int *arrvalxp = NULL;
    int idvalx = 0;
    actrns_encode_strval(valstrp, &arrvalxp, &idvalx);

    actrns_encode_inttab(arrvalxp, idvalx, outstrp);

    free(arrvalxp);
    arrvalxp = NULL;
}

static void actrns_decode_string(const char *valstrp, char *outstrp)
{
    int *arrvalxp = NULL;
    int idvalx = 0;

    actrns_decode_inttab(valstrp, &idvalx, &arrvalxp);

    actrns_decode_strval(arrvalxp, idvalx - 1, outstrp);

    free(arrvalxp);
    arrvalxp = NULL;
}


int main()
{
    const char *charx = "this is my first string tests !!!";
    char txt[500], dectxt[500];

    actrns_encode_string(charx, txt);

    actrns_decode_string(txt, dectxt);
}
