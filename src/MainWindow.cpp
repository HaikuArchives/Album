#define DEBUG 1
#include <Debug.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <MenuBar.h>
#include <TextControl.h>
#include <Node.h>
#include <Path.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <TranslatorFormats.h>
#include <NodeMonitor.h>
#include <Clipboard.h>
#include <Roster.h>
#include "MainWindow.h"
#include "SplitView.h"
#include "FileAttrDialog.h"
#include "App.h"
#include "JpegTagExtractor.h"

#ifndef __HAIKU__
#define B_SYSTEM_TEMP_DIRECTORY B_COMMON_TEMP_DIRECTORY
#endif



MainWindow::MainWindow(BRect frame, const char *title): 
	BWindow(frame, title, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_WILL_ACCEPT_FIRST_CLICK | B_ASYNCHRONOUS_CONTROLS),
	fThumbFormat(B_GIF_FORMAT),
	fWriteAttr("IPRO:thumbnail"),
	fThumbWidth(64),
	fThumbHeight(64)
{

    fMenuBar = new BMenuBar(BRect(), "MenuBar");
    AddChild(fMenuBar);
    
	// File Menu
    BMenu *menuFile = new BMenu("File");
    fMenuBar->AddItem(menuFile);
    BMenuItem* itemAbout = new BMenuItem(_("About")B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED));
    itemAbout->SetTarget(be_app);
    menuFile->AddItem(itemAbout);
    menuFile->AddSeparatorItem();
    menuFile->AddItem(fMoveToTrash = new BMenuItem(_("Move to Trash"), new BMessage(CMD_ITEM_TRASH), 'T'));
    menuFile->AddSeparatorItem();
    BMenuItem *itemQuit = new BMenuItem(_("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	itemQuit->SetTarget(be_app);
    menuFile->AddItem(itemQuit);
    
    // Edit Menu
    BMenu *editMenu = new BMenu(_("Edit"));
    fMenuBar->AddItem(editMenu);
    editMenu->AddItem(new BMenuItem(_("Select All"), new BMessage(B_SELECT_ALL), 'A'));
    editMenu->AddItem(fEditMark = new BMenuItem(_("Mark"), new BMessage(CMD_ITEM_MARK), 'M'));
    editMenu->AddItem(fEditUnmark = new BMenuItem(_("Unmark"), new BMessage(CMD_ITEM_UNMARK), 'U'));
    editMenu->AddSeparatorItem();
    editMenu->AddItem(fRemoveFromView=new BMenuItem(_("Remove from View"), new BMessage(CMD_ITEM_REMOVE)));
    editMenu->AddSeparatorItem();
    BMenuItem *fShowPrefs = new BMenuItem(_("Preferences")B_UTF8_ELLIPSIS, new BMessage(CMD_SHOW_PREFS));
    fShowPrefs->SetTarget(be_app);
    editMenu->AddItem(fShowPrefs);
    
    // View Menu
    BMenu *viewMenu = new BMenu(_("View"));
    fMenuBar->AddItem(viewMenu);
	
	// View Sort Submenu
    BMenu *menuSort = new BMenu(_("Order By"));
    viewMenu->AddItem(menuSort);
    menuSort->SetRadioMode(true);
    menuSort->AddItem(new BMenuItem(_("None"), new BMessage(CMD_SORT_NONE)));
    menuSort->AddItem(new BMenuItem(_("File Name"), new BMessage(CMD_SORT_NAME)));
    menuSort->AddItem(new BMenuItem(_("File Size"), new BMessage(CMD_SORT_SIZE)));
    menuSort->AddItem(new BMenuItem(_("First Created"), new BMessage(CMD_SORT_CTIME)));
    menuSort->AddItem(new BMenuItem(_("Last Modified"), new BMessage(CMD_SORT_MTIME)));
    menuSort->AddItem(new BMenuItem(_("Folder"), new BMessage(CMD_SORT_DIR)));
	menuSort->ItemAt(0)->SetMarked(true);
	
	// View Columns Submenu
    BMenu *menuColumns = new BMenu(_("Columns"));
    viewMenu->AddItem(menuColumns);
    menuColumns->SetRadioMode(true);
    menuColumns->AddItem(new BMenuItem(_("Auto"), new BMessage(CMD_COL_0)));
    menuColumns->AddItem(new BMenuItem("5", new BMessage(CMD_COL_5)));
    menuColumns->AddItem(new BMenuItem("10", new BMessage(CMD_COL_10)));
	menuColumns->ItemAt(0)->SetMarked(true);
	
	// View Labels Submenu
    BMenu *menuShow = new BMenu(_("Labels"));
    viewMenu->AddItem(menuShow);
    menuShow->AddItem(fShowName = new BMenuItem(_("File Name"), new BMessage(CMD_LABEL_NAME)));
    menuShow->AddItem(fShowSize = new BMenuItem(_("File Size"), new BMessage(CMD_LABEL_SIZE)));
    menuShow->AddItem(fShowDimension = new BMenuItem(_("Dimensions"), new BMessage(CMD_LABEL_DIM)));
    menuShow->AddItem(fShowMTime = new BMenuItem(_("Modification time"), new BMessage(CMD_LABEL_MTIME)));
	fShowName->SetMarked(true);
	fShowDimension->SetMarked(true);
    
	// Other view options
    viewMenu->AddSeparatorItem();
    viewMenu->AddItem(fOnlyMarked = new BMenuItem(_("Only Marked"), new BMessage(CMD_VIEW_MARKED)));
    viewMenu->AddItem(fViewTrash = new BMenuItem(_("Include Trash"), new BMessage(CMD_VIEW_TRASH)));
    
    // Edit Menu
    fAttrMenu = new BMenu(_("Attributes"));
    fMenuBar->AddItem(fAttrMenu);
    fAttrMenu->AddItem(new BMenuItem(_("Copy Tags"), new BMessage(CMD_TAG_COPY)));
    fAttrMenu->AddItem(new BMenuItem(_("Write Tracker Icons"), new BMessage(CMD_ATTR_ICONS)));
    fAttrMenu->AddItem(new BMenuItem(_("Write Thumbnails"), new BMessage(CMD_ATTR_THUMBS)));
    fAttrMenu->AddItem(new BMenuItem(_("Rebuild Thumbnails"), new BMessage(CMD_ATTR_THUMBS_REBUILD)));
	fAttrMenu->AddSeparatorItem();
    fAttrMenu->AddItem(new BMenuItem(_("Delete Selected"), new BMessage(CMD_ATTR_REMOVE)));
     

	// Toolbar (top)
	BRect r = Bounds();
	r.top = fMenuBar->Frame().bottom + 1;
	r.bottom = r.top + 40;
	fToolbar = new MainToolbar(r, B_FOLLOW_LEFT_RIGHT);
	AddChild(fToolbar);

	// Container
	r = Bounds();
	r.top = fToolbar->Frame().bottom + 1;
	r.right +=1;
	r.bottom +=1;
	SplitView *view = new SplitView(r, NULL, B_HORIZONTAL, B_FOLLOW_ALL, B_PULSE_NEEDED | B_FRAME_EVENTS);

	// Image Browser (right)
	fBrowser = new MainView(BRect(0,0,r.Width()*0.75,r.Height()), new BMessage(MSG_ITEM_SELECTED), B_FOLLOW_ALL_SIDES);
	fBrowser->SetMask(ITEM_FLAG_DIMMED);
	// Sidebar (left)
	fSidebar = new MainSidebar(BRect(0,0,r.Width()*0.25,r.Height()), fBrowser, B_FOLLOW_TOP_BOTTOM);
	view->AddChild(fSidebar);
	view->AddChild(new BScrollView(NULL, fBrowser, B_FOLLOW_ALL, 0, true, true, B_PLAIN_BORDER));
	AddChild(view);
	
	fOnlyMarked->SetMarked(fBrowser->Mask() & ITEM_FLAG_MARKED);
	fViewTrash->SetMarked(fBrowser->Mask() & ITEM_FLAG_DIMMED);
	

	// Start the loader thread.
	fLoader = new ImageLoader("ImageLoader");
	fLoader->SetThumbnailSize(fThumbWidth, fThumbHeight);
	fLoader->Run();
	fLoader->StartWatchingAll(fBrowser);

	// Create a tempory repository for negotiated drops from image editors etc.
	BPath path;
	if (find_directory(B_SYSTEM_TEMP_DIRECTORY, &path) == B_OK) {
		path.Append("Album");
		if (create_directory(path.Path(), 0x777) == B_OK) {
			fRepository.SetTo(path.Path());
			node_ref nref;
			fRepository.GetNodeRef(&nref);
			// this should display newly created files
			watch_node(&nref, B_WATCH_DIRECTORY, fLoader,fLoader);
		}
	}

	ItemSelected(NULL);
	UpdateItemFlags();	
	
	// Animation effects depend on this: 
	SetPulseRate(75000);	
	
}



MainWindow::~MainWindow()
{
	fLoader->Stop();
	if (fLoader->Lock() == B_OK)
		fLoader->Quit();
}



bool MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}




