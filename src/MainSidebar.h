#ifndef _MAINSIDEBAR_H_
#define _MAINSIDEBAR_H_

#include <MainView.h>

class BView;
class BTextControl;
class BOutlineListView;
class BStringView;
class BString;
class NameValueItem;

enum {
	MSG_NAME_CHANGED = 'nmch',
	MSG_ATTR_CHANGED = 'atch',
	MSG_ATTR_DELETE = 'atdl',
	MSG_TAG_DRAGGED = 'tgdg',
	MSG_TAG_SELECTED = 'tgsl'
};

enum {
	UPDATE_FRAME = 1,
	UPDATE_STATS = 2,
	UPDATE_TAGS = 4,
	UPDATE_ATTRS = 8,
	UPDATE_ORDER = 16,
};


class MainSidebar : public BView 
{
	public:
	
	MainSidebar(BRect frame, AlbumView *main, uint32 resizing); 
	virtual void AttachedToWindow();
	virtual void Pulse();
	
	void Clear();
	int ShowSummary();
	void Update(uint32 mask = 0xffff);
	void GetSelectedTags(BMessage *tags);
	void GetSelectedAttrs(BMessage *attrs);


	private:
	bool fUpdateStats, fUpdateTags, fUpdateAttrs;
	AlbumView *fMain;
	BObjectList<NameValueItem> fTags, fAttrs, fGroups;	

	BOutlineListView *fTagList;
	BOutlineListView *fAttrList;
	BTextControl *fFName;
	BStringView *fFSize;
	BStringView *fCTime;
	BStringView *fMTime;
	
	
	
};


#endif	// _MAINSIDEBAR_H_
