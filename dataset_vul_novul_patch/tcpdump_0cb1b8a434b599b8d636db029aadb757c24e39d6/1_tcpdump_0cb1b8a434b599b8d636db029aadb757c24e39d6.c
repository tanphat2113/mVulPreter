olsr_print(netdissect_options *ndo,
           const u_char *pptr, u_int length, int is_ipv6)
{
    union {
        const struct olsr_common *common;
        const struct olsr_msg4 *msg4;
        const struct olsr_msg6 *msg6;
        const struct olsr_hello *hello;
        const struct olsr_hello_link *hello_link;
        const struct olsr_tc *tc;
        const struct olsr_hna4 *hna;
    } ptr;

    u_int msg_type, msg_len, msg_tlen, hello_len;
    uint16_t name_entry_type, name_entry_len;
    u_int name_entry_padding;
    uint8_t link_type, neighbor_type;
    const u_char *tptr, *msg_data;

    tptr = pptr;

    if (length < sizeof(struct olsr_common)) {
        goto trunc;
    }

    ND_TCHECK2(*tptr, sizeof(struct olsr_common));

    ptr.common = (const struct olsr_common *)tptr;
    length = min(length, EXTRACT_16BITS(ptr.common->packet_len));

    ND_PRINT((ndo, "OLSRv%i, seq 0x%04x, length %u",
            (is_ipv6 == 0) ? 4 : 6,
            EXTRACT_16BITS(ptr.common->packet_seq),
            length));

    tptr += sizeof(struct olsr_common);

    /*
     * In non-verbose mode, just print version.
     */
    if (ndo->ndo_vflag < 1) {
        return;
    }

    while (tptr < (pptr+length)) {
        union
        {
            const struct olsr_msg4 *v4;
            const struct olsr_msg6 *v6;
         } msgptr;
         int msg_len_valid = 0;
 
        ND_TCHECK2(*tptr, sizeof(struct olsr_msg4));
         if (is_ipv6)
         {
             msgptr.v6 = (const struct olsr_msg6 *) tptr;
             msg_type = msgptr.v6->msg_type;
             msg_len = EXTRACT_16BITS(msgptr.v6->msg_len);
            if ((msg_len >= sizeof (struct olsr_msg6))
                    && (msg_len <= length))
                msg_len_valid = 1;

            /* infinite loop check */
            if (msg_type == 0 || msg_len == 0) {
                return;
            }

            ND_PRINT((ndo, "\n\t%s Message (%#04x), originator %s, ttl %u, hop %u"
                    "\n\t  vtime %.3fs, msg-seq 0x%04x, length %u%s",
                    tok2str(olsr_msg_values, "Unknown", msg_type),
                    msg_type, ip6addr_string(ndo, msgptr.v6->originator),
                    msgptr.v6->ttl,
                    msgptr.v6->hopcount,
                    ME_TO_DOUBLE(msgptr.v6->vtime),
                    EXTRACT_16BITS(msgptr.v6->msg_seq),
                    msg_len, (msg_len_valid == 0) ? " (invalid)" : ""));
            if (!msg_len_valid) {
                return;
            }

            msg_tlen = msg_len - sizeof(struct olsr_msg6);
            msg_data = tptr + sizeof(struct olsr_msg6);
         }
         else /* (!is_ipv6) */
         {
             msgptr.v4 = (const struct olsr_msg4 *) tptr;
             msg_type = msgptr.v4->msg_type;
             msg_len = EXTRACT_16BITS(msgptr.v4->msg_len);
            if ((msg_len >= sizeof (struct olsr_msg4))
                    && (msg_len <= length))
                msg_len_valid = 1;

            /* infinite loop check */
            if (msg_type == 0 || msg_len == 0) {
                return;
            }

            ND_PRINT((ndo, "\n\t%s Message (%#04x), originator %s, ttl %u, hop %u"
                    "\n\t  vtime %.3fs, msg-seq 0x%04x, length %u%s",
                    tok2str(olsr_msg_values, "Unknown", msg_type),
                    msg_type, ipaddr_string(ndo, msgptr.v4->originator),
                    msgptr.v4->ttl,
                    msgptr.v4->hopcount,
                    ME_TO_DOUBLE(msgptr.v4->vtime),
                    EXTRACT_16BITS(msgptr.v4->msg_seq),
                    msg_len, (msg_len_valid == 0) ? " (invalid)" : ""));
            if (!msg_len_valid) {
                return;
            }

            msg_tlen = msg_len - sizeof(struct olsr_msg4);
            msg_data = tptr + sizeof(struct olsr_msg4);
        }

        switch (msg_type) {
        case OLSR_HELLO_MSG:
        case OLSR_HELLO_LQ_MSG:
            if (msg_tlen < sizeof(struct olsr_hello))
                goto trunc;
            ND_TCHECK2(*msg_data, sizeof(struct olsr_hello));

            ptr.hello = (const struct olsr_hello *)msg_data;
            ND_PRINT((ndo, "\n\t  hello-time %.3fs, MPR willingness %u",
                   ME_TO_DOUBLE(ptr.hello->htime), ptr.hello->will));
            msg_data += sizeof(struct olsr_hello);
            msg_tlen -= sizeof(struct olsr_hello);

            while (msg_tlen >= sizeof(struct olsr_hello_link)) {
                int hello_len_valid = 0;

                /*
                 * link-type.
                 */
                ND_TCHECK2(*msg_data, sizeof(struct olsr_hello_link));

                ptr.hello_link = (const struct olsr_hello_link *)msg_data;

                hello_len = EXTRACT_16BITS(ptr.hello_link->len);
                link_type = OLSR_EXTRACT_LINK_TYPE(ptr.hello_link->link_code);
                neighbor_type = OLSR_EXTRACT_NEIGHBOR_TYPE(ptr.hello_link->link_code);

                if ((hello_len <= msg_tlen)
                        && (hello_len >= sizeof(struct olsr_hello_link)))
                    hello_len_valid = 1;

                ND_PRINT((ndo, "\n\t    link-type %s, neighbor-type %s, len %u%s",
                       tok2str(olsr_link_type_values, "Unknown", link_type),
                       tok2str(olsr_neighbor_type_values, "Unknown", neighbor_type),
                       hello_len,
                       (hello_len_valid == 0) ? " (invalid)" : ""));

                if (hello_len_valid == 0)
                    break;

                msg_data += sizeof(struct olsr_hello_link);
                msg_tlen -= sizeof(struct olsr_hello_link);
                hello_len -= sizeof(struct olsr_hello_link);

                ND_TCHECK2(*msg_data, hello_len);
                if (msg_type == OLSR_HELLO_MSG) {
                    if (olsr_print_neighbor(ndo, msg_data, hello_len) == -1)
                        goto trunc;
                } else {
                    if (is_ipv6) {
                        if (olsr_print_lq_neighbor6(ndo, msg_data, hello_len) == -1)
                            goto trunc;
                    } else {
                        if (olsr_print_lq_neighbor4(ndo, msg_data, hello_len) == -1)
                            goto trunc;
                    }
                }

                msg_data += hello_len;
                msg_tlen -= hello_len;
            }
            break;

        case OLSR_TC_MSG:
        case OLSR_TC_LQ_MSG:
            if (msg_tlen < sizeof(struct olsr_tc))
                goto trunc;
            ND_TCHECK2(*msg_data, sizeof(struct olsr_tc));

            ptr.tc = (const struct olsr_tc *)msg_data;
            ND_PRINT((ndo, "\n\t    advertised neighbor seq 0x%04x",
                   EXTRACT_16BITS(ptr.tc->ans_seq)));
            msg_data += sizeof(struct olsr_tc);
            msg_tlen -= sizeof(struct olsr_tc);

            if (msg_type == OLSR_TC_MSG) {
                if (olsr_print_neighbor(ndo, msg_data, msg_tlen) == -1)
                    goto trunc;
            } else {
                if (is_ipv6) {
                    if (olsr_print_lq_neighbor6(ndo, msg_data, msg_tlen) == -1)
                        goto trunc;
                } else {
                    if (olsr_print_lq_neighbor4(ndo, msg_data, msg_tlen) == -1)
                        goto trunc;
                }
            }
            break;

        case OLSR_MID_MSG:
        {
            size_t addr_size = sizeof(struct in_addr);

            if (is_ipv6)
                addr_size = sizeof(struct in6_addr);

            while (msg_tlen >= addr_size) {
                ND_TCHECK2(*msg_data, addr_size);
                ND_PRINT((ndo, "\n\t  interface address %s",
                        is_ipv6 ? ip6addr_string(ndo, msg_data) :
                        ipaddr_string(ndo, msg_data)));

                msg_data += addr_size;
                msg_tlen -= addr_size;
            }
            break;
        }

        case OLSR_HNA_MSG:
            if (is_ipv6)
            {
                int i = 0;

                ND_PRINT((ndo, "\n\t  Advertised networks (total %u)",
                        (unsigned int) (msg_tlen / sizeof(struct olsr_hna6))));

                while (msg_tlen >= sizeof(struct olsr_hna6)) {
                    const struct olsr_hna6 *hna6;

                    ND_TCHECK2(*msg_data, sizeof(struct olsr_hna6));

                    hna6 = (const struct olsr_hna6 *)msg_data;

                    ND_PRINT((ndo, "\n\t    #%i: %s/%u",
                            i, ip6addr_string(ndo, hna6->network),
                            mask62plen (hna6->mask)));

                    msg_data += sizeof(struct olsr_hna6);
                    msg_tlen -= sizeof(struct olsr_hna6);
                }
            }
            else
            {
                int col = 0;

                ND_PRINT((ndo, "\n\t  Advertised networks (total %u)",
                        (unsigned int) (msg_tlen / sizeof(struct olsr_hna4))));

                while (msg_tlen >= sizeof(struct olsr_hna4)) {
                    ND_TCHECK2(*msg_data, sizeof(struct olsr_hna4));

                    ptr.hna = (const struct olsr_hna4 *)msg_data;

                    /* print 4 prefixes per line */
                    if (!ptr.hna->network[0] && !ptr.hna->network[1] &&
                        !ptr.hna->network[2] && !ptr.hna->network[3] &&
                        !ptr.hna->mask[GW_HNA_PAD] &&
                        ptr.hna->mask[GW_HNA_FLAGS]) {
                            /* smart gateway */
                            ND_PRINT((ndo, "%sSmart-Gateway:%s%s%s%s%s %u/%u",
                                col == 0 ? "\n\t    " : ", ", /* indent */
                                /* sgw */
                                /* LINKSPEED */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_LINKSPEED) ? " LINKSPEED" : "",
                                /* IPV4 */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_IPV4) ? " IPV4" : "",
                                /* IPV4-NAT */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_IPV4_NAT) ? " IPV4-NAT" : "",
                                /* IPV6 */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_IPV6) ? " IPV6" : "",
                                /* IPv6PREFIX */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_IPV6PREFIX) ? " IPv6-PREFIX" : "",
                                /* uplink */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_LINKSPEED) ?
                                 deserialize_gw_speed(ptr.hna->mask[GW_HNA_UPLINK]) : 0,
                                /* downlink */
                                (ptr.hna->mask[GW_HNA_FLAGS] &
                                 GW_HNA_FLAG_LINKSPEED) ?
                                 deserialize_gw_speed(ptr.hna->mask[GW_HNA_DOWNLINK]) : 0
                                ));
                    } else {
                        /* normal route */
                        ND_PRINT((ndo, "%s%s/%u",
                                col == 0 ? "\n\t    " : ", ",
                                ipaddr_string(ndo, ptr.hna->network),
                                mask2plen(EXTRACT_32BITS(ptr.hna->mask))));
                    }

                    msg_data += sizeof(struct olsr_hna4);
                    msg_tlen -= sizeof(struct olsr_hna4);

                    col = (col + 1) % 4;
                }
            }
            break;
 
         case OLSR_NAMESERVICE_MSG:
         {
            u_int name_entries = EXTRACT_16BITS(msg_data+2);
            u_int addr_size = 4;
            int name_entries_valid = 0;
             u_int i;
 
             if (is_ipv6)
                 addr_size = 16;
 
             if ((name_entries > 0)
                     && ((name_entries * (4 + addr_size)) <= msg_tlen))
                 name_entries_valid = 1;
 
            if (msg_tlen < 4)
                goto trunc;
            ND_TCHECK2(*msg_data, 4);
             ND_PRINT((ndo, "\n\t  Version %u, Entries %u%s",
                    EXTRACT_16BITS(msg_data),
                    name_entries, (name_entries_valid == 0) ? " (invalid)" : ""));

            if (name_entries_valid == 0)
                break;

            msg_data += 4;
            msg_tlen -= 4;

            for (i = 0; i < name_entries; i++) {
                int name_entry_len_valid = 0;

                if (msg_tlen < 4)
                    break;
                ND_TCHECK2(*msg_data, 4);

                name_entry_type = EXTRACT_16BITS(msg_data);
                name_entry_len = EXTRACT_16BITS(msg_data+2);

                msg_data += 4;
                msg_tlen -= 4;

                if ((name_entry_len > 0) && ((addr_size + name_entry_len) <= msg_tlen))
                    name_entry_len_valid = 1;

                ND_PRINT((ndo, "\n\t    #%u: type %#06x, length %u%s",
                        (unsigned int) i, name_entry_type,
                        name_entry_len, (name_entry_len_valid == 0) ? " (invalid)" : ""));

                if (name_entry_len_valid == 0)
                    break;

                /* 32-bit alignment */
                name_entry_padding = 0;
                if (name_entry_len%4 != 0)
                    name_entry_padding = 4-(name_entry_len%4);

                if (msg_tlen < addr_size + name_entry_len + name_entry_padding)
                    goto trunc;

                ND_TCHECK2(*msg_data, addr_size + name_entry_len + name_entry_padding);

                if (is_ipv6)
                    ND_PRINT((ndo, ", address %s, name \"",
                            ip6addr_string(ndo, msg_data)));
                else
                    ND_PRINT((ndo, ", address %s, name \"",
                            ipaddr_string(ndo, msg_data)));
                (void)fn_printn(ndo, msg_data + addr_size, name_entry_len, NULL);
                ND_PRINT((ndo, "\""));

                msg_data += addr_size + name_entry_len + name_entry_padding;
                msg_tlen -= addr_size + name_entry_len + name_entry_padding;
            } /* for (i = 0; i < name_entries; i++) */
            break;
        } /* case OLSR_NAMESERVICE_MSG */

            /*
             * FIXME those are the defined messages that lack a decoder
             * you are welcome to contribute code ;-)
             */
        case OLSR_POWERINFO_MSG:
        default:
            print_unknown_data(ndo, msg_data, "\n\t    ", msg_tlen);
            break;
        } /* switch (msg_type) */
        tptr += msg_len;
    } /* while (tptr < (pptr+length)) */

    return;

 trunc:
    ND_PRINT((ndo, "[|olsr]"));
}
