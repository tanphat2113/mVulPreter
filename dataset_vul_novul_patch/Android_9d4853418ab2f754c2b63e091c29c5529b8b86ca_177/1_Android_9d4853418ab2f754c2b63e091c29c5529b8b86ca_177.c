zlib_run(struct zlib *zlib)
 /* Like zlib_advance but also handles a stream of IDAT chunks. */
{
 /* The 'extra_bytes' field is set by zlib_advance if there is extra
    * compressed data in the chunk it handles (if it sees Z_STREAM_END before
    * all the input data has been used.)  This function uses the value to update
    * the correct chunk length, so the problem should only ever be detected once
    * for each chunk.  zlib_advance outputs the error message, though see the
    * IDAT specific check below.
    */
   zlib->extra_bytes = 0;

 if (zlib->idat != NULL)
 {
 struct IDAT_list *list = zlib->idat->idat_list_head;
 struct IDAT_list *last = zlib->idat->idat_list_tail;
 int        skip = 0;

 /* 'rewrite_offset' is the offset of the LZ data within the chunk, for
       * IDAT it should be 0:
       */
      assert(zlib->rewrite_offset == 0);

 /* Process each IDAT_list in turn; the caller has left the stream
       * positioned at the start of the first IDAT chunk data.
       */
 for (;;)
 {
 const unsigned int count = list->count;
 unsigned int i;

 for (i = 0; i<count; ++i)
 {
 int rc;

 if (skip > 0) /* Skip CRC and next IDAT header */
               skip_12(zlib->file);

            skip = 12; /* for the next time */

            rc = zlib_advance(zlib, list->lengths[i]);

 switch (rc)
 {
 case ZLIB_OK: /* keep going */
 break;

 case ZLIB_STREAM_END: /* stop */
 /* There may be extra chunks; if there are and one of them is
                   * not zero length output the 'extra data' message.  Only do
                   * this check if errors are being output.
                   */
 if (zlib->global->errors && zlib->extra_bytes == 0)
 {
 struct IDAT_list *check = list;
 int j = i+1, jcount = count;

 for (;;)
 {
 for (; j<jcount; ++j)
 if (check->lengths[j] > 0)
 {
                              chunk_message(zlib->chunk,
 "extra compressed data");
 goto end_check;
 }

 if (check == last)
 break;

                        check = check->next;
                        jcount = check->count;
                        j = 0;
 }
 }

               end_check:
 /* Terminate the list at the current position, reducing the
                   * length of the last IDAT too if required.
                   */
 list->lengths[i] -= zlib->extra_bytes;
 list->count = i+1;
                  zlib->idat->idat_list_tail = list;
 /* FALL THROUGH */

 default:
 return rc;
 }
 }

 /* At the end of the compressed data and Z_STREAM_END was not seen. */
 if (list == last)
 return ZLIB_OK;

 list = list->next;
 }
 }

 else

    {
       struct chunk *chunk = zlib->chunk;
       int rc;
       assert(zlib->rewrite_offset < chunk->chunk_length);
 
       rc = zlib_advance(zlib, chunk->chunk_length - zlib->rewrite_offset);

 /* The extra bytes in the chunk are handled now by adjusting the chunk
       * length to exclude them; the zlib data is always stored at the end of
       * the PNG chunk (although clearly this is not necessary.)  zlib_advance
       * has already output a warning message.
       */
      chunk->chunk_length -= zlib->extra_bytes;
 return rc;
 }
}
