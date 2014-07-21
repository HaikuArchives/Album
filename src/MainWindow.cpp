#include <Debug.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include "App.h"
#include "MainWindow.h"
#include "SplitView.h"
#include <TextControl.h>
#include "FileNameDialog.h"
#include "FileAttrDialog.h"
#include <Node.h>
#include <TranslatorFormats.h>
#include <FindDirectory.h>
#include <Path.h>
#include <Entry.h>
#include <NodeMonitor.h>




MainWindow::MainWindow(BRect frame, const char *title): 
	BWindow(frame, title, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS),
	fThumbFormat(B_GIF_FORMAT),
	fWriteAttr("IPRO:thumbnail")
{
    BMenuBar *menuBar = new BMenuBar(BRect(), "MenuBar");
    AddChild(menuBar);
    
	// File Menu
    BMenu *menuFile = new BMenu("File");
    menuBar->AddItem(menuFile);
    BMenuItem* itemAbout = new BMenuItem(_("About")B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED));
    itemAbout->SetTarget(be_app);
    menuFile->AddItem(itemAbout);
    menuFile->AddSeparatorItem();
    fMoveToTrash = new BMenuItem(_("Move to Trash"), new BMessage(MSG_FILE_TRASH), 'T');
    fMoveToTrash->SetEnabled(false);
    menuFile->AddItem(fMoveToTrash);
    menuFile->AddSeparatorItem();
    BMenuItem *itemQuit = new BMenuItem(_("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	itemQuit->SetTarget(be_app);
    menuFile->AddItem(itemQuit);
    // Edit Menu
    BMenu *editMenu = new BMenu(_("Edit"));
    menuBar->AddItem(editMenu);
    editMenu->AddItem(new BMenuItem(_("Select All"), new BMessage(B_SELECT_ALL), 'A'));
    editMenu->AddItem(new BMenuItem(_("Mark"), new BMessage(MSG_TOOLBAR_MARK), 'M'));
    editMenu->AddItem(new BMenuItem(_("Unmark"), new BMessage(MSG_EDIT_UNMARK), 'U'));
    editMenu->AddSeparatorItem();
    editMenu->AddItem(new BMenuItem(_("Remove Selected"), new BMessage(MSG_TOOLBAR_REMOVE)));
    editMenu->AddSeparatorItem();
    BMenuItem *fShowPrefs = new BMenuItem(_("Preferences")B_UTF8_ELLIPSIS, new BMessage(CMD_SHOW_PREFS));
    fShowPrefs->SetTarget(be_app);
    editMenu->AddItem(fShowPrefs);
    // View Menu
    BMenu *viewMenu = new BMenu(_("View"));
    menuBar->AddItem(viewMenu);
	// View Sort Submenu
    BMenu *menuSort = new BMenu(_("Order By"));
    viewMenu->AddItem(menuSort);
    menuSort->SetRadioMode(true);
    BMenuItem* item = new BMenuItem(_("None"), new BMessage(MSG_VIEW_SORT_NONE));
	item->SetMarked(true);
    menuSort->AddItem(item);
    menuSort->AddItem(new BMenuItem(_("File Name"), new BMessage(MSG_VIEW_SORT_NAME)));
    menuSort->AddItem(new BMenuItem(_("File Size"), new BMessage(MSG_VIEW_SORT_SIZE)));
    menuSort->AddItem(new BMenuItem(_("First Created"), new BMessage(MSG_VIEW_SORT_CTIME)));
    menuSort->AddItem(new BMenuItem(_("Last Modified"), new BMessage(MSG_VIEW_SORT_MTIME)));
    menuSort->AddItem(new BMenuItem(_("Folder"), new BMessage(MSG_VIEW_SORT_DIR)));
	// View Columns Submenu
    BMenu *menuColumns = new BMenu(_("Columns"));
    viewMenu->AddItem(menuColumns);
    menuColumns->SetRadioMode(true);
    item = new BMenuItem(_("Auto"), new BMessage(MSG_VIEW_COL_0));
	item->SetMarked(true);
    menuColumns->AddItem(item);
    menuColumns->AddItem(new BMenuItem("5", new BMessage(MSG_VIEW_COL_5)));
    menuColumns->AddItem(new BMenuItem("10", new BMessage(MSG_VIEW_COL_10)));
	// View Labels Submenu
    BMenu *menuShow = new BMenu(_("Labels"));
    viewMenu->AddItem(menuShow);
	fShowName = new BMenuItem(_("File Name"), new BMessage(MSG_VIEW_LABEL_NAME));
    menuShow->AddItem(fShowName);
	fShowName->SetMarked(true);
	fShowSize = new BMenuItem(_("File Size"), new BMessage(MSG_VIEW_LABEL_SIZE));
    menuShow->AddItem(fShowSize);
	fShowDimension = new BMenuItem(_("Dimensions"), new BMessage(MSG_VIEW_LABEL_DIM));
	fShowDimension->SetMarked(true);
    menuShow->AddItem(fShowDimension);
    
	// Other view options
    viewMenu->AddSeparatorItem();
	fOnlyMarked = new BMenuItem(_("Only Marked"), new BMessage(MSG_VIEW_MARKED));
    viewMenu->AddItem(fOnlyMarked);
    // Edit Menu
    fAttrMenu = new BMenu(_("Attributes"));
    fAttrMenu->SetEnabled(false);
    menuBar->AddItem(fAttrMenu);
    fAttrMenu->AddItem(new BMenuItem(_("Copy Tags"), new BMessage(MSG_TOOLBAR_TAGCOPY)));
    fAttrMenu->AddItem(new BMenuItem(_("Write Tracker Icons"), new BMessage(MSG_FILEATTR_ICONS)));
    fAttrMenu->AddItem(new BMenuItem(_("Write Thumbnails"), new BMessage(MSG_FILEATTR_THUMBS)));
	fAttrMenu->AddSeparatorItem();
    fAttrMenu->AddItem(new BMenuItem(_("Remove Selected"), new BMessage(MSG_ATTR_DELETE)));
     

	// Toolbar (top)
	BRect r = Bounds();
	r.top = menuBar->Frame().bottom + 1;
	r.bottom = r.top + 32;
	fToolbar = new MainToolbar(r, B_FOLLOW_LEFT_RIGHT);
	AddChild(fToolbar);

	// Container
	r = Bounds();
	r.top = fToolbar->Frame().bottom + 1;
	r.right +=1;
	r.bottom +=1;
	SplitView *view = new SplitView(r, "SplitView", B_HORIZONTAL, B_FOLLOW_ALL, B_PULSE_NEEDED | B_FRAME_EVENTS);


	// Image Browser (right)
	fBrowser = new MainView(BRect(0,0,300,400),  B_FOLLOW_ALL_SIDES);
	AlbumFileItem::InitBitmaps();
	fBrowser->SetMask(ITEM_FLAG_DIMMED);
	
	// Sidebar (left)
	fSidebar = new MainSidebar(BRect(0,0,200,400), fBrowser, B_FOLLOW_TOP_BOTTOM);
	view->AddChild(fSidebar);
	// right
	view->AddChild(new BScrollView("Scroller", fBrowser, B_FOLLOW_ALL, 0, true, true, B_PLAIN_BORDER));

	
	
	// Auto-arrange children.
	AddChild(view);
	// Animation effects depend on this: 
	SetPulseRate(100000);	
	// Start the loader thread.
	fLoader = new ImageLoader("Loader");
	fLoader->Run();
	fLoader->StartWatchingAll(fBrowser);
	
	// Create a tempory repository for negotiated drops from image ditors etc.
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
	
}


MainWindow::~MainWindow()
{
	//fLoader->StopWatchingAll(this);
	// Try break.
	fLoader->Stop();
	// Blocks until it locks.
	fLoader->Lock();
	fLoader->Quit();
	// BLooper deleted itself.
}

/// The close box was clicked.
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
			fSidebar->Update();
			fToolbar->SetSelected(fBrowser->CountSelected());
			break;
		case MSG_PREFS_CHANGED:
			PrefsReceived(message);
			break;
		case MSG_LOADER_UPDATE:
			UpdateReceived(message);
			break;
		case MSG_TOOLBAR_ZOOM:
			int32 value;
			message->FindInt32("be:value", &value);
			fBrowser->SetZoom(value/20.0);
			break;
		case MSG_TOOLBAR_STOP:
			fLoader->Stop();
			break;
		case MSG_ITEM_SELECTED:
			ItemSelected(message);
			break;
		case MSG_TOOLBAR_REMOVE:
			DeleteSelected();
			break;
		case MSG_TOOLBAR_MARK:
			MarkSelected(true);
			break;
		case MSG_EDIT_UNMARK:
			MarkSelected(false);
			break;
		case MSG_ATTR_DELETE:
			DeleteSelectedAttributes();
			break;
		case MSG_TOOLBAR_TAGCOPY:
		case MSG_TAG_DRAGGED:
			CopySelectedTags();
			break;
		case MSG_NAME_CHANGED:
			RenameSelected(message);
			break;
		case MSG_ATTR_CHANGED:
			AttributeChanged(message);
			break;
		case MSG_TAG_SELECTED:
			AttributeSelected(message);
			break;
		case MSG_FILEATTR_ICONS:
		case MSG_FILEATTR_THUMBS:
			WriteTrackerIcons(message);
			break;
		case MSG_VIEW_SORT_NAME:
			fBrowser->SetOrderBy(AlbumFileItem::CmpRef);
			fBrowser->Arrange();
			break;
		case MSG_VIEW_SORT_NONE:
			fBrowser->SetOrderBy(AlbumFileItem::CmpSerial);
			fBrowser->Arrange();
			break;
		case MSG_VIEW_SORT_SIZE:
			fBrowser->SetOrderBy(AlbumFileItem::CmpSize);
			fBrowser->Arrange();
			break;
		case MSG_VIEW_SORT_CTIME:
			fBrowser->SetOrderBy(AlbumFileItem::CmpCTime);
			fBrowser->Arrange();
			break;
		case MSG_VIEW_SORT_MTIME:
			fBrowser->SetOrderBy(AlbumFileItem::CmpMTime);
			fBrowser->Arrange();
			break;
		case MSG_VIEW_SORT_DIR:
			fBrowser->SetOrderBy(AlbumFileItem::CmpDir);
			fBrowser->Arrange();
			break;
		case MSG_VIEW_COL_0:
			fBrowser->SetColumns(0);
			break;
		case MSG_VIEW_COL_5:
			fBrowser->SetColumns(5);
			break;
		case MSG_VIEW_COL_10:
			fBrowser->SetColumns(10);
			break;
		case MSG_VIEW_MARKED:
			fBrowser->SetMask(fBrowser->Mask() ^ ITEM_FLAG_MARKED);
			fOnlyMarked->SetMarked(fBrowser->Mask() & ITEM_FLAG_MARKED);
			break;
		case MSG_VIEW_TRASH:
			fBrowser->SetMask(fBrowser->Mask() ^ ITEM_FLAG_DIMMED);
			fViewTrash->SetMarked(fBrowser->Mask() & ITEM_FLAG_DIMMED);
			break;
		case MSG_VIEW_LABEL_NAME:
			fShowName->SetMarked(!fShowName->IsMarked());
			SetLabelFlags();
			break;
		case MSG_VIEW_LABEL_SIZE:
			fShowSize->SetMarked(!fShowSize->IsMarked());
			SetLabelFlags();
			break;
		case MSG_VIEW_LABEL_DIM:
			fShowDimension->SetMarked(!fShowDimension->IsMarked());
			SetLabelFlags();
			break;
		case MSG_FILE_TRASH:
			MoveToTrash();
			break;
   		default:
   			// Pass down unrecognised commands!
   			BWindow::MessageReceived(message);
   			break;
	}
}


