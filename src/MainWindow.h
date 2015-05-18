#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <Window.h>
#include <Directory.h>
#include "MainView.h"
#include "ImageLoader.h"
#include "MainToolbar.h"
#include "MainSidebar.h"

class BMenuBar;
class BMenuItem;
class BDirectory;

#define CENTER_IN_FRAME(r,frame) r.OffsetTo(frame.LeftTop() + BPoint((frame.Width()-r.Width())/2,(frame.Height()-r.Height())/2))

enum {

	CMD_ITEM_TRASH = 'iTrs',
	CMD_ITEM_REMOVE = 'iDel',	
	CMD_ITEM_MARK = 'iMrk',
	CMD_ITEM_UNMARK = 'iUmk',
	CMD_SORT_NONE = 'ord0',
	CMD_SORT_NAME = 'ord1',
	CMD_SORT_CTIME = 'ord2',
	CMD_SORT_MTIME = 'ord3',
	CMD_SORT_SIZE = 'ord4',
	CMD_SORT_DIR = 'ord5',
	CMD_COL_0 = 'vc00',
	CMD_COL_5 = 'vc05',
	CMD_COL_10 = 'vc10',
	CMD_VIEW_MARKED = 'vMrk',
	CMD_VIEW_IPTC = 'vIPT',
	CMD_VIEW_TRASH = 'vTrs',
	CMD_LABEL_NAME = 'lbl0',
	CMD_LABEL_SIZE = '1bl1',
	CMD_LABEL_DIM = 'lbl2',
	CMD_LABEL_MTIME = 'lbl3',
	CMD_TAG_COPY = 'tCpy',
	CMD_ATTR_REMOVE = 'aDel',
	CMD_ATTR_ICONS = 'aIcn',
	CMD_ATTR_THUMBS = 'aThm',
	CMD_ATTR_THUMBS_REBUILD = 'aThr',
	MSG_ITEM_SELECTED = 'iSel',	
	
};




class MainWindow : public BWindow  
{
	public:
	
	MainWindow(BRect frame, const char *title);
	~MainWindow();
	virtual	bool QuitRequested();
	virtual void MessageReceived(BMessage *message);

	private:
	
	void PrefsReceived(BMessage *message);
	void DataReceived(BMessage *message);
	void RefsReceived(BMessage *message);
	void NoticeReceived(BMessage *message);
	void UpdateReceived(BMessage *message);
	void DeleteReceived(BMessage *message);
	void ItemSelected(BMessage *message);
	void ItemRenamed(BMessage *message);
	void AttributeSelected(BMessage *message);
	void AttributeChanged(BMessage *message);

	void DeleteSelection();
	void MarkSelection(bool enabled);
	void CopySelectedTags();	
	void DeleteSelectedAttributes();
	void WriteThumbnails(int mode);
	void MoveToTrash();
	void UpdateItemFlags();
	void CopyToClipboard();
	void PasteFromClipboard();

	BMenuBar *fMenuBar;
	BMenuItem *fFlickerFree;
	BMenuItem *fOnlyMarked;
	BMenuItem *fViewTrash;
	BMenuItem *fMoveToTrash, *fRemoveFromView, *fEditMark, *fEditUnmark;
	BMenuItem *fShowName, *fShowSize, *fShowDimension, *fShowMTime;
	BMenu *fAttrMenu;
	BWindow *fFileOpDlg;	
	
	MainView *fBrowser;
	ImageLoader *fLoader;
	MainToolbar *fToolbar;
	MainSidebar *fSidebar;	
	int32 fThumbFormat;
	BString fWriteAttr;
	BDirectory fRepository;
	float fThumbWidth, fThumbHeight;	
	uint32 fLabelMask;
};

#endif
