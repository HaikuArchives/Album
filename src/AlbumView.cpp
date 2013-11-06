#include "AlbumView.h"
#include <LayoutPlan.h>
#include <ScrollBar.h>
#include <Window.h>



AlbumView::AlbumView(BRect frame, const char *name, BMessage *message, uint32 resizing, uint32 flags): 
	BufferedView(frame, name, resizing, flags | B_WILL_DRAW | B_FRAME_EVENTS, true),
	BInvoker(message, NULL),
	fPage(0,0,10,10),
	fItems(20, true),
	fSelection(20, false),
	fOrderBy(NULL),
	fZoom(1.0),
	fLastSelected(-1),
	fColumns(0),
	fDoubleClick(false),
	fMask(0),
	fNoDataMsg("Drop files here.")
{
}


/**
	The view is on.
	All BView functions can be called now.
*/
void AlbumView::AttachedToWindow()
{
	SetTarget(this, Looper());
	// Initial scrollbar positions
	UpdateScrollbars(Frame().Width(), Frame().Height());	
	
}


BRect AlbumView::Adjust(BRect rect)
{
	rect.left *= fZoom;
	rect.top *= fZoom;
	rect.right *= fZoom;
	rect.bottom *= fZoom;
	return rect;
}


void AlbumView::FrameResized(float width, float height)
{
	if (fColumns == 0)
		Arrange(true);
	UpdateScrollbars(width, height);
	
	// update the splash message
	if (CountItems() == 0)
		Invalidate();	
}


/**
	Renders all visible items.
*/
void AlbumView::DrawOffscreen(BView *view, BRect update)
{
	DrawBackground(view, update);

	view->SetScale(fZoom);
	AlbumItem *item;
	for (int i = 0; (item = ItemAt(i)); i++) {
 			if (IsItemVisible(item) && update.Intersects(Adjust(item->Frame())))
				item->DrawItem(view, 0);
	}
}

void AlbumView::DrawBackground(BView *view, BRect update)
{
	if (fItems.CountItems() == 0) {
		view->SetFontSize(16.0);
		view->SetHighColor(96,96,96);
		BRect rc = Bounds();
		view->DrawString(fNoDataMsg.String(), BPoint((rc.Width() - view->StringWidth(fNoDataMsg.String()))/2, rc.Height()/2));
	}
}

void AlbumView::MessageReceived(BMessage *message)
{
 	switch (message->what) {
 		case B_SELECT_ALL:
 			Select(0, CountItems());
 			break;
   		default:
   			BView::MessageReceived(message);
   			break;
	}
}

/**
	Keyboard navigation.
*/
void AlbumView::KeyDown(const char *bytes, int32 numBytes)
{
	if (fLastSelected < 0)
		return;
	BRect frame = ItemAt(fLastSelected)->Frame();
    AlbumItem *next = NULL;
    int32 i = fLastSelected;

    switch (bytes[0]) {
    case B_LEFT_ARROW:
        if (i > 0)
        	i--;
        break;
    case B_RIGHT_ARROW:
        if (i >= 0 && i < CountItems()-1)
        	i++;
        break;
    case B_UP_ARROW:
		while (i > 0) {
          	next = ItemAt(--i);
          	if (next->Frame().top < frame.top && next->Frame().left < frame.right )
          		break;
		}
    	break;
    case B_DOWN_ARROW:
		while (i >= 0 && i < CountItems()-1) {
           	next = ItemAt(++i);
           	if (next->Frame().top > frame.bottom && next->Frame().left >= frame.left )
           		break;
		}
    	break;
    default:
        BView::KeyDown(bytes, numBytes);
        break;
    }

	if (i != fLastSelected) {
	   	int32 mods = 0;
	   	Window()->CurrentMessage()->FindInt32("modifiers", &mods);
	    if (!(mods & B_SHIFT_KEY))
	    	DeselectAll();
	    Select(i,1,true);
	    fLastSelected = i;
	    BMessage msg;
		if (Message())
			msg = *Message();
        msg.AddInt32("index", i);
		Invoke(&msg);
	}
}


