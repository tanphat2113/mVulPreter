int mbedtls_x509_crt_verify_with_profile( mbedtls_x509_crt *crt,
                     mbedtls_x509_crt *trust_ca,
                     mbedtls_x509_crl *ca_crl,
                     const mbedtls_x509_crt_profile *profile,
                     const char *cn, uint32_t *flags,
                     int (*f_vrfy)(void *, mbedtls_x509_crt *, int, uint32_t *),
                     void *p_vrfy )
{
    size_t cn_len;
    int ret;
    int pathlen = 0, selfsigned = 0;
    mbedtls_x509_crt *parent;
    mbedtls_x509_name *name;
     mbedtls_x509_sequence *cur = NULL;
     mbedtls_pk_type_t pk_type;
 
     *flags = 0;
 
    if( profile == NULL )
    {
        ret = MBEDTLS_ERR_X509_BAD_INPUT_DATA;
        goto exit;
    }

     if( cn != NULL )
     {
         name = &crt->subject;
        cn_len = strlen( cn );

        if( crt->ext_types & MBEDTLS_X509_EXT_SUBJECT_ALT_NAME )
        {
            cur = &crt->subject_alt_names;

            while( cur != NULL )
            {
                if( cur->buf.len == cn_len &&
                    x509_memcasecmp( cn, cur->buf.p, cn_len ) == 0 )
                    break;

                if( cur->buf.len > 2 &&
                    memcmp( cur->buf.p, "*.", 2 ) == 0 &&
                    x509_check_wildcard( cn, &cur->buf ) == 0 )
                {
                    break;
                }

                cur = cur->next;
            }

            if( cur == NULL )
                *flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
        }
        else
        {
            while( name != NULL )
            {
                if( MBEDTLS_OID_CMP( MBEDTLS_OID_AT_CN, &name->oid ) == 0 )
                {
                    if( name->val.len == cn_len &&
                        x509_memcasecmp( name->val.p, cn, cn_len ) == 0 )
                        break;

                    if( name->val.len > 2 &&
                        memcmp( name->val.p, "*.", 2 ) == 0 &&
                        x509_check_wildcard( cn, &name->val ) == 0 )
                        break;
                }

                name = name->next;
            }

            if( name == NULL )
                *flags |= MBEDTLS_X509_BADCERT_CN_MISMATCH;
        }
    }

    /* Check the type and size of the key */
    pk_type = mbedtls_pk_get_type( &crt->pk );

    if( x509_profile_check_pk_alg( profile, pk_type ) != 0 )
        *flags |= MBEDTLS_X509_BADCERT_BAD_PK;

    if( x509_profile_check_key( profile, pk_type, &crt->pk ) != 0 )
        *flags |= MBEDTLS_X509_BADCERT_BAD_KEY;

    /* Look for a parent in trusted CAs */
    for( parent = trust_ca; parent != NULL; parent = parent->next )
    {
        if( x509_crt_check_parent( crt, parent, 0, pathlen == 0 ) == 0 )
            break;
    }

    if( parent != NULL )
    {
         ret = x509_crt_verify_top( crt, parent, ca_crl, profile,
                                    pathlen, selfsigned, flags, f_vrfy, p_vrfy );
         if( ret != 0 )
            goto exit;
     }
     else
     {
        /* Look for a parent upwards the chain */
        for( parent = crt->next; parent != NULL; parent = parent->next )
            if( x509_crt_check_parent( crt, parent, 0, pathlen == 0 ) == 0 )
                break;

        /* Are we part of the chain or at the top? */
        if( parent != NULL )
        {
             ret = x509_crt_verify_child( crt, parent, trust_ca, ca_crl, profile,
                                          pathlen, selfsigned, flags, f_vrfy, p_vrfy );
             if( ret != 0 )
                goto exit;
         }
         else
         {
             ret = x509_crt_verify_top( crt, trust_ca, ca_crl, profile,
                                        pathlen, selfsigned, flags, f_vrfy, p_vrfy );
             if( ret != 0 )
                goto exit;
         }
     }
 
exit:
    if( ret != 0 )
    {
        *flags = (uint32_t) -1;
        return( ret );
    }

     if( *flags != 0 )
         return( MBEDTLS_ERR_X509_CERT_VERIFY_FAILED );
 
    return( 0 );
}
