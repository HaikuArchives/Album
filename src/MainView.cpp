#define DEBUG 1
#include <Debug.h>
#include <Roster.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <JpegTagExtractor.h>
#include <FindDirectory.h>
#include <Path.h>
#include <Node.h>
#include <Autolock.h>
#include <TranslationKit.h>
#include "MainView.h"
#include "OpenWithMenu.h"
#include "App.h"
#include "MainWindow.h"


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
	An image browser item - one per entry.
*/
AlbumFileItem::AlbumFileItem(BRect frame, BBitmap *bitmap):
	AlbumItem(frame, bitmap)
{
	fPadding = 10;
	// for "no order" sorting
	static int counter = 0;
	fSerial = counter++;
}


/**
	Overlays indicator icons.
*/
void AlbumFileItem::DrawItem(BView *owner)
{
	static BBitmap* markIcon = BTranslationUtils::GetBitmap('PNG ', "item-mark");	
	static BBitmap* exifIcon = BTranslationUtils::GetBitmap('PNG ', "item-exif");	
	static BBitmap* iptcIcon = BTranslationUtils::GetBitmap('PNG ', "item-iptc");	
	static BBitmap* lockIcon = BTranslationUtils::GetBitmap('PNG ', "item-lock");		
	
	AlbumItem::DrawItem(owner);
	owner->SetDrawingMode(B_OP_ALPHA);
	if (Flags() & ITEM_FLAG_DIMMED) {
		owner->SetPenSize(3);
		owner->SetHighColor(255,0,0,255);
		BRect rect = ((BRect)Frame()).InsetByCopy(fPadding, fPadding);
		owner->StrokeLine(rect.LeftTop(), rect.RightBottom());
		owner->StrokeLine(rect.RightTop(), rect.LeftBottom());
	}
	
	// indicator icons look best positioned this way
	BPoint pos = Frame().LeftTop() + BPoint(fPadding, fPadding-7);
	
	if ((Flags() & ITEM_FLAG_MARKED) && markIcon) {
		owner->DrawBitmap(markIcon, pos);
		pos.x += markIcon->Bounds().right;
	}
	if ((Flags() & ITEM_FLAG_LOCKED) && lockIcon) {
		owner->DrawBitmap(lockIcon, pos);
		pos.x += lockIcon->Bounds().right;
	}
	if ((Flags() & ITEM_FLAG_EXIF) && exifIcon) {
		owner->DrawBitmap(exifIcon, pos);
		pos.x += exifIcon->Bounds().right;
	}
	if ((Flags() & ITEM_FLAG_IPTC) && iptcIcon) {
		owner->DrawBitmap(iptcIcon, pos);
		pos.x += iptcIcon->Bounds().right;
	}
		

}


uint16 AlbumFileItem::CountLabels()
{
	return sizeof 4;
}


void AlbumFileItem::GetLabel(uint16 index, BString *label)
{
	switch (index)
	{
		case 0:
			label->SetTo(fRef.name);
			break;
		case 1:
			App::FormatFileSize(label->LockBuffer(30), 30, fFSize);
			label->UnlockBuffer();
			break;
		case 2:
			sprintf(label->LockBuffer(30), "%d×%d", fImgWidth, fImgHeight);
			label->UnlockBuffer();
			break;
		case 3:
			App::FormatTimestamp(label->LockBuffer(30), 30, fMTime);
			label->UnlockBuffer();
			break;		
	}
}


/**
	Label visibility.
	16 MSBs of Flags() are reserved for labels.
*/
bool AlbumFileItem::IsLabelVisible(uint16 index)
{
	return Flags() & (0x00010000 << index);
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
	BEntry entry(&ref);
	BPath path;
	if (entry.GetPath(&path) == B_OK) {
  		// Trash is per-volume dir.
		BString paths = path.Path();
  		status_t ret = find_directory(B_TRASH_DIRECTORY, ref.device, false, trashpath, B_PATH_NAME_LENGTH);
		SetFlags(ITEM_FLAG_DIMMED, ret == B_OK && paths.FindFirst(trashpath) == 0);
		// Check if it's read-only
		SetFlags(ITEM_FLAG_LOCKED, access(paths.String(), W_OK) != 0);
	}
	
	
	
}