void MainWindow::MessageReceived(BMessage *message)
{
 	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
			NoticeReceived(message);
			break;
		case B_SIMPLE_DATA:
			DataReceived(message);
			break;
		case B_MIME_DATA:
			message->PrintToStream();
			break;
		case B_SELECT_ALL:
			fBrowser->Select(0, fBrowser->CountItems());
			ItemSelected(NULL);
			break;
		case B_PASTE:
			PasteFromClipboard();
			break;			
		case B_COPY:
			CopyToClipboard();
			break;			
		case B_CUT:
			CopyToClipboard();
			DeleteSelection();
			break;
		case MSG_PREFS_CHANGED:
			PrefsReceived(message);
			break;
		case MSG_LOADER_UPDATE:
			UpdateReceived(message);
			break;
		case MSG_TOOLBAR_ZOOM: {
			int32 value;
			message->FindInt32("be:value", &value);
			fBrowser->SetZoom(value/20.0);
			break;
		}
		case MSG_TOOLBAR_STOP:
			fLoader->Stop();
			break;
		case MSG_ITEM_SELECTED:
			ItemSelected(message);
			break;
		case CMD_ITEM_REMOVE:
			DeleteSelection();
			break;
		case CMD_ITEM_TRASH:
			MoveToTrash();
			break;			
		case CMD_ITEM_MARK:
			MarkSelection(true);
			break;
		case CMD_ITEM_UNMARK:
			MarkSelection(false);
			break;
		case MSG_NAME_CHANGED:
			ItemRenamed(message);
			break;
		case MSG_ATTR_CHANGED:
			AttributeChanged(message);
			break;
		case MSG_TAG_SELECTED:
			AttributeSelected(message);
			break;
		case CMD_SORT_NONE:
			fBrowser->SetOrderBy(AlbumFileItem::CmpSerial);
			fBrowser->Arrange();
			break;
		case CMD_SORT_NAME:
			fBrowser->SetOrderBy(AlbumFileItem::CmpRef);
			fBrowser->Arrange();
			break;
		case CMD_SORT_SIZE:
			fBrowser->SetOrderBy(AlbumFileItem::CmpSize);
			fBrowser->Arrange();
			break;
		case CMD_SORT_CTIME:
			fBrowser->SetOrderBy(AlbumFileItem::CmpCTime);
			fBrowser->Arrange();
			break;
		case CMD_SORT_MTIME:
			fBrowser->SetOrderBy(AlbumFileItem::CmpMTime);
			fBrowser->Arrange();
			break;
		case CMD_SORT_DIR:
			fBrowser->SetOrderBy(AlbumFileItem::CmpDir);
			fBrowser->Arrange();
			break;
		case CMD_COL_0:
			fBrowser->SetColumns(0);
			break;
		case CMD_COL_5:
			fBrowser->SetColumns(5);
			break;
		case CMD_COL_10:
			fBrowser->SetColumns(10);
			break;
		case CMD_VIEW_MARKED:
			fBrowser->SetMask(fBrowser->Mask() ^ ITEM_FLAG_MARKED);
			fOnlyMarked->SetMarked(fBrowser->Mask() & ITEM_FLAG_MARKED);
			ItemSelected(NULL);
			break;
		case CMD_VIEW_TRASH:
			fBrowser->SetMask(fBrowser->Mask() ^ ITEM_FLAG_DIMMED);
			fViewTrash->SetMarked(fBrowser->Mask() & ITEM_FLAG_DIMMED);
			ItemSelected(NULL);
			break;
		case CMD_LABEL_NAME:
			fShowName->SetMarked(!fShowName->IsMarked());
			UpdateItemFlags();
			break;
		case CMD_LABEL_SIZE:
			fShowSize->SetMarked(!fShowSize->IsMarked());
			UpdateItemFlags();
			break;
		case CMD_LABEL_DIM:
			fShowDimension->SetMarked(!fShowDimension->IsMarked());
			UpdateItemFlags();
			break;
		case CMD_LABEL_MTIME:
			fShowMTime->SetMarked(!fShowMTime->IsMarked());
			UpdateItemFlags();
			break;
		case CMD_TAG_COPY:
			CopySelectedTags();
			break;
		case CMD_ATTR_REMOVE:
			DeleteSelectedAttributes();
			break;
		case CMD_ATTR_THUMBS_REBUILD:
			WriteThumbnails(0);
			break;
		case CMD_ATTR_THUMBS:
			WriteThumbnails(1);
			break;
		case CMD_ATTR_ICONS:
			WriteThumbnails(2);
   		default:
   			BWindow::MessageReceived(message);
   			break;
	}
}




