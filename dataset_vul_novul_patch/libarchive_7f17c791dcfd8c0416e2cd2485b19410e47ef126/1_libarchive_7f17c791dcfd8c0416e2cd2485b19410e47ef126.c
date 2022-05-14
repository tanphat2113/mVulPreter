read_Header(struct archive_read *a, struct _7z_header_info *h,
    int check_header_id)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	const unsigned char *p;
	struct _7z_folder *folders;
	struct _7z_stream_info *si = &(zip->si);
	struct _7zip_entry *entries;
	uint32_t folderIndex, indexInFolder;
	unsigned i;
	int eindex, empty_streams, sindex;

	if (check_header_id) {
		/*
		 * Read Header.
		 */
		if ((p = header_bytes(a, 1)) == NULL)
			return (-1);
		if (*p != kHeader)
			return (-1);
	}

	/*
	 * Read ArchiveProperties.
	 */
	if ((p = header_bytes(a, 1)) == NULL)
		return (-1);
	if (*p == kArchiveProperties) {
		for (;;) {
			uint64_t size;
			if ((p = header_bytes(a, 1)) == NULL)
				return (-1);
			if (*p == 0)
				break;
			if (parse_7zip_uint64(a, &size) < 0)
				return (-1);
		}
		if ((p = header_bytes(a, 1)) == NULL)
			return (-1);
	}

	/*
	 * Read MainStreamsInfo.
	 */
	if (*p == kMainStreamsInfo) {
		if (read_StreamsInfo(a, &(zip->si)) < 0)
			return (-1);
		if ((p = header_bytes(a, 1)) == NULL)
			return (-1);
	}
	if (*p == kEnd)
		return (0);

	/*
	 * Read FilesInfo.
	 */
	if (*p != kFilesInfo)
		return (-1);

	if (parse_7zip_uint64(a, &(zip->numFiles)) < 0)
		return (-1);
	if (UMAX_ENTRY < zip->numFiles)
		return (-1);

	zip->entries = calloc((size_t)zip->numFiles, sizeof(*zip->entries));
	if (zip->entries == NULL)
		return (-1);
	entries = zip->entries;

	empty_streams = 0;
	for (;;) {
		int type;
		uint64_t size;
		size_t ll;

		if ((p = header_bytes(a, 1)) == NULL)
			return (-1);
		type = *p;
		if (type == kEnd)
			break;

		if (parse_7zip_uint64(a, &size) < 0)
			return (-1);
		if (zip->header_bytes_remaining < size)
			return (-1);
		ll = (size_t)size;
 
 		switch (type) {
 		case kEmptyStream:
 			h->emptyStreamBools = calloc((size_t)zip->numFiles,
 			    sizeof(*h->emptyStreamBools));
 			if (h->emptyStreamBools == NULL)
				return (-1);
			if (read_Bools(
			    a, h->emptyStreamBools, (size_t)zip->numFiles) < 0)
				return (-1);
			empty_streams = 0;
			for (i = 0; i < zip->numFiles; i++) {
				if (h->emptyStreamBools[i])
					empty_streams++;
			}
			break;
		case kEmptyFile:
			if (empty_streams <= 0) {
				/* Unexcepted sequence. Skip this. */
				if (header_bytes(a, ll) == NULL)
 					return (-1);
 				break;
 			}
 			h->emptyFileBools = calloc(empty_streams,
 			    sizeof(*h->emptyFileBools));
 			if (h->emptyFileBools == NULL)
				return (-1);
			if (read_Bools(a, h->emptyFileBools, empty_streams) < 0)
				return (-1);
			break;
		case kAnti:
			if (empty_streams <= 0) {
				/* Unexcepted sequence. Skip this. */
				if (header_bytes(a, ll) == NULL)
 					return (-1);
 				break;
 			}
 			h->antiBools = calloc(empty_streams,
 			    sizeof(*h->antiBools));
 			if (h->antiBools == NULL)
				return (-1);
			if (read_Bools(a, h->antiBools, empty_streams) < 0)
				return (-1);
			break;
		case kCTime:
		case kATime:
		case kMTime:
			if (read_Times(a, h, type) < 0)
				return (-1);
			break;
		case kName:
		{
			unsigned char *np;
			size_t nl, nb;

			/* Skip one byte. */
			if ((p = header_bytes(a, 1)) == NULL)
				return (-1);
			ll--;

 			if ((ll & 1) || ll < zip->numFiles * 4)
 				return (-1);
 
 			zip->entry_names = malloc(ll);
 			if (zip->entry_names == NULL)
 				return (-1);
			np = zip->entry_names;
			nb = ll;
			/*
			 * Copy whole file names.
			 * NOTE: This loop prevents from expanding
			 * the uncompressed buffer in order not to
			 * use extra memory resource.
			 */
			while (nb) {
				size_t b;
				if (nb > UBUFF_SIZE)
					b = UBUFF_SIZE;
				else
					b = nb;
				if ((p = header_bytes(a, b)) == NULL)
					return (-1);
				memcpy(np, p, b);
				np += b;
				nb -= b;
			}
			np = zip->entry_names;
			nl = ll;

			for (i = 0; i < zip->numFiles; i++) {
				entries[i].utf16name = np;
#if defined(_WIN32) && !defined(__CYGWIN__) && defined(_DEBUG)
				entries[i].wname = (wchar_t *)np;
#endif

				/* Find a terminator. */
				while (nl >= 2 && (np[0] || np[1])) {
					np += 2;
					nl -= 2;
				}
				if (nl < 2)
					return (-1);/* Terminator not found */
				entries[i].name_len = np - entries[i].utf16name;
				np += 2;
				nl -= 2;
			}
			break;
		}
		case kAttributes:
		{
			int allAreDefined;

 			if ((p = header_bytes(a, 2)) == NULL)
 				return (-1);
 			allAreDefined = *p;
 			h->attrBools = calloc((size_t)zip->numFiles,
 			    sizeof(*h->attrBools));
 			if (h->attrBools == NULL)
				return (-1);
			if (allAreDefined)
				memset(h->attrBools, 1, (size_t)zip->numFiles);
			else {
				if (read_Bools(a, h->attrBools,
				      (size_t)zip->numFiles) < 0)
					return (-1);
			}
			for (i = 0; i < zip->numFiles; i++) {
				if (h->attrBools[i]) {
					if ((p = header_bytes(a, 4)) == NULL)
						return (-1);
					entries[i].attr = archive_le32dec(p);
				}
			}
			break;
		}
		case kDummy:
			if (ll == 0)
				break;
		default:
			if (header_bytes(a, ll) == NULL)
				return (-1);
			break;
		}
	}

	/*
	 * Set up entry's attributes.
	 */
	folders = si->ci.folders;
	eindex = sindex = 0;
	folderIndex = indexInFolder = 0;
	for (i = 0; i < zip->numFiles; i++) {
		if (h->emptyStreamBools == NULL || h->emptyStreamBools[i] == 0)
			entries[i].flg |= HAS_STREAM;
		/* The high 16 bits of attributes is a posix file mode. */
		entries[i].mode = entries[i].attr >> 16;
		if (entries[i].flg & HAS_STREAM) {
			if ((size_t)sindex >= si->ss.unpack_streams)
				return (-1);
			if (entries[i].mode == 0)
				entries[i].mode = AE_IFREG | 0666;
			if (si->ss.digestsDefined[sindex])
				entries[i].flg |= CRC32_IS_SET;
			entries[i].ssIndex = sindex;
			sindex++;
		} else {
			int dir;
			if (h->emptyFileBools == NULL)
				dir = 1;
			else {
				if (h->emptyFileBools[eindex])
					dir = 0;
				else
					dir = 1;
				eindex++;
			}
			if (entries[i].mode == 0) {
				if (dir)
					entries[i].mode = AE_IFDIR | 0777;
				else
					entries[i].mode = AE_IFREG | 0666;
			} else if (dir &&
			    (entries[i].mode & AE_IFMT) != AE_IFDIR) {
				entries[i].mode &= ~AE_IFMT;
				entries[i].mode |= AE_IFDIR;
			}
			if ((entries[i].mode & AE_IFMT) == AE_IFDIR &&
			    entries[i].name_len >= 2 &&
			    (entries[i].utf16name[entries[i].name_len-2] != '/' ||
			     entries[i].utf16name[entries[i].name_len-1] != 0)) {
				entries[i].utf16name[entries[i].name_len] = '/';
				entries[i].utf16name[entries[i].name_len+1] = 0;
				entries[i].name_len += 2;
			}
			entries[i].ssIndex = -1;
		}
		if (entries[i].attr & 0x01)
			entries[i].mode &= ~0222;/* Read only. */

		if ((entries[i].flg & HAS_STREAM) == 0 && indexInFolder == 0) {
			/*
			 * The entry is an empty file or a directory file,
			 * those both have no contents.
			 */
			entries[i].folderIndex = -1;
			continue;
		}
		if (indexInFolder == 0) {
			for (;;) {
				if (folderIndex >= si->ci.numFolders)
					return (-1);
				if (folders[folderIndex].numUnpackStreams)
					break;
				folderIndex++;
			}
		}
		entries[i].folderIndex = folderIndex;
		if ((entries[i].flg & HAS_STREAM) == 0)
			continue;
		indexInFolder++;
		if (indexInFolder >= folders[folderIndex].numUnpackStreams) {
			folderIndex++;
			indexInFolder = 0;
		}
	}

	return (0);
}
