static int tga_readheader(FILE *fp, unsigned int *bits_per_pixel,
                          unsigned int *width, unsigned int *height, int *flip_image)
{
    int palette_size;
    unsigned char tga[TGA_HEADER_SIZE];
    unsigned char id_len, /*cmap_type,*/ image_type;
    unsigned char pixel_depth, image_desc;
    unsigned short /*cmap_index,*/ cmap_len, cmap_entry_size;
    unsigned short /*x_origin, y_origin,*/ image_w, image_h;

    if (!bits_per_pixel || !width || !height || !flip_image) {
        return 0;
    }

    if (fread(tga, TGA_HEADER_SIZE, 1, fp) != 1) {
        fprintf(stderr,
                "\nError: fread return a number of element different from the expected.\n");
        return 0 ;
    }
     id_len = tga[0];
     /*cmap_type = tga[1];*/
     image_type = tga[2];
    /*cmap_index = get_ushort(&tga[3]);*/
    cmap_len = get_ushort(&tga[5]);
     cmap_entry_size = tga[7];
 
 
 #if 0
    x_origin = get_ushort(&tga[8]);
    y_origin = get_ushort(&tga[10]);
 #endif
    image_w = get_ushort(&tga[12]);
    image_h = get_ushort(&tga[14]);
     pixel_depth = tga[16];
     image_desc  = tga[17];
 
    *bits_per_pixel = (unsigned int)pixel_depth;
    *width  = (unsigned int)image_w;
    *height = (unsigned int)image_h;

    /* Ignore tga identifier, if present ... */
    if (id_len) {
        unsigned char *id = (unsigned char *) malloc(id_len);
        if (id == 0) {
            fprintf(stderr, "tga_readheader: memory out\n");
            return 0;
        }
        if (!fread(id, id_len, 1, fp)) {
            fprintf(stderr,
                    "\nError: fread return a number of element different from the expected.\n");
            free(id);
            return 0 ;
        }
        free(id);
    }

    /* Test for compressed formats ... not yet supported ...
    if (image_type > 8) {
        fprintf(stderr, "Sorry, compressed tga files are not currently supported.\n");
        return 0 ;
    }

    *flip_image = !(image_desc & 32);

    /* Palettized formats are not yet supported, skip over the palette, if present ... */
    palette_size = cmap_len * (cmap_entry_size / 8);

    if (palette_size > 0) {
        fprintf(stderr, "File contains a palette - not yet supported.");
        fseek(fp, palette_size, SEEK_CUR);
    }
    return 1;
}