/**
	User settings changed.
*/
void MainWindow::PrefsReceived(BMessage *message)
{
	// Thumbnail Attribute Name
	const char *s;
	if (message->FindString("thumb_attr", &s) == B_OK) 
		fLoader->SetAttrNames(s);
	
	if (message->FindFloat("thumb_width", &fThumbWidth) == B_OK &&
		message->FindFloat("thumb_height", &fThumbHeight) == B_OK)
			fLoader->SetThumbnailSize(fThumbWidth,fThumbHeight);
			
	BString format;
	if (message->FindString("thumb_format", &format) == B_OK) {
		if (format == "GIF")
			fThumbFormat = B_GIF_FORMAT;
		else if (format == "JPEG")
			fThumbFormat = B_JPEG_FORMAT;
		else if (format == "PNG")
			fThumbFormat = B_PNG_FORMAT;
		else if (format == "PPM")
			fThumbFormat = B_PPM_FORMAT;
		else if (format == "TGA")
			fThumbFormat = B_TGA_FORMAT;
		else if (format == "BMP")
			fThumbFormat = B_BMP_FORMAT;
		else if (format == "TIFF")
			fThumbFormat = B_TIFF_FORMAT;
	}

	if (message->FindString("thumb_attr", &s) == B_OK) {
		sscanf(s, "%30s", fWriteAttr.LockBuffer(30));
		fWriteAttr.UnlockBuffer();
	}

	int32 options;
	if (message->FindInt32("load_options", &options) == B_OK) 
		fLoader->SetLoadOptions(options);
	
	if (message->FindInt32("display_options", &options) == B_OK) {
		fBrowser->SetBuffering(options & 1);
	}
		
}




