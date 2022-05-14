on_response(void *data, krb5_error_code retval, otp_response response)
{
    struct request_state rs = *(struct request_state *)data;

    free(data);

     if (retval == 0 && response != otp_response_success)
         retval = KRB5_PREAUTH_FAILED;
 
     rs.respond(rs.arg, retval, NULL, NULL, NULL);
 }
