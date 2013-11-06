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

\file NameValueItem.cpp
\author Matjaž Kovač
\brief Name-Value BListItem
*/

#include <View.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <stdlib.h>
#include <NameValueItem.h>

// strftime
#define TIME_FORMAT "%x %X"

/**
	Copy constructor
*/
NameValueItem::NameValueItem(const NameValueItem& source):
	BListItem(source.OutlineLevel(), source.IsExpanded()),
	fValueColor(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	fNameColor(tint_color(fValueColor, B_LIGHTEN_2_TINT))
{
	fName = source.Name();
	fLabel = source.Label();
	fType = source.Type();
	fValue = source.Value();
	fReadOnly = source.IsReadOnly();
	fDivider = source.Divider();
}


/**
	Creates a new instance without value.
	\param name Name or label text
	\param level Outline level (BOutlineListView only)
	\param expanded Is outline expanded (BOutlineListView only)
*/
NameValueItem::NameValueItem(const char *name, uint32 level, bool expanded):
	BListItem(level, expanded),
	fName(name),
	fLabel(name),
	fValueColor(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	fNameColor(tint_color(fValueColor, B_LIGHTEN_2_TINT)),
	fDivider(0),
	fReadOnly(false)
{
}


/**
	Creates a new instance with a string value.
	\param name Name or label text
	\param value Value part
	\param level Outline level (BOutlineListView only)
	\param expanded Is outline expanded (BOutlineListView only)
*/
NameValueItem::NameValueItem(const char *name, const char *value, uint32 level, bool expanded):
	BListItem(level, expanded),
	fName(name),
	fValue(value),
	fLabel(name),
	fValueColor(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	fNameColor(tint_color(fValueColor, B_LIGHTEN_2_TINT)),
	fType(B_STRING_TYPE)
{
}


/**
	Creates a new instance and convertes an arbitrary value.
	\param name Name or label text
	\param value Value part
	\param level Outline level (BOutlineListView only)
	\param expanded Is outline expanded (BOutlineListView only)
*/
NameValueItem::NameValueItem(const char *name, type_code type, const void *value, ssize_t size, 
							uint32 level, bool expanded):
	BListItem(level, expanded),
	fName(name),
	fValue(NULL),
	fLabel(name),
	fValueColor(ui_color(B_MENU_ITEM_TEXT_COLOR)),
	fNameColor(tint_color(fValueColor, B_LIGHTEN_2_TINT)),
	fDivider(0),
	fReadOnly(false),
	fType(type)
{
	SetValue(type, value, size);
}



void NameValueItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	owner->PushState();
	// Normal colors can be adjusted for each item.
	rgb_color value_color = fValueColor;
	rgb_color name_color = fNameColor;
	// Background
	if (IsSelected() || complete) {
		rgb_color back;
		if (IsSelected()) {
			// It is best not to cache ui_color values as they may change anytime.
			back = ui_color(B_MENU_SELECTION_BACKGROUND_COLOR);
			value_color = ui_color(B_MENU_SELECTED_ITEM_TEXT_COLOR);
			name_color = tint_color(value_color, B_DARKEN_1_TINT);
		}
		else
			back = owner->ViewColor();
		owner->SetHighColor(back);
		owner->FillRect(frame);
	}
	// Text
	font_height fh;
	owner->GetFontHeight(&fh);
	owner->SetDrawingMode(B_OP_OVER);
	owner->MovePenTo(frame.left + OutlineLevel() + 1, frame.bottom - fh.descent);
	owner->SetHighColor(name_color);
	owner->DrawString(fLabel.String());
	fDivider = owner->PenLocation().x +1;
	owner->MovePenBy(2, 0);
	owner->SetHighColor(value_color);
	owner->DrawString(fValue.String());
	// We're done here.
	owner->PopState();
}


/**
	Prepares the item.
	Called by BListView.
*/
void NameValueItem::Update(BView* owner, const BFont* font)
{
	SetWidth(owner->Frame().Width());
	font_height fh;
	font->GetHeight(&fh);
	SetHeight(fh.ascent + fh.descent + fh.leading + 1);
}


/**
	The on-screen width of the label.
*/
float NameValueItem::Divider() const
{
	return fDivider;
}


bool NameValueItem::IsReadOnly() const
{
	return fReadOnly;
}


const char* NameValueItem::Name() const
{
	return fName.String();
}

const char* NameValueItem::Label() const
{
	return fLabel.String();
}


uint32 NameValueItem::Type() const
{
	return fType;
}


/**
	Returns the value part.
*/
const char* NameValueItem::Value() const
{
	return fValue.String();
}


/**
	Sets the name part.
*/
void NameValueItem::SetName(const char *name)
{
	fName = name;
}

/**
	Sets the name part.
*/
void NameValueItem::SetLabel(const char *label)
{
	fLabel = label;
}


/**
	Sets the ReadOnly flag.
	No special processing.
*/
void NameValueItem::SetReadOnly(bool enable)
{
	fReadOnly = enable;
}


/**
	Sets the value based on a string input.
*/
void NameValueItem::SetValue(const char *value, bool merge)
{
	if (!merge)
		fValue = value;
	else if (fValue.FindFirst(value) < 0) {
		fValue += "; ";
		fValue += value;
		SetValueColor((rgb_color){255,0,0});
	}
}


/** 
    Copies and converts source data.
    The source is copied for string parts and sprintf()'d for those that support conversion.
    Currently 'size' does not matter, only scalars and strings are supported.
	Very useful for feeding in results of a BMessage::FindData() call.

	\return B_OK - Successfully converted or copied
	\returns B_BAD_TYPE - Conversion not supported for this type
	\returns B_BAD_VALUE - Conversion failed
	
	\todo Optimize for size.
*/
status_t NameValueItem::SetValue(type_code type, const void *data, ssize_t size)
{
	fType = type;

	// Fast case for strings and MIME-strings.
	if (type == B_STRING_TYPE || type == 'MIMS') {
		fValue.SetTo((char*)data);
		return B_OK;
	}
	
	// No BString functions can be used until UnlockBuffer()!
	char *s = fValue.LockBuffer(24);
	int n = -1;
	switch (type) {
		case B_DOUBLE_TYPE:
			n = sprintf(s, "%g", *(double*)data);
			break;
		case B_FLOAT_TYPE:
			n = sprintf(s, "%g", *(float*)data);
			break;
		case B_INT32_TYPE:
			n = sprintf(s, "%ld", *(int32*)data);
			break;
		case B_UINT32_TYPE:
			n = sprintf(s, "%lu", *(uint32*)data);
			break;
		case B_INT16_TYPE:
			n = sprintf(s, "%hd", *(int16*)data);
			break;
		case B_UINT16_TYPE:
			n = sprintf(s, "%hu", *(uint16*)data);
			break;
		case B_INT8_TYPE:
			n = sprintf(s, "%hd", *(int8*)data);
			break;
		case B_UINT8_TYPE:
			n = sprintf(s, "%hu", *(uint8*)data);
			break;
		case B_INT64_TYPE:
			n = sprintf(s, "%qd", *(int64*)data);
			break;
		case B_UINT64_TYPE:
			n = sprintf(s, "%qu", *(uint64*)data);
			break;
		case B_CHAR_TYPE:
			n = sprintf(s, "%c", *(char*)data);
			break;
		case B_BOOL_TYPE:
			n = sprintf(s, "%s", *(bool*)data ? "true" : "false");
			break;
		case B_TIME_TYPE: {
   			struct tm *ti = localtime ((time_t*)data);
			strftime(s, 24, TIME_FORMAT, ti);
			break;
		}
	}
	fValue.UnlockBuffer();
	return n ? B_OK : B_BAD_VALUE;
}


/**
	Sets the color in which to display the value part.
*/
void NameValueItem::SetValueColor(rgb_color color)
{
	fValueColor = color;
}




