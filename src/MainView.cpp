#include <Roster.h>

#include <PopUpMenu.h>
#include <MenuItem.h>

#include "App.h"
#include "MainView.h"
#include "indicatorbitmaps.h"
#include "OpenWithMenu.h"
// constants
#include <JpegTagExtractor.h>
#include <FindDirectory.h>
#include <Entry.h>
#include <Path.h>
#include <Node.h>



// Indicator bitmaps
BBitmap* AlbumFileItem::fMarkBmp = NULL;	
BBitmap* AlbumFileItem::fExifBmp = NULL;	
BBitmap* AlbumFileItem::fIptcBmp = NULL;	
BBitmap* AlbumFileItem::fLockBmp = NULL;	


/**
	An image browser item - one per entry.
*/
AlbumFileItem::AlbumFileItem(BRect frame, BBitmap *bitmap):
	AlbumItem(frame, bitmap)
{
	fPadding = 7;
	// for "no order" sorting
	static int counter = 0;
	fSerial = counter++;
}


/**
	Overlays indicator icons.
*/
void AlbumFileItem::DrawItem(BView *owner, uint32 flags)
{
	AlbumItem::DrawItem(owner, flags);
	owner->SetDrawingMode(B_OP_ALPHA);
	if (fFlags & ITEM_FLAG_DIMMED) {
		owner->SetPenSize(4.5);
		BRect rect = fFrame.InsetByCopy(4+fPadding,4+fPadding);
		owner->StrokeLine(rect.LeftTop(), rect.RightBottom());
		owner->StrokeLine(rect.RightTop(), rect.LeftBottom());
	}
	BPoint pos = fFrame.LeftTop() + BPoint(fPadding, 0);
	if (fFlags & ITEM_FLAG_MARKED) {
		owner->DrawBitmap(fMarkBmp, pos);
		pos.x += mark_png_width;
	}
	if (fFlags & HAS_EXIF) {
		owner->DrawBitmap(fExifBmp, pos);
		pos.x += exif_png_width;
	}
	if (fFlags & HAS_IPTC) {
		owner->DrawBitmap(fIptcBmp, pos);
		pos.x += iptc_png_width;
	}
	if (fFlags & ITEM_FLAG_LOCKED) {
		owner->DrawBitmap(fLockBmp, pos);
		pos.x += lock_png_width;
	}
		

}

/**
	Inits labels.
*/
void AlbumFileItem::Update(BView *owner)
{
	AlbumItem::Update(owner);
	fLabel[0] = fRef.name;
	sprintf(fLabel[1].LockBuffer(20), "%.1f KB", 1.0*fFSize/1024);
	fLabel[1].UnlockBuffer();	
	if (owner) {
		owner->TruncateString(&fLabel[0], B_TRUNCATE_END, fFrame.Width() - fPadding);
		owner->TruncateString(&fLabel[1], B_TRUNCATE_END, fFrame.Width() - fPadding);
	}
	
	int16 width, height;
	if (fTags.FindInt16("Width", &width) == B_OK && fTags.FindInt16("Height", &height) == B_OK) {
		char *buf = fLabel[2].LockBuffer(32);
		sprintf(buf,"%dx%d", width, height);
		fLabel[2].UnlockBuffer();
	}
}


const char* AlbumFileItem::Label(int index)
{
	if (index == 0 && (Flags() & ITEM_FLAG_LABEL0) == 0 ||
		index == 1 && (Flags() & ITEM_FLAG_LABEL1) == 0 ||
		index == 2 && (Flags() & ITEM_FLAG_LABEL2) == 0	)
		return NULL;	
	return fLabel[index].String();
}


int AlbumFileItem::CountLabels()
{
	return 3;
}


const entry_ref& AlbumFileItem::Ref() const
{
	return fRef;
}


