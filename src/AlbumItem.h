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

\file AlbumViewItem.h
\brief Generic Thumbnail Browser Item
*/

#ifndef _AlbumItem_H_
#define _AlbumItem_H_

#include <Bitmap.h>
#include <Message.h>
#include <String.h>

enum {
	ALBUMITEM_HILIT 	= 0x01,	
	ALBUMITEM_SEPARATOR = 0x02,	
};

class AlbumItem 
{
	public:
	
	AlbumItem(BRect frame, BBitmap *bitmap);
	virtual ~AlbumItem();
	
	virtual void DrawItem(BView *owner);
	virtual void Update(BView *owner);
	
	virtual void SetFrame(BRect frame);
	const BRect& Frame() const;
	
	void SetFlags(uint32 flags);
	void SetFlags(uint32 mask, bool enable);
	uint32 Flags();
	
	void SetBitmap(BBitmap *bitmap);
	const BBitmap* Bitmap() const;
	
	virtual uint16 CountLabels();
	virtual void GetLabel(uint16 index, BString *label);
	virtual bool IsLabelVisible(uint16 index);

	void Select(bool enable = true);
	bool IsSelected();

	void SetBackColor(rgb_color color);	
	const rgb_color BackColor();

	void SetHighlight(float highlight);
	const float Highlight();
				
	private:
	
	BRect fFrame;
	uint32 fFlags;
	BBitmap *fBitmap;
	bool fSelected;

	protected:
	
	int16 fPadding;
	float fTextHeight;
	rgb_color fBackColor, fHighlightColor;
	float fHighlight;
};


#endif

