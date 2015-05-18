/**
Copyright (c) 2006-2015 by Matjaz Kovac

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

\file AlbumView.cpp
\brief Generic Thumbnail Browser
*/

#define DEBUG 1
#include <Debug.h>
#include <Window.h>
#include <ScrollBar.h>
#include <LayoutPlan.h>
#include "AlbumView.h"



AlbumView::AlbumView(BRect frame, const char *name, BMessage *message, uint32 resizing, uint32 flags): 
	BufferedView(frame, name, resizing, flags | B_WILL_DRAW | B_FRAME_EVENTS, true),
	BInvoker(message, NULL),
	fItems(20, true),
	fOrderBy(NULL),
	fPage(0,0,10,10),
	fZoom(1.0),
	fMask(0),
	fColumns(0),
	fLastSelected(-1),
	fDoubleClick(false),
	fMayDrag(false),
	fSeparatorHeight(40.0),
	fSmallStep(30.0)
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
	view->SetScale(fZoom);
	AlbumItem *item;
	for (int i = 0; (item = ItemAt(i)); i++) {
 			if (IsItemVisible(item) && update.Intersects(Adjust(item->Frame()))) {
				item->DrawItem(view);
 			}
	}
}


/**
	Keyboard navigation.
*/
void AlbumView::KeyDown(const char *bytes, int32 numBytes)
{
	AlbumItem *item = ItemAt(fLastSelected);
	if (item == NULL)
		return;
    BRect b = Bounds();
	BRect frame = item->Frame();
	// you never know...
	if (!frame.IsValid())
		return;
	BRect r;
    int32 i = fLastSelected;
    switch (bytes[0]) {
    	case B_LEFT_ARROW:
        	while (i > 0 && !IsItemVisible(item = ItemAt(--i))) {}
        	break;
    	case B_RIGHT_ARROW:
        	while (i >= 0 && i < CountItems()-1 && !IsItemVisible(item = ItemAt(++i))) {}
        	break;
    	case B_UP_ARROW:
			while (i > 0) {
				item = ItemAt(--i);
          		r = item->Frame();
          		if (IsItemVisible(item) && r.top < frame.top && (r.right==frame.right || r.left < frame.right))
          			break;
			}
    		break;
    	case B_DOWN_ARROW:
			while (i >= 0 && i < CountItems()-1) {
				item = ItemAt(++i);
          		r = item->Frame();
           		if (IsItemVisible(item) && r.top > frame.bottom && ( r.left==frame.left || r.right > frame.left) )
           			break;
			}
    		break;
    	case B_PAGE_DOWN:
			while (i >= 0 && i < CountItems()-1) {
				item = ItemAt(++i);
          		r = item->Frame();
           		if (IsItemVisible(item) && r.top > frame.bottom+b.Height() && ( r.left==frame.left || r.right > frame.left) )
           			break;
			}
    		break;
    	case B_PAGE_UP:
			while (i > 0) {
				item = ItemAt(--i);
          		r = item->Frame();
          		if (IsItemVisible(item) && r.top < frame.top-b.Height() && (r.right==frame.right || r.left < frame.right))
          			break;
			}
    		break;
    	default:
        	BView::KeyDown(bytes, numBytes);
        	break;
    }
	     
	// change selection
	if (i != fLastSelected && IsItemVisible(item)) {
	   	int32 mods = 0;
	   	Window()->CurrentMessage()->FindInt32("modifiers", &mods);

	    if (mods & B_SHIFT_KEY) {
			// block selection
	    	SelectBlock(fLastSelected, i, !ItemAt(i)->IsSelected());
	    	Select(i);
	    }
	    else  {
	    	// normal selection
	    	DeselectAll();
	    	Select(i);
	    }    
	    fLastSelected = i;
	    
	    // notify 
		SelectionChanged();		
		if (Message()) {
			BMessage msg = *Message();
        	msg.AddInt32("index", fLastSelected);
			Invoke(&msg);
		}
		
		// bring into view  
		r = Adjust(ItemAt(fLastSelected)->Frame());
		float dx=0,dy=0;      
		if (r.right > b.right)
			dx = r.right - b.right;
		else if (r.left < b.left)
			dx = r.left - b.left;
		if (r.bottom > b.bottom)
			dy = r.bottom - b.bottom;
		else if (r.top < b.top)
			dy = r.top - b.top;
		ScrollBy(dx,dy);				
	}
}



/**
	Mouse click scenarios.
*/
void AlbumView::MouseDown(BPoint where)
{
    // This is an event hook so there must be a Looper.
	BMessage *message = Window()->CurrentMessage();

    int32 mods = 0, clicks = 0, buttons=0;
    message->FindInt32("modifiers", &mods);
    message->FindInt32("clicks", &clicks);
    message->FindInt32("buttons", &buttons);
		
    // Scale back.
	where.x /= fZoom;
	where.y /= fZoom;
	
	int32 i = IndexOf(&where);
	int32 changes = 0;
	if (i >= 0) {
		AlbumItem *item = ItemAt(i);
		
		// double-clicks are handled later in MouseUp()
		fDoubleClick = (fLastSelected == i && clicks == 2 && (buttons & B_PRIMARY_MOUSE_BUTTON));
        fMayDrag = !fDoubleClick && (buttons & B_PRIMARY_MOUSE_BUTTON);			
        if (mods & B_SHIFT_KEY) 
        	// Block selection
			changes += SelectBlock(fLastSelected, i, !item->IsSelected());
        else if (mods & B_COMMAND_KEY)
        	// Modify selection
        	changes += Select(i, 1, !item->IsSelected());
		else {
			// Normal selection
			if (!item->IsSelected())
				changes += DeselectAll();
        	changes += Select(i);
		}
        fLastWhere = where;
        fLastSelected = i;
    }
    else
	   	changes += DeselectAll();

	if (changes > 0) {
		//PRINT(("selection changed\n"));
    	SelectionChanged();
    	if (!fDoubleClick && Message()) {
			BMessage msg = *Message();
			msg.AddInt32("buttons", buttons);
    		msg.AddPoint("where", where);        
        	msg.AddInt32("index", fLastSelected);
			Invoke(&msg);
    	}
	}
    
    
}