const uint32 AlbumFileItem::Serial()
{
	return fSerial;	
}







/**
	A new specialized AlbumView instance with custom selection message.
*/
MainView::MainView(BRect frame, BMessage *message, uint32 resizing):
	AlbumView(frame, "Browser", message, resizing, B_PULSE_NEEDED),
	fNoDataMsg(S_DROPFILES)
{
}



void MainView::Draw(BRect update)
{
	if (CountItems() == 0) {
		// Show splash msg
		BFont font;
		GetFont(&font);
		SetFontSize(16.0);
		BRect rc = Bounds();
		FillRect(rc,B_SOLID_LOW);
		rgb_color c = HighColor();
		SetHighColor(tint_color(c, B_LIGHTEN_1_TINT));
		DrawString(fNoDataMsg.String(), BPoint((rc.Width() - StringWidth(fNoDataMsg.String()))/2, rc.Height()/2));
		SetHighColor(c);		
		SetFont(&font);
	}
	else {
		
		AlbumView::Draw(update);
	}
}


void MainView::MessageReceived(BMessage *message)
{
 	switch (message->what) {
 		case CMD_ITEM_LAUNCH:
 			LaunchItem(message);
 			break;
		default:
			AlbumView::MessageReceived(message);
 	}			
}



void MainView::KeyDown(const char *bytes, int32 numBytes)
{
    switch (bytes[0]) {
    	case B_DELETE:
    		Looper()->PostMessage(CMD_ITEM_REMOVE);
        	break;
    	case B_ENTER: 
    		// Launch preferred app.
    		LaunchItem(NULL);
			break;
        default: 
        	AlbumView::KeyDown(bytes,numBytes);
    }
}



void MainView::MouseDown(BPoint where)
{
	AlbumView::MouseDown(where);
	Window()->Activate();
    MakeFocus();
	
	int32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if (buttons & B_SECONDARY_MOUSE_BUTTON) {
		AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(ItemAt(IndexOf(&where)));
		ShowContextMenu(item, fLastWhere);
	}
}




/**
	Gradually removes highlights.
*/
void MainView::Pulse()
{
	for (int i = 0; i < CountItems(); i++) {
		AlbumItem *item = ItemAt(i);
		if (item->Highlight() > 0) {
			item->SetHighlight(item->Highlight() - 0.25);
			InvalidateItem(item);
		}
	}
}



/**
	Hide nonmarked items or trash.
*/
bool MainView::IsItemVisible(AlbumItem *item)
{
	if (item == NULL)
		return false;
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
			if (!IsItemVisible(item))
				continue;
			item->SetFlags(ALBUMITEM_SEPARATOR, item->Ref().directory != ref.directory);
			ref = item->Ref();
		}
	}
	else 
		for (int i = 0; i < CountItems(); i++) {
			AlbumItem *item = ItemAt(i);
			item->SetFlags(ALBUMITEM_SEPARATOR, false);
		}

}


