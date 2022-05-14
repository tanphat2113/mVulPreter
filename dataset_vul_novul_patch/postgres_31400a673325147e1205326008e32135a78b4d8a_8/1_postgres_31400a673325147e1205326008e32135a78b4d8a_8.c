ltree_in(PG_FUNCTION_ARGS)
{
	char	   *buf = (char *) PG_GETARG_POINTER(0);
	char	   *ptr;
	nodeitem   *list,
			   *lptr;
	int			num = 0,
				totallen = 0;
	int			state = LTPRS_WAITNAME;
	ltree	   *result;
	ltree_level *curlevel;
	int			charlen;
	int			pos = 0;

	ptr = buf;
	while (*ptr)
	{
		charlen = pg_mblen(ptr);
		if (charlen == 1 && t_iseq(ptr, '.'))
			num++;
 		ptr += charlen;
 	}
 
 	list = lptr = (nodeitem *) palloc(sizeof(nodeitem) * (num + 1));
 	ptr = buf;
 	while (*ptr)
	{
		charlen = pg_mblen(ptr);

		if (state == LTPRS_WAITNAME)
		{
			if (ISALNUM(ptr))
			{
				lptr->start = ptr;
				lptr->wlen = 0;
				state = LTPRS_WAITDELIM;
			}
			else
				UNCHAR;
		}
		else if (state == LTPRS_WAITDELIM)
		{
			if (charlen == 1 && t_iseq(ptr, '.'))
			{
				lptr->len = ptr - lptr->start;
				if (lptr->wlen > 255)
					ereport(ERROR,
							(errcode(ERRCODE_NAME_TOO_LONG),
							 errmsg("name of level is too long"),
							 errdetail("Name length is %d, must "
									   "be < 256, in position %d.",
									   lptr->wlen, pos)));

				totallen += MAXALIGN(lptr->len + LEVEL_HDRSIZE);
				lptr++;
				state = LTPRS_WAITNAME;
			}
			else if (!ISALNUM(ptr))
				UNCHAR;
		}
		else
			/* internal error */
			elog(ERROR, "internal error in parser");

		ptr += charlen;
		lptr->wlen++;
		pos++;
	}

	if (state == LTPRS_WAITDELIM)
	{
		lptr->len = ptr - lptr->start;
		if (lptr->wlen > 255)
			ereport(ERROR,
					(errcode(ERRCODE_NAME_TOO_LONG),
					 errmsg("name of level is too long"),
					 errdetail("Name length is %d, must "
							   "be < 256, in position %d.",
							   lptr->wlen, pos)));

		totallen += MAXALIGN(lptr->len + LEVEL_HDRSIZE);
		lptr++;
	}
	else if (!(state == LTPRS_WAITNAME && lptr == list))
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("syntax error"),
				 errdetail("Unexpected end of line.")));

	result = (ltree *) palloc0(LTREE_HDRSIZE + totallen);
	SET_VARSIZE(result, LTREE_HDRSIZE + totallen);
	result->numlevel = lptr - list;
	curlevel = LTREE_FIRST(result);
	lptr = list;
	while (lptr - list < result->numlevel)
	{
		curlevel->len = (uint16) lptr->len;
		memcpy(curlevel->name, lptr->start, lptr->len);
		curlevel = LEVEL_NEXT(curlevel);
		lptr++;
	}

	pfree(list);
	PG_RETURN_POINTER(result);
}
