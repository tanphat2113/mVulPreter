static double digitize(double value, int depth, int do_round)
{
 /* 'value' is in the range 0 to 1, the result is the same value rounded to a
    * multiple of the digitization factor - 8 or 16 bits depending on both the
    * sample depth and the 'assume' setting.  Digitization is normally by

     * rounding and 'do_round' should be 1, if it is 0 the digitized value will
     * be truncated.
     */
   PNG_CONST unsigned int digitization_factor = (1U << depth) -1;
 
    /* Limiting the range is done as a convenience to the caller - it's easier to
     * do it once here than every time at the call site.
    */
 if (value <= 0)
      value = 0;

 else if (value >= 1)
      value = 1;

   value *= digitization_factor;
 if (do_round) value += .5;
 return floor(value)/digitization_factor;
}
