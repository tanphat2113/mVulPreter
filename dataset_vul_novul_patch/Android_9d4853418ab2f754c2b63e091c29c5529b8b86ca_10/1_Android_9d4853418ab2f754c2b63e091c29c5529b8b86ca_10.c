int writepng_init(mainprog_info *mainprog_ptr)
{
    png_structp  png_ptr; /* note:  temporary variables! */
    png_infop  info_ptr;
 int color_type, interlace_type;


 
     /* could also replace libpng warning-handler (final NULL), but no need: */
 
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, mainprog_ptr,
       writepng_error_handler, NULL);
     if (!png_ptr)
         return 4;   /* out of memory */

    info_ptr = png_create_info_struct(png_ptr);
 if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
 return 4; /* out of memory */
 }


 /* setjmp() must be called in every function that calls a PNG-writing
     * libpng function, unless an alternate error handler was installed--
     * but compatible error handlers must either use longjmp() themselves
     * (as in this program) or some other method to return control to
     * application code, so here we go: */

 if (setjmp(mainprog_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
 return 2;
 }


 /* make sure outfile is (re)opened in BINARY mode */

    png_init_io(png_ptr, mainprog_ptr->outfile);


 /* set the compression levels--in general, always want to leave filtering
     * turned on (except for palette images) and allow all of the filters,
     * which is the default; want 32K zlib window, unless entire image buffer
     * is 16K or smaller (unknown here)--also the default; usually want max
     * compression (NOT the default); and remaining compression flags should
     * be left alone */

    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
/*
    >> this is default for no filtering; Z_FILTERED is default otherwise:
    png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
    >> these are all defaults:
    png_set_compression_mem_level(png_ptr, 8);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_method(png_ptr, 8);
 */


 /* set the image parameters appropriately */

 if (mainprog_ptr->pnmtype == 5)
        color_type = PNG_COLOR_TYPE_GRAY;
 else if (mainprog_ptr->pnmtype == 6)
        color_type = PNG_COLOR_TYPE_RGB;
 else if (mainprog_ptr->pnmtype == 8)
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
 else {
        png_destroy_write_struct(&png_ptr, &info_ptr);
 return 11;
 }

    interlace_type = mainprog_ptr->interlaced? PNG_INTERLACE_ADAM7 :
                                               PNG_INTERLACE_NONE;

    png_set_IHDR(png_ptr, info_ptr, mainprog_ptr->width, mainprog_ptr->height,
      mainprog_ptr->sample_depth, color_type, interlace_type,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

 if (mainprog_ptr->gamma > 0.0)
        png_set_gAMA(png_ptr, info_ptr, mainprog_ptr->gamma);

 if (mainprog_ptr->have_bg) { /* we know it's RGBA, not gray+alpha */
        png_color_16  background;

        background.red = mainprog_ptr->bg_red;
        background.green = mainprog_ptr->bg_green;
        background.blue = mainprog_ptr->bg_blue;
        png_set_bKGD(png_ptr, info_ptr, &background);
 }

 if (mainprog_ptr->have_time) {
        png_time  modtime;

 png_convert_from_time_t(&modtime, mainprog_ptr->modtime);
        png_set_tIME(png_ptr, info_ptr, &modtime);
 }

 if (mainprog_ptr->have_text) {
        png_text  text[6];
 int  num_text = 0;

 if (mainprog_ptr->have_text & TEXT_TITLE) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Title";
            text[num_text].text = mainprog_ptr->title;
 ++num_text;
 }
 if (mainprog_ptr->have_text & TEXT_AUTHOR) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Author";
            text[num_text].text = mainprog_ptr->author;
 ++num_text;
 }
 if (mainprog_ptr->have_text & TEXT_DESC) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Description";
            text[num_text].text = mainprog_ptr->desc;
 ++num_text;
 }
 if (mainprog_ptr->have_text & TEXT_COPY) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Copyright";
            text[num_text].text = mainprog_ptr->copyright;
 ++num_text;
 }
 if (mainprog_ptr->have_text & TEXT_EMAIL) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "E-mail";
            text[num_text].text = mainprog_ptr->email;
 ++num_text;
 }
 if (mainprog_ptr->have_text & TEXT_URL) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "URL";
            text[num_text].text = mainprog_ptr->url;
 ++num_text;
 }
        png_set_text(png_ptr, info_ptr, text, num_text);
 }


 /* write all chunks up to (but not including) first IDAT */

    png_write_info(png_ptr, info_ptr);


 /* if we wanted to write any more text info *after* the image data, we
     * would set up text struct(s) here and call png_set_text() again, with
     * just the new data; png_set_tIME() could also go here, but it would
     * have no effect since we already called it above (only one tIME chunk
     * allowed) */


 /* set up the transformations:  for now, just pack low-bit-depth pixels
     * into bytes (one, two or four pixels per byte) */

    png_set_packing(png_ptr);
/*  png_set_shift(png_ptr, &sig_bit);  to scale low-bit-depth values */


 /* make sure we save our pointers for use in writepng_encode_image() */

    mainprog_ptr->png_ptr = png_ptr;
    mainprog_ptr->info_ptr = info_ptr;


 /* OK, that's all we need to do for now; return happy */

 return 0;
}
