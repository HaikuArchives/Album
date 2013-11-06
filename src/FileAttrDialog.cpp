#include <Debug.h>
#include <Window.h>
#include <Button.h>
#include "FileAttrDialog.h"
#include <Entry.h>
#include <Node.h>
#include <NodeInfo.h>
#include <StatusBar.h>
#include <Bitmap.h>
#include <TranslationKit.h>
#include <string.h>

FileAttrDialog::FileAttrDialog(BRect frame, const char *title, BMessage *message):
	BWindow(frame, title, B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	//TODO: Layout and font sensitivity

	ResizeTo(320,70);
	BView *view = new BView(Bounds(), NULL, B_FOLLOW_ALL, 0);	
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = view->Bounds();
	rect.InsetBy(10,10);
	fProgress = new BStatusBar(rect, "Progress", "Start", "Done");
	view->AddChild(fProgress);

	AddChild(view);

}


void FileAttrDialog::MessageReceived(BMessage *message) 
{
	switch (message->what) {
		case MSG_FILEATTR_ICONS:
			WriteTrackerIcon(message);
			Quit();
			break;
		case MSG_FILEATTR_THUMBS:
			WriteThumbnails(message);
			Quit();
			break;
		case MSG_FILEATTR_WRITE:
		case MSG_FILEATTR_REMOVE:
			WriteAttributes(message);
			Quit();
			break;
		default:
			BWindow::MessageReceived(message);
	}	
}

/**
	Call this from another thread!
*/
void FileAttrDialog::Execute(BPoint where, BMessage *message)
{
	BRect frame(0,0,200,60);
	frame.OffsetBy(where);
	FileAttrDialog *window = new FileAttrDialog(frame, "Updating ...", message); 
	ssize_t count = 0;
	type_code type;
	message->GetInfo("refs", &type, &count);
	if (count > 10)
		window->Show();
	else
		window->Run();
	window->PostMessage(message);	
	PRINT(("done\n"));
}


/**
	Writes or deletes BFS attributes.
	Files are referenced with 'refs' entry_ref field array.
*/
status_t FileAttrDialog::WriteAttributes(BMessage *targets)
{
	fProgress->Reset();	
	ssize_t count;
	type_code type;
	targets->GetInfo("refs", &type, &count);
	fProgress->SetMaxValue(count);
	
	BNode node;	
	entry_ref ref;
	for (int i = 0; targets->FindRef("refs", i, &ref) == B_OK; i++) {
        fProgress->Update(1.0, ref.name);
        UpdateIfNeeded();
		node.SetTo(&ref);
		if (node.InitCheck() != B_OK)
			continue;
		char *name;
		for (int i = 0; targets->GetInfo(B_ANY_TYPE, i, &name, &type) == B_OK; i++) {
			if (type == B_REF_TYPE)
				continue;
			if (targets->what == MSG_FILEATTR_REMOVE)
				node.RemoveAttr(name);
			else {
				ssize_t size;
				const void *data;
				if (targets->FindData(name, type, &data, &size) == B_OK)
					node.WriteAttr(name, type, 0, data, size);
			}
		}
		node.Sync();
	}
	return B_OK;
}


/*
	Sets Tracker icons to small previews of the content.
	Files are referenced with 'refs' entry_ref field array.
*/
status_t FileAttrDialog::WriteTrackerIcon(BMessage *targets)
{
	fProgress->Reset();	
	ssize_t count;
	type_code type;
	targets->GetInfo("refs", &type, &count);
	fProgress->SetMaxValue(count);

	BNode node;	
	entry_ref ref;
	for (int i = 0; targets->FindRef("refs", i, &ref) == B_OK; i++) {
        fProgress->Update(1.0, ref.name);
        UpdateIfNeeded();
		node.SetTo(&ref);
		if (node.InitCheck() != B_OK)
			continue;
			
		BBitmap *bitmap = NULL;
		if (targets->FindPointer("bitmap", i, (void**)&bitmap) != B_OK)
			continue;

        BNodeInfo nodeinfo(&node);
        BView view(BRect(0, 0, 31,31),"canvas", B_WILL_DRAW,0);
	
		// B_LARGE_ICON == 32	
        BBitmap *icon = new BBitmap(BRect(0, 0, B_LARGE_ICON - 1, B_LARGE_ICON - 1), B_CMAP8, true);
        icon->AddChild(&view);
        icon->Lock();
        view.DrawBitmap(bitmap, icon->Bounds());
        view.RemoveSelf();
        icon->Unlock();
        nodeinfo.SetIcon(icon, B_LARGE_ICON);
        delete icon;

		// B_MINI_ICON == 16
        icon = new BBitmap(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), B_CMAP8, true);
        icon->AddChild(&view);
        icon->Lock();
        view.DrawBitmap(bitmap, icon->Bounds());
        view.RemoveSelf();
        icon->Unlock();
        nodeinfo.SetIcon(icon, B_MINI_ICON);
        delete icon;
        
        node.Sync();
		PRINT(("icon set\n"));
        
	}
    return B_OK;
}



/*
	Files are referenced with 'refs' entry_ref field array.
*/
status_t FileAttrDialog::WriteThumbnails(BMessage *targets)
{
	fProgress->Reset();	
	ssize_t count;
	type_code type;
	targets->GetInfo("refs", &type, &count);
	fProgress->SetMaxValue(count);

	// destination attribute
	const char *attrname;
	if (targets->FindString("name", &attrname) != B_OK)
		return B_ERROR;			
	// output image format
	int32 format;
	if (targets->FindInt32("format", &format) != B_OK)
		format = B_BMP_FORMAT;

	BNode node;	
	entry_ref ref;
	for (int i = 0; targets->FindRef("refs", i, &ref) == B_OK; i++) {
        fProgress->Update(1.0, ref.name);
        UpdateIfNeeded();
		node.SetTo(&ref);
		if (node.InitCheck() != B_OK)
			continue;
		BBitmap *bitmap = NULL;
		if (targets->FindPointer("bitmap", i, (void**)&bitmap) != B_OK)
			continue;

		// translate to the selected output format
	    BTranslatorRoster *roster = BTranslatorRoster::Default();
	    BBitmapStream in(bitmap);
	    BMallocIO out;
	    status_t retcode = roster->Translate(&in, NULL, NULL, &out, format);
	    if (retcode == B_OK)
	    	node.WriteAttr(attrname, B_RAW_TYPE, 0, out.Buffer(), out.BufferLength());
	    in.DetachBitmap(&bitmap);
		PRINT(("write thumbnail: %s\n", strerror(retcode)));
	}
    return B_OK;
}