/**
	Replies to drag&drop messages.
	First tries the negotiated DnD protocol (Tracker-like) and then the simple one (plain refs).
*/
void MainWindow::DataReceived(BMessage *message)
{
	// try to decode negotiated dragndrop
	int32 action, i = 0;
	while (message->FindInt32("be:actions", i++, &action) == B_OK) {
		if (action == B_COPY_TARGET) {
			// reply to action initiator
			BMessage reply(action);
			BString filetype;
			// some apps are more liberal about the presence of these:
			if (message->FindString("be:types", &filetype) == B_OK) 
				reply.AddString("be:types", filetype.String());
			if (message->FindString("be:filetypes", &filetype) == B_OK) 
				reply.AddString("be:filetypes", filetype.String());
			// the directory we would like to see files in
			BPath path(&fRepository, NULL);
			entry_ref ref;
			get_ref_for_path(path.Path(), &ref);
			reply.AddRef("directory", &ref);

			// the desired file name
			BString clipname;
			message->PrintToStream();
			if (message->FindString("be:clip_name", &clipname) == B_OK) {
				path.Append(clipname.String());
				// the initiator probably needs a non-existing entry for new files...
				BEntry entry(path.Path());
				if (entry.Exists()) {
					// make something up
					char s[20];
					time_t curtime = time (NULL);
					strftime(s, sizeof(s)," %y-%m-%d %H:%M:%S", localtime(&curtime));
					clipname += s;
				}
				reply.AddString("name", clipname.String());
			}
			message->SendReply(&reply);
			return;
		}
	}

	// Try simple dragndrop instead;
	RefsReceived(message);
}