int32 MainView::GetSelectedRefs(BMessage *message)
{
	int32 n = 0;
	for (int i = 0; i < CountItems(); i++) {
		AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(ItemAt(i));
		if (IsItemVisible(item) && item->IsSelected()) {
			message->AddRef("refs", &item->Ref());			
			n++;
		}
	}
	return n;
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

	BMessage dragmsg(B_SIMPLE_DATA);
	dragmsg.AddMessenger("messenger", BMessenger(this));
	int32 n = GetSelectedRefs(&dragmsg);
	
	if (item0->Bitmap() == NULL) 
		DragMessage(&dragmsg, item0->Frame());
	else {
		// Make a bitmap to drag around
		BBitmap *preview = new BBitmap(item0->Bitmap()->Bounds(),B_RGBA32, true);
		preview->Lock();
		BView canvas(preview->Bounds(), NULL, 0, 0);
		preview->AddChild(&canvas);
		canvas.SetHighColor(B_TRANSPARENT_COLOR);
		
		canvas.FillRect(preview->Bounds());	
		canvas.SetDrawingMode(B_OP_ALPHA);
		canvas.SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);		
		canvas.SetHighColor(0, 0, 0, 144);
		canvas.DrawBitmap(item0->Bitmap());		
		char buf[8];
		sprintf(buf,"%d", n);
		canvas.SetHighColor(0,255,0);
		canvas.SetFontSize(26);
		canvas.DrawString(buf, canvas.Bounds().LeftBottom());
		canvas.Sync();
		// this is a stack object, can't let DragMessage() delete it.
		canvas.RemoveSelf();
		preview->Unlock();
	
		// Start dragging, the rest will be handled by app_server.
		where -= item0->Frame().LeftTop();
		DragMessage(&dragmsg, preview, B_OP_ALPHA, where);
	}
}



/**
	Launches the default app associated with an item's file.
*/
void MainView::LaunchItem(BMessage *message)
{
    AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(SelectedItem(0));
    if (!IsItemVisible(item))
    	return;    
	const char *appsig;
	if (message != NULL && message->FindString("app", &appsig) == B_OK && appsig) {
		// Does not check app flags if it is argv-only app. 
		// Works for most native BeOS apps.
		entry_ref ref;
		BMessage msg(B_REFS_RECEIVED);
		if (message->FindRef("refs",&ref) == B_OK)
			msg.AddRef("refs",&ref);
		else
			GetSelectedRefs(&msg);
		be_roster->Launch(appsig, &msg);
	}
	else 
		// No app specified, launch the preferred app.
		be_roster->Launch(&item->Ref());
}


/**
	Shows item options menu.
*/
void MainView::ShowContextMenu(AlbumFileItem* item, BPoint where)
{
	if (item == NULL)
		return;
		
	// adjust scale
	where.x *= Zoom();
	where.y *= Zoom();
		
	BPopUpMenu *fPopUp = new BPopUpMenu(_("Item Options"));
    // Show Location - open the parent directory in Tracker.
    BMessage *msg;
    BEntry entry(&item->Ref());
    BEntry parent;
    entry.GetParent(&parent);
    entry_ref ref;
    if (parent.GetRef(&ref) == B_OK) {
		msg = new BMessage(CMD_ITEM_LAUNCH);
        msg->AddRef("refs", &ref);
        msg->AddString("app", "application/x-vnd.Be-TRAK");
        BMenuItem *openFolder = new BMenuItem(_("Open Folder"), msg);
        fPopUp->AddItem(openFolder);
        openFolder->SetEnabled(CountSelected() == 1);
    }
	// Open With - launch a supporting app.
	BMenu *menu = new OpenWithMenu(_("Open With"B_UTF8_ELLIPSIS), &item->Ref(), CMD_ITEM_LAUNCH);
	menu->SetEnabled(CountSelected() == 1);
	menu->SetTargetForItems(this);
	fPopUp->AddItem(menu);
	fPopUp->AddSeparatorItem();
	fPopUp->AddItem(new BMenuItem(_("Mark"), new BMessage(CMD_ITEM_MARK)));
	fPopUp->AddItem(new BMenuItem(_("Unmark"), new BMessage(CMD_ITEM_UNMARK)));
	fPopUp->AddItem(new BMenuItem(_("Remove from View"), new BMessage(CMD_ITEM_REMOVE)));
	fPopUp->AddSeparatorItem();
	fPopUp->AddItem(new BMenuItem(_("Move to Trash"), new BMessage(CMD_ITEM_TRASH)));
    fPopUp->SetTargetForItems(this);
	fPopUp->SetAsyncAutoDestruct(true);
	ConvertToScreen(&where);
	// async mode: return ASAP, post messages later
    fPopUp->Go(where, true, false, true);
	Window()->Sync();
    
}



