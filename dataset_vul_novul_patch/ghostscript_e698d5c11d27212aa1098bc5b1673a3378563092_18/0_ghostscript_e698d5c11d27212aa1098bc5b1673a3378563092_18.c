 jbig2_decode_text_region(Jbig2Ctx *ctx, Jbig2Segment *segment,
                          const Jbig2TextRegionParams *params,
                         const Jbig2SymbolDict *const *dicts, const uint32_t n_dicts,
                          Jbig2Image *image, const byte *data, const size_t size, Jbig2ArithCx *GR_stats, Jbig2ArithState *as, Jbig2WordStream *ws)
 {
     /* relevent bits of 6.4.4 */
    uint32_t NINSTANCES;
    uint32_t ID;
    int32_t STRIPT;
    int32_t FIRSTS;
    int32_t DT;
    int32_t DFS;
    int32_t IDS;
    int32_t CURS;
    int32_t CURT;
    int S, T;
    int x, y;
    bool first_symbol;
    uint32_t index, SBNUMSYMS;
    Jbig2Image *IB = NULL;
    Jbig2HuffmanState *hs = NULL;
    Jbig2HuffmanTable *SBSYMCODES = NULL;
    int code = 0;
    int RI;

    SBNUMSYMS = 0;
    for (index = 0; index < n_dicts; index++) {
        SBNUMSYMS += dicts[index]->n_symbols;
    }
    jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number, "symbol list contains %d glyphs in %d dictionaries", SBNUMSYMS, n_dicts);

    if (params->SBHUFF) {
        Jbig2HuffmanTable *runcodes = NULL;
        Jbig2HuffmanParams runcodeparams;
        Jbig2HuffmanLine runcodelengths[35];
        Jbig2HuffmanLine *symcodelengths = NULL;
        Jbig2HuffmanParams symcodeparams;
        int err, len, range, r;

        jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number, "huffman coded text region");
        hs = jbig2_huffman_new(ctx, ws);
        if (hs == NULL) {
            jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "failed to allocate storage for text region");
            return -1;
        }

        /* 7.4.3.1.7 - decode symbol ID Huffman table */
        /* this is actually part of the segment header, but it is more
           convenient to handle it here */

        /* parse and build the runlength code huffman table */
        for (index = 0; index < 35; index++) {
            runcodelengths[index].PREFLEN = jbig2_huffman_get_bits(hs, 4, &code);
            if (code < 0)
                goto cleanup1;
            runcodelengths[index].RANGELEN = 0;
            runcodelengths[index].RANGELOW = index;
            jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number, "  read runcode%d length %d", index, runcodelengths[index].PREFLEN);
        }
        runcodeparams.HTOOB = 0;
        runcodeparams.lines = runcodelengths;
        runcodeparams.n_lines = 35;
        runcodes = jbig2_build_huffman_table(ctx, &runcodeparams);
        if (runcodes == NULL) {
            jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "error constructing symbol id runcode table!");
            code = -1;
            goto cleanup1;
        }

        /* decode the symbol id codelengths using the runlength table */
        symcodelengths = jbig2_new(ctx, Jbig2HuffmanLine, SBNUMSYMS);
        if (symcodelengths == NULL) {
            jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "memory allocation failure reading symbol ID huffman table!");
            code = -1;
            goto cleanup1;
        }
        index = 0;
        while (index < SBNUMSYMS) {
            code = jbig2_huffman_get(hs, runcodes, &err);
            if (err != 0 || code < 0 || code >= 35) {
                jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "error reading symbol ID huffman table!");
                code = err ? err : -1;
                goto cleanup1;
            }

            if (code < 32) {
                len = code;
                range = 1;
            } else {
                if (code == 32) {
                    if (index < 1) {
                        jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "error decoding symbol id table: run length with no antecedent!");
                        code = -1;
                        goto cleanup1;
                    }
                    len = symcodelengths[index - 1].PREFLEN;
                } else {
                    len = 0;    /* code == 33 or 34 */
                }
                err = 0;
                if (code == 32)
                    range = jbig2_huffman_get_bits(hs, 2, &err) + 3;
                else if (code == 33)
                    range = jbig2_huffman_get_bits(hs, 3, &err) + 3;
                else if (code == 34)
                    range = jbig2_huffman_get_bits(hs, 7, &err) + 11;
                if (err < 0)
                    goto cleanup1;
            }
            jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number, "  read runcode%d at index %d (length %d range %d)", code, index, len, range);
            if (index + range > SBNUMSYMS) {
                jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number,
                            "runlength extends %d entries beyond the end of symbol id table!", index + range - SBNUMSYMS);
                range = SBNUMSYMS - index;
            }
            for (r = 0; r < range; r++) {
                symcodelengths[index + r].PREFLEN = len;
                symcodelengths[index + r].RANGELEN = 0;
                symcodelengths[index + r].RANGELOW = index + r;
            }
            index += r;
        }

        if (index < SBNUMSYMS) {
            jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "runlength codes do not cover the available symbol set");
        }
        symcodeparams.HTOOB = 0;
        symcodeparams.lines = symcodelengths;
        symcodeparams.n_lines = SBNUMSYMS;

        /* skip to byte boundary */
        jbig2_huffman_skip(hs);

        /* finally, construct the symbol id huffman table itself */
        SBSYMCODES = jbig2_build_huffman_table(ctx, &symcodeparams);