/**
	One or more files dragged into into the view.
*/
void MainWindow::RefsReceived(BMessage *message)

{
	// Check if files were dropped into their own window.
	BMessenger msgr;
	if (message->FindMessenger("messenger", &msgr) == B_OK)
		if (msgr == BMessenger(fBrowser))
			return;


	BMessage msg(B_SIMPLE_DATA);
	entry_ref ref;
	for (int i = 0; message->FindRef("refs", i, &ref) == B_OK; i++)
		msg.AddRef("refs", &ref);
	fLoader->PostMessage(&msg, NULL, this);
}




/**
	Handles B_OBSERVER_NOTICE_CHANGE.
*/
void MainWindow::NoticeReceived(BMessage *message)
{
	int32 change = 0;
	message->FindInt32("be:observe_change_what", &change);
	switch (change) {
		case MSG_LOADER_UPDATE:
			UpdateReceived(message);
			break;
		case MSG_LOADER_DONE:
			fToolbar->UpdateProgress(1,0);
			break;
		case MSG_LOADER_DONE_BUT_RUNNING:
			fToolbar->UpdateProgress(1,0, _("Querying"));
			break;
		case MSG_LOADER_DELETED:
			DeleteReceived(message);
			break;
	}
}




/**
	File info received.
*/
void MainWindow::UpdateReceived(BMessage *message)
{
	BBitmap *bitmap = NULL;
	message->FindPointer("bitmap", (void**)&bitmap);

	entry_ref ref;
	if (message->FindRef("ref", &ref) != B_OK) {
		// Like.. what?
		PRINT(("Invalid item.\n"));
		return;
	}

	bool redraw = false;
	uint32 changes = 0;

	AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(fBrowser->EachItem(AlbumFileItem::EqRef, &ref));	
	if (item) {
		// Update an existing item
		if (message->FindRef("newref", &ref) == B_OK) {
			item->SetRef(ref);
			// name changed
			redraw = true;
			changes |= UPDATE_STATS;
		}
		if (bitmap) {
			// invalidate the current rect
			fBrowser->InvalidateItem(item);
			// and change it...
			item->SetBitmap(bitmap);
			item->SetHighlight(1.0);
			redraw = true;
		}
	}
	else {	
		// create a new item with an impossible frame
		item = new AlbumFileItem(BRect(-1,-1,0,0), bitmap);
		item->SetRef(ref);
		item->SetHighlight(1.0);
		if (fBrowser->AddItem(item)) {
			redraw = true;
			// delete the splash message
			if (fBrowser->CountItems() == 1)
				fBrowser->Invalidate();			
		}
		changes |= UPDATE_STATS;		
	}
	
	

	// File Stats
	off_t fsize;
	if (message->FindInt64("fsize", &fsize) == B_OK) {
		if (fsize != item->fFSize)
			changes |= UPDATE_STATS;
		item->fFSize = fsize;	
	}

	ssize_t size;
	const time_t *ctime;
	if (message->FindData("ctime", B_TIME_TYPE, (const void**)&ctime, &size) == B_OK) {
		if (fsize != item->fCTime)
			changes |= UPDATE_STATS;
		item->fCTime = *ctime;
	}

	const time_t *mtime;
	if (message->FindData("mtime", B_TIME_TYPE, (const void**)&mtime, &size) == B_OK) {
		if (fsize != item->fMTime)
			changes |= UPDATE_STATS;
		item->fMTime = *mtime;
	}

	// JPEG Tags
	BMessage metadata;
	if (message->FindMessage("tags", &metadata) == B_OK) {
		item->fTags = metadata;
		metadata.FindInt16("Width", &item->fImgWidth);
		metadata.FindInt16("Height", &item->fImgHeight);
		changes |= UPDATE_TAGS;	
	}

	// BFS Attributes
	if (message->FindMessage("attributes", &metadata) == B_OK) {
		item->fAttributes = metadata;
		// counting on non-BFS volume not getting this part at all...
		bool marked = false;
		uint32 oldflags = item->Flags();
		item->SetFlags(ITEM_FLAG_MARKED, (metadata.FindBool("Marked", &marked) == B_OK) && marked);
		redraw = oldflags != item->Flags();
		changes |= UPDATE_ATTRS;	
	}

	// Other features	
	int32 flags = 0;	
	if (message->FindInt32("flags", &flags) == B_OK) {
		item->SetFlags(ITEM_FLAG_EXIF, flags & HAS_EXIF);
		item->SetFlags(ITEM_FLAG_IPTC, flags & HAS_IPTC);	
		redraw = true;	
	}
	
	// Update display
	BRect r = item->Frame();
	item->SetFlags((item->Flags() & 0xffff) | fLabelMask);
	item->Update(fBrowser);
	if (r != item->Frame() || (changes & UPDATE_STATS)) {
		// reflow necessary
		fBrowser->SortItems();
		fBrowser->Arrange();
	}

	if(redraw) {
		fBrowser->InvalidateItem(item);
	}
	
	if (item->IsSelected())
		fSidebar->Update(changes);

	// Progress Bar
	int32 total = 0, done = 0;
	if (message->FindInt32("total", &total) == B_OK) {
		message->FindInt32("done", &done);
		fToolbar->UpdateProgress(total, done);
	}
	
	fToolbar->SetCounter(fBrowser->CountItems());

}





