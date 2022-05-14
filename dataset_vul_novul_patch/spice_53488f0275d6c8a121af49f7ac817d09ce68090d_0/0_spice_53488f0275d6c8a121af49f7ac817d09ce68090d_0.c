 void red_channel_pipes_add_type(RedChannel *channel, int pipe_item_type)
 {
    RingItem *link, *next;
 
    RING_FOREACH_SAFE(link, next, &channel->clients) {
         red_channel_client_pipe_add_type(
             SPICE_CONTAINEROF(link, RedChannelClient, channel_link),
             pipe_item_type);
    }
}