void AlbumFileItem::SetRef(entry_ref &ref)
{
	fRef = ref;

	// Check if this reference is in the Trash dir.
   	char trashpath[B_PATH_NAME_LENGTH];
  	// unfortunately this only works for /boot
  	find_directory(B_TRASH_DIRECTORY, ref.device, false, trashpath, B_PATH_NAME_LENGTH);
	BEntry entry(&ref);
	BPath path;
	if (entry.GetPath(&path) == B_OK) {
		BString paths = path.Path();
		// Trash items are marked with special gfx.
		if (paths.FindFirst(trashpath) == 0)  {
			fFlags |= ITEM_FLAG_DIMMED;
		}
		else {
			fFlags &= ~ITEM_FLAG_DIMMED;
		}
		// Check if it's read-only
		if (access(paths.String(), W_OK) != 0)
			fFlags |= ITEM_FLAG_LOCKED;
		else		
			fFlags &= ~ITEM_FLAG_LOCKED;
	}
}


/**
	BObjectList EachFunction
*/
AlbumItem* AlbumFileItem::EqRef(AlbumItem *item, void *param)
{
	AlbumFileItem *p = dynamic_cast<AlbumFileItem*>(item);
	if (!p)
		return 0;
	return (p->fRef == *(entry_ref*)param) ? p : 0;
}


/**
	BObjectList CompareFunction
*/
int AlbumFileItem::CmpRef(const AlbumItem *a, const AlbumItem *b)
{
	const AlbumFileItem *p0 = dynamic_cast<const AlbumFileItem*>(a);
	const AlbumFileItem *p1 = dynamic_cast<const AlbumFileItem*>(b);
	return strcmp(p0->fRef.name, p1->fRef.name);
}

/**
	BObjectList CompareFunction
*/
int AlbumFileItem::CmpSerial(const AlbumItem *a, const AlbumItem *b)
{
	const AlbumFileItem *p0 = dynamic_cast<const AlbumFileItem*>(a);
	const AlbumFileItem *p1 = dynamic_cast<const AlbumFileItem*>(b);
	return p0->fSerial - p1->fSerial;
}

/**
	BObjectList CompareFunction
*/
int AlbumFileItem::CmpSize(const AlbumItem *a, const AlbumItem *b)
{
	const AlbumFileItem *p0 = dynamic_cast<const AlbumFileItem*>(a);
	const AlbumFileItem *p1 = dynamic_cast<const AlbumFileItem*>(b);
	return p0->fFSize - p1->fFSize;
}

/**
	BObjectList CompareFunction
*/
int AlbumFileItem::CmpCTime(const AlbumItem *a, const AlbumItem *b)
{
	const AlbumFileItem *p0 = dynamic_cast<const AlbumFileItem*>(a);
	const AlbumFileItem *p1 = dynamic_cast<const AlbumFileItem*>(b);
	return p0->fCTime - p1->fCTime;
}


/**
	BObjectList CompareFunction
*/
int AlbumFileItem::CmpMTime(const AlbumItem *a, const AlbumItem *b)
{
	const AlbumFileItem *p0 = dynamic_cast<const AlbumFileItem*>(a);
	const AlbumFileItem *p1 = dynamic_cast<const AlbumFileItem*>(b);
	// desc
	return p1->fMTime - p0->fMTime;
}


/**
	BObjectList CompareFunction
*/
int AlbumFileItem::CmpDir(const AlbumItem *a, const AlbumItem *b)
{
	const AlbumFileItem *p0 = dynamic_cast<const AlbumFileItem*>(a);
	const AlbumFileItem *p1 = dynamic_cast<const AlbumFileItem*>(b);
	if (!p0 || !p1)
		return 0;
	ino_t dif =  p0->Ref().directory - p1->Ref().directory;
	if (!dif)
		return p0->fSerial - p1->fSerial;
	return dif;
}