/**
	A file disappeared from the volume.
*/
void MainWindow::DeleteReceived(BMessage *message)
{
	entry_ref ref;
	message->FindRef("ref", &ref);
	AlbumItem *item = fBrowser->EachItem(AlbumFileItem::EqRef, &ref);
	if (item) {
		bool selected = item->IsSelected();
		fBrowser->InvalidateItem(item);
		delete fBrowser->RemoveItem(fBrowser->IndexOf(item));
		fBrowser->Arrange();
		if (selected)
			fSidebar->Update();
	}
	fToolbar->SetCounter(fBrowser->CountItems());
	fToolbar->SetSelected(fBrowser->CountSelected());
}





/**
	An item was selected/deselected.
*/
void MainWindow::ItemSelected(BMessage *message)
{
	int n = fBrowser->CountSelected();
	fMoveToTrash->SetEnabled(n > 0);
	fRemoveFromView->SetEnabled(n > 0);
	fEditMark->SetEnabled(n > 0);
	fEditUnmark->SetEnabled(n > 0);
	for (int i=0;i<fAttrMenu->CountItems();i++)
		fAttrMenu->ItemAt(i)->SetEnabled(n>0);
	fToolbar->SetAttrSelected(false);
	fToolbar->SetSelected(n);
	fSidebar->Update();
	
	if (message && n == 1) {
		AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(fBrowser->SelectedItem(0));
		// Launch preferred app.
		bool launch = false;		
		if (item && message->FindBool("launch",&launch) == B_OK && launch) 
			be_roster->Launch(&item->Ref());			
	}
}







/**
	File name edit box changed.
*/
void MainWindow::ItemRenamed(BMessage *message)
{
	BTextControl *source;
	if (message->FindPointer("source", (void**)&source) == B_OK) {
		BMessage msg(CMD_OP_RENAME);
		msg.AddString("name", source->Text());	
		int32 n = fBrowser->GetSelectedRefs(&msg);
		// Progress bar state
		msg.AddFloat("total", n);
		msg.AddFloat("delta", 1);	
		
		BRect r(0,0,280,60);
		CENTER_IN_FRAME(r,Frame());
		FileAttrDialog::Execute(r, &msg);
	}
}




/**
	An attribute was selected/deselected.
*/
void MainWindow::AttributeSelected(BMessage *message)
{
	int32 index;
	bool selected = message->FindInt32("index",&index) == B_OK;
	fToolbar->SetAttrSelected(selected);
}




