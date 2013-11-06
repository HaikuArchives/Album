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

\file IconButton.cpp
*/

#include <IconButton.h>
#include <algorithm>

IconButton::IconButton(BRect frame, const char *name, const char *label, BBitmap *iconOff, BBitmap *iconOn, BMessage *message, uint32 resizing, uint32 flags):
	BButton(frame, name, label, message, resizing, flags | B_WILL_DRAW),
	fIconOn(iconOn),
	fIconOff(iconOff)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetFontSize(be_plain_font->Size()-2.0);
}


IconButton::~IconButton()
{
	delete fIconOn;
	delete fIconOff;
}


void IconButton::AttachedToWindow()
{
	SetTarget(this);
}


void IconButton::Draw(BRect update)
{
	SetDrawingMode(B_OP_COPY);
	BRect bounds = Bounds();
	SetLowColor(ViewColor());
	FillRect(bounds, B_SOLID_LOW);
	SetDrawingMode(B_OP_ALPHA);
	if (Value() > 0 && fIconOn) {
		BPoint pos(bounds.left + (bounds.Width() - fIconOn->Bounds().Width()) / 2.0, 1);
		DrawBitmap(fIconOn, pos);
	}
	else if (fIconOff) {
		BPoint pos(bounds.left + (bounds.Width() - fIconOff->Bounds().Width()) / 2.0, 1);
		DrawBitmap(fIconOff, pos);
	}


	SetDrawingMode(B_OP_OVER);
	if (!IsEnabled()) {
		SetHighColor(LowColor());
		FillRect(Bounds(), B_MIXED_COLORS);
	}
	
	SetHighColor(ui_color(B_MENU_ITEM_TEXT_COLOR));
	if (Label()) {
		font_height fh;
		GetFontHeight(&fh);
		BPoint pos((bounds.Width() - StringWidth(Label())) / 2.0 +1, bounds.bottom - fh.descent -1);
		DrawString(Label(), pos);
	}

	if (IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(bounds);
	}


}


/**

void IconButton::MouseDown(BPoint where)
{
	if (IsEnabled()) {
		SetValue(B_CONTROL_ON);
		Invalidate();
		SetMouseEventMask(B_POINTER_EVENTS);
	}
}


void IconButton::MouseUp(BPoint where)
{
	if (IsEnabled()) {
		SetValue(B_CONTROL_OFF);
		Invalidate();
		if (Bounds().Contains(where)) {
			Invoke();
		}
	}
}
*/

void IconButton::GetPreferredSize(float *width, float *height)
{
	*height = 2;
	if (fIconOff) {
		*width = fIconOff->Bounds().Height() + 2;
		*height += fIconOff->Bounds().Height();
	}
	if (Label()) {
		font_height fh;
		GetFontHeight(&fh);
		*width = std::max(*width, StringWidth(Label())) + 2;
		*height += fh.ascent + fh.descent + fh.leading;
	}

}
