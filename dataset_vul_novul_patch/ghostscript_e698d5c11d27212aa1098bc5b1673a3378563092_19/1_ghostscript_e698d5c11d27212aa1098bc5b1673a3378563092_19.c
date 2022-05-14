 jbig2_text_region(Jbig2Ctx *ctx, Jbig2Segment *segment, const byte *segment_data)
 {
    int offset = 0;
     Jbig2RegionSegmentInfo region_info;
     Jbig2TextRegionParams params;
     Jbig2Image *image = NULL;
     Jbig2SymbolDict **dicts = NULL;
    int n_dicts = 0;
     uint16_t flags = 0;
     uint16_t huffman_flags = 0;
     Jbig2ArithCx *GR_stats = NULL;
     int code = 0;
     Jbig2WordStream *ws = NULL;
     Jbig2ArithState *as = NULL;
    int table_index = 0;
     const Jbig2HuffmanParams *huffman_params = NULL;
 
     /* 7.4.1 */
    if (segment->data_length < 17)
        goto too_short;
    jbig2_get_region_segment_info(&region_info, segment_data);
    offset += 17;

    /* 7.4.3.1.1 */
    flags = jbig2_get_uint16(segment_data + offset);
    offset += 2;

    jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number, "text region header flags 0x%04x", flags);

    /* zero params to ease cleanup later */
    memset(&params, 0, sizeof(Jbig2TextRegionParams));

    params.SBHUFF = flags & 0x0001;
    params.SBREFINE = flags & 0x0002;
    params.LOGSBSTRIPS = (flags & 0x000c) >> 2;
    params.SBSTRIPS = 1 << params.LOGSBSTRIPS;
    params.REFCORNER = (Jbig2RefCorner)((flags & 0x0030) >> 4);
    params.TRANSPOSED = flags & 0x0040;
    params.SBCOMBOP = (Jbig2ComposeOp)((flags & 0x0180) >> 7);
    params.SBDEFPIXEL = flags & 0x0200;
    /* SBDSOFFSET is a signed 5 bit integer */
    params.SBDSOFFSET = (flags & 0x7C00) >> 10;
    if (params.SBDSOFFSET > 0x0f)
        params.SBDSOFFSET -= 0x20;
    params.SBRTEMPLATE = flags & 0x8000;

    if (params.SBDSOFFSET) {
        jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number, "text region has SBDSOFFSET %d", params.SBDSOFFSET);
    }

    if (params.SBHUFF) {        /* Huffman coding */
        /* 7.4.3.1.2 */
        huffman_flags = jbig2_get_uint16(segment_data + offset);
        offset += 2;

        if (huffman_flags & 0x8000)
            jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "reserved bit 15 of text region huffman flags is not zero");
    } else {                    /* arithmetic coding */

        /* 7.4.3.1.3 */
        if ((params.SBREFINE) && !(params.SBRTEMPLATE)) {
            params.sbrat[0] = segment_data[offset];
            params.sbrat[1] = segment_data[offset + 1];
            params.sbrat[2] = segment_data[offset + 2];
            params.sbrat[3] = segment_data[offset + 3];
            offset += 4;
        }
    }

    /* 7.4.3.1.4 */
    params.SBNUMINSTANCES = jbig2_get_uint32(segment_data + offset);
    offset += 4;

    if (params.SBHUFF) {
        /* 7.4.3.1.5 - Symbol ID Huffman table */
        /* ...this is handled in the segment body decoder */

        /* 7.4.3.1.6 - Other Huffman table selection */
        switch (huffman_flags & 0x0003) {
        case 0:                /* Table B.6 */
            params.SBHUFFFS = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_F);
            break;
        case 1:                /* Table B.7 */
            params.SBHUFFFS = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_G);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom FS huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFFS = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        case 2:                /* invalid */
        default:
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "text region specified invalid FS huffman table");
            goto cleanup1;
            break;
        }
        if (params.SBHUFFFS == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified FS huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x000c) >> 2) {
        case 0:                /* Table B.8 */
            params.SBHUFFDS = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_H);
            break;
        case 1:                /* Table B.9 */
            params.SBHUFFDS = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_I);
            break;
        case 2:                /* Table B.10 */
            params.SBHUFFDS = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_J);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom DS huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFDS = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        }
        if (params.SBHUFFDS == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified DS huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x0030) >> 4) {
        case 0:                /* Table B.11 */
            params.SBHUFFDT = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_K);
            break;
        case 1:                /* Table B.12 */
            params.SBHUFFDT = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_L);
            break;
        case 2:                /* Table B.13 */
            params.SBHUFFDT = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_M);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom DT huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFDT = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        }
        if (params.SBHUFFDT == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified DT huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x00c0) >> 6) {
        case 0:                /* Table B.14 */
            params.SBHUFFRDW = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_N);
            break;
        case 1:                /* Table B.15 */
            params.SBHUFFRDW = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_O);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom RDW huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFRDW = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        case 2:                /* invalid */
        default:
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "text region specified invalid RDW huffman table");
            goto cleanup1;
            break;
        }
        if (params.SBHUFFRDW == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified RDW huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x0300) >> 8) {
        case 0:                /* Table B.14 */
            params.SBHUFFRDH = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_N);
            break;
        case 1:                /* Table B.15 */
            params.SBHUFFRDH = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_O);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom RDH huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFRDH = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        case 2:                /* invalid */
        default:
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "text region specified invalid RDH huffman table");
            goto cleanup1;
            break;
        }
        if (params.SBHUFFRDH == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified RDH huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x0c00) >> 10) {
        case 0:                /* Table B.14 */
            params.SBHUFFRDX = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_N);
            break;
        case 1:                /* Table B.15 */
            params.SBHUFFRDX = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_O);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom RDX huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFRDX = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        case 2:                /* invalid */
        default:
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "text region specified invalid RDX huffman table");
            goto cleanup1;
            break;
        }
        if (params.SBHUFFRDX == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified RDX huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x3000) >> 12) {
        case 0:                /* Table B.14 */
            params.SBHUFFRDY = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_N);
            break;
        case 1:                /* Table B.15 */
            params.SBHUFFRDY = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_O);
            break;
        case 3:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom RDY huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFRDY = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        case 2:                /* invalid */
        default:
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "text region specified invalid RDY huffman table");
            goto cleanup1;
            break;
        }
        if (params.SBHUFFRDY == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified RDY huffman table");
            goto cleanup1;
        }

        switch ((huffman_flags & 0x4000) >> 14) {
        case 0:                /* Table B.1 */
            params.SBHUFFRSIZE = jbig2_build_huffman_table(ctx, &jbig2_huffman_params_A);
            break;
        case 1:                /* Custom table from referred segment */
            huffman_params = jbig2_find_table(ctx, segment, table_index);
            if (huffman_params == NULL) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Custom RSIZE huffman table not found (%d)", table_index);
                goto cleanup1;
            }
            params.SBHUFFRSIZE = jbig2_build_huffman_table(ctx, huffman_params);
            ++table_index;
            break;
        }
        if (params.SBHUFFRSIZE == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to allocate text region specified RSIZE huffman table");
            goto cleanup1;
        }

        if (huffman_flags & 0x8000) {
            jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "text region huffman flags bit 15 is set, contrary to spec");
        }

        /* 7.4.3.1.7 */
        /* For convenience this is done in the body decoder routine */
    }

    jbig2_error(ctx, JBIG2_SEVERITY_INFO, segment->number,
                "text region: %d x %d @ (%d,%d) %d symbols", region_info.width, region_info.height, region_info.x, region_info.y, params.SBNUMINSTANCES);

    /* 7.4.3.2 (2) - compose the list of symbol dictionaries */
    n_dicts = jbig2_sd_count_referred(ctx, segment);
    if (n_dicts != 0) {
        dicts = jbig2_sd_list_referred(ctx, segment);
    } else {
        code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "text region refers to no symbol dictionaries!");
        goto cleanup1;
    }
    if (dicts == NULL) {
         code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "unable to retrive symbol dictionaries! previous parsing error?");
         goto cleanup1;
     } else {
        int index;
 
         if (dicts[0] == NULL) {
             code = jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "unable to find first referenced symbol dictionary!");
            goto cleanup1;
        }
        for (index = 1; index < n_dicts; index++)
            if (dicts[index] == NULL) {
                jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "unable to find all referenced symbol dictionaries!");
                n_dicts = index;
            }
    }

    /* 7.4.3.2 (3) */
    {
        int stats_size = params.SBRTEMPLATE ? 1 << 10 : 1 << 13;

        GR_stats = jbig2_new(ctx, Jbig2ArithCx, stats_size);
        if (GR_stats == NULL) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "could not allocate GR_stats");
            goto cleanup1;
        }
        memset(GR_stats, 0, stats_size);
    }

    image = jbig2_image_new(ctx, region_info.width, region_info.height);
    if (image == NULL) {
        code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "couldn't allocate text region image");
        goto cleanup2;
    }

    ws = jbig2_word_stream_buf_new(ctx, segment_data + offset, segment->data_length - offset);
    if (ws == NULL) {
        code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "couldn't allocate ws in text region image");
        goto cleanup2;
    }

    as = jbig2_arith_new(ctx, ws);
    if (as == NULL) {
        code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "couldn't allocate as in text region image");
        goto cleanup2;
     }
 
     if (!params.SBHUFF) {
        int SBSYMCODELEN, index;
        int SBNUMSYMS = 0;
 
         for (index = 0; index < n_dicts; index++) {
             SBNUMSYMS += dicts[index]->n_symbols;
        }

        params.IADT = jbig2_arith_int_ctx_new(ctx);
        params.IAFS = jbig2_arith_int_ctx_new(ctx);
        params.IADS = jbig2_arith_int_ctx_new(ctx);
        params.IAIT = jbig2_arith_int_ctx_new(ctx);
        if ((params.IADT == NULL) || (params.IAFS == NULL) || (params.IADS == NULL) || (params.IAIT == NULL)) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "couldn't allocate text region image data");
            goto cleanup3;
         }
 
         /* Table 31 */
        for (SBSYMCODELEN = 0; (1 << SBSYMCODELEN) < SBNUMSYMS; SBSYMCODELEN++) {
         }
         params.IAID = jbig2_arith_iaid_ctx_new(ctx, SBSYMCODELEN);
         params.IARI = jbig2_arith_int_ctx_new(ctx);
        params.IARDW = jbig2_arith_int_ctx_new(ctx);
        params.IARDH = jbig2_arith_int_ctx_new(ctx);
        params.IARDX = jbig2_arith_int_ctx_new(ctx);
        params.IARDY = jbig2_arith_int_ctx_new(ctx);
        if ((params.IAID == NULL) || (params.IARI == NULL) ||
                (params.IARDW == NULL) || (params.IARDH == NULL) || (params.IARDX == NULL) || (params.IARDY == NULL)) {
            code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "couldn't allocate text region image data");
            goto cleanup4;
        }
    }

    code = jbig2_decode_text_region(ctx, segment, &params,
                                    (const Jbig2SymbolDict * const *)dicts, n_dicts, image,
                                    segment_data + offset, segment->data_length - offset, GR_stats, as, ws);
    if (code < 0) {
        jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to decode text region image data");
        goto cleanup4;
    }

    if ((segment->flags & 63) == 4) {
        /* we have an intermediate region here. save it for later */
        segment->result = jbig2_image_clone(ctx, image);
    } else {
        /* otherwise composite onto the page */
        jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number,
                    "composing %dx%d decoded text region onto page at (%d, %d)", region_info.width, region_info.height, region_info.x, region_info.y);
        jbig2_page_add_result(ctx, &ctx->pages[ctx->current_page], image, region_info.x, region_info.y, region_info.op);
    }

