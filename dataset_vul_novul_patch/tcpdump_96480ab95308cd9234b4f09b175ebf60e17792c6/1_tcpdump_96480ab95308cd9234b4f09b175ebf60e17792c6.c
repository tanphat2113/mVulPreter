print_trans(netdissect_options *ndo,
            const u_char *words, const u_char *data1, const u_char *buf, const u_char *maxbuf)
{
    u_int bcc;
    const char *f1, *f2, *f3, *f4;
    const u_char *data, *param;
    const u_char *w = words + 1;
    int datalen, paramlen;

    if (request) {
	ND_TCHECK2(w[12 * 2], 2);
	paramlen = EXTRACT_LE_16BITS(w + 9 * 2);
	param = buf + EXTRACT_LE_16BITS(w + 10 * 2);
	datalen = EXTRACT_LE_16BITS(w + 11 * 2);
	data = buf + EXTRACT_LE_16BITS(w + 12 * 2);
	f1 = "TotParamCnt=[d] \nTotDataCnt=[d] \nMaxParmCnt=[d] \nMaxDataCnt=[d]\nMaxSCnt=[d] \nTransFlags=[w] \nRes1=[w] \nRes2=[w] \nRes3=[w]\nParamCnt=[d] \nParamOff=[d] \nDataCnt=[d] \nDataOff=[d] \nSUCnt=[d]\n";
	f2 = "|Name=[S]\n";
	f3 = "|Param ";
	f4 = "|Data ";
    } else {
	ND_TCHECK2(w[7 * 2], 2);
	paramlen = EXTRACT_LE_16BITS(w + 3 * 2);
	param = buf + EXTRACT_LE_16BITS(w + 4 * 2);
	datalen = EXTRACT_LE_16BITS(w + 6 * 2);
	data = buf + EXTRACT_LE_16BITS(w + 7 * 2);
	f1 = "TotParamCnt=[d] \nTotDataCnt=[d] \nRes1=[d]\nParamCnt=[d] \nParamOff=[d] \nRes2=[d] \nDataCnt=[d] \nDataOff=[d] \nRes3=[d]\nLsetup=[d]\n";
	f2 = "|Unknown ";
	f3 = "|Param ";
	f4 = "|Data ";
    }

    smb_fdata(ndo, words + 1, f1, min(words + 1 + 2 * words[0], maxbuf),
        unicodestr);

    ND_TCHECK2(*data1, 2);
    bcc = EXTRACT_LE_16BITS(data1);
     ND_PRINT((ndo, "smb_bcc=%u\n", bcc));
     if (bcc > 0) {
 	smb_fdata(ndo, data1 + 2, f2, maxbuf - (paramlen + datalen), unicodestr);
	if (strcmp((const char *)(data1 + 2), "\\MAILSLOT\\BROWSE") == 0) {
 	    print_browse(ndo, param, paramlen, data, datalen);
 	    return;
 	}
 
	if (strcmp((const char *)(data1 + 2), "\\PIPE\\LANMAN") == 0) {
 	    print_ipc(ndo, param, paramlen, data, datalen);
 	    return;
 	}
 
 	if (paramlen)
 	    smb_fdata(ndo, param, f3, min(param + paramlen, maxbuf), unicodestr);
	if (datalen)
	    smb_fdata(ndo, data, f4, min(data + datalen, maxbuf), unicodestr);
    }
    return;
trunc:
    ND_PRINT((ndo, "%s", tstr));
}
