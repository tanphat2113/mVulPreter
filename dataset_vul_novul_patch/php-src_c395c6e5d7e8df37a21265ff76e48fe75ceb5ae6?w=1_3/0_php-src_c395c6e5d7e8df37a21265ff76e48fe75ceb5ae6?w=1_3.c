void gdImageFill(gdImagePtr im, int x, int y, int nc)
{
	int l, x1, x2, dy;
	int oc;   /* old pixel value */
	int wx2,wy2;

	int alphablending_bak;

	/* stack of filled segments */
	/* struct seg stack[FILL_MAX],*sp = stack;; */
	struct seg *stack = NULL;
	struct seg *sp;

	if (!im->trueColor && nc > (im->colorsTotal -1)) {
 		return;
 	}
 
	alphablending_bak = im->alphaBlendingFlag;
 	im->alphaBlendingFlag = 0;
 
 	if (nc==gdTiled){
		_gdImageFillTiled(im,x,y,nc);
		im->alphaBlendingFlag = alphablending_bak;
		return;
	}

 	wx2=im->sx;wy2=im->sy;
 	oc = gdImageGetPixel(im, x, y);
 	if (oc==nc || x<0 || x>wx2 || y<0 || y>wy2) {
		im->alphaBlendingFlag = alphablending_bak;
 		return;
 	}
 
	/* Do not use the 4 neighbors implementation with
	 * small images
	 */
	if (im->sx < 4) {
		int ix = x, iy = y, c;
		do {
			do {
				c = gdImageGetPixel(im, ix, iy);
				if (c != oc) {
					goto done;
				}
				gdImageSetPixel(im, ix, iy, nc);
			} while(ix++ < (im->sx -1));
			ix = x;
		} while(iy++ < (im->sy -1));
		goto done;
	}

	stack = (struct seg *)safe_emalloc(sizeof(struct seg), ((int)(im->sy*im->sx)/4), 1);
	sp = stack;

	/* required! */
	FILL_PUSH(y,x,x,1);
	/* seed segment (popped 1st) */
 	FILL_PUSH(y+1, x, x, -1);
	while (sp>stack) {
		FILL_POP(y, x1, x2, dy);

		for (x=x1; x>=0 && gdImageGetPixel(im,x, y)==oc; x--) {
			gdImageSetPixel(im,x, y, nc);
		}
		if (x>=x1) {
			goto skip;
		}
		l = x+1;

                /* leak on left? */
		if (l<x1) {
			FILL_PUSH(y, l, x1-1, -dy);
		}
		x = x1+1;
		do {
			for (; x<=wx2 && gdImageGetPixel(im,x, y)==oc; x++) {
				gdImageSetPixel(im, x, y, nc);
			}
			FILL_PUSH(y, l, x-1, dy);
			/* leak on right? */
			if (x>x2+1) {
				FILL_PUSH(y, x2+1, x-1, -dy);
			}
skip:			for (x++; x<=x2 && (gdImageGetPixel(im, x, y)!=oc); x++);

			l = x;
		} while (x<=x2);
	}

 	efree(stack);
 
 done:
	im->alphaBlendingFlag = alphablending_bak;
 }
