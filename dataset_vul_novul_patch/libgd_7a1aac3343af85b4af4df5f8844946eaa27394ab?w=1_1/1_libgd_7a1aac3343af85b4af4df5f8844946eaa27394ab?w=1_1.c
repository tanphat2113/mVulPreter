int gdTransformAffineCopy(gdImagePtr dst,
		  int dst_x, int dst_y,
		  const gdImagePtr src,
		  gdRectPtr src_region,
		  const double affine[6])
{
	int c1x,c1y,c2x,c2y;
	int backclip = 0;
	int backup_clipx1, backup_clipy1, backup_clipx2, backup_clipy2;
	register int x, y, src_offset_x, src_offset_y;
	double inv[6];
	int *dst_p;
	gdPointF pt, src_pt;
	gdRect bbox;
	int end_x, end_y;
	gdInterpolationMethod interpolation_id_bak = GD_DEFAULT;
	interpolation_method interpolation_bak;

	/* These methods use special implementations */
 	if (src->interpolation_id == GD_BILINEAR_FIXED || src->interpolation_id == GD_BICUBIC_FIXED || src->interpolation_id == GD_NEAREST_NEIGHBOUR) {
 		interpolation_id_bak = src->interpolation_id;
 		interpolation_bak = src->interpolation;
 		gdImageSetInterpolationMethod(src, GD_BICUBIC);
 	}
 

	gdImageClipRectangle(src, src_region);

	if (src_region->x > 0 || src_region->y > 0
		|| src_region->width < gdImageSX(src)
		|| src_region->height < gdImageSY(src)) {
		backclip = 1;

		gdImageGetClip(src, &backup_clipx1, &backup_clipy1,
		&backup_clipx2, &backup_clipy2);

		gdImageSetClip(src, src_region->x, src_region->y,
			src_region->x + src_region->width - 1,
			src_region->y + src_region->height - 1);
	}

	if (!gdTransformAffineBoundingBox(src_region, affine, &bbox)) {
		if (backclip) {
			gdImageSetClip(src, backup_clipx1, backup_clipy1,
					backup_clipx2, backup_clipy2);
		}
		gdImageSetInterpolationMethod(src, interpolation_id_bak);
		return GD_FALSE;
	}

	gdImageGetClip(dst, &c1x, &c1y, &c2x, &c2y);

	end_x = bbox.width  + (int) fabs(bbox.x);
	end_y = bbox.height + (int) fabs(bbox.y);

	/* Get inverse affine to let us work with destination -> source */
	gdAffineInvert(inv, affine);

	src_offset_x =  src_region->x;
	src_offset_y =  src_region->y;

	if (dst->alphaBlendingFlag) {
		for (y = bbox.y; y <= end_y; y++) {
			pt.y = y + 0.5;
			for (x = 0; x <= end_x; x++) {
				pt.x = x + 0.5;
				gdAffineApplyToPointF(&src_pt, &pt, inv);
				gdImageSetPixel(dst, dst_x + x, dst_y + y, getPixelInterpolated(src, src_offset_x + src_pt.x, src_offset_y + src_pt.y, 0));
			}
		}
	} else {
		for (y = 0; y <= end_y; y++) {
			pt.y = y + 0.5 + bbox.y;
			if ((dst_y + y) < 0 || ((dst_y + y) > gdImageSY(dst) -1)) {
				continue;
			}
			dst_p = dst->tpixels[dst_y + y] + dst_x;

			for (x = 0; x <= end_x; x++) {
				pt.x = x + 0.5 + bbox.x;
				gdAffineApplyToPointF(&src_pt, &pt, inv);

				if ((dst_x + x) < 0 || (dst_x + x) > (gdImageSX(dst) - 1)) {
					break;
				}
				*(dst_p++) = getPixelInterpolated(src, src_offset_x + src_pt.x, src_offset_y + src_pt.y, -1);
			}
		}
	}

	/* Restore clip if required */
	if (backclip) {
		gdImageSetClip(src, backup_clipx1, backup_clipy1,
				backup_clipx2, backup_clipy2);
	}

	gdImageSetInterpolationMethod(src, interpolation_id_bak);
	return GD_TRUE;
}
