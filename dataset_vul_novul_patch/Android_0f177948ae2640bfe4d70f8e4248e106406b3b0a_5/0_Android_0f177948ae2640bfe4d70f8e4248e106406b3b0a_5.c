status_t OMXNodeInstance::allocateBufferWithBackup(
        OMX_U32 portIndex, const sp<IMemory> &params,
        OMX::buffer_id *buffer, OMX_U32 allottedSize) {
 if (params == NULL || buffer == NULL) {
        ALOGE("b/25884056");
 return BAD_VALUE;

     }
 
     Mutex::Autolock autoLock(mLock);
    if (allottedSize > params->size() || portIndex >= NELEM(mNumPortBuffers)) {
         return BAD_VALUE;
     }
 
    // metadata buffers are not connected cross process; only copy if not meta
    bool copy = mMetadataType[portIndex] == kMetadataBufferTypeInvalid;

    BufferMeta *buffer_meta = new BufferMeta(
            params, portIndex,
            (portIndex == kPortIndexInput) && copy /* copyToOmx */,
            (portIndex == kPortIndexOutput) && copy /* copyFromOmx */,
            NULL /* data */);
 
     OMX_BUFFERHEADERTYPE *header;
 
    OMX_ERRORTYPE err = OMX_AllocateBuffer(
            mHandle, &header, portIndex, buffer_meta, allottedSize);
 if (err != OMX_ErrorNone) {
        CLOG_ERROR(allocateBufferWithBackup, err,
                SIMPLE_BUFFER(portIndex, (size_t)allottedSize, params->pointer()));
 delete buffer_meta;
        buffer_meta = NULL;

 *buffer = 0;

 return StatusFromOMXError(err);

     }
 
     CHECK_EQ(header->pAppPrivate, buffer_meta);
    memset(header->pBuffer, 0, header->nAllocLen);
 
     *buffer = makeBufferID(header);
 
    addActiveBuffer(portIndex, *buffer);

    sp<GraphicBufferSource> bufferSource(getGraphicBufferSource());
 if (bufferSource != NULL && portIndex == kPortIndexInput) {
        bufferSource->addCodecBuffer(header);
 }

    CLOG_BUFFER(allocateBufferWithBackup, NEW_BUFFER_FMT(*buffer, portIndex, "%zu@%p :> %u@%p",
            params->size(), params->pointer(), allottedSize, header->pBuffer));

 return OK;
}
