bool xmp_init()
{
     RESET_ERROR;
     try {
        // XMP SDK 5.1.2 needs this because it has been stripped off local
        // text conversion the one that was done in Exempi with libiconv.
         bool result = SXMPFiles::Initialize(kXMPFiles_IgnoreLocalText);
         SXMPMeta::SetDefaultErrorCallback(&_xmp_error_callback, nullptr, 1);
         return result;
        SXMPMeta::SetDefaultErrorCallback(&_xmp_error_callback, nullptr, 1);
        return result;
    }
    catch (const XMP_Error &e) {
        set_error(e);
    }
    return false;
}
