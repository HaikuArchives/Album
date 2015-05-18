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

\file AlbumViewItem.cpp
\brief Generic Thumbnail Browser Item
*/

#include <View.h>
#include "AlbumItem.h"


#ifndef __HAIKU__
#define ITEM_TEXT_COLOR (rgb_color){0,0,0,0}
#define ITEM_SELECTED_TEXT_COLOR ui_color(B_MENU_SELECTED_ITEM_TEXT_COLOR)
#define ITEM_SELECTED_BACKGROUND_COLOR ui_color(B_MENU_SELECTION_BACKGROUND_COLOR)
#define ITEM_HIGHLIGHT_TEXT_COLOR (rgb_color){255,0,0}
#else
#define ITEM_TEXT_COLOR ui_color(B_LIST_ITEM_TEXT_COLOR)
#define ITEM_SELECTED_TEXT_COLOR ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR)
#define ITEM_SELECTED_BACKGROUND_COLOR ui_color(B_LIST_SELECTED_BACKGROUND_COLOR)
#define ITEM_HIGHLIGHT_TEXT_COLOR ui_color(B_CONTROL_HIGHLIGHT_COLOR)
#endif

/**
	\warning Takes over the ownership of 'bitmap'.
*/
AlbumItem::AlbumItem(BRect frame, BBitmap *bitmap):
	fFrame(frame),
	fFlags(0),
	fBitmap(bitmap),
	fSelected(false),
	fPadding(6),
	fTextHeight(10),
	fBackColor(B_TRANSPARENT_COLOR),
	fHighlightColor(ITEM_HIGHLIGHT_TEXT_COLOR)
{
}


AlbumItem::~AlbumItem()
{
	delete fBitmap;
}


/**
	Draws a centered image and labels, if any.
	Derived classes can use 'flags' for additional features.
	\warning Alters BView state.
*/
void AlbumItem::DrawItem(BView *owner)
{
	owner->SetDrawingMode(B_OP_ALPHA);		

	// Background
	rgb_color back = IsSelected() ? ITEM_SELECTED_BACKGROUND_COLOR : fBackColor;
	owner->SetHighColor(back);
	owner->FillRoundRect(fFrame, 6, 6);

	// Highlight 
	if (fHighlight > 0) {
		back = fHighlightColor;
		back.alpha = (int)(255*fHighlight);
		owner->SetHighColor(back);
		owner->FillRoundRect(fFrame, 6, 6);
	}
	
	
	// Icon
	BPoint pos = BPoint(fFrame.left + fPadding, fFrame.top + fPadding);
	if (fBitmap) {
		owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
#ifdef __HAIKU__
		// by hey68you@gmail.com
		BRect rect = fBitmap->Bounds();
		owner->DrawBitmap(fBitmap, rect, rect.OffsetBySelf(pos), B_FILTER_BITMAP_BILINEAR);
#else
		owner->DrawBitmap(fBitmap, pos);
#endif		
		pos.y += fBitmap->Bounds().Height();
	}
	
	// Labels
	rgb_color fore = IsSelected() ? ITEM_SELECTED_TEXT_COLOR : ITEM_TEXT_COLOR;
	owner->SetHighColor(fore);
	owner->SetDrawingMode(B_OP_OVER);
	for (int i=0; i < CountLabels(); i++) {
		if (!IsLabelVisible(i))
			continue;
		BString label;
		GetLabel(i, &label);
		pos.y += fTextHeight;
		owner->TruncateString(&label, B_TRUNCATE_END, fFrame.Width()-fPadding);
		pos.x = fFrame.left + (fFrame.Width() - owner->StringWidth(label.String()))/2.0;
		owner->DrawString(label.String(), pos);	

	}
}



/**
	Resizes Frame() to contents.
	Needs valid 'owner' for font info.
*/
void AlbumItem::Update(BView *owner)
{
	fFrame.right = fFrame.left + 2 * fPadding;
	fFrame.bottom = fFrame.top + 2 * fPadding;	
	
	if (fBitmap) {
		fFrame.right += fBitmap->Bounds().Width();
		fFrame.bottom += fBitmap->Bounds().Height();
	}
	
	if (owner) {
		font_height fh;
		owner->GetFontHeight(&fh);
		fTextHeight = fh.ascent + fh.descent + fh.leading;
	}
	
	for (int i=0; i < CountLabels(); i++)  {
		if (IsLabelVisible(i)) 
			fFrame.bottom += fTextHeight;
	}

}



void AlbumItem::SetFrame(BRect frame)
{
	fFrame = frame;
}



const BRect& AlbumItem::Frame() const
{
	return fFrame;
}



void AlbumItem::SetFlags(uint32 flags)
{
	fFlags = flags;
}


/**
	Turns on or off specific flags.
*/
void AlbumItem::SetFlags(uint32 mask, bool enable)
{
	if (enable)
		fFlags |= mask;
	else
		fFlags &= ~mask;
}



uint32 AlbumItem::Flags()
{
	return fFlags;
}



/**
	Deletes the previously set bitmap and sets a new one.
	\warning Takes over the ownership of 'bitmap'.
*/
void AlbumItem::SetBitmap(BBitmap *bitmap)
{
	if (fBitmap)
		delete fBitmap;
	fBitmap = bitmap;
}


const BBitmap* AlbumItem::Bitmap() const
{
	return fBitmap;
}



/**
	Returns the number of label strings.
	Descendants override this.
*/
uint16 AlbumItem::CountLabels()
{
	return 0;
}


/**
	Returns the n-th label string.
	Descendants override this.
*/
void AlbumItem::GetLabel(uint16 index, BString *label)
{
}


/**
	Is n-th label visible?
	Descendants override this.
*/
bool AlbumItem::IsLabelVisible(uint16 index)
{
	return true;
}



void AlbumItem::Select(bool enable)
{
	fSelected = enable;
}


bool AlbumItem::IsSelected()
{
	return fSelected;
}


void AlbumItem::SetBackColor(rgb_color color)
{
	fBackColor = color;
}


const rgb_color AlbumItem::BackColor()
{
	return fBackColor;
}


/**
	Special items intensity.
	1.0 is full effect, 0.0 no highlight.
*/
void AlbumItem::SetHighlight(float highlight)
{
	fHighlight = highlight;
	if (fHighlight < 0)
		fHighlight = 0;
}


const float AlbumItem::Highlight()
{
	return fHighlight;
}

