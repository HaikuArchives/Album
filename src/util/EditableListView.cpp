/**
\file EditableListView.cpp 
\brief EditableListView implementation and helpers.

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

#include <Debug.h>
#include <EditableListView.h>


ListViewEditBox::ListViewEditBox(BRect frame, const char *name, const char *text, uint32 resizingMode, uint32 flags):
	BTextView(frame, name, frame.OffsetToSelf(0,0), resizingMode, flags | B_WILL_DRAW)
{
	DisallowChar(B_ESCAPE);
	DisallowChar(B_TAB);
	MakeResizable(true);
	SetText(text);
}


/**
	The view is now fully functional.	
*/
void ListViewEditBox::AttachedToWindow()
{
	SelectAll();
	//MakeFocus();
}



	
/**
	Handles keystrokes.
	Relays B_ESCAPE, B_UP_ARROW, B_DOWN_ARROW to its parent.
*/
void ListViewEditBox::KeyDown(const char *bytes, int32 numBytes)
{
    switch (bytes[0]) {
		case B_RETURN:
	    case B_ESCAPE:
	    case B_UP_ARROW:
	    case B_DOWN_ARROW:
	    	if (IsFocus())
	        	Parent()->KeyDown(bytes, numBytes);
	        return;
	    default:
	        BTextView::KeyDown(bytes, numBytes);
    }
}





EditableListView::EditableListView(BRect frame, const char *name, list_view_type type, uint32 resizing, uint32 flags):
	BOutlineListView(frame, name, type, resizing, flags | B_WILL_DRAW),
	fEditBox(NULL)
{
}


void EditableListView::DetachedFromWindow()
{
	SetEditedItem(-1);
}


/**
	Std Kbd Navigation
*/	
void EditableListView::KeyDown(const char *bytes, int32 numBytes)
{
    switch (bytes[0]) {
		case B_RETURN:
			if (fEditBox)  {
				BMessage msg;
				if (Message())
					msg = *Message();
				if (ConfirmEdit(&msg))
					Invoke(&msg);
				SetEditedItem(-1);
			}
			else {
				SetEditedItem(CurrentSelection());
			}
			break;
	    case B_ESCAPE:
			SetEditedItem(-1);
			break;
	    default:
	        BOutlineListView::KeyDown(bytes, numBytes);
    }
}


/**
	Removes all items, cancels edit.
*/
void EditableListView::MakeEmpty()
{
	SetEditedItem(-1);
	BOutlineListView::MakeEmpty();
}




void EditableListView::MakeFocus(bool focused)
{
	if (IsFocus() != focused)
	{
		BOutlineListView::MakeFocus(focused);
		Invalidate();
	}	
}




void EditableListView::SelectionChanged()
{
	SetEditedItem(-1);
}


/**
	Edit box ok'd.
	Edit box value is stored in 'message'
	which is then Invoke()'d, if this function returns true.
	
*/
bool EditableListView::ConfirmEdit(BMessage *message)
{
	BTextView *box = dynamic_cast<BTextView*>(fEditBox);
	if (box) {
		message->AddString("text", box->Text());
		return true;		
	}
	return false;
}

/**
	Creates an edit box.
*/
BView* EditableListView::NewEditBox(int32 index)
{
	return NULL;
}


/**
	Starts/cancels edit.
*/
void EditableListView::SetEditedItem(int32 index)
{
	//fEditBox = FindView("EditBox");
	if (fEditBox) {
		RemoveChild(fEditBox);
		fEditBox->RemoveSelf();
		delete fEditBox;
		fEditBox = NULL;
		// return focus here
		MakeFocus();
	}	
	if (index >= 0) {
		BView *edit = NewEditBox(index);
		if (edit) {
			fEditBox = edit;
			AddChild(fEditBox);
			fEditBox->MakeFocus();
		}
	}
}


bool EditableListView::IsEditing()
{
	return fEditBox;
}

 BView* EditableListView::EditBox()
{
	return fEditBox;
}
