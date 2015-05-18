#ifndef _MAINVIEW_H_
#define _MAINVIEW_H_

#include "AlbumView.h"
#include <Entry.h>
#include <Locker.h>

enum {
	ITEM_FLAG_MARKED = 	0x0100,
	ITEM_FLAG_DIMMED = 	0x0200,
	ITEM_FLAG_LOCKED = 	0x0400,
	ITEM_FLAG_EXIF = 	0x0800,
	ITEM_FLAG_IPTC = 	0x1000,	
	
	ITEM_FLAG_LABEL0 = 0x00010000,
	ITEM_FLAG_LABEL1 = 0x00020000,
	ITEM_FLAG_LABEL2 = 0x00040000,
	ITEM_FLAG_LABEL3 = 0x00080000,
};


/**
	An AlbumItem with file attributes, EXIF/IPTC indicators etc.
*/
class AlbumFileItem : public AlbumItem {
	public:
	
	BMessage fTags, fAttributes;
	int16 fImgWidth, fImgHeight;
	off_t fFSize;
	time_t fCTime, fMTime;	

	static AlbumItem* EqRef(AlbumItem *item, void *param);
	static int CmpRef(const AlbumItem *a, const AlbumItem *b);
	static int CmpSerial(const AlbumItem *a, const AlbumItem *b);
	static int CmpSize(const AlbumItem *a, const AlbumItem *b);
	static int CmpCTime(const AlbumItem *a, const AlbumItem *b);
	static int CmpMTime(const AlbumItem *a, const AlbumItem *b);
	static int CmpDir(const AlbumItem *a, const AlbumItem *b);

	AlbumFileItem(BRect frame, BBitmap *bitmap);
	virtual void DrawItem(BView *owner);
	virtual uint16 CountLabels();
	virtual void GetLabel(uint16 index, BString *label);
	virtual bool IsLabelVisible(uint16 index);
	void SetRef(entry_ref &ref);
	const entry_ref& Ref() const;	
	const uint32 Serial();
	
	private:


	entry_ref fRef;
	uint32 fSerial;
};


enum {
	CMD_ITEM_LAUNCH = 'iOpn'
};

class MainView : public AlbumView
{
	public:
	
	MainView(BRect frame, BMessage *message, uint32 resizing); 
	virtual void Draw(BRect update);
	virtual void MessageReceived(BMessage *message);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MouseDown(BPoint where);
	virtual void Pulse();
	virtual bool IsItemVisible(AlbumItem *item);
	virtual void SortItems();
	int32 GetSelectedRefs(BMessage *message);
	
	private:
	
	void LaunchItem(BMessage *message);
	void ShowContextMenu(AlbumFileItem* item, BPoint where);
	void ItemDragged(int32 index, BPoint where);

	BString fNoDataMsg;
	BLocker fSelectLock;

};

#endif