/**
	Writes a single attribute, received from the Sidebar.
*/
void MainWindow::AttributeChanged(BMessage *message)
{
	BMessage msg(CMD_OP_ATTR_WRITE);
	fBrowser->GetSelectedRefs(&msg);

	const char *name;
	if (message->FindString("name", &name) == B_OK) {
		type_code type;
		if (message->GetInfo("value", &type) == B_OK) {
			BMessage attrs;
			const void *data;
			ssize_t size;
			message->FindData("value", B_ANY_TYPE, &data, &size);
			if (attrs.AddData(name, type, data, size) == B_OK) {
				msg.AddMessage("attributes", &attrs);
				BRect r(0,0,280,60);
				CENTER_IN_FRAME(r,Frame());
				FileAttrDialog::Execute(r, &msg);
			}
		}		
	}
}




/**
	Removes all selected items, leaving their files intact.
*/
void MainWindow::DeleteSelection()
{
	// Purge Node Monitor cached nodes.
	BMessage msg(CMD_LOADER_DELETE);
	fBrowser->GetSelectedRefs(&msg);
	fLoader->PostMessage(&msg, NULL, this);	

	// Reset views
	fBrowser->DeleteSelected();
	fSidebar->Update();
	fToolbar->SetCounter(fBrowser->CountItems());
	fToolbar->SetSelected(0);
}




/**
	Adds or removes 'marked' attribute.
*/
void MainWindow::MarkSelection(bool enabled)
{

	BMessage msg(enabled ? CMD_OP_ATTR_WRITE : CMD_OP_ATTR_REMOVE);
	fBrowser->GetSelectedRefs(&msg);
	BMessage attrs;
	attrs.AddBool("Marked", true);
	msg.AddMessage("attributes", &attrs);
	
	BRect r(0,0,280,60);
	CENTER_IN_FRAME(r,Frame());
	FileAttrDialog::Execute(r, &msg);
	
	// Update display (changes visibility, do this last)
	AlbumItem *item;
	for (int i = 0; (item = fBrowser->ItemAt(i)); i++) {
		if (item->IsSelected() && fBrowser->IsItemVisible(item)) {
			item->SetFlags(ITEM_FLAG_MARKED, enabled);
			fBrowser->InvalidateItem(item);
		}
	}
	ItemSelected(NULL);
	
}





void MainWindow::CopySelectedTags()
{
	// get the selected names
	BMessage tags;
	fSidebar->GetSelectedTags(&tags);
	AlbumFileItem *item;
	BNode node;
	for (int i = 0; (item = (AlbumFileItem*)fBrowser->ItemAt(i)); i++) {
		// targeted node
		if (!item->IsSelected() || node.SetTo(&item->Ref()) != B_OK)
			continue;
		char *name;
		type_code type;
		for (int i = 0; tags.GetInfo(B_ANY_TYPE, i, &name, &type) == B_OK; i++) {
			const void *data;
			ssize_t size;
			if (item->fTags.FindData(name, type, &data, &size) == B_OK)
				node.WriteAttr(name, type, 0, data, size);
		}
	}
}





void MainWindow::DeleteSelectedAttributes()
{
	BMessage msg(CMD_OP_ATTR_REMOVE);
	int32 n = fBrowser->GetSelectedRefs(&msg);
	// Progress bar state
	msg.AddFloat("total", n);
	msg.AddFloat("delta", 1);	
		
	BMessage attrs;
	fSidebar->GetSelectedAttrs(&attrs);
	msg.AddMessage("attributes",&attrs);

	BRect r(0,0,280,60);
	CENTER_IN_FRAME(r,Frame());
	FileAttrDialog::Execute(r, &msg);
}





