static Image *ReadLABELImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    geometry[MaxTextExtent],
    *property;

  const char
    *label;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  TypeMetric
    metrics;

  size_t
    height,
    width;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  (void) ResetImagePage(image,"0x0+0+0");
  property=InterpretImageProperties(image_info,image,image_info->filename);
  (void) SetImageProperty(image,"label",property);
  property=DestroyString(property);
  label=GetImageProperty(image,"label");
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  draw_info->text=ConstantString(label);
  metrics.width=0;
  metrics.ascent=0.0;
  status=GetMultilineTypeMetrics(image,draw_info,&metrics);
  if ((image->columns == 0) && (image->rows == 0))
    {
      image->columns=(size_t) (metrics.width+draw_info->stroke_width+0.5);
      image->rows=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
    }
  else
    if (((image->columns == 0) || (image->rows == 0)) ||
        (fabs(image_info->pointsize) < MagickEpsilon))
      {
        double
          high,
          low;

        /*
          Auto fit text into bounding box.
        */
        for ( ; ; draw_info->pointsize*=2.0)
        {
          (void) FormatLocaleString(geometry,MaxTextExtent,"%+g%+g",
            -metrics.bounds.x1,metrics.ascent);
          if (draw_info->gravity == UndefinedGravity)
            (void) CloneString(&draw_info->geometry,geometry);
          status=GetMultilineTypeMetrics(image,draw_info,&metrics);
          (void) status;
          width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
          height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
          if ((image->columns != 0) && (image->rows != 0))
            {
              if ((width >= image->columns) && (height >= image->rows))
                break;
            }
          else
            if (((image->columns != 0) && (width >= image->columns)) ||
                ((image->rows != 0) && (height >= image->rows)))
              break;
        }
        high=draw_info->pointsize;
        for (low=1.0; (high-low) > 0.5; )
        {
          draw_info->pointsize=(low+high)/2.0;
          (void) FormatLocaleString(geometry,MaxTextExtent,"%+g%+g",
            -metrics.bounds.x1,metrics.ascent);
          if (draw_info->gravity == UndefinedGravity)
            (void) CloneString(&draw_info->geometry,geometry);
          status=GetMultilineTypeMetrics(image,draw_info,&metrics);
          width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
          height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
          if ((image->columns != 0) && (image->rows != 0))
            {
              if ((width < image->columns) && (height < image->rows))
                low=draw_info->pointsize+0.5;
              else
                high=draw_info->pointsize-0.5;
            }
          else
            if (((image->columns != 0) && (width < image->columns)) ||
                ((image->rows != 0) && (height < image->rows)))
              low=draw_info->pointsize+0.5;
            else
              high=draw_info->pointsize-0.5;
        }
        draw_info->pointsize=(low+high)/2.0-0.5;
      }
  status=GetMultilineTypeMetrics(image,draw_info,&metrics);
  if (status == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (image->columns == 0)
    image->columns=(size_t) (metrics.width+draw_info->stroke_width+0.5);
  if (image->columns == 0)
    image->columns=(size_t) (draw_info->pointsize+draw_info->stroke_width+0.5);
  if (image->rows == 0)
    image->rows=(size_t) (metrics.ascent-metrics.descent+
       draw_info->stroke_width+0.5);
   if (image->rows == 0)
     image->rows=(size_t) (draw_info->pointsize+draw_info->stroke_width+0.5);
   if (draw_info->gravity == UndefinedGravity)
     {
       (void) FormatLocaleString(geometry,MaxTextExtent,"%+g%+g",
        -metrics.bounds.x1+draw_info->stroke_width/2.0,metrics.ascent+
        draw_info->stroke_width/2.0);
      (void) CloneString(&draw_info->geometry,geometry);
    }
  if (draw_info->direction == RightToLeftDirection)
    {
      if (draw_info->direction == RightToLeftDirection)
        (void) FormatLocaleString(geometry,MaxTextExtent,"%+g%+g",
          image->columns-(metrics.bounds.x2+draw_info->stroke_width/2.0),
          metrics.ascent+draw_info->stroke_width/2.0);
      (void) CloneString(&draw_info->geometry,geometry);
    }
  if (SetImageBackgroundColor(image) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) AnnotateImage(image,draw_info);
  if (image_info->pointsize == 0.0)
    {
      char
        pointsize[MaxTextExtent];

      (void) FormatLocaleString(pointsize,MaxTextExtent,"%.20g",
        draw_info->pointsize);
      (void) SetImageProperty(image,"label:pointsize",pointsize);
    }
  draw_info=DestroyDrawInfo(draw_info);
  return(GetFirstImageInList(image));
}
