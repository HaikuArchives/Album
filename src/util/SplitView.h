/**
\file SplitView.h
\brief Auto-layout View

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

#ifndef _SPLITVIEW_H_
#define _SPLITVIEW_H_

#include <View.h>
#include "LayoutPlan.h"

class SplitView : public BView {
	public:

	SplitView(BRect frame, const char *name, uint32 mode, uint32 resize, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);	
	virtual ~SplitView();
	virtual void AllAttached();
	virtual void Draw(BRect update);
	virtual void FrameResized(float width, float height);
	virtual void MouseDown(BPoint where);	
	virtual void MouseUp(BPoint where);	
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	virtual void Pulse();
		
	virtual void Arrange();
	virtual void DrawSplitter(BRect frame, bool active);
	void MoveSplitter(BView *pane, BPoint delta);
	
	private:
	
	StripesLayout fLayout;
	bool fDragged, fDoubleClick;
	BPoint fGrabPoint;
	BView *fSelected, *fAniPane;
	float fSnap, fAniTo, fAniFrom;
	
};

#endif
