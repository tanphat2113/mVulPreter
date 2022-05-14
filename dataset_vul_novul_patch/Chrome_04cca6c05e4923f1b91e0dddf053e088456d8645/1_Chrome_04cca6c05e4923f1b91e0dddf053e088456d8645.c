bool InlineFlowBox::nodeAtPoint(const HitTestRequest& request, HitTestResult& result, int x, int y, int tx, int ty)
{
    IntRect overflowRect(visualOverflowRect());
    flipForWritingMode(overflowRect);
    overflowRect.move(tx, ty);
    if (!overflowRect.intersects(result.rectForPoint(x, y)))
        return false;

    for (InlineBox* curr = lastChild(); curr; curr = curr->prevOnLine()) {
        if ((curr->renderer()->isText() || !curr->boxModelObject()->hasSelfPaintingLayer()) && curr->nodeAtPoint(request, result, x, y, tx, ty)) {
            renderer()->updateHitTestResult(result, IntPoint(x - tx, y - ty));
            return true;
         }
     }
 
    FloatPoint boxOrigin = locationIncludingFlipping();
    boxOrigin.move(tx, ty);
    FloatRect rect(boxOrigin, IntSize(width(), height()));
     if (visibleToHitTesting() && rect.intersects(result.rectForPoint(x, y))) {
         renderer()->updateHitTestResult(result, flipForWritingMode(IntPoint(x - tx, y - ty))); // Don't add in m_x or m_y here, we want coords in the containing block's space.
         if (!result.addNodeToRectBasedTestResult(renderer()->node(), x, y, rect))
            return true;
    }
    
    return false;
}
