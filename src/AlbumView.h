#ifndef _ALBUMVIEW_H_
#define _ALBUMVIEW_H_


#include <BufferedView.h>
#include <Invoker.h>
#include "AlbumItem.h"
#include <ObjectList.h>
#include <RingBuf.h>

class BInvoker;

enum {
	ALBUM_ITEM_SELECTED = 'AlbS'
};


class AlbumView : public BufferedView, public BInvoker
{
	public:
	
	AlbumView(BRect frame, const char *name, BMessage *message, uint32 resizing, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS); 
	virtual void AttachedToWindow();
	virtual void DrawOffscreen(BView *view, BRect update);
	virtual void DrawBackground(BView *view, BRect update);
	virtual void FrameResized(float width, float height);
	virtual void MessageReceived(BMessage *message);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MouseDown(BPoint where);
	virtual void MouseUp(BPoint where);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	// custom hooks
	virtual bool IsItemVisible(AlbumItem *item);
	virtual void ItemDragged(int32 index, BPoint where);
	virtual void Arrange(bool invalidate = true);
	virtual void SortItems();
	
	const BRect& PageBounds() const;
	void SetPageBounds(BRect frame);
	void SetZoom(float scale);
	float Zoom();
	void SetColumns(int cols);
	int32 CountItems();
	AlbumItem* ItemAt(int32 index);
	int32 IndexOf(AlbumItem *item);
	AlbumItem* AddItem(AlbumItem *item, int32 index = -1);
	AlbumItem* RemoveItem(int32 index);
	AlbumItem* EachItem(BObjectList<AlbumItem>::EachFunction func, void *param);
    void SetOrderBy(BObjectList<AlbumItem>::CompareFunction func);
	BObjectList<AlbumItem>::CompareFunction OrderByFunc();
	void InvalidateItem(AlbumItem *item);
	void DeselectAll();
	void Select(int32 index, int32 count = 1, bool enabled = true);
	int32 CountSelected();
	AlbumItem* GetSelection(int32 index);
	void DeleteSelected();
	void SetMask(uint32 mask);
	uint32 Mask();
	

	private:

	inline BRect Adjust(BRect rect);
	void UpdateScrollbars(float width, float height);
	
	BRect fPage;
	BObjectList<AlbumItem> fItems, fSelection;
	BObjectList<AlbumItem>::CompareFunction fOrderBy;
	float fZoom;
	int fLastSelected;
	int fColumns;
	bool fDoubleClick, fMayDrag;
	uint32 fMask;
	BPoint fLastWhere;
	BString fNoDataMsg;
	
};




#endif
