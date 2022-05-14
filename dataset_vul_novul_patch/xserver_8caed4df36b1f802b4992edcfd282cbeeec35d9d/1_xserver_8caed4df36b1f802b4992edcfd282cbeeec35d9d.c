 ProcXSendExtensionEvent(ClientPtr client)
 {
    int ret;
     DeviceIntPtr dev;
     xEvent *first;
     XEventClass *list;
    struct tmask tmp[EMASKSIZE];

    REQUEST(xSendExtensionEventReq);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        (stuff->num_events * bytes_to_int32(sizeof(xEvent))))
        return BadLength;

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixWriteAccess);
    if (ret != Success)
        return ret;

    if (stuff->num_events == 0)
        return ret;

     /* The client's event type must be one defined by an extension. */
 
     first = ((xEvent *) &stuff[1]);
    if (!((EXTENSION_EVENT_BASE <= first->u.u.type) &&
          (first->u.u.type < lastEvent))) {
        client->errorValue = first->u.u.type;
        return BadValue;
     }
 
     list = (XEventClass *) (first + stuff->num_events);
        return ret;

    ret = (SendEvent(client, dev, stuff->destination,
                     stuff->propagate, (xEvent *) &stuff[1],
                     tmp[stuff->deviceid].mask, stuff->num_events));

    return ret;
}
