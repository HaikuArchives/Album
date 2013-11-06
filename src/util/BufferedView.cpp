/**
Copyright (c) 2005-2006 by Matjaz Kovac

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

\file BufferedView.cpp
\author Matjaž Kovač
\brief BufferedView class implementation
*/

#include <BufferedView.h>
#include <Bitmap.h>

/**
\class BufferedView
This abstract BView descendant implements a display rendering technique
known as double-buffering. 
The general idea is to do all the drawing in an off-screen bitmap (buffer) then 
bitblt it on the screen in a single operation.
The purpose of the whole excercise is to get rid of that annoying flicker
during animation or when rendering complex scenes.
All this is rather straight forward in the BeOS as BView objects can render to
BBitmap objects besides the normal app_server communication.

Descendants need to implement DrawOffscreen() instead
of Draw() and do all their drawing through the supplied 'view'.
This makes it possible to switch to off-screen mode or the normal, 
less wasteful app_server drawing at run-time with SetBuffering().
*/

/**
	Constructs a new instance.

	Similat to BView constructor, with an additional 'async' flag that causes DrawBitmpaAsync() to be used.	
*/

BufferedView::BufferedView(BRect rect, const char *name, uint32 mode, uint32 flags, bool async):
	BView(rect, name, mode, flags),
	fBuffering(false),
	fAsync(async),
	fBuffer(NULL),
	fBufferView(NULL),
	fGranularity(128)
	
{
	fBackColor = ViewColor();
}


BufferedView::~BufferedView()
{
	DeleteBuffer();
	delete fBufferView;
}

/**
Off-screen rendering.

If IsBuffering() is false this method behaves exactly like BView::Draw().
\warning Do not use for drawing, override DrawOffscreen() instead!
*/
void BufferedView::Draw(BRect update)
{
	if (IsBuffering() && fBuffer) {
	
		BRect bounds = Bounds();	
		if (bounds.Width() > fBuffer->Bounds().Width() || bounds.Height() > fBuffer->Bounds().Height())
			CreateBuffer(bounds.IntegerWidth(), bounds.IntegerHeight());

		fBuffer->Lock();
		fBufferView->ResizeTo(bounds.Width(), bounds.Height());
		fBufferView->ScrollTo(bounds.LeftTop());

		BRect source = update;
		source.OffsetBy(-bounds.left, -bounds.top);
		// Clear background
		fBufferView->SetLowColor(fBackColor);
		fBufferView->SetDrawingMode(B_OP_COPY);
		fBufferView->FillRect(update, B_SOLID_LOW);
		// Render the destination patch.
		fBufferView->PushState();
		DrawOffscreen(fBufferView, update);
		fBufferView->Sync();
		fBufferView->PopState();
		// Blit
		Sync();
        if (fAsync) 
	        DrawBitmapAsync(fBuffer, source, update);
        else
		   DrawBitmap(fBuffer, source, update);
		fBuffer->Unlock();
	}
	else 
		// no double-buffering
		DrawOffscreen(this, update);
}


/**
	Allocates the off-screen bitmap.

	The problem here is how to determine the maximum view size since
	there may be multiple screens with different resolutions.
	The simplest solution is to increase the off-screen bitmap
	on demand. This is done in oversized chunks (tiles) to minimise
	memory allocation operations.
*/
void BufferedView::CreateBuffer(int32 width, int32 height)
{	
	DeleteBuffer();
	// allocate a bit more to allow for window resize.
	int32 w = (width / 128 + 1) * 128;
	int32 h = (height / 128 + 1) * 128;
	fBuffer = new BBitmap(BRect(0, 0, w, h), B_RGB32, true);
	if (fBuffer) {
		fBuffer->Lock();
		fBufferView->SetLowColor(fBackColor);
        fBuffer->AddChild(fBufferView);
		fBuffer->Unlock();
	}	
}

/**
	Frees up allocated objects.
*/
void BufferedView::DeleteBuffer()
{
	if (fBuffer) {
		fBuffer->Lock();
		fBufferView->RemoveSelf();
		fBuffer->Unlock();
	    delete fBuffer;
		fBuffer = NULL;
	}
}


/**
	Enables or disable doublebuffering.
	
	ViewColor() is stored and set to B_TRANSPARENT_COLOR to disable redundant
	background painting since we touch every pixel every time.
*/
void BufferedView::SetBuffering(bool enabled)
{
	fBuffering = enabled;
	if (fBuffering) {
	    SetViewColor(B_TRANSPARENT_COLOR);
		SetDrawingMode(B_OP_COPY);
		if (!fBufferView) {
			fBufferView = new BView(Bounds(), "OffscreenBitmap", 0, 0);
			fBufferView->MoveTo(0,0);
			fBufferView->SetViewColor(fBackColor);
		}
		CreateBuffer(Bounds().IntegerWidth(), Bounds().IntegerHeight());
	}
	else {
		DeleteBuffer();
	    delete fBufferView;
	    fBufferView = NULL;
		SetViewColor(fBackColor);
	}
}


bool BufferedView::IsBuffering()
{
	return fBuffering && fBuffer;
}



