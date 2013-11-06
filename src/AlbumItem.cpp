#define DEBUG 1
#include <Debug.h>
#include <View.h>

#include "AlbumItem.h"


#define RGBEQ(x,y) (x.red==y.red && x.green==y.green && x.blue==y.blue)

/**
	\warning Takes over the ownership of 'bitmap'.
*/
AlbumItem::AlbumItem(BRect frame, BBitmap *bitmap):
	fFrame(frame),
	fFlags(0),
	fBitmap(bitmap),
	fPadding(6),
	fSelected(false),
	fTextHeight(10),
	fBackColor(B_TRANSPARENT_COLOR)
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
void AlbumItem::DrawItem(BView *owner, uint32 flags)
{
	// Set Colors;
	rgb_color back;
	if (fFlags & ITEM_FLAG_HILIT)
		back = (rgb_color){255,31,31};
	else if (IsSelected())
		back = ui_color(B_MENU_SELECTION_BACKGROUND_COLOR);
	else
		back = fBackColor;
	rgb_color fore = ui_color(IsSelected() ? B_MENU_SELECTED_ITEM_TEXT_COLOR : B_MENU_ITEM_TEXT_COLOR);

	// Background
	if (!RGBEQ(back, B_TRANSPARENT_COLOR)) {
		owner->SetDrawingMode(B_OP_COPY);
		owner->SetLowColor(back);
		owner->FillRoundRect(fFrame, 6, 6, B_SOLID_LOW);
	}
	
	// Icon
	owner->SetDrawingMode(B_OP_OVER);
	BPoint pos = BPoint(fFrame.left + fPadding, fFrame.top + fPadding);
	if (fBitmap) {
		owner->DrawBitmap(fBitmap, pos);
		pos.y += fBitmap->Bounds().Height();
	}

	
	// Label
	if (!(fFlags & ITEM_FLAG_NOLABELS)) {
		owner->SetHighColor(fore);
		for (int i=0; i < CountLabels(); i++) {
			if (Label(i)) {
				pos.y += fTextHeight;
				pos.x = fFrame.left + (fFrame.Width() - owner->StringWidth(Label(i)))/2.0;
				owner->DrawString(Label(i), pos);
			}
		}
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
	if (!(fFlags & ITEM_FLAG_NOLABELS)) {
		if (owner) {
			font_height fh;
			owner->GetFontHeight(&fh);
			fTextHeight = fh.ascent + fh.descent + fh.leading;
		}
		for (int i=0; i < CountLabels(); i++) 
			if (Label(i)) 
				fFrame.bottom += fTextHeight;
	}
}


const BRect& AlbumItem::Frame() const
{
	return fFrame;
}


void AlbumItem::SetFrame(BRect frame)
{
	fFrame = frame;
}


void AlbumItem::SetFlags(uint32 flags)
{
	fFlags = flags;
}


uint32 AlbumItem::Flags()
{
	return fFlags;
}

const BBitmap* AlbumItem::Bitmap() const
{
	return fBitmap;
}


/**
	Deletes the previously set bitmap and sets a new one.
	\warning Takes over the ownership of 'bitmap'.
*/
void AlbumItem::SetBitmap(BBitmap *bitmap)
{
	delete fBitmap;
	fBitmap = bitmap;
}


/**
	Returns the n-th label string.
	Dummy call - implemented in descendats.
*/
const char* AlbumItem::Label(int index)
{
	return NULL;
}

/**
	Sets the n-th label string.
	Dummy call - implemented in descendats.
*/
void AlbumItem::SetLabel(int index)
{
}

/**
	Returns the number of label strings.
*/
int AlbumItem::CountLabels()
{
	return 0;
}


bool AlbumItem::IsSelected()
{
	return fSelected;
}


void AlbumItem::SetSelected(bool enable)
{
	fSelected = enable;
}


/**
	Background color when not IsSelected().
	B_TRANSPARENT_COLOR disables background filling.
*/
void AlbumItem::SetBackColor(rgb_color color)
{
	fBackColor = color;
}


rgb_color AlbumItem::BackColor()
{
	return fBackColor;
}

