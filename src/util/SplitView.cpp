/**
Copyright (c) 2006-2008 by Matjaz Kovac

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

\file SplitView.cpp
\brief Auto-layout View
\author M.Kovac

Depends on LayoutPlan.cpp

*/
#include <Window.h>
#include <Message.h>
#include <SplitView.h>


/**
	Creates a new instance.
	Add children normally. When the instance itself is attached it will arrange them.
	B_PULSE_NEED flag enables the "drawer" animation effect.
*/
SplitView::SplitView(BRect frame, const char *name, uint32 mode, uint32 resize, uint32 flags):
	BView(frame, name, resize, flags | B_WILL_DRAW),
	fLayout(BRect(), mode),
	fDragged(false),
	fSelected(NULL),
	fAniPane(NULL),
	fSnap(5),
	fAniTo(0), fAniFrom(0)
{
	fLayout.SetSpacing(7, 7);
}


SplitView::~SplitView()
{
}

/**
	Arranges children.	
*/
void SplitView::AllAttached()
{
	Arrange();
}

/**
	Draws all spliters.	
*/
void SplitView::Draw(BRect update)
{
	BView *child;	
	for (int i = 0; i < CountChildren() -1; i++) {
		child = ChildAt(i);
		BRect frame = child->Frame();
		if (fLayout.Mode() == B_HORIZONTAL) {
			frame.left = frame.right+1;
			frame.right = frame.left + fLayout.Spacing().x-2;
		}
		else {
			frame.top = frame.bottom+1;
			frame.bottom = frame.top + fLayout.Spacing().y-2;
		}
		if (frame.Intersects(update))
			DrawSplitter(frame, child == fSelected && fDragged);		
	}
}

/**
	Frame() changed.
*/
void SplitView::FrameResized(float width, float height)
{
	Draw(Bounds());
}

/**
	Starts dragging.
*/
void SplitView::MouseDown(BPoint where)
{
    // This is an event hook so there must be a Looper.
    BMessage *message = Looper()->CurrentMessage();
    int32 clicks = 0;
    message->FindInt32("clicks", &clicks);
    fDoubleClick = clicks == 2;
	if (!fDragged) {
		fDragged = true;
		fGrabPoint = where;
		BView *child = NULL;
		for (int i=0; (child = ChildAt(i)); i++) {
			if (fLayout.Mode() == B_HORIZONTAL && child->Frame().left > where.x)
				break;
			if (fLayout.Mode() == B_VERTICAL && child->Frame().top > where.y)
				break;
		}
		if (child)
			fSelected = child->PreviousSibling();
		Draw(Bounds());
		// Subscribe to off-view mouse events.
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
	}
}

/**
	Ends dragging.
*/
void SplitView::MouseUp(BPoint where)
{
	fDragged = false;
	if (fDoubleClick && fSelected && !fAniPane) {
		fAniPane = fSelected;
		if (fLayout.Mode() == B_HORIZONTAL)
			fAniFrom = fAniPane->Frame().Width();
		else if (fLayout.Mode() == B_VERTICAL) 
			fAniFrom = fAniPane->Frame().Height();
	}
	Draw(Bounds());
}

/**
	Tracks mouse movement.
*/
void SplitView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	if (fDragged && fSelected) {
		// Check limits.
		BPoint d(0,0);
		// Snap to grid, reduces updates.
		d.x = (floor(d.x / fSnap)) * fSnap;
		d.y = (floor(d.y / fSnap)) * fSnap;

		if (fLayout.Mode() == B_HORIZONTAL) {
			d.x = point.x - fGrabPoint.x;
			if (fSelected->Frame().Width() + d.x < 0)
				d.x = 0;
		}
		else if (fLayout.Mode() == B_VERTICAL) {
			d.y = point.y - fGrabPoint.y;
			if (fSelected->Frame().Height() + d.y < 0)
				d.y = 0;
		}
		// Resize		
		MoveSplitter(fSelected, d);
		// Save this for the next delta.
		fGrabPoint += d;
		fAniTo = 0;
		if (fLayout.Mode() == B_HORIZONTAL)
			fAniFrom = fSelected->Frame().Width();
		else if (fLayout.Mode() == B_VERTICAL) 
			fAniFrom = fSelected->Frame().Height();
	}
}

/**
	Animates fold-in/out.
*/
void SplitView::Pulse()
{
	if (fAniPane) {
		BRect frame = fAniPane->Frame();
		if (fLayout.Mode() == B_HORIZONTAL && fAniTo <= fAniFrom && frame.Width() > 0)
			MoveSplitter(fAniPane, BPoint(-frame.Width()/2 - 1, 0));
		else if (fLayout.Mode() == B_HORIZONTAL && fAniTo > fAniFrom && frame.Width() < fAniTo)
			MoveSplitter(fAniPane, BPoint((fAniTo - frame.Width()) / 2 + 1, 0));
		else if (fLayout.Mode() == B_VERTICAL && fAniTo <= fAniFrom && frame.Height() > 0)
			MoveSplitter(fAniPane, BPoint(0, -frame.Height()/2 - 1));
		else if (fLayout.Mode() == B_VERTICAL && fAniTo > fAniFrom && frame.Height() < fAniTo)
			MoveSplitter(fAniPane, BPoint(0, (fAniTo - frame.Height()) / 2 + 1));
		else {
			fAniTo = fAniFrom;
			fAniPane = NULL;
		}
	}
}


/**
	Lays out child views.
*/
void SplitView::Arrange()
{
	fLayout.Frame()= Bounds();
	fLayout.Reset();
	BView *child;
	uint32 hint = LAYOUT_HINT_NONE;
	for (int32 i = 0; (child = ChildAt(i)); i++) {
		if (i == CountChildren() - 1)
			hint |= (LAYOUT_HINT_LAST | LAYOUT_HINT_CLIP);
		BRect frame = fLayout.Next(child->Frame(), hint);
		child->MoveTo(frame.LeftTop());
		child->ResizeTo(frame.Width(), frame.Height());
	}
}

/**
	Draws space between child views.
*/
void SplitView::DrawSplitter(BRect frame, bool active)
{
	// Do not cache ui_color values, they may change anytime.
	rgb_color back = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color hi = tint_color(back, B_LIGHTEN_1_TINT);
	rgb_color lo = tint_color(back, B_DARKEN_1_TINT);

	// Border
	BeginLineArray(4);
	AddLine(frame.LeftTop(), frame.RightTop(), hi);
	AddLine(frame.LeftTop(), frame.LeftBottom(), hi);
	AddLine(frame.RightBottom(), frame.LeftBottom(), lo);
	AddLine(frame.RightBottom(), frame.RightTop(), lo);
	EndLineArray();

	// Background
	frame.InsetBy(1,1);
	SetLowColor(back);
	SetHighColor(tint_color(back, B_LIGHTEN_MAX_TINT));
	//SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
	FillRect(frame, active ? B_MIXED_COLORS : B_SOLID_LOW);
	
}

/**
	Resizes a child view (pane) and its sibling.
*/
void SplitView::MoveSplitter(BView *pane, BPoint d)
{
	pane->ResizeBy(d.x, d.y);
	BView *sibling = pane->NextSibling();
	if (sibling) {
		// Don't overlap the next view (a smaller update rect).
		if (d.x > 0 || d.y  > 0)
			sibling->ResizeBy(-d.x, -d.y);
		sibling->MoveBy(d.x, d.y);
		if (d.x < 0 || d.y < 0)
			sibling->ResizeBy(-d.x, -d.y);
	}	
	Draw(Bounds());
}
