/**
\file EditableListView.h
\brief EditableListView prototypes.

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
*/

#ifndef _LISTVIEWEDITBOX_H_
#define _LISTVIEWEDITBOX_H_

#include <TextView.h>
#include <OutlineListView.h>

/**
	Simple BListView edit box.
*/
class ListViewEditBox : public BTextView
{
	public:

	ListViewEditBox(BRect frame, const char *name, const char *text, uint32 resizingMode = B_FOLLOW_NONE, uint32 flags = B_WILL_DRAW);
	virtual void AttachedToWindow();
	virtual void KeyDown(const char *bytes, int32 numBytes);
};


class EditableListView : public BOutlineListView
{
	public:
	
	EditableListView(BRect frame, const char *name, list_view_type type = B_SINGLE_SELECTION_LIST, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
                 uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	virtual void DetachedFromWindow();
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MakeEmpty();
	virtual void SelectionChanged();
	virtual bool ConfirmEdit(BMessage *message);
	virtual BView *NewEditBox(int32 index);
	void SetEditedItem(int32 index);
	bool IsEditing();
	BView* EditBox();
	
	private:
	
	BView *fEditBox;
};


#endif