/**
	Make Tracker icons or thumbnail attributes.
	
	mode 0 - thumbnail attributes, reload images
	mode 1 - thumbnail attributes, current bitmaps
	mode 2 - Tracker icons
*/
void MainWindow::WriteThumbnails(int mode)
{
	
	BRect r(0,0,280,60);
	CENTER_IN_FRAME(r,Frame());
	fFileOpDlg = FileAttrDialog::Execute(r, NULL);
/*
	Items can be processed with a 'refs' array but that would totally 
	hog the processing window's message loop, so we break it up.
*/
	int32 n = fBrowser->CountSelected();
	int32 d = 0;
	AlbumFileItem *item;
	for (int i = 0; (item = dynamic_cast<AlbumFileItem*>(fBrowser->ItemAt(i))); i++) {
		if (!item->IsSelected() || !fBrowser->IsItemVisible(item))
			continue;
		PRINT(("%d %d\n",n,i));
		BMessage msg(mode == 2 ? CMD_OP_ICONS : CMD_OP_THUMBS);
		msg.AddString("thumb_atrr", fWriteAttr.String());
		msg.AddInt32("thumb_format", fThumbFormat);
		msg.AddFloat("thumb_width", fThumbWidth);
		msg.AddFloat("thumb_height", fThumbHeight);	
		msg.AddRef("refs", &item->Ref());
		// don't reload for icons
		if (mode > 0)
			msg.AddPointer("bitmap", item->Bitmap());
		// Progress bar state
		msg.AddFloat("total", n);
		msg.AddFloat("delta", ++d);	
		// offload to another thread...	
		fFileOpDlg->PostMessage(&msg, NULL, this);
	}
}





/**
	Moves selected entries to the Trash folder.
*/
void MainWindow::MoveToTrash()
{
	BMessage msg(CMD_OP_TRASH);
	int32 n = fBrowser->GetSelectedRefs(&msg);
	// Progress bar state
	msg.AddFloat("total", n);
	msg.AddFloat("delta", 1);	
	
	BRect r(0,0,240,60);
	CENTER_IN_FRAME(r,Frame());
	FileAttrDialog::Execute(r, &msg);
}





/**
	Changes thumbnail captions.
*/
void MainWindow::UpdateItemFlags()
{
	fLabelMask = 0;
	fLabelMask |= fShowName->IsMarked() ? ITEM_FLAG_LABEL0 : 0;
	fLabelMask |= fShowSize->IsMarked() ? ITEM_FLAG_LABEL1 : 0;
	fLabelMask |= fShowDimension->IsMarked() ? ITEM_FLAG_LABEL2 : 0;
	fLabelMask |= fShowMTime->IsMarked() ? ITEM_FLAG_LABEL3 : 0;

	AlbumItem *item;
	for (int i = 0; (item = fBrowser->ItemAt(i)); i++) {
		item->SetFlags((item->Flags() & 0xffff) | fLabelMask);
		item->Update(fBrowser);
	}

	fBrowser->Arrange(false);
	fBrowser->Invalidate();
}


/**
	Copies references to The Clipboard.
	Does not work for Tracker.
	Experimental.
*/
void MainWindow::CopyToClipboard()
{
	if (fBrowser->CountSelected() == 0)
		return;
		
	BMessage* clip = (BMessage *)NULL;
  	if (be_clipboard->Lock()) {
		be_clipboard->Clear();
		if ((clip=be_clipboard->Data()))
		{
			AlbumFileItem *item;
			for (int i=0; (item = dynamic_cast<AlbumFileItem*>(fBrowser->ItemAt(i))); i++) {
				if (item->IsSelected() && fBrowser->IsItemVisible(item)) {
					char s[20];
					sprintf(s,"r_%ld",item->Serial());
					clip->AddRef(s,&item->Ref());
				}
			}
			be_clipboard->Commit();
		}
		be_clipboard->Unlock();
 	}	
}

/**
	Paster sreferences from The Clipboard.
	Can paste Tracker files.
	Experimental.
*/
void MainWindow::PasteFromClipboard()
{
	BMessage* clip = (BMessage *)NULL;
  	if (be_clipboard->Lock()) {
		if ((clip=be_clipboard->Data()))
		{
			// find all refs and add them...
			BMessage msg(B_SIMPLE_DATA);
			entry_ref ref;
			char *name;
			uint32 type;
			int32 count;
			for ( int32 i = 0; clip->GetInfo(B_REF_TYPE, i, &name, &type, &count) == B_OK; i++ ) {			
				if (clip->FindRef(name,&ref) == B_OK)
					msg.AddRef("refs", &ref);
			}
			if (!msg.IsEmpty())
				fLoader->PostMessage(&msg, NULL, this);			
		}
		be_clipboard->Unlock();
 	}	
}

