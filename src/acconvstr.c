
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char acbyte;

static void accmpr_getbytes(const acbyte *valarrp, int mlen, int *idvxp, int *lfp, int *rgp)
{
    if (*rgp == -1) {
	int xval = (int) valarrp[mlen - 1 - (*idvxp)++];
	*lfp = (xval & 120) >> 4;
	*rgp = xval & 15;
    }
    else {
	*lfp = *rgp;
	*rgp = -1;
    }
}

static void accmpr_decode_idx(const acbyte *valarrp, int mlen,  int *idvxp, int *idrxp, int idrx, int *rgp)
{
    int lf = 0, op = 1, pr = 0, idx = 0, pwr = 0;
    
    while (op && (*idvxp <= mlen)) {
	accmpr_getbytes(valarrp, mlen, idvxp, &lf, rgp);

	op = lf & 1;
	pr = lf >> 1;

	if (pr) {
	    idx += (pwr ? (1 << (pwr * 3)) : 1) * pr;
	}

	if ((++pwr > 3) && op) {
	    idx += 1000;
	    break;
	}
    }

    idrxp[idrx] = idx + idrx % 8;
}

static void accmpr_decode_idxarr(const acbyte *valarrp, int len, int *maxlvp, int **idxarp)
{
    *idxarp = calloc(2 * len + 1, sizeof(int));
    *maxlvp = 0;
    

    if (idxarp == NULL) {
	return;
    }

    int itr = 0, rg = -1;
    while (itr <= len) {
	accmpr_decode_idx(valarrp, len, &itr, *idxarp, (*maxlvp)++, &rg);
    }
}

static const char *vrxtab = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char accmpr_get_randc()
{
    srand(time(0));

    return vrxtab[rand() % 62];
}

static void accmpr_decode_strtab(const char *str, int *maxlvp, int **idxarp)
{
    char *codes;

    codes = calloc(strlen(str) + 30, sizeof(char));

    char crnd = accmpr_get_randc();

    sprintf(codes, "%c:%s:%c%c%c:%d:%c", crnd, str, str[strlen(str)-1], crnd, str[0], strlen(str), crnd);

    accmpr_decode_idxarr((const acbyte*)codes, strlen(codes), maxlvp, idxarp);

    free(codes);
    codes = NULL;
}

#define posex  33 
#define posda  (posex + 1) 
#define posvx  (posda + 10)	
#define lista  200

void accmpr_decode_str(const char *charx, char *txt)
{
    int lnx = 0;
    int *lnarr = NULL;
    
    accmpr_decode_strtab(charx, &lnx, &lnarr);

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
		sprintf(buff, "%c%c", (acbyte)(lsd + posvx), (acbyte)lvd);
	    }
	    else {
		sprintf(buff, "%c", (acbyte)lvd);
	    }
	}

	strcat(txt, buff);

	idx++;
    }

    free(lnarr);
    lnarr = NULL;
}

int main() 
{
    const char *charx = "this is my first string tests !!!";
    char txt[500];

    accmpr_decode_str(charx, txt);
    
    int aa = strlen(txt) - strlen(charx);

    printf(txt);

    return 0;
}
