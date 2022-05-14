status_t OMXNodeInstance::getConfig(

         OMX_INDEXTYPE index, void *params, size_t /* size */) {
     Mutex::Autolock autoLock(mLock);
 
    if (isProhibitedIndex_l(index)) {
        android_errorWriteLog(0x534e4554, "29422020");
        return BAD_INDEX;
    }

     OMX_ERRORTYPE err = OMX_GetConfig(mHandle, index, params);
     OMX_INDEXEXTTYPE extIndex = (OMX_INDEXEXTTYPE)index;
 if (err != OMX_ErrorNoMore) {
        CLOG_IF_ERROR(getConfig, err, "%s(%#x)", asString(extIndex), index);
 }
 return StatusFromOMXError(err);
}