/**
	Only once!
*/
void AlbumFileItem::InitBitmaps()
{
	fMarkBmp = new BBitmap(BRect(0,0,mark_png_width,mark_png_height), B_RGBA32);
	memcpy(fMarkBmp->Bits(), mark_png_bits, mark_png_size);	
	fExifBmp = new BBitmap(BRect(0,0,exif_png_width,exif_png_height), B_RGBA32);	
	memcpy(fExifBmp->Bits(), exif_png_bits, exif_png_size);	
	fIptcBmp = new BBitmap(BRect(0,0,iptc_png_width,iptc_png_height), B_RGBA32);	
	memcpy(fIptcBmp->Bits(), iptc_png_bits, iptc_png_size);	
	fLockBmp = new BBitmap(BRect(0,0,lock_png_width,lock_png_height), B_RGBA32);	
	memcpy(fLockBmp->Bits(), lock_png_bits, lock_png_size);	
}








MainView::MainView(BRect frame, uint32 resizing):
	AlbumView(frame, "Browser", new BMessage(MSG_ITEM_SELECTED), resizing, B_PULSE_NEEDED | B_NAVIGABLE_JUMP),
	fAniItems(8)
{
}

void MainView::MessageReceived(BMessage *message)
{
 	switch (message->what) {
		case MSG_ITEM_SELECTED:
			ItemSelected(message);
			break;
		case MSG_ITEM_OPEN:
			ItemInvoked(message);
			break;
		default:
			AlbumView::MessageReceived(message);
 	}			
}

/**
	Gradually removes highlights.
*/
void MainView::Pulse()
{
	AlbumItem *item;
	if ((item = fAniItems.Get())) {
		HighlightItem(item, false);
	}
}


/**
	Hide nonmarked items or trash.
*/
bool MainView::IsItemVisible(AlbumItem *item)
{
	return (!(Mask() & ITEM_FLAG_MARKED) || (item->Flags() & ITEM_FLAG_MARKED)) 
		&& ((Mask() & ITEM_FLAG_DIMMED) || !(item->Flags() & ITEM_FLAG_DIMMED));
}


void MainView::SortItems()
{
	AlbumView::SortItems();
	if (OrderByFunc() == AlbumFileItem::CmpDir) {
		entry_ref ref;
		for (int i = 0; i < CountItems(); i++) {
			AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(ItemAt(i));
			if (!item || !IsItemVisible(item))
				continue;
			if (item->Ref().directory != ref.directory)
				item->SetFlags(item->Flags() | ITEM_FLAG_SEPARATOR);
			else
				item->SetFlags(item->Flags() & ~ITEM_FLAG_SEPARATOR);
			ref = item->Ref();
		}
	}
	else 
		for (int i = 0; i < CountItems(); i++) {
			AlbumItem *item = ItemAt(i);
			item->SetFlags(item->Flags() & ~ITEM_FLAG_SEPARATOR);
		}

}



void MainView::HighlightItem(AlbumItem *item, bool enabled)
{
	if (!enabled) {
			item->SetFlags(item->Flags() & ~ITEM_FLAG_HILIT);
	}
	else  {
		if (fAniItems.Full()) 
			HighlightItem(fAniItems.Get(), false);
		if (fAniItems.Put(item))
			item->SetFlags(item->Flags() | ITEM_FLAG_HILIT);
	}
	InvalidateItem(item);
}



void MainView::GetSelectedRefs(BMessage *message)
{
	AlbumItem *item;
	for (int i = 0; (item = GetSelection(i)); i++) {
		AlbumFileItem *fitem = dynamic_cast<AlbumFileItem*>(item);
		if (fitem)
			message->AddRef("refs", &fitem->Ref());			
	}
}


/**
	An item was clicked.
*/
void MainView::ItemSelected(BMessage *message)
{
	int32 index = -1;
	AlbumFileItem *item = NULL;
	if (message->FindPointer("item", (void**)&item) == B_OK) {
		// Double-click detected.
		if (item)
			be_roster->Launch(&item->Ref());
		return;		
	}
	else if (message->FindInt32("index", &index) == B_OK) {
		item = dynamic_cast<AlbumFileItem*>(ItemAt(index));
		int32 buttons = 0;
		message->FindInt32("buttons", &buttons);
		if (buttons & B_SECONDARY_MOUSE_BUTTON) {
			BPoint where;
			message->FindPoint("where", &where);
			ShowContextMenu(item, where);
			return;
		}
	}
	MakeFocus(this);
	// Pass it up to owner.
	AlbumView::MessageReceived(message);
}


