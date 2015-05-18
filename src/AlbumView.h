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

\file AlbumView.cpp
\brief Generic Thumbnail Browser
*/
#ifndef _ALBUMVIEW_H_
#define _ALBUMVIEW_H_

#include <BufferedView.h>
#include <Invoker.h>
#include <ObjectList.h>
#include <RingBuf.h>
#include "AlbumItem.h"


class AlbumView : public BufferedView, public BInvoker
{
	public:
	
	AlbumView(BRect frame, const char *name, BMessage *message, uint32 resizing, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS); 
	virtual void AttachedToWindow();
	virtual void DrawOffscreen(BView *view, BRect update);
	virtual void FrameResized(float width, float height);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MouseDown(BPoint where);
	virtual void MouseUp(BPoint where);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	virtual void SelectionChanged() {};
	virtual bool IsItemVisible(AlbumItem *item);
	virtual void ItemDragged(int32 index, BPoint where) {};
	virtual void Arrange(bool invalidate = true);
	virtual void SortItems();
	
	const BRect& PageBounds() const;
	void SetPageBounds(BRect frame);
	void SetZoom(float scale);
	float Zoom();
	void SetColumns(int16 cols);

	int32 CountItems();
	AlbumItem* ItemAt(int32 index);
	int32 IndexOf(AlbumItem *item);
	int32 IndexOf(BPoint *point);
	AlbumItem* AddItem(AlbumItem *item, int32 index = -1);
	AlbumItem* RemoveItem(int32 index);
	AlbumItem* EachItem(BObjectList<AlbumItem>::EachFunction func, void *param);
    void SetOrderBy(BObjectList<AlbumItem>::CompareFunction func);
	BObjectList<AlbumItem>::CompareFunction OrderByFunc();
	void InvalidateItem(AlbumItem *item);

	AlbumItem* SelectedItem(int32 index);
	int32 DeselectAll();
	int32 Select(int32 index, int32 count = 1, bool enabled = true);
	int32 SelectBlock(int32 from, int32 to, bool enabled = true);
	int32 CountSelected();
	void DeleteSelected();

	void SetMask(uint32 mask);
	const uint32 Mask();
	

	private:

	inline BRect Adjust(BRect rect);
	void UpdateScrollbars(float width, float height);
	
	BObjectList<AlbumItem> fItems;
	BObjectList<AlbumItem>::CompareFunction fOrderBy;
	BRect fPage;
	float fZoom;
	uint32 fMask;
	int16 fColumns;

	protected:
	// TODO: implement getters/setters
	
	int32 fLastSelected;
	BPoint fLastWhere;
	bool fDoubleClick, fMayDrag;
	float fSeparatorHeight;
	float fSmallStep;	
};




#endif