/**
	Mouse click scenarios.
*/
void AlbumView::MouseDown(BPoint where)
{
	BMessage msg;
	if (Message())
		msg = *Message();
    // This is an event hook so there must be a Looper.
    BMessage *message = Window()->CurrentMessage();
    int32 mods = 0, clicks = 0, buttons=0;
    message->FindInt32("modifiers", &mods);
    message->FindInt32("clicks", &clicks);
    message->FindInt32("buttons", &buttons);
    msg.AddInt32("buttons", buttons);
    msg.AddPoint("where", where);
		
    AlbumItem *item;
    // Scale back.
	where.x /= fZoom;
	where.y /= fZoom;
	for (int32 i = 0; (item = ItemAt(i)); i++) {
		if (!IsItemVisible(item))
			continue;
		if (!item->Frame().Contains(where))
			continue;
        if (mods & B_SHIFT_KEY) {
        	// Block selection
        	int from = fLastSelected;
        	int to = i;
			if (from > to)
    			to ^= from ^= to ^= from;
            Select(from, to - from + 1, true);
        }
        else if (mods & B_COMMAND_KEY)
        	// Modify selection
        	Select(i, 1, !item->IsSelected());
		else if (!item->IsSelected()) {
			// Normal selection
        	DeselectAll();
        	Select(i);
		}
		// double-clicks are handled later in MouseUp()
		fDoubleClick = (fLastSelected == i && clicks == 2);
        fLastSelected = i;
        fLastWhere = where;
        fMayDrag = !fDoubleClick && (buttons == B_PRIMARY_MOUSE_BUTTON);
        msg.AddInt32("index", i);
        break;
    }
    if (!item)
    	DeselectAll();
	Invoke(&msg);
}


/**
	Completes double-clicks etc.
*/
void AlbumView::MouseUp(BPoint where)
{
	if (fDoubleClick) {
		BMessage msg;
		if (Message())
			msg = *Message();
        msg.AddPointer("item", ItemAt(fLastSelected));
		Invoke(&msg);
	}
	fDoubleClick = false;
	fMayDrag = false;
}


/**
 * Mouse tracking.
 * Detect item dragging.
 */
void AlbumView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
    if (transit == B_INSIDE_VIEW && fMayDrag) {
	    // Scale back.
		point.x -= fLastWhere.x*fZoom;
		point.y -= fLastWhere.y*fZoom;
		// Ignore minor jitter...
		if (point.x*point.x + point.y*point.y > 36) {
		   fMayDrag = false;
		   fDoubleClick = false;
		   ItemDragged(fLastSelected, fLastWhere);
		}
    }
}


/**
	Determines an item's visibility.
*/
bool AlbumView::IsItemVisible(AlbumItem *item)
{
	return !fMask || (fMask & item->Flags());
}


/**
*/
void AlbumView::ItemDragged(int32 index, BPoint where)
{
}


/**
	Lays out items one after another.
*/
void AlbumView::Arrange(bool invalidate)
{
	// scale back the page relative to zoom ratio
	BRect bounds = Bounds();
	bounds.left /= fZoom;
	bounds.top /= fZoom;
	bounds.right /= fZoom;
	bounds.bottom /= fZoom;
	FlowLayout layout(bounds.OffsetToCopy(0,0), fColumns);
	layout.SetSpacing(1,1);
	// The entire set must be examined.
	float width = 0;
	float height = 0;
	AlbumItem *item;
	for (int32 i = 0; (item = ItemAt(i)); i++) {
		if (!IsItemVisible(item))
			continue;
		BRect frame0 = item->Frame();
		// separator
		uint32 hint = item->Flags() & ITEM_FLAG_SEPARATOR ? LAYOUT_HINT_BREAK : 0;
		BRect frame = layout.Next(frame0, hint);
		if (hint == LAYOUT_HINT_BREAK) {
			layout.Last().OffsetBy(0,60);
			frame = layout.Last();
		}
		if (frame != frame0) {
			// rects outside the bounds are new
			if (invalidate && bounds.Intersects(frame0) && frame0.left >= 0) 
				// clear the old rect
				Invalidate(Adjust(frame0));
			// reposition
			item->SetFrame(frame);
			if (invalidate && bounds.Intersects(frame)) 
				Invalidate(Adjust(frame));
		}
		if (frame.right > width)
			width = frame.right;
		if (frame.bottom > height)
			height = frame.bottom;
	}
	SetPageBounds(BRect(0,0,width,height));
}


void AlbumView::SortItems()
{
	if (fOrderBy) {
		fItems.SortItems(fOrderBy);
		fSelection.SortItems(fOrderBy);
	}
}