/**
	Completes double-clicks etc.
*/
void AlbumView::MouseUp(BPoint where)
{
	if (fDoubleClick && Message()) {
		BMessage msg = *Message();
		msg.AddBool("launch", true);
        msg.AddInt32("index", fLastSelected);
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
		if (point.x*point.x + point.y*point.y > 50) {
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
		uint32 hint = item->Flags() & ALBUMITEM_SEPARATOR ? LAYOUT_HINT_BREAK : 0;
		BRect frame = layout.Next(frame0, hint);
		if (hint == LAYOUT_HINT_BREAK) {
			// shift the last frame, so we get a gap in the layout
			layout.Last().OffsetBy(0,fSeparatorHeight);
			frame = layout.Last();
		}
		if (frame != frame0) {
			// rects outside the bounds are new
			if (invalidate && bounds.Intersects(frame0) && frame0.left >= 0) {
				// clear the old rect
				Invalidate(Adjust(frame0));				
			}
			// reposition to the new location
			item->SetFrame(frame);
			if (invalidate && bounds.Intersects(frame))  {
				// show on the new location
				Invalidate(Adjust(frame));
			}
		}
		if (frame.right > width)
			width = frame.right;
		if (frame.bottom > height)
			height = frame.bottom;
	}
	SetPageBounds(BRect(0,0,width,height));
}


/**
	Applies SortBy function.
*/
void AlbumView::SortItems()
{
	if (fOrderBy) {
		fItems.SortItems(fOrderBy);
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

void AlbumView::SetColumns(int16 cols)
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


/**
	First item containing point.
*/
int32 AlbumView::IndexOf(BPoint *p)
{
	if (p == NULL)
		return -1;
		
	AlbumItem *item;
	for (int32 i = 0; (item = ItemAt(i)); i++) {
		if (!IsItemVisible(item) || !item->Frame().Contains(*p))
			continue;
		return i;
	}	
	return -1;
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
	return fItems.RemoveItemAt(index);
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
	BRect r = item->Frame();
	Invalidate(Adjust(r));
}



int32 AlbumView::DeselectAll()
{
	int32 n = 0;
	AlbumItem *item = NULL;
	for  (int i = 0; (item = ItemAt(i)); i++) {
		if (!item->IsSelected())
			continue;
		item->Select(false);
		InvalidateItem(item);
		n++;
	}
	fLastSelected = -1;
	return n;
}


/**
	Adds or removes items from the selection.
	Returns number of changes.
*/
int32 AlbumView::Select(int32 index, int32 count, bool enabled)
{
	int32 n = 0;
	for (int i = index; i < index + count ;i++) {
		AlbumItem *item = ItemAt(i);
		if (IsItemVisible(item) && item->IsSelected() != enabled) {
			item->Select(enabled);
			InvalidateItem(item);
			n++;	
		}
	}
	return n;
}


int32 AlbumView::SelectBlock(int32 from, int32 to, bool enabled)
{
	if (from > to)
		to ^= from ^= to ^= from;
	return Select(from, to - from + 1, enabled);	
}


int32 AlbumView::CountSelected()
{
	int32 n = 0;
	AlbumItem *item;
	for (int32 i=0; (item = ItemAt(i)); i++) {
		if (item->IsSelected())
			n++;
	}
	return n;
}


AlbumItem* AlbumView::SelectedItem(int32 index)
{
	int32 n = 0;
	AlbumItem *item;
	for (int32 i=0; (item = ItemAt(i)); i++) {
		if (!item->IsSelected())
			continue;
		if (index == n++)
			return item;
	}
	return NULL;
}


void AlbumView::DeleteSelected()
{
	AlbumItem *item = NULL;
	for  (int i = CountItems()-1; (item = ItemAt(i)); i--)
		if (item->IsSelected())
			fItems.RemoveItem(item, true);
	
	Arrange(false);
	Invalidate();
}


/**
	Sets visibility mask.
	Only items with Flags() bits in Mask() are visible. 	
*/
void AlbumView::SetMask(uint32 mask)
{
	fMask = mask;
	Arrange(false);
	Invalidate();
}



/**
	Visibility mask.
	Only items with Flags() bits in Mask() are visible. 
*/
const uint32 AlbumView::Mask()
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
		hscroll->SetSteps(fSmallStep, range/10);
	}
	BScrollBar *vscroll = ScrollBar(B_VERTICAL);
	if (vscroll) {
		float full = PageBounds().bottom * fZoom;
		float range = full - height;
		if (range < 0)
			range = 0;
		vscroll->SetProportion(height / full);
		vscroll->SetRange(0, range);
		vscroll->SetSteps(fSmallStep, range/10);
	}
}