/**
	User preferencess changed, update accordingly.
	This should happen at least at startup.
*/
void MainWindow::PrefsReceived(BMessage *message)
{
	// Thumbnail Attribute Name
	const char *s;
	if (message->FindString("thumb_attr", &s) == B_OK) 
		fLoader->SetAttrNames(s);
	
	float w, h;
	if (message->FindFloat("thumb_width", &w) == B_OK &&
		message->FindFloat("thumb_height", &h) == B_OK)
			fLoader->SetThumbnailSize(w,h);
			
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

	int32 options = 1;
	if (message->FindInt32("load_options", &options) == B_OK) 
		fLoader->SetLoadOptions(options);
	
	if (message->FindInt32("display_options", &options) == B_OK)
		fBrowser->SetBuffering(options & 1);
		
		
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
			message->PrintToStream();
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
			
			if (message->FindString("be:clip_name", &clipname) == B_OK) {
				PRINT(("dir> %s\n", path.Path()));
				reply.AddString("name", clipname.String());
				// try to make it a new file, but if does not work, that's ok too.
				path.Append(clipname.String());
				BEntry entry(path.Path());
				entry.Remove();
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
			fToolbar->UpdateProgress(fBrowser->CountItems(), 0);
			PRINT(("done loading\n"));
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
	entry_ref ref;
	message->FindPointer("bitmap", (void**)&bitmap);
	message->FindRef("ref", &ref);
	int mask = 0;
	// Find/Create item	
	AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(fBrowser->EachItem(AlbumFileItem::EqRef, &ref));	
	if (item) {
		// Update an existing item
		if (message->FindRef("newref", &ref) == B_OK) {
			item->SetRef(ref);
			fBrowser->HighlightItem(item);
			mask |= UPDATE_STATS;
		}
		if (bitmap) {
			item->SetBitmap(bitmap);
			mask |= UPDATE_FRAME;
			fBrowser->InvalidateItem(item);
		}
	}
	else {	
		item = new AlbumFileItem(BRect(-1,-1,0,0), bitmap);
		item->SetRef(ref);
		if (fBrowser->AddItem(item)) {
			fBrowser->HighlightItem(item);
			mask |= UPDATE_STATS;
			// delete the splash message
			if (fBrowser->CountItems() == 1)
				fBrowser->Invalidate();
		}
	}
	
	// Feature Indicators
	int32 flags = 0;
	flags |= fShowName->IsMarked() ? ITEM_FLAG_LABEL0 : 0;
	flags |= fShowSize->IsMarked() ? ITEM_FLAG_LABEL1 : 0;
	flags |= fShowDimension->IsMarked() ? ITEM_FLAG_LABEL2 : 0;
	// some flags are already set
	item->SetFlags(item->Flags() | flags);
	if (message->FindInt32("flags", &flags) == B_OK) {
		item->SetFlags(item->Flags() | flags);
		PRINT(("flags %x\n", flags));
	}

	// File Stats
	off_t fsize;
	if (message->FindInt64("fsize", &fsize) == B_OK) {
		if (item->fFSize != fsize)
			mask |= UPDATE_STATS;
		item->fFSize = fsize;
	}
	ssize_t size;
	const time_t *time;
	if (message->FindData("ctime", B_TIME_TYPE, (const void**)&time, &size) == B_OK) {
		if (item->fCTime != *time)
			mask |= UPDATE_STATS;
		item->fCTime = *time;
	}
	if (message->FindData("mtime", B_TIME_TYPE, (const void**)&time, &size) == B_OK) {
		if (item->fMTime != *time)
			mask |= UPDATE_STATS;
		item->fMTime = *time;
	}

	// JPEG Tags
	BMessage metadata;
	if (message->FindMessage("tags", &metadata) == B_OK) {
		item->fTags = metadata;
		mask |= UPDATE_TAGS;
	}

	// BFS Attributes
	if (message->FindMessage("attributes", &metadata) == B_OK) {
		item->fAttributes = metadata;
		bool marked;
		if (metadata.FindBool("Marked", &marked) == B_OK && marked)
			item->SetFlags(item->Flags() | ITEM_FLAG_MARKED);
		mask |= UPDATE_ATTRS;
	}

	// Update display

	
	if (mask & (UPDATE_STATS|UPDATE_FRAME)) {
		item->Update(fBrowser);
		fBrowser->SortItems();
		fBrowser->Arrange();
	}

	if (mask & UPDATE_FRAME) {
		fBrowser->InvalidateItem(item);
	}
	
	if (item->IsSelected())
		fSidebar->Update(mask);

	// Progress Bar
	int32 total = 0, done = 0;
	if (message->FindInt32("total", &total) == B_OK) {
		message->FindInt32("done", &done);
		fToolbar->UpdateProgress(total, done);
		fToolbar->SetCounter(fBrowser->CountItems());
	}
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
	fSidebar->Update();
	//DisableUpdates();
	//int n = fSidebar->ShowSummary();
	int n = fBrowser->CountSelected();
	fToolbar->SetAttrSelected(false);
	fToolbar->SetSelected(n);
	//EnableUpdates();
	fAttrMenu->SetEnabled(n);
	fMoveToTrash->SetEnabled(n);
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
	Removes all selected items, leaving their files intact.
*/
void MainWindow::DeleteSelected()
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


void MainWindow::MarkSelected(bool enabled)
{
	BMessage target(enabled ? MSG_FILEATTR_WRITE : MSG_FILEATTR_REMOVE);
	AlbumItem *item;
	for (int i = 0; (item = fBrowser->GetSelection(i)); i++) {
		if (enabled) 
			item->SetFlags(item->Flags() | ITEM_FLAG_MARKED); 
		else
			item->SetFlags(item->Flags() & ~ITEM_FLAG_MARKED); 
		fBrowser->InvalidateItem(item);
	}
	// Store flags
	target.AddBool("Marked", true);
	fBrowser->GetSelectedRefs(&target);
	FileAttrDialog::Execute(Frame().LeftTop(), &target);
}


void MainWindow::RenameSelected(BMessage *message)
{
	BTextControl *source;
	if (message->FindPointer("source", (void**)&source) == B_OK) {
		BMessage target(MSG_FILEENTRY_RENAME);
		target.AddString("name", source->Text());	
		fBrowser->GetSelectedRefs(&target);
		FileNameDialog::Execute(Frame().LeftTop(), &target);
	}
}


void MainWindow::CopySelectedTags()
{
	// get the selected names
	BMessage tags;
	fSidebar->GetSelectedTags(&tags);
	AlbumFileItem *item;
	BNode node;
	for (int i = 0; (item = (AlbumFileItem*)fBrowser->GetSelection(i)); i++) {
		// targeted node
		if (node.SetTo(&item->Ref()) != B_OK)
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
	// get the selected names
	BMessage target(MSG_FILEATTR_REMOVE);
	fBrowser->GetSelectedRefs(&target);
	fSidebar->GetSelectedAttrs(&target);
	FileAttrDialog::Execute(Frame().LeftTop() + BPoint(70,70), &target);
}


/**
	Writes a single attribute, received from the Sidebar.
*/
void MainWindow::AttributeChanged(BMessage *message)
{
	// get the selected names
	BMessage target(MSG_FILEATTR_WRITE);
	fBrowser->GetSelectedRefs(&target);
	const char *name;
	if (message->FindString("name", &name) == B_OK) {
		type_code type;
		if (message->GetInfo("value", &type) == B_OK) {
			const void *data;
			ssize_t size;
			message->FindData("value", B_ANY_TYPE, &data, &size);
			if (target.AddData(name, type, data, size) == B_OK)
				FileAttrDialog::Execute(Frame().LeftTop() + BPoint(70,70), &target);
		}
	}
	
}


void MainWindow::WriteTrackerIcons(BMessage *target)
{
	target->AddString("name", fWriteAttr.String());
	target->AddInt32("format", fThumbFormat);
	AlbumFileItem *item;
	for (int i = 0; (item = (AlbumFileItem*)fBrowser->GetSelection(i)); i++) {
		target->AddRef("refs", &item->Ref());
		target->AddPointer("bitmap", item->Bitmap());
	}
	FileAttrDialog::Execute(Frame().LeftTop() + BPoint(70,70), target);
	
}


/**
	Moves selected entries to the Trash folder.
*/
void MainWindow::MoveToTrash()
{
	BMessage target(MSG_FILEENTRY_TRASH);
	fBrowser->GetSelectedRefs(&target);
	FileNameDialog::Execute(Frame().LeftTop() + BPoint(70,70), &target);
}



/**
	Changes thumbnail captions.
*/
void MainWindow::SetLabelFlags()
{
	uint32 mask = ITEM_FLAG_LABEL0 | ITEM_FLAG_LABEL1 | ITEM_FLAG_LABEL2;
	int32 flags = 0;
	flags |= fShowName->IsMarked() ? ITEM_FLAG_LABEL0 : 0;
	flags |= fShowSize->IsMarked() ? ITEM_FLAG_LABEL1 : 0;
	flags |= fShowDimension->IsMarked() ? ITEM_FLAG_LABEL2 : 0;
	AlbumItem *item;
	for (int i = 0; (item = fBrowser->ItemAt(i)); i++) {
		item->SetFlags((item->Flags() & ~mask) | flags);
		item->Update(fBrowser);
	}
	fBrowser->Arrange(false);
	fBrowser->Invalidate();
}


