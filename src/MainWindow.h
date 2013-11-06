#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <Window.h>
#include "MainView.h"
#include "ImageLoader.h"
#include "MainToolbar.h"
#include "MainSidebar.h"
#include <Directory.h>

class BMenuItem;
class BMenu;

enum {

	MSG_FILE_TRASH = 'ftrs',
	MSG_EDIT_REMOVE = 'ermv',
	MSG_EDIT_UNMARK = 'eumk',
	MSG_VIEW_SORT_NONE = 'vsno',
	MSG_VIEW_SORT_NAME = 'vsnm',
	MSG_VIEW_SORT_CTIME = 'vsct',
	MSG_VIEW_SORT_MTIME = 'vsmt',
	MSG_VIEW_SORT_SIZE = 'vssz',
	MSG_VIEW_SORT_DIR = 'vsdr',
	MSG_VIEW_FLICKER = 'vflc',
	MSG_VIEW_COL_0 = 'vc00',
	MSG_VIEW_COL_5 = 'vc05',
	MSG_VIEW_COL_10 = 'vc10',
	MSG_VIEW_MARKED = 'vmrk',
	MSG_VIEW_TRASH = 'vtrs',
	MSG_VIEW_LABELS = 'vlbl',
	MSG_VIEW_LABEL_NAME = 'vlb0',
	MSG_VIEW_LABEL_SIZE = 'vlb1',
	MSG_VIEW_LABEL_DIM = 'vlb2',
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
	void AttributeSelected(BMessage *message);
	void DeleteSelected();
	void MarkSelected(bool enabled);
	void RenameSelected(BMessage *message);
	void CopySelectedTags();	
	void DeleteSelectedAttributes();
	void AttributeChanged(BMessage *attr);
	void WriteTrackerIcons(BMessage *message);
	void MoveToTrash();
	void SetLabelFlags();
	
	MainView *fBrowser;
	ImageLoader *fLoader;
	MainToolbar *fToolbar;
	MainSidebar *fSidebar;
	BMenuItem *fFlickerFree;
	BMenuItem *fOnlyMarked;
	BMenuItem *fViewTrash;
	BMenuItem *fMoveToTrash;
	BMenuItem *fShowName, *fShowSize, *fShowDimension;
	
	BMenu *fAttrMenu;
	int32 fThumbFormat;
	BString fWriteAttr;
	BDirectory fRepository;
		
};

#endif
