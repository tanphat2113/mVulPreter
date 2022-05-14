mget(struct magic_set *ms, const unsigned char *s, struct magic *m,
    size_t nbytes, size_t o, unsigned int cont_level, int mode, int text,
    int flip, int recursion_level, int *printed_something,
    int *need_separator, int *returnval)
{
	uint32_t soffset, offset = ms->offset;
	uint32_t count = m->str_range;
	int rv, oneed_separator, in_type;
	char *sbuf, *rbuf;
	union VALUETYPE *p = &ms->ms_value;
	struct mlist ml;

	if (recursion_level >= 20) {
		file_error(ms, 0, "recursion nesting exceeded");
		return -1;
	}

	if (mcopy(ms, p, m->type, m->flag & INDIR, s, (uint32_t)(offset + o),
	    (uint32_t)nbytes, count) == -1)
		return -1;

	if ((ms->flags & MAGIC_DEBUG) != 0) {
		fprintf(stderr, "mget(type=%d, flag=%x, offset=%u, o=%zu, "
		    "nbytes=%zu, count=%u)\n", m->type, m->flag, offset, o,
		    nbytes, count);
		mdebug(offset, (char *)(void *)p, sizeof(union VALUETYPE));
#ifndef COMPILE_ONLY
		file_mdump(m);
#endif
	}

	if (m->flag & INDIR) {
		int off = m->in_offset;
		if (m->in_op & FILE_OPINDIRECT) {
			const union VALUETYPE *q = CAST(const union VALUETYPE *,
			    ((const void *)(s + offset + off)));
			switch (cvt_flip(m->in_type, flip)) {
			case FILE_BYTE:
				off = q->b;
				break;
			case FILE_SHORT:
				off = q->h;
				break;
			case FILE_BESHORT:
				off = (short)((q->hs[0]<<8)|(q->hs[1]));
				break;
			case FILE_LESHORT:
				off = (short)((q->hs[1]<<8)|(q->hs[0]));
				break;
			case FILE_LONG:
				off = q->l;
				break;
			case FILE_BELONG:
			case FILE_BEID3:
				off = (int32_t)((q->hl[0]<<24)|(q->hl[1]<<16)|
						 (q->hl[2]<<8)|(q->hl[3]));
				break;
			case FILE_LEID3:
			case FILE_LELONG:
				off = (int32_t)((q->hl[3]<<24)|(q->hl[2]<<16)|
						 (q->hl[1]<<8)|(q->hl[0]));
				break;
			case FILE_MELONG:
				off = (int32_t)((q->hl[1]<<24)|(q->hl[0]<<16)|
						 (q->hl[3]<<8)|(q->hl[2]));
				break;
			}
			if ((ms->flags & MAGIC_DEBUG) != 0)
				fprintf(stderr, "indirect offs=%u\n", off);
 		}
 		switch (in_type = cvt_flip(m->in_type, flip)) {
 		case FILE_BYTE:
			if (OFFSET_OOB(nbytes, offset, 1))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = p->b & off;
					break;
				case FILE_OPOR:
					offset = p->b | off;
					break;
				case FILE_OPXOR:
					offset = p->b ^ off;
					break;
				case FILE_OPADD:
					offset = p->b + off;
					break;
				case FILE_OPMINUS:
					offset = p->b - off;
					break;
				case FILE_OPMULTIPLY:
					offset = p->b * off;
					break;
				case FILE_OPDIVIDE:
					offset = p->b / off;
					break;
				case FILE_OPMODULO:
					offset = p->b % off;
					break;
				}
			} else
				offset = p->b;
			if (m->in_op & FILE_OPINVERSE)
 				offset = ~offset;
 			break;
 		case FILE_BESHORT:
			if (OFFSET_OOB(nbytes, offset, 2))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) %
						 off;
					break;
				}
			} else
				offset = (short)((p->hs[0]<<8)|
						 (p->hs[1]));
			if (m->in_op & FILE_OPINVERSE)
 				offset = ~offset;
 			break;
 		case FILE_LESHORT:
			if (OFFSET_OOB(nbytes, offset, 2))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) %
						 off;
					break;
				}
			} else
				offset = (short)((p->hs[1]<<8)|
						 (p->hs[0]));
			if (m->in_op & FILE_OPINVERSE)
 				offset = ~offset;
 			break;
 		case FILE_SHORT:
			if (OFFSET_OOB(nbytes, offset, 2))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = p->h & off;
					break;
				case FILE_OPOR:
					offset = p->h | off;
					break;
				case FILE_OPXOR:
					offset = p->h ^ off;
					break;
				case FILE_OPADD:
					offset = p->h + off;
					break;
				case FILE_OPMINUS:
					offset = p->h - off;
					break;
				case FILE_OPMULTIPLY:
					offset = p->h * off;
					break;
				case FILE_OPDIVIDE:
					offset = p->h / off;
					break;
				case FILE_OPMODULO:
					offset = p->h % off;
					break;
				}
			}
			else
				offset = p->h;
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
 			break;
 		case FILE_BELONG:
 		case FILE_BEID3:
			if (OFFSET_OOB(nbytes, offset, 4))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) %
						 off;
					break;
				}
			} else
				offset = (int32_t)((p->hl[0]<<24)|
						 (p->hl[1]<<16)|
						 (p->hl[2]<<8)|
						 (p->hl[3]));
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
 			break;
 		case FILE_LELONG:
 		case FILE_LEID3:
			if (OFFSET_OOB(nbytes, offset, 4))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) %
						 off;
					break;
				}
			} else
				offset = (int32_t)((p->hl[3]<<24)|
						 (p->hl[2]<<16)|
						 (p->hl[1]<<8)|
						 (p->hl[0]));
			if (m->in_op & FILE_OPINVERSE)
 				offset = ~offset;
 			break;
 		case FILE_MELONG:
			if (OFFSET_OOB(nbytes, offset, 4))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) %
						 off;
					break;
				}
			} else
				offset = (int32_t)((p->hl[1]<<24)|
						 (p->hl[0]<<16)|
						 (p->hl[3]<<8)|
						 (p->hl[2]));
			if (m->in_op & FILE_OPINVERSE)
 				offset = ~offset;
 			break;
 		case FILE_LONG:
			if (OFFSET_OOB(nbytes, offset, 4))
 				return 0;
 			if (off) {
 				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = p->l & off;
					break;
				case FILE_OPOR:
					offset = p->l | off;
					break;
				case FILE_OPXOR:
					offset = p->l ^ off;
					break;
				case FILE_OPADD:
					offset = p->l + off;
					break;
				case FILE_OPMINUS:
					offset = p->l - off;
					break;
				case FILE_OPMULTIPLY:
					offset = p->l * off;
					break;
				case FILE_OPDIVIDE:
					offset = p->l / off;
					break;
				case FILE_OPMODULO:
					offset = p->l % off;
					break;
				}
			} else
				offset = p->l;
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
			break;
		default:
			break;
		}

		switch (in_type) {
		case FILE_LEID3:
		case FILE_BEID3:
			offset = ((((offset >>  0) & 0x7f) <<  0) |
				 (((offset >>  8) & 0x7f) <<  7) |
				 (((offset >> 16) & 0x7f) << 14) |
				 (((offset >> 24) & 0x7f) << 21)) + 10;
			break;
		default:
			break;
		}

		if (m->flag & INDIROFFADD) {
			offset += ms->c.li[cont_level-1].off;
			if (offset == 0) {
				if ((ms->flags & MAGIC_DEBUG) != 0)
					fprintf(stderr,
					    "indirect *zero* offset\n");
				return 0;
			}
			if ((ms->flags & MAGIC_DEBUG) != 0)
				fprintf(stderr, "indirect +offs=%u\n", offset);
		}
		if (mcopy(ms, p, m->type, 0, s, offset, nbytes, count) == -1)
			return -1;
		ms->offset = offset;

		if ((ms->flags & MAGIC_DEBUG) != 0) {
			mdebug(offset, (char *)(void *)p,
			    sizeof(union VALUETYPE));
#ifndef COMPILE_ONLY
			file_mdump(m);
#endif
		}
	}

 	/* Verify we have enough data to match magic type */
 	switch (m->type) {
 	case FILE_BYTE:
		if (OFFSET_OOB(nbytes, offset, 1))
 			return 0;
 		break;
 
 	case FILE_SHORT:
 	case FILE_BESHORT:
 	case FILE_LESHORT:
		if (OFFSET_OOB(nbytes, offset, 2))
 			return 0;
 		break;
 
	case FILE_LONG:
	case FILE_BELONG:
	case FILE_LELONG:
	case FILE_MELONG:
	case FILE_DATE:
	case FILE_BEDATE:
	case FILE_LEDATE:
	case FILE_MEDATE:
	case FILE_LDATE:
	case FILE_BELDATE:
	case FILE_LELDATE:
	case FILE_MELDATE:
 	case FILE_FLOAT:
 	case FILE_BEFLOAT:
 	case FILE_LEFLOAT:
		if (OFFSET_OOB(nbytes, offset, 4))
 			return 0;
 		break;
 
 	case FILE_DOUBLE:
 	case FILE_BEDOUBLE:
 	case FILE_LEDOUBLE:
		if (OFFSET_OOB(nbytes, offset, 8))
 			return 0;
 		break;
 
 	case FILE_STRING:
 	case FILE_PSTRING:
 	case FILE_SEARCH:
		if (OFFSET_OOB(nbytes, offset, m->vallen))
 			return 0;
 		break;
 
 	case FILE_REGEX:
		if (OFFSET_OOB(nbytes, offset, 0))
 			return 0;
 		break;
 
 	case FILE_INDIRECT:
		if (OFFSET_OOB(nbytes, offset, 0))
 			return 0;
 		sbuf = ms->o.buf;
 		soffset = ms->offset;
		ms->o.buf = NULL;
		ms->offset = 0;
		rv = file_softmagic(ms, s + offset, nbytes - offset,
		    BINTEST, text);
		if ((ms->flags & MAGIC_DEBUG) != 0)
			fprintf(stderr, "indirect @offs=%u[%d]\n", offset, rv);
		rbuf = ms->o.buf;
		ms->o.buf = sbuf;
		ms->offset = soffset;
		if (rv == 1) {
			if ((ms->flags & (MAGIC_MIME|MAGIC_APPLE)) == 0 &&
			    file_printf(ms, F(m->desc, "%u"), offset) == -1)
				return -1;
			if (file_printf(ms, "%s", rbuf) == -1)
				return -1;
			free(rbuf);
		}
 		return rv;
 
 	case FILE_USE:
		if (OFFSET_OOB(nbytes, offset, 0))
 			return 0;
 		sbuf = m->value.s;
 		if (*sbuf == '^') {
			sbuf++;
			flip = !flip;
		}
		if (file_magicfind(ms, sbuf, &ml) == -1) {
			file_error(ms, 0, "cannot find entry `%s'", sbuf);
			return -1;
		}

		oneed_separator = *need_separator;
		if (m->flag & NOSPACE)
			*need_separator = 0;
		rv = match(ms, ml.magic, ml.nmagic, s, nbytes, offset + o,
		    mode, text, flip, recursion_level, printed_something,
		    need_separator, returnval);
		if (rv != 1)
		    *need_separator = oneed_separator;
		return rv;

	case FILE_NAME:
		if (file_printf(ms, "%s", m->desc) == -1)
			return -1;
		return 1;
	case FILE_DEFAULT:	/* nothing to check */
	case FILE_CLEAR:
	default:
		break;
	}
	if (!mconvert(ms, m, flip))
		return 0;
	return 1;
}
