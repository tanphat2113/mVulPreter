 irc_server_gnutls_callback (void *data, gnutls_session_t tls_session,
                             const gnutls_datum_t *req_ca, int nreq,
                             const gnutls_pk_algorithm_t *pk_algos,
                            int pk_algos_len, gnutls_retr_st *answer)
 {
     struct t_irc_server *server;
     gnutls_retr_st tls_struct;
    const gnutls_datum_t *cert_list;
    gnutls_datum_t filedatum;
    unsigned int cert_list_len, status;
    time_t cert_time;
    char *cert_path0, *cert_path1, *cert_path2, *cert_str, *hostname;
    const char *weechat_dir;
    int rc, ret, i, j, hostname_match;
#if LIBGNUTLS_VERSION_NUMBER >= 0x010706
    gnutls_datum_t cinfo;
    int rinfo;
#endif

    /* make C compiler happy */
    (void) req_ca;
    (void) nreq;
    (void) pk_algos;
    (void) pk_algos_len;
    
    rc = 0;
    
    if (!data)
        return -1;
    
    server = (struct t_irc_server *) data;
    hostname = server->current_address;
     hostname = server->current_address;
     hostname_match = 0;
     
    weechat_printf (server->buffer,
                    _("gnutls: connected using %d-bit Diffie-Hellman shared "
                      "secret exchange"),
                    IRC_SERVER_OPTION_INTEGER (server,
                                               IRC_SERVER_OPTION_SSL_DHKEY_SIZE));
    if (gnutls_certificate_verify_peers2 (tls_session, &status) < 0)
     {
         weechat_printf (server->buffer,
                        _("%sgnutls: error while checking peer's certificate"),
                        weechat_prefix ("error"));
        rc = -1;
    }
    else
    {
        /* some checks */
        if (status & GNUTLS_CERT_INVALID)
         {
             weechat_printf (server->buffer,
                            _("%sgnutls: peer's certificate is NOT trusted"),
                             weechat_prefix ("error"));
             rc = -1;
         }
         else
         {
            weechat_printf (server->buffer,
                            _("gnutls: peer's certificate is trusted"));
        }
        if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
        {
            weechat_printf (server->buffer,
                            _("%sgnutls: peer's certificate issuer is unknown"),
                            weechat_prefix ("error"));
            rc = -1;
        }
        if (status & GNUTLS_CERT_REVOKED)
        {
            weechat_printf (server->buffer,
                            _("%sgnutls: the certificate has been revoked"),
                            weechat_prefix ("error"));
            rc = -1;
        }
        /* check certificates */
        if (gnutls_x509_crt_init (&cert_temp) >= 0)
        {
            cert_list = gnutls_certificate_get_peers (tls_session, &cert_list_len);
            if (cert_list)
             {
                 weechat_printf (server->buffer,
                                NG_("gnutls: receiving %d certificate",
                                    "gnutls: receiving %d certificates",
                                    cert_list_len),
                                cert_list_len);
                for (i = 0, j = (int) cert_list_len; i < j; i++)
                 {
                    if (gnutls_x509_crt_import (cert_temp, &cert_list[i], GNUTLS_X509_FMT_DER) >= 0)
                     {
                        /* checking if hostname matches in the first certificate */
                        if (i == 0 && gnutls_x509_crt_check_hostname (cert_temp, hostname) != 0)
                         {
                            hostname_match = 1;
                        }
 #if LIBGNUTLS_VERSION_NUMBER >= 0x010706
                        /* displaying infos about certificate */
 #if LIBGNUTLS_VERSION_NUMBER < 0x020400
                        rinfo = gnutls_x509_crt_print (cert_temp, GNUTLS_X509_CRT_ONELINE, &cinfo);
 #else
                        rinfo = gnutls_x509_crt_print (cert_temp, GNUTLS_CRT_PRINT_ONELINE, &cinfo);
 #endif
                        if (rinfo == 0)
                        {
                            weechat_printf (server->buffer,
                                            _(" - certificate[%d] info:"), i + 1);
                            weechat_printf (server->buffer,
                                            "   - %s", cinfo.data);
                            gnutls_free (cinfo.data);
                        }
 #endif
                        /* check expiration date */
                        cert_time = gnutls_x509_crt_get_expiration_time (cert_temp);
                        if (cert_time < time(NULL))
                        {
                            weechat_printf (server->buffer,
                                            _("%sgnutls: certificate has expired"),
                                            weechat_prefix ("error"));
                            rc = -1;
                        }
                        /* check expiration date */
                        cert_time = gnutls_x509_crt_get_activation_time (cert_temp);
                        if (cert_time > time(NULL))
                        {
                            weechat_printf (server->buffer,
                                            _("%sgnutls: certificate is not yet activated"),
                                            weechat_prefix ("error"));
                            rc = -1;
                         }
                     }
                }
                if (hostname_match == 0)
                {
                    weechat_printf (server->buffer,
                                    _("%sgnutls: the hostname in the "
                                      "certificate does NOT match \"%s\""),
                                    weechat_prefix ("error"), hostname);
                    rc = -1;
                 }
             }
         }
     }
    /* using client certificate if it exists */
    cert_path0 = (char *) IRC_SERVER_OPTION_STRING(server,
                                                   IRC_SERVER_OPTION_SSL_CERT);
    if (cert_path0 && cert_path0[0])
     {
        weechat_dir = weechat_info_get ("weechat_dir", "");
        cert_path1 = weechat_string_replace (cert_path0, "%h", weechat_dir);
        cert_path2 = (cert_path1) ?
            weechat_string_expand_home (cert_path1) : NULL;
        if (cert_path2)
         {
            cert_str = weechat_file_get_content (cert_path2);
            if (cert_str)
             {
                weechat_printf (server->buffer,
                                _("gnutls: sending one certificate"));
                filedatum.data = (unsigned char *) cert_str;
                filedatum.size = strlen (cert_str);
                /* certificate */
                gnutls_x509_crt_init (&server->tls_cert);
                gnutls_x509_crt_import (server->tls_cert, &filedatum,
                                        GNUTLS_X509_FMT_PEM);
                /* key */
                gnutls_x509_privkey_init (&server->tls_cert_key);
                ret = gnutls_x509_privkey_import (server->tls_cert_key,
                                                  &filedatum,
                                                  GNUTLS_X509_FMT_PEM);
                if (ret < 0)
                {
                    ret = gnutls_x509_privkey_import_pkcs8 (server->tls_cert_key,
                                                            &filedatum,
                                                            GNUTLS_X509_FMT_PEM,
                                                            NULL,
                                                            GNUTLS_PKCS_PLAIN);
                }
                if (ret < 0)
                 {
                     weechat_printf (server->buffer,
                                    _("%sgnutls: invalid certificate \"%s\", "
                                      "error: %s"),
                                    weechat_prefix ("error"), cert_path2,
                                    gnutls_strerror (ret));
                    rc = -1;
                }
                else
                {
                    tls_struct.type = GNUTLS_CRT_X509;
                    tls_struct.ncerts = 1;
                    tls_struct.deinit_all = 0;
                    tls_struct.cert.x509 = &server->tls_cert;
                    tls_struct.key.x509 = server->tls_cert_key;
 #if LIBGNUTLS_VERSION_NUMBER >= 0x010706
                    /* client certificate info */
 #if LIBGNUTLS_VERSION_NUMBER < 0x020400
                    rinfo = gnutls_x509_crt_print (server->tls_cert,
                                                   GNUTLS_X509_CRT_ONELINE,
                                                   &cinfo);
 #else
                    rinfo = gnutls_x509_crt_print (server->tls_cert,
                                                   GNUTLS_CRT_PRINT_ONELINE,
                                                   &cinfo);
 #endif
                    if (rinfo == 0)
                    {
                        weechat_printf (server->buffer,
                                        _(" - client certificate info (%s):"),
                                        cert_path2);
                        weechat_printf (server->buffer, "  - %s", cinfo.data);
                        gnutls_free (cinfo.data);
                    }
 #endif
                    memcpy (answer, &tls_struct, sizeof (gnutls_retr_st));
                    free (cert_str);
                 }
             }
            else
            {
                weechat_printf (server->buffer,
                                _("%sgnutls: unable to read certifcate \"%s\""),
                                weechat_prefix ("error"), cert_path2);
            }
         }
        if (cert_path1)
            free (cert_path1);
        if (cert_path2)
            free (cert_path2);
     }
     
     /* an error should stop the handshake unless the user doesn't care */
    return rc;
}