cleanup4:
    if (!params.SBHUFF) {
        jbig2_arith_iaid_ctx_free(ctx, params.IAID);
        jbig2_arith_int_ctx_free(ctx, params.IARI);
        jbig2_arith_int_ctx_free(ctx, params.IARDW);
        jbig2_arith_int_ctx_free(ctx, params.IARDH);
        jbig2_arith_int_ctx_free(ctx, params.IARDX);
        jbig2_arith_int_ctx_free(ctx, params.IARDY);
    }

cleanup3:
    if (!params.SBHUFF) {
        jbig2_arith_int_ctx_free(ctx, params.IADT);
        jbig2_arith_int_ctx_free(ctx, params.IAFS);
        jbig2_arith_int_ctx_free(ctx, params.IADS);
        jbig2_arith_int_ctx_free(ctx, params.IAIT);
    }
    jbig2_free(ctx->allocator, as);
    jbig2_word_stream_buf_free(ctx, ws);

cleanup2:
    jbig2_free(ctx->allocator, GR_stats);
    jbig2_image_release(ctx, image);

cleanup1:
    if (params.SBHUFF) {
        jbig2_release_huffman_table(ctx, params.SBHUFFFS);
        jbig2_release_huffman_table(ctx, params.SBHUFFDS);
        jbig2_release_huffman_table(ctx, params.SBHUFFDT);
        jbig2_release_huffman_table(ctx, params.SBHUFFRDX);
        jbig2_release_huffman_table(ctx, params.SBHUFFRDY);
        jbig2_release_huffman_table(ctx, params.SBHUFFRDW);
        jbig2_release_huffman_table(ctx, params.SBHUFFRDH);
        jbig2_release_huffman_table(ctx, params.SBHUFFRSIZE);
    }
    jbig2_free(ctx->allocator, dicts);

    return code;

too_short:
    return jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "Segment too short");
}
