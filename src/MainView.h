#ifndef _MAINVIEW_H_
#define _MAINVIEW_H_

#include "AlbumView.h"
#include <Entry.h>

enum {
	ITEM_FLAG_LABEL0 =   0x01000000,
	ITEM_FLAG_LABEL1 =   0x02000000,
	ITEM_FLAG_LABEL2 =   0x04000000,
	ITEM_FLAG_LABEL3 =   0x08000000,
	
};
/**
	An AlbumItem with file attributes, EXIF/IPTC indicators etc.
*/
class AlbumFileItem : public AlbumItem {
	public:
	AlbumFileItem(BRect frame, BBitmap *bitmap);
	virtual void DrawItem(BView *owner, uint32 flags);
	virtual void Update(BView *owner);
	virtual const char* Label(int index = 0);
	virtual int CountLabels();
	void SetRef(entry_ref &ref);
	const entry_ref& Ref() const;

	static AlbumItem* EqRef(AlbumItem *item, void *param);
	static int CmpRef(const AlbumItem *a, const AlbumItem *b);
	static int CmpSerial(const AlbumItem *a, const AlbumItem *b);
	static int CmpSize(const AlbumItem *a, const AlbumItem *b);
	static int CmpCTime(const AlbumItem *a, const AlbumItem *b);
	static int CmpMTime(const AlbumItem *a, const AlbumItem *b);
	static int CmpDir(const AlbumItem *a, const AlbumItem *b);
	static void InitBitmaps();
	
	BMessage fTags, fAttributes;
	off_t fFSize;
	time_t fCTime, fMTime;	

	private:
	entry_ref fRef;
	int fSerial;
	BString fLabel[3];
	static BBitmap *fMarkBmp;
	static BBitmap *fExifBmp;
	static BBitmap *fIptcBmp;
	static BBitmap *fLockBmp;
};

enum {
	MSG_ITEM_SELECTED = 'islc',
	MSG_ITEM_OPEN = 'iopn',
};

class MainView : public AlbumView
{
	public:
	
	MainView(BRect frame, uint32 resizing); 
	void MessageReceived(BMessage *message);
	virtual void Pulse();
	virtual bool IsItemVisible(AlbumItem *item);
	virtual void ItemDragged(int32 index, BPoint where);
	virtual void SortItems();
	void HighlightItem(AlbumItem*, bool enabled = true);
	void GetSelectedRefs(BMessage *message);
	
	private:
	
	void ItemSelected(BMessage *message);
	void ItemInvoked(BMessage *message);
	void ShowContextMenu(AlbumFileItem* item, BPoint where);
	
	RingBuffer<AlbumItem*> fAniItems;
};

#endif
