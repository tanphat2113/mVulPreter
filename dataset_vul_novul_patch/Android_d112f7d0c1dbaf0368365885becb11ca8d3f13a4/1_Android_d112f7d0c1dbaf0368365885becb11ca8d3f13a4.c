 status_t NuPlayer::GenericSource::setBuffers(
         bool audio, Vector<MediaBuffer *> &buffers) {
    if (mIsWidevine && !audio && mVideoTrack.mSource != NULL) {
         return mVideoTrack.mSource->setBuffers(buffers);
     }
     return INVALID_OPERATION;
}
