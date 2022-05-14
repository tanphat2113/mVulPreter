print_attr_string(netdissect_options *ndo,
                  register const u_char *data, u_int length, u_short attr_code)
{
   register u_int i;

   ND_TCHECK2(data[0],length);

   switch(attr_code)
    {
       case TUNNEL_PASS:
            if (length < 3)
           {
              ND_PRINT((ndo, "%s", tstr));
              return;
           }
            if (*data && (*data <=0x1F) )
               ND_PRINT((ndo, "Tag[%u] ", *data));
            else
              ND_PRINT((ndo, "Tag[Unused] "));
           data++;
           length--;
           ND_PRINT((ndo, "Salt %u ", EXTRACT_16BITS(data)));
           data+=2;
           length-=2;
        break;
      case TUNNEL_CLIENT_END:
      case TUNNEL_SERVER_END:
      case TUNNEL_PRIV_GROUP:
      case TUNNEL_ASSIGN_ID:
      case TUNNEL_CLIENT_AUTH:
      case TUNNEL_SERVER_AUTH:
            if (*data <= 0x1F)
            {
               if (length < 1)
              {
                 ND_PRINT((ndo, "%s", tstr));
                 return;
              }
               if (*data)
                 ND_PRINT((ndo, "Tag[%u] ", *data));
               else
                ND_PRINT((ndo, "Tag[Unused] "));
              data++;
              length--;
            }
         break;
       case EGRESS_VLAN_NAME:
            ND_PRINT((ndo, "%s (0x%02x) ",
                   tok2str(rfc4675_tagged,"Unknown tag",*data),
                   *data));
           data++;
           length--;
         break;
    }
 
   for (i=0; *data && i < length ; i++, data++)
        ND_PRINT((ndo, "%c", (*data < 32 || *data > 126) ? '.' : *data));
 
    return;

   trunc:
      ND_PRINT((ndo, "%s", tstr));
}
