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

\file LayoutPlan.cpp
\brief Sequential LayoutPlan Engine implementation
*/

#include <LayoutPlan.h>


/**
\class LayoutPlan
LayoutPlan engine interface.

The idea is to create an instance of a particular LayoutPlan descendant, then
feed it element frames with FitNext(). The returned adjusted frames can then be used for
view positioning, drawing or whatever. 
*/
LayoutPlan::LayoutPlan(BRect frame):
	fFrame(frame),
	fSpacing(0,0)
{
}


/**
	Get the constraint frame.

	The constraint frame is merely a guide. Element frames may be placed out of bounds,
	but this should be explicitly hinted. 
*/
BRect& LayoutPlan::Frame()
{
	return (BRect&)fFrame;
}

BRect& LayoutPlan::Last()
{
	return (BRect&)fLast;
}


/**
	\fn virtual BRect LayoutPlan::Next(BRect frame, uint32 hint=0)
	Lays out an element frame.
	Essentially defines the LayoutPlan rules of a descendant class.
	After each call the internal state may be changed. It is assumed that the engine was Reset() before the first call.
	An input rectangle is adjusted based on the general LayoutPlan strategy, internal state and an optional hints.
	Lower 8 bits of the hint mask are reserved for standard ::layout_hints and should be either ignored or
	implemented as close to the defined meaning as possible.
	
	\param frame An element frame to be laid out.
	\param hint Special cases and exceptions.
	\returns Adjusted input frame copy
*/

BPoint LayoutPlan::Spacing()
{
	return fSpacing;
}

void LayoutPlan::SetSpacing(float x, float y)
{
	fSpacing.x = x;
	fSpacing.y = y;
}

void LayoutPlan::Reset()
{
	Last().Set(0,0,-1,-1);
}








/**
	Creates a new flow LayoutPlan engine.
	See Next().
*/
FlowLayout::FlowLayout(BRect frame, int columns):
	LayoutPlan(frame),
	fMaxCol(columns)
{
	Reset();
}


/**
	Reset the engine state.
*/
void FlowLayout::Reset()
{
	LayoutPlan::Reset();
	fRowHeight = 0;
	fCol = 0;
}



/**
	Fit an element frame into the LayoutPlan.
	
	Frames are fitted in a flowing fashion. 
	Next element is fitted right of the previous one if there is
	still room in Frame() otherwise the flow is broken and continues
	in the next line.

	The following hints are honored:		
	- LAYOUT_HINT_BREAK forces line break <b>after</b> this element.
	- LAYOUT_HINT_OVERRUN prevents line break <b>on</b>  this element if Frame() is overrun.
	- LAYOUT_HINT_CLIP clips any rectangles sticking out.
*/
BRect FlowLayout::Next(BRect rect, uint32 hint)
{
	if (!Last().IsValid())
		rect.OffsetTo(Frame().LeftTop() + Spacing());
	else {
		if ((!fMaxCol && Last().right + 1 + rect.Width() > Frame().Width() && !(hint & LAYOUT_HINT_OVERRUN)) 
			|| (fMaxCol && fCol >= fMaxCol -1)
			|| hint & LAYOUT_HINT_BREAK) {
			// line break
			rect.OffsetTo(Frame().left + Spacing().x, Last().top + fRowHeight + 1 + Spacing().y);
			fRowHeight = 0;
			fCol = 0;
		}
		else {
			// normal row run
			rect.OffsetTo(Last().right + 1 + Spacing().x, Last().top);
			if ((hint & LAYOUT_HINT_CLIP) && rect.Intersects(Frame()))
				rect = Frame() & rect;
			fCol++;
		}
	}
	if (rect.Height() > fRowHeight)
		fRowHeight = rect.Height();
	Last() = rect;
	return rect;
}





StripesLayout::StripesLayout(BRect frame, uint32 orientation):
	LayoutPlan(frame),
	fMode(orientation)
{
}

StripesLayout::StripesLayout(uint32 orientation):
	LayoutPlan(BRect()),
	fMode(orientation)
{
}



/**
	Lays out an element frame.
	
	Frames are placed one next to the other in one row (or column). Unless hinted otherwise, 
	frames are extended down (or right) so they form a sort of stripes.
	Since the width (or height) of a stripe is defined by its input rectangle this only works
	well until Frame() is exceeded. 
		
	The following hints are honored:		
	- LAYOUT_HINT_LAST will extend 'frame' to completely fill the remaining Frame() area, leaving no more room.
	- LAYOUT_HINT_NO_RESIZE will only change offsets without resizing.
*/
BRect StripesLayout::Next(BRect rect, uint32 hint)
{
	if (fMode == B_VERTICAL) {
		if (!Last().IsValid())
			// first rect
			rect.OffsetTo(rect.left,Frame().top);
		else
			// next rect
			rect.OffsetTo(rect.left, Last().bottom + Spacing().y);

		if (hint & LAYOUT_HINT_VCENTER)
			rect.OffsetTo(Frame().left + (Frame().Width()-rect.Width())/2.0, rect.top);
			
		if (!(hint & LAYOUT_HINT_NO_RESIZE)) {
			// adjust size
			if (!(hint & LAYOUT_HINT_HCENTER))
				rect.right = Frame().right;
			if (hint & LAYOUT_HINT_LAST) 
				rect.bottom = Frame().bottom;
			if ((hint & LAYOUT_HINT_CLIP) && rect.Intersects(Frame()))
				rect = Frame() & rect;				
		}
	}
	else if (fMode == B_HORIZONTAL) {
		if (!Last().IsValid()) 
			// first rect
			rect.OffsetTo(Frame().left, rect.top);
		else 
			// next rect
			rect.OffsetTo(Last().right  + Spacing().x, rect.top);

		if (hint & LAYOUT_HINT_VCENTER)
			rect.OffsetTo(rect.left, Frame().top + (Frame().Height()-rect.Height())/2.0);
		else
			rect.top = Frame().top;	
		
		if (!(hint & LAYOUT_HINT_NO_RESIZE)) {
			if (!(hint & LAYOUT_HINT_VCENTER))
				rect.bottom = Frame().bottom;
			if (hint & LAYOUT_HINT_LAST) 
				rect.right = Frame().right;
		}
	}
	Last() = rect;
	return rect;
}


