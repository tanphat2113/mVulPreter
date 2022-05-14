static void nsc_encode_subsampling(NSC_CONTEXT* context)
 {
 	UINT16 x;
 	UINT16 y;
	BYTE* co_dst;
	BYTE* cg_dst;
	INT8* co_src0;
	INT8* co_src1;
	INT8* cg_src0;
	INT8* cg_src1;
 	UINT32 tempWidth;
 	UINT32 tempHeight;
 	tempWidth = ROUND_UP_TO(context->width, 8);
 	tempHeight = ROUND_UP_TO(context->height, 2);
 
 	for (y = 0; y < tempHeight >> 1; y++)
 	{
		co_dst = context->priv->PlaneBuffers[1] + y * (tempWidth >> 1);
		cg_dst = context->priv->PlaneBuffers[2] + y * (tempWidth >> 1);
		co_src0 = (INT8*) context->priv->PlaneBuffers[1] + (y << 1) * tempWidth;
		co_src1 = co_src0 + tempWidth;
		cg_src0 = (INT8*) context->priv->PlaneBuffers[2] + (y << 1) * tempWidth;
		cg_src1 = cg_src0 + tempWidth;
 
 		for (x = 0; x < tempWidth >> 1; x++)
 		{
			*co_dst++ = (BYTE)(((INT16) * co_src0 + (INT16) * (co_src0 + 1) +
			                    (INT16) * co_src1 + (INT16) * (co_src1 + 1)) >> 2);
			*cg_dst++ = (BYTE)(((INT16) * cg_src0 + (INT16) * (cg_src0 + 1) +
			                    (INT16) * cg_src1 + (INT16) * (cg_src1 + 1)) >> 2);
			co_src0 += 2;
			co_src1 += 2;
			cg_src0 += 2;
 			cg_src1 += 2;
 		}
 	}
 }
