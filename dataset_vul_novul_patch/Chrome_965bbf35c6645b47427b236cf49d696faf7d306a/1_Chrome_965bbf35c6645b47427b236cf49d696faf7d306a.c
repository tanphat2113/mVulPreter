InlineBoxPosition ComputeInlineBoxPositionTemplate(
    const PositionTemplate<Strategy>& position,
    TextAffinity affinity,
    TextDirection primary_direction) {
  int caret_offset = position.ComputeEditingOffset();
  Node* const anchor_node = position.AnchorNode();
  LayoutObject* layout_object =
      anchor_node->IsShadowRoot()
          ? ToShadowRoot(anchor_node)->host().GetLayoutObject()
          : anchor_node->GetLayoutObject();

  DCHECK(layout_object) << position;

  if (layout_object->IsText()) {
    return ComputeInlineBoxPositionForTextNode(layout_object, caret_offset,
                                                affinity, primary_direction);
   }
 
  if (layout_object->IsLayoutBlockFlow()) {
    if (CanHaveChildrenForEditing(anchor_node) &&
        HasRenderedNonAnonymousDescendantsWithHeight(layout_object)) {
      const PositionTemplate<Strategy>& downstream_equivalent =
          DownstreamIgnoringEditingBoundaries(position);
      if (downstream_equivalent != position) {
        return ComputeInlineBoxPosition(
            downstream_equivalent, TextAffinity::kUpstream, primary_direction);
      }
      const PositionTemplate<Strategy>& upstream_equivalent =
          UpstreamIgnoringEditingBoundaries(position);
      if (upstream_equivalent == position ||
          DownstreamIgnoringEditingBoundaries(upstream_equivalent) == position)
        return InlineBoxPosition();
      return ComputeInlineBoxPosition(
          upstream_equivalent, TextAffinity::kUpstream, primary_direction);
    }
   }
 
  if (!layout_object->IsAtomicInlineLevel())
     return InlineBoxPosition();
  if (!layout_object->IsBox())
     return InlineBoxPosition();
  InlineBox* const inline_box = ToLayoutBox(layout_object)->InlineBoxWrapper();
  if (!inline_box)
     return InlineBoxPosition();
  if ((caret_offset > inline_box->CaretMinOffset() &&
       caret_offset < inline_box->CaretMaxOffset()))
    return InlineBoxPosition(inline_box, caret_offset);
  return AdjustInlineBoxPositionForTextDirection(
      inline_box, caret_offset, layout_object->Style()->GetUnicodeBidi(),
      primary_direction);
 }
