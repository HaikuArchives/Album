/**
\file LayoutPlan.h
\brief Sequential Layout Engine

Copyright (c) 2006 by Matjaz Kovac

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
*/

#ifndef _LAYOUTPLAN_H_
#define _LAYOUTPLAN_H_

#include <Rect.h>
#include <View.h>
/**
	Optional instructions for FitNext() implementations.
*/
enum layout_hints {
	LAYOUT_HINT_NONE = 0,
	LAYOUT_HINT_LAST = 1, ///< Complete the layout and finalize the state.
	LAYOUT_HINT_NO_RESIZE = 2, ///< Only reposition, no resize.
	LAYOUT_HINT_BREAK = 4, ///< Force a flow break <b>after</b> a frame.
	LAYOUT_HINT_OVERRUN = 8, ///< Prevent a flow break <b>on</b> a frame.
	LAYOUT_HINT_CLIP = 16, ///< Shorten a frame if the constraint frame is overrun.
	LAYOUT_HINT_HCENTER = 32, 
	LAYOUT_HINT_VCENTER = 64, 
};

/** 
	Abstract layout engine interface.
*/
class LayoutPlan
{
		
public:
	LayoutPlan(BRect frame);
	inline BRect& Frame();
	inline BRect& Last();
	BPoint Spacing();
	void SetSpacing(float x, float y);
	virtual void Reset();
	virtual BRect Next(BRect frame, uint32 hint = LAYOUT_HINT_NONE) = 0;
private:
	BRect fFrame, fLast;
	BPoint fSpacing;
};


/**
	Left-to-right flow layout.
*/
class FlowLayout : public LayoutPlan
{
public:
	FlowLayout(BRect frame, int columns = 0);
	virtual void Reset();
	virtual BRect Next(BRect frame, uint32 hint = LAYOUT_HINT_NONE);
private:
	float fRowHeight;
	int fCol, fMaxCol;
};

/**
	Vertical or horizontal stripes layout.
*/
class StripesLayout : public LayoutPlan
{
	public:
	
	/// mode: B_HORIZONTAL, B_VERTICAL
	StripesLayout(BRect frame, uint32 orientation);
	StripesLayout(uint32 orientation);
	virtual BRect Next(BRect frame, uint32 hint = LAYOUT_HINT_NONE);
	inline uint32 Mode();	
	private:
	uint32 fMode;
};


uint32 StripesLayout::Mode()
{
	return fMode;
}


#endif
