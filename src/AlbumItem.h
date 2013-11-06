#ifndef _AlbumItem_H_
#define _AlbumItem_H_

#include <Bitmap.h>
#include <Message.h>
#include <String.h>

enum {
	ITEM_FLAG_MARKED =   0x00010000,
	ITEM_FLAG_DIMMED =   0x00020000,	
	ITEM_FLAG_HILIT =    0x00040000,	
	ITEM_FLAG_SEPARATOR =0x00080000,	
	ITEM_FLAG_NOLABELS = 0x00100000,	
	ITEM_FLAG_LOCKED = 	 0x00200000,	
};

class AlbumItem 
{
	public:
	
	AlbumItem(BRect frame, BBitmap *bitmap);
	virtual ~AlbumItem();
	
	virtual void DrawItem(BView *owner, uint32 flags);
	virtual void Update(BView *owner);
	virtual void SetFrame(BRect frame);
	const BRect& Frame() const;
	void SetFlags(uint32 flags);
	uint32 Flags();
	void SetBitmap(BBitmap *bitmap);
	const BBitmap* Bitmap() const;
	virtual const char* Label(int index = 0);
	virtual void SetLabel(int index = 0);
	virtual int CountLabels();
	void SetSelected(bool enable);
	bool IsSelected();
	void SetBackColor(rgb_color color);
	rgb_color BackColor();
			
	protected:
	BRect fFrame;
	uint32 fFlags;
	BBitmap *fBitmap;
	int16 fPadding;
	bool fSelected;
	float fTextHeight;
	rgb_color fBackColor, fTextColor;
};


#endif