cleanup1:
        jbig2_free(ctx->allocator, symcodelengths);
        jbig2_release_huffman_table(ctx, runcodes);

        if (SBSYMCODES == NULL) {
            jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "could not construct Symbol ID huffman table!");
            jbig2_huffman_free(ctx, hs);
            return ((code != 0) ? code : -1);
        }
    }

    /* 6.4.5 (1) */
    jbig2_image_clear(ctx, image, params->SBDEFPIXEL);

    /* 6.4.6 */
    if (params->SBHUFF) {
        STRIPT = jbig2_huffman_get(hs, params->SBHUFFDT, &code);
    } else {
        code = jbig2_arith_int_decode(params->IADT, as, &STRIPT);
    }
    if (code < 0)
        goto cleanup2;

    /* 6.4.5 (2) */
    STRIPT *= -(params->SBSTRIPS);
    FIRSTS = 0;
    NINSTANCES = 0;

    /* 6.4.5 (3) */
    while (NINSTANCES < params->SBNUMINSTANCES) {
        /* (3b) */
        if (params->SBHUFF) {
            DT = jbig2_huffman_get(hs, params->SBHUFFDT, &code);
        } else {
            code = jbig2_arith_int_decode(params->IADT, as, &DT);
        }
        if (code < 0)
            goto cleanup2;
        DT *= params->SBSTRIPS;
        STRIPT += DT;

        first_symbol = TRUE;
        /* 6.4.5 (3c) - decode symbols in strip */
        for (;;) {
            /* (3c.i) */
            if (first_symbol) {
                /* 6.4.7 */
                if (params->SBHUFF) {
                    DFS = jbig2_huffman_get(hs, params->SBHUFFFS, &code);
                } else {
                    code = jbig2_arith_int_decode(params->IAFS, as, &DFS);
                }
                if (code < 0)
                    goto cleanup2;
                FIRSTS += DFS;
                CURS = FIRSTS;
                first_symbol = FALSE;
            } else {
                if (NINSTANCES > params->SBNUMINSTANCES) {
                    code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "too many NINSTANCES (%d) decoded", NINSTANCES);
                    break;
                }
                /* (3c.ii) / 6.4.8 */
                if (params->SBHUFF) {
                    IDS = jbig2_huffman_get(hs, params->SBHUFFDS, &code);
                } else {
                    code = jbig2_arith_int_decode(params->IADS, as, &IDS);
                }
                if (code) {
                    /* decoded an OOB, reached end of strip */
                    break;
                }
                CURS += IDS + params->SBDSOFFSET;
            }

            /* (3c.iii) / 6.4.9 */
            if (params->SBSTRIPS == 1) {
                CURT = 0;
            } else if (params->SBHUFF) {
                CURT = jbig2_huffman_get_bits(hs, params->LOGSBSTRIPS, &code);
            } else {
                code = jbig2_arith_int_decode(params->IAIT, as, &CURT);
            }
            if (code < 0)
                goto cleanup2;
            T = STRIPT + CURT;

            /* (3b.iv) / 6.4.10 - decode the symbol id */
            if (params->SBHUFF) {
                ID = jbig2_huffman_get(hs, SBSYMCODES, &code);
            } else {
                code = jbig2_arith_iaid_decode(params->IAID, as, (int *)&ID);
            }
            if (code < 0)
                goto cleanup2;
            if (ID >= SBNUMSYMS) {
                code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "symbol id out of range! (%d/%d)", ID, SBNUMSYMS);
                goto cleanup2;
            }

            /* (3c.v) / 6.4.11 - look up the symbol bitmap IB */
            {
                uint32_t id = ID;

                index = 0;
                while (id >= dicts[index]->n_symbols)
                    id -= dicts[index++]->n_symbols;
                IB = jbig2_image_clone(ctx, dicts[index]->glyphs[id]);
                /* SumatraPDF: fail on missing glyphs */
                if (!IB) {
                    code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "missing glyph %d/%d!", index, id);
                    goto cleanup2;
                }
            }
            if (params->SBREFINE) {
                if (params->SBHUFF) {
                    RI = jbig2_huffman_get_bits(hs, 1, &code);
                } else {
                    code = jbig2_arith_int_decode(params->IARI, as, &RI);
                }
                if (code < 0)
                    goto cleanup2;
            } else {
                RI = 0;
            }
            if (RI) {
                Jbig2RefinementRegionParams rparams;
                Jbig2Image *IBO;
                int32_t RDW, RDH, RDX, RDY;
                Jbig2Image *refimage;
                int BMSIZE = 0;
                int code1 = 0;
                int code2 = 0;
                int code3 = 0;
                int code4 = 0;
                int code5 = 0;

                /* 6.4.11 (1, 2, 3, 4) */
                if (!params->SBHUFF) {
                    code1 = jbig2_arith_int_decode(params->IARDW, as, &RDW);
                    code2 = jbig2_arith_int_decode(params->IARDH, as, &RDH);
                    code3 = jbig2_arith_int_decode(params->IARDX, as, &RDX);
                    code4 = jbig2_arith_int_decode(params->IARDY, as, &RDY);
                } else {
                    RDW = jbig2_huffman_get(hs, params->SBHUFFRDW, &code1);
                    RDH = jbig2_huffman_get(hs, params->SBHUFFRDH, &code2);
                    RDX = jbig2_huffman_get(hs, params->SBHUFFRDX, &code3);
                    RDY = jbig2_huffman_get(hs, params->SBHUFFRDY, &code4);
                    BMSIZE = jbig2_huffman_get(hs, params->SBHUFFRSIZE, &code5);
                    jbig2_huffman_skip(hs);
                }

                if ((code1 < 0) || (code2 < 0) || (code3 < 0) || (code4 < 0) || (code5 < 0)) {
                    code = jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "failed to decode data");
                    goto cleanup2;
                }

                /* 6.4.11 (6) */
                IBO = IB;
                refimage = jbig2_image_new(ctx, IBO->width + RDW, IBO->height + RDH);
                if (refimage == NULL) {
                    jbig2_image_release(ctx, IBO);
                    if (params->SBHUFF) {
                        jbig2_release_huffman_table(ctx, SBSYMCODES);
                    }
                    return jbig2_error(ctx, JBIG2_SEVERITY_FATAL, segment->number, "couldn't allocate reference image");
                }
                jbig2_image_clear(ctx, refimage, 0x00);

                /* Table 12 */
                rparams.GRTEMPLATE = params->SBRTEMPLATE;
                rparams.reference = IBO;
                rparams.DX = (RDW >> 1) + RDX;
                rparams.DY = (RDH >> 1) + RDY;
                rparams.TPGRON = 0;
                memcpy(rparams.grat, params->sbrat, 4);
                code = jbig2_decode_refinement_region(ctx, segment, &rparams, as, refimage, GR_stats);
                if (code < 0) {
                    jbig2_image_release(ctx, refimage);
                    goto cleanup2;
                }
                IB = refimage;

                jbig2_image_release(ctx, IBO);

                /* 6.4.11 (7) */
                if (params->SBHUFF) {
                    jbig2_huffman_advance(hs, BMSIZE);
                }

            }

            /* (3c.vi) */
            if ((!params->TRANSPOSED) && (params->REFCORNER > 1)) {
                CURS += IB->width - 1;
            } else if ((params->TRANSPOSED) && !(params->REFCORNER & 1)) {
                CURS += IB->height - 1;
            }

            /* (3c.vii) */
            S = CURS;

            /* (3c.viii) */
            if (!params->TRANSPOSED) {
                switch (params->REFCORNER) {
                case JBIG2_CORNER_TOPLEFT:
                    x = S;
                    y = T;
                    break;
                case JBIG2_CORNER_TOPRIGHT:
                    x = S - IB->width + 1;
                    y = T;
                    break;
                case JBIG2_CORNER_BOTTOMLEFT:
                    x = S;
                    y = T - IB->height + 1;
                    break;
                default:
                case JBIG2_CORNER_BOTTOMRIGHT:
                    x = S - IB->width + 1;
                    y = T - IB->height + 1;
                    break;
                }
            } else {            /* TRANSPOSED */
                switch (params->REFCORNER) {
                case JBIG2_CORNER_TOPLEFT:
                    x = T;
                    y = S;
                    break;
                case JBIG2_CORNER_TOPRIGHT:
                    x = T - IB->width + 1;
                    y = S;
                    break;
                case JBIG2_CORNER_BOTTOMLEFT:
                    x = T;
                    y = S - IB->height + 1;
                    break;
                default:
                case JBIG2_CORNER_BOTTOMRIGHT:
                    x = T - IB->width + 1;
                    y = S - IB->height + 1;
                    break;
                }
            }

            /* (3c.ix) */
#ifdef JBIG2_DEBUG
            jbig2_error(ctx, JBIG2_SEVERITY_DEBUG, segment->number,
                        "composing glyph id %d: %dx%d @ (%d,%d) symbol %d/%d", ID, IB->width, IB->height, x, y, NINSTANCES + 1, params->SBNUMINSTANCES);
#endif
            code = jbig2_image_compose(ctx, image, IB, x, y, params->SBCOMBOP);
            if (code < 0) {
                jbig2_image_release(ctx, IB);
                goto cleanup2;
            }

            /* (3c.x) */
            if ((!params->TRANSPOSED) && (params->REFCORNER < 2)) {
                CURS += IB->width - 1;
            } else if ((params->TRANSPOSED) && (params->REFCORNER & 1)) {
                CURS += IB->height - 1;
            }

            /* (3c.xi) */
            NINSTANCES++;

            jbig2_image_release(ctx, IB);
        }
        /* end strip */
    }
    /* 6.4.5 (4) */

cleanup2:
    if (params->SBHUFF) {
        jbig2_release_huffman_table(ctx, SBSYMCODES);
    }
    jbig2_huffman_free(ctx, hs);

    return code;
}
