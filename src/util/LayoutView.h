/**
\file LayoutView.h
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

#ifndef _LAYOUTVIEW_H_
#define _LAYOUTVIEW_H_

#include <LayoutPlan.h>

class LayoutView : public BView {
	public:

	LayoutView(BRect frame, const char *name, LayoutPlan *layout, uint32 resize = B_FOLLOW_NONE, uint32 flags = 0);	
	virtual ~LayoutView();
	virtual void AllAttached();
	virtual void FrameResized(float width, float height);
	LayoutPlan& Layout();
	void SetDefaultHint(uint32 hint);	
	void SetPadding(float left, float top, float right, float bottom);
	virtual void Arrange();
	
	private:
	
	LayoutPlan *fLayout;
	uint32 fDefaultHint;
	BRect fPadding;
};

#endif
