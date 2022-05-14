 static char *get_header(FILE *fp)
 {
    long start;
     /* First 1024 bytes of doc must be header (1.7 spec pg 1102) */
    char *header;
    header = calloc(1, 1024);
    start = ftell(fp);
     fseek(fp, 0, SEEK_SET);
     SAFE_E(fread(header, 1, 1023, fp), 1023, "Failed to load PDF header.\n");
     fseek(fp, start, SEEK_SET);
     return header;
 }
