eap_print(netdissect_options *ndo,
          register const u_char *cp,
          u_int length)
{
    const struct eap_frame_t *eap;
    const u_char *tptr;
    u_int tlen, type, subtype;
    int count=0, len;

    tptr = cp;
    tlen = length;
    eap = (const struct eap_frame_t *)cp;
    ND_TCHECK(*eap);

    /* in non-verbose mode just lets print the basic info */
    if (ndo->ndo_vflag < 1) {
	ND_PRINT((ndo, "%s (%u) v%u, len %u",
               tok2str(eap_frame_type_values, "unknown", eap->type),
               eap->type,
               eap->version,
               EXTRACT_16BITS(eap->length)));
	return;
    }

    ND_PRINT((ndo, "%s (%u) v%u, len %u",
           tok2str(eap_frame_type_values, "unknown", eap->type),
           eap->type,
           eap->version,
           EXTRACT_16BITS(eap->length)));

    tptr += sizeof(const struct eap_frame_t);
    tlen -= sizeof(const struct eap_frame_t);
 
     switch (eap->type) {
     case EAP_FRAME_TYPE_PACKET:
        ND_TCHECK_8BITS(tptr);
         type = *(tptr);
        ND_TCHECK_16BITS(tptr+2);
         len = EXTRACT_16BITS(tptr+2);
         ND_PRINT((ndo, ", %s (%u), id %u, len %u",
                tok2str(eap_code_values, "unknown", type),
               type,
               *(tptr+1),
               len));

         ND_TCHECK2(*tptr, len);
 
         if (type <= 2) { /* For EAP_REQUEST and EAP_RESPONSE only */
            ND_TCHECK_8BITS(tptr+4);
             subtype = *(tptr+4);
             ND_PRINT((ndo, "\n\t\t Type %s (%u)",
                   tok2str(eap_type_values, "unknown", subtype),
                   subtype));
 
             switch (subtype) {
             case EAP_TYPE_IDENTITY:
                if (len - 5 > 0) {
                    ND_PRINT((ndo, ", Identity: "));
                    safeputs(ndo, tptr + 5, len - 5);
                }
                break;

            case EAP_TYPE_NOTIFICATION:
                if (len - 5 > 0) {
                    ND_PRINT((ndo, ", Notification: "));
                    safeputs(ndo, tptr + 5, len - 5);
                }
                break;

            case EAP_TYPE_NAK:
                count = 5;

                /*
                 * one or more octets indicating
                 * the desired authentication
                  * type one octet per type
                  */
                 while (count < len) {
                    ND_TCHECK_8BITS(tptr+count);
                     ND_PRINT((ndo, " %s (%u),",
                            tok2str(eap_type_values, "unknown", *(tptr+count)),
                            *(tptr + count)));
                    count++;
                }
                 break;
 
             case EAP_TYPE_TTLS:
             case EAP_TYPE_TLS:
                ND_TCHECK_8BITS(tptr + 5);
                if (subtype == EAP_TYPE_TTLS)
                    ND_PRINT((ndo, " TTLSv%u",
                           EAP_TTLS_VERSION(*(tptr + 5))));
                 ND_PRINT((ndo, " flags [%s] 0x%02x,",
                        bittok2str(eap_tls_flags_values, "none", *(tptr+5)),
                        *(tptr + 5)));
 
                 if (EAP_TLS_EXTRACT_BIT_L(*(tptr+5))) {
                    ND_TCHECK_32BITS(tptr + 6);
 		    ND_PRINT((ndo, " len %u", EXTRACT_32BITS(tptr + 6)));
                 }
                 break;
 
             case EAP_TYPE_FAST:
                ND_TCHECK_8BITS(tptr + 5);
                 ND_PRINT((ndo, " FASTv%u",
                        EAP_TTLS_VERSION(*(tptr + 5))));
                 ND_PRINT((ndo, " flags [%s] 0x%02x,",
                        bittok2str(eap_tls_flags_values, "none", *(tptr+5)),
                        *(tptr + 5)));
 
                 if (EAP_TLS_EXTRACT_BIT_L(*(tptr+5))) {
                    ND_TCHECK_32BITS(tptr + 6);
                     ND_PRINT((ndo, " len %u", EXTRACT_32BITS(tptr + 6)));
                 }
 
                /* FIXME - TLV attributes follow */
                break;
 
             case EAP_TYPE_AKA:
             case EAP_TYPE_SIM:
                ND_TCHECK_8BITS(tptr + 5);
                 ND_PRINT((ndo, " subtype [%s] 0x%02x,",
                        tok2str(eap_aka_subtype_values, "unknown", *(tptr+5)),
                        *(tptr + 5)));

                /* FIXME - TLV attributes follow */
                break;

            case EAP_TYPE_MD5_CHALLENGE:
            case EAP_TYPE_OTP:
            case EAP_TYPE_GTC:
            case EAP_TYPE_EXPANDED_TYPES:
            case EAP_TYPE_EXPERIMENTAL:
            default:
                break;
            }
        }
        break;

    case EAP_FRAME_TYPE_LOGOFF:
    case EAP_FRAME_TYPE_ENCAP_ASF_ALERT:
    default:
        break;
    }
    return;

 trunc:
    ND_PRINT((ndo, "\n\t[|EAP]"));
}
