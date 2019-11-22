
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

static void actrns_encode_strtab(const char *valstrp, int **arrvalxp, int *idvalp)
{
    int stlen = strlen(valstrp);
    
    *arrvalxp = calloc(2 * stlen, sizeof(int));

    int idx = 0;

    while (idx < stlen) {
	actrns_encode_bytes((const acbyte *)valstrp, stlen, &idx, *arrvalxp, idvalp);
    }
}

static void actrns_decode_strtab(int *arrvalxp, int tablen, const char *valstrp, int *idstrp)
{
    int idx = 0;
    while (idx < tablen) {
	actrns_decode_bytes(arrvalxp, tablen, &idx, valstrp, idstrp);
    }
}

static void actrns_enocde_string(const char *valstrp, char *outstrp)
{
    int *arrvalxp = NULL;
    int idvalx = 0;
    actrns_encode_strtab(valstrp, &arrvalxp, &idvalx);

    int idstdx = 0;
    actrns_decode_strtab(arrvalxp, idvalx, outstrp, &idstdx);
}

int main()
{
    const char *charx = "this is my first string tests !!!";
    char txt[500];

    actrns_enocde_string(charx, txt);
}
