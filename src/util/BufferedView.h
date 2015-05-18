/**
\file BufferedView.h
\brief BufferedView definitions.

Copyright (c) 2007 by Matjaz Kovac

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

#ifndef _BUFFEREDVIEW_H_
#define _BUFFEREDVIEW_H_

#include <View.h>

class BBitmap;

/// A double-buffering capable view
class BufferedView : public BView 
{
	public:

	BufferedView(BRect frame, const char *name, uint32 mode, uint32 flags, bool async = true); 
	virtual ~BufferedView();
		
	virtual void Draw(BRect update);
	/// Actual drawing
	virtual void DrawOffscreen(BView *view, BRect update) = 0;
	void CreateBuffer(int32 width, int32 height);
	void DeleteBuffer();
	void SetBuffering(bool enabled);
	inline bool IsBuffering();

	private:

	bool fBuffering;
	bool fAsync;
	BBitmap *fBuffer;
	BView *fBufferView;
	rgb_color fBackColor;
};

#endif