const BRect& AlbumView::PageBounds() const
{
	return fPage;
}


/**
	Uses app_server for actual scaling.
*/
void AlbumView::SetZoom(float scale)
{
	fZoom = scale;
	Arrange(false);
	Invalidate();
}

float AlbumView::Zoom()
{
	return fZoom;
}

void AlbumView::SetColumns(int cols)
{
	fColumns = cols;
	Arrange(false);
	Invalidate();
}

void AlbumView::SetPageBounds(BRect bounds)
{
	fPage = bounds;
	UpdateScrollbars(Frame().Width(), Frame().Height());
}


int32 AlbumView::CountItems()
{
	return fItems.CountItems();
}


AlbumItem* AlbumView::ItemAt(int32 index)
{
	return fItems.ItemAt(index);
}


int32 AlbumView::IndexOf(AlbumItem *item)
{
	return fItems.IndexOf(item);
}


AlbumItem* AlbumView::AddItem(AlbumItem *item, int32 index)
{
	bool ok;
	if (fOrderBy) 
		ok = fItems.BinaryInsert(item, fOrderBy);
	else if (index < 0)
		ok = fItems.AddItem(item);
	else
		ok = fItems.AddItem(item, index);
	return ok ? item : NULL;
	
	
}


AlbumItem* AlbumView::RemoveItem(int32 index)
{
	AlbumItem *item = fItems.RemoveItemAt(index);
	// NOP if NULL
	fSelection.RemoveItem(item);
	return item;
}


AlbumItem* AlbumView::EachItem(BObjectList<AlbumItem>::EachFunction func, void *param)
{
	return fItems.EachElement(func, param);
}


void AlbumView::SetOrderBy(BObjectList<AlbumItem>::CompareFunction func)
{
	if (func != fOrderBy) {
		fOrderBy = func;
		SortItems();
	}
}


BObjectList<AlbumItem>::CompareFunction AlbumView::OrderByFunc()
{
	return fOrderBy;
}


void AlbumView::InvalidateItem(AlbumItem *item)
{
	Invalidate(Adjust(item->Frame()));
}



void AlbumView::DeselectAll()
{
	AlbumItem *item = NULL;
	for  (int i = 0; (item = fSelection.ItemAt(i)); i++) {
		item->SetSelected(false);
		InvalidateItem(item);
	}
	fSelection.MakeEmpty();
	fLastSelected = -1;
}


/**
	Adds or removes items from the selection.
*/
void AlbumView::Select(int32 index, int32 count, bool enabled)
{
	for (int i = index; i < index + count ;i++) {
		AlbumItem *item = ItemAt(i);
		if (IsItemVisible(item) && item && item->IsSelected() != enabled) {
			item->SetSelected(enabled);
			InvalidateItem(item);
			if (enabled)
				fSelection.AddItem(item);
			else
				fSelection.RemoveItem(item);
		}
	}
}

int32 AlbumView::CountSelected()
{
	return fSelection.CountItems();
}


AlbumItem* AlbumView::GetSelection(int32 index)
{
	return fSelection.ItemAt(index);
}


void AlbumView::DeleteSelected()
{
	AlbumItem *item = NULL;
	for  (int i = 0; (item = fSelection.ItemAt(i)); i++)
		fItems.RemoveItem(item, true);
	Arrange(false);
	Invalidate();
	fSelection.MakeEmpty();
}


/**
*/
void AlbumView::SetMask(uint32 mask)
{
	fMask = mask;
	Arrange(false);
	Invalidate();
}


uint32 AlbumView::Mask()
{
	return fMask;
}


void AlbumView::UpdateScrollbars(float width, float height)
{
	BScrollBar *hscroll = ScrollBar(B_HORIZONTAL);
	if (hscroll) {
		float full = PageBounds().right * fZoom;
		float range = full - width;
		if (range < 0)
			range = 0;
		hscroll->SetProportion(width / full);
		hscroll->SetRange(0, range);
		hscroll->SetSteps(32, range/10);
	}
	BScrollBar *vscroll = ScrollBar(B_VERTICAL);
	if (vscroll) {
		float full = PageBounds().bottom * fZoom;
		float range = full - height;
		if (range < 0)
			range = 0;
		vscroll->SetProportion(height / full);
		vscroll->SetRange(0, range);
		vscroll->SetSteps(32, range/10);
	}
}