/**
	Launches the default app associated with an item's file.
*/
void MainView::ItemInvoked(BMessage *message)
{
    entry_ref ref;
    if (message->FindRef("refs", &ref) != B_OK)
    	return;
	const char *appsig;
	if (message->FindString("app", &appsig) == B_OK && appsig) {
		// Does not check app flags if it is argv-only app. 
		// Works for most native BeOS apps.
		BMessage msg(B_REFS_RECEIVED);
		msg.AddRef("refs", &ref);
		be_roster->Launch(appsig, &msg);
	}
	else
		// No app specified, launch the preferred app.
		be_roster->Launch(&ref);
}


/**
	Shows item options menu.
*/
void MainView::ShowContextMenu(AlbumFileItem* item, BPoint where)
{
    BPopUpMenu *fPopUp = new BPopUpMenu("ItemOptions");
    if (item) {
        // Show Location - open the parent directory in Tracker.
        BMessage *msg;
        BEntry entry(&item->Ref());
        BEntry parent;
        entry.GetParent(&parent);
        entry_ref ref;

        if (parent.GetRef(&ref) == B_OK) {
            msg = new BMessage(MSG_ITEM_OPEN);
            msg->AddRef("refs", &ref);
            msg->AddString("app", "application/x-vnd.Be-TRAK");
            fPopUp->AddItem(new BMenuItem(_("Open Folder"), msg));
        }
        // Open With - launch a supporting app.
        BMenu *menu = new OpenWithMenu(_("Open With"B_UTF8_ELLIPSIS), &item->Ref(), MSG_ITEM_OPEN);
        menu->SetTargetForItems(this);
        fPopUp->AddItem(menu);

    }
   // Invoke the popupmenu
    fPopUp->SetTargetForItems(this);
	// Must be in async mode for this to work:
	fPopUp->SetAsyncAutoDestruct(true);
	ConvertToScreen(&where);
	// Post messages, async mode - Go() returns ASAP.
    fPopUp->Go(where, true, false, true);
	
}



/**
	Prepares a drag&drop message.
	
	Non-negotiated drag'n'drop delivers a B_SIMPLE_DATA to a target.
	Most programs respond by opening the supplied files.
	Tracker wil do a file copy.
*/
void MainView::ItemDragged(int32 index, BPoint where)
{
	AlbumItem *item0 = ItemAt(index);
	if (!item0)
		return;	
	where -= item0->Frame().LeftTop();

	BMessage dragmsg(B_SIMPLE_DATA);
    int count = 0;
	AlbumFileItem *item;
	for (int i = 0; (item = (AlbumFileItem*)GetSelection(i)); i++) {
		dragmsg.AddRef("refs", &item->Ref());
		count++;			
	}
	// mark the source
	dragmsg.AddMessenger("messenger", BMessenger(this));

	// Make a bitmap to drag around
	BBitmap *preview = new BBitmap(item0->Bitmap(), true);
	preview->Lock();
	BView canvas(preview->Bounds(), NULL, 0, B_WILL_DRAW);
	preview->AddChild(&canvas);
	char buf[10];
	BPoint p = canvas.Bounds().LeftBottom() + BPoint(2,-4);
	sprintf(buf,"%d", count);
	canvas.SetDrawingMode(B_OP_OVER);
	canvas.SetFontSize(31.0);
	canvas.SetHighColor(0,0,0);
	canvas.DrawString(buf, p + BPoint(2,2));
	canvas.SetHighColor(0,255,0);
	canvas.DrawString(buf, p);
	// BViews must not belong to anybody when deleted.
	canvas.RemoveSelf();
	//delete canvas;
	preview->Unlock();

	// Start dragging, the rest will be handled by app_server.
	DragMessage(&dragmsg, preview, B_OP_BLEND, where);
}

