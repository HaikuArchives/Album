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

\file LayoutView.cpp
\brief Auto-layout View
\author M.Kovac

Depends on LayoutPlan.cpp

*/

#include <LayoutView.h>


LayoutView::LayoutView(BRect frame, const char *name, LayoutPlan *layout, uint32 resize, uint32 flags):
	BView(frame, name, resize, flags),
	fLayout(layout),
	fDefaultHint(LAYOUT_HINT_NONE),
	fPadding(0,0,0,0)
{
}


LayoutView::~LayoutView()
{
	delete fLayout;
}

/**
	
*/
void LayoutView::AllAttached()
{
	Arrange();
}


/**
	Frame() changed.
*/
void LayoutView::FrameResized(float width, float height)
{
	Arrange();
}


LayoutPlan& LayoutView::Layout()
{
	return *fLayout;
}

void LayoutView::SetDefaultHint(uint32 hint)
{
	fDefaultHint = hint;
}

void LayoutView::SetPadding(float left, float top, float right, float bottom)
{
	fPadding.Set(left, top, right, bottom);
}

/**
	Lays out child views.
*/
void LayoutView::Arrange()
{
	if (fLayout) {
		fLayout->Frame() = Bounds();
		fLayout->Frame().left += fPadding.left;
		fLayout->Frame().right -= fPadding.right;
		fLayout->Frame().top += fPadding.top;
		fLayout->Frame().bottom -= fPadding.bottom;
		fLayout->Reset();
		BView *child;
		for (int32 i = 0; (child = ChildAt(i)); i++) {
			child->ResizeToPreferred();
			uint hint = fDefaultHint;
			if (i == CountChildren()-1)
				// may trigger some special processing
				hint |= LAYOUT_HINT_LAST;
			BRect frame = fLayout->Next(child->Frame(), hint);
			child->MoveTo(frame.LeftTop());
			child->ResizeTo(frame.Width(), frame.Height());
		}
	}
}

