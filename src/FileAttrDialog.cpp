#include <Window.h>
#include <Button.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <Path.h>
#include <Node.h>
#include <NodeInfo.h>
#include <StatusBar.h>
#include <Bitmap.h>
#include <TranslationKit.h>
#include <string.h>
#include "FileAttrDialog.h"
#include "App.h"
#include "ImageLoader.h"


FileAttrDialog::FileAttrDialog(BRect frame, const char *title):
	BWindow(frame, title, B_BORDERED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	BView *view = new BView(Bounds(), NULL, B_FOLLOW_ALL, 0);	
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = view->Bounds();
	rect.InsetBy(10,8);
	fProgress = new BStatusBar(rect, "Progress", "Start", "Done");
	view->AddChild(fProgress);
	AddChild(view);
}

/**
	Class Factory
	Call this from another thread!
*/
FileAttrDialog* FileAttrDialog::Execute(BRect frame, BMessage *message, BHandler *replyHandler)
{
	FileAttrDialog *window = new FileAttrDialog(frame, _("File Operation")); 
	if (message) {
		window->Run();
		window->PostMessage(message, NULL, replyHandler);
	}
	else
		window->Show();
	return window;	
}


void FileAttrDialog::MessageReceived(BMessage *message) 
{
	switch (message->what) {
		case CMD_OP_THUMBS:
			ProcessFiles(WriteThumbnailOp, message);
			break;
		case CMD_OP_ICONS:
			ProcessFiles(WriteTrackerIconOp, message);
			break;
		case CMD_OP_ATTR_WRITE:
			ProcessFiles(WriteAttributesOp, message);
			break;
		case CMD_OP_ATTR_REMOVE:
			ProcessFiles(RemoveAttributesOp, message);
			break;
		case CMD_OP_TRASH:
			ProcessFiles(MoveToTrashOp, message);
			break;
		case CMD_OP_RENAME:
			ProcessFiles(RenameOp, message);
			break;
		default:
			BWindow::MessageReceived(message);
	}	
}


bool FileAttrDialog::QuitRequested()
{
	return BWindow::QuitRequested();
}

/*
	Processes one or more items.
	
	One item
	refs: single file reference.
	count: the expected total count.
	delta: - part of the total.
	
	All items in one go
	refs: an array.
	count: the array size
	delta: 1.0 
	
*/
void FileAttrDialog::ProcessFiles(FileOpFunction operation, BMessage *message)
{
	float total, delta;
	fProgress->Reset();	
	message->FindFloat("total", &total);	
	fProgress->SetMaxValue(total);
	
	entry_ref ref;
	int32 i;
	for (i = 0; message->FindRef("refs", i, &ref) == B_OK; i++) {		
		message->FindFloat("delta", &delta);
		fProgress->Update(delta, ref.name);	
        UpdateIfNeeded();
        
        status_t ret = operation(&ref, i, message);
        if (ret == B_BAD_THREAD_ID || ret == B_BAD_PORT_ID)
        	// The caller is gone - don't linger on.
        	Quit();
	}
	
	if (delta >= total || i == total)
		// All done.
		Quit();
	
}



/**
	Processes one file.
*/
status_t FileAttrDialog::WriteThumbnailOp(entry_ref *ref, int32 index, BMessage *message)
{
	BNode node(ref);
	if (node.InitCheck() != B_OK)
		return B_ERROR;

	int32 format;
	// FindString() returns pointers
	const char *attrname;
	float w, h;
	if (message->FindString("thumb_atrr", &attrname) != B_OK)
		return B_BAD_VALUE;
	message->FindInt32("thumb_format", &format);
	message->FindFloat("thumb_width", &w);
	message->FindFloat("thumb_height", &h);
			
	BBitmap *bitmap;
	if (message->FindPointer("bitmap", index, (void**)&bitmap) != B_OK) {
		// No worries - we'll make our own.
	 	bitmap = ImageLoader::ReadImagePreview(ref, w, h);
		// Make sure somebody takes ownership of the newly allocated data!
		BMessage reply(MSG_LOADER_UPDATE);
		reply.AddRef("ref", ref);
		reply.AddPointer("bitmap", bitmap);
		status_t ret = message->SendReply(&reply);
		// memory leak hazard
		if (ret != B_OK) {
			delete bitmap;
			return ret;
		}
	}
					
	if (bitmap) {
    	BBitmapStream in(bitmap);
    	BMallocIO out;
    	status_t retcode = BTranslatorRoster::Default()->Translate(&in, NULL, NULL, &out, format,B_TRANSLATOR_BITMAP);
    	if (retcode == B_OK) {
    		node.WriteAttr(attrname, B_RAW_TYPE, 0, out.Buffer(), out.BufferLength());
    	}
    	in.DetachBitmap(&bitmap);
	}			
	return B_OK;

}

/**
	Processes one file.
*/
status_t FileAttrDialog::WriteTrackerIconOp(entry_ref *ref, int32 index, BMessage *message)
{
	BNode node(ref);
	if (node.InitCheck() != B_OK)
		return B_ERROR;
			
	BBitmap *bitmap = NULL;
	if (message->FindPointer("bitmap", index, (void**)&bitmap) != B_OK)
		return B_BAD_VALUE;

	BNodeInfo nodeinfo(&node);
    BView view(BRect(0, 0, 31,31), NULL, B_WILL_DRAW, 0);
	
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

    return B_OK;
}

/**
	Processes one file.
*/
status_t FileAttrDialog::WriteAttributesOp(entry_ref *ref, int32 index, BMessage *message)
{
	BNode node(ref);
	if (node.InitCheck() != B_OK)
		return B_ERROR;
		
	BMessage attrs;
	
	if (message->FindMessage("attributes", &attrs) != B_OK)
		return B_BAD_VALUE;

	type_code type;
	char *name;
	for (int i = 0; attrs.GetInfo(B_ANY_TYPE, i, &name, &type) == B_OK; i++) {
		ssize_t size;
		const void *data;
		if (attrs.FindData(name, type, &data, &size) == B_OK) {
			node.WriteAttr(name, type, 0, data, size);
		}
	}
	return B_OK;
}


/**
	Processes one file.
*/
status_t FileAttrDialog::RemoveAttributesOp(entry_ref *ref, int32 index, BMessage *message)
{
	BNode node(ref);
	if (node.InitCheck() != B_OK)
		return B_ERROR;
		
	BMessage attrs;
	if (message->FindMessage("attributes", &attrs) != B_OK)
		return B_BAD_VALUE;

	type_code type;
	char *name;
	for (int i = 0; attrs.GetInfo(B_ANY_TYPE, i, &name, &type) == B_OK; i++) {
		node.RemoveAttr(name);
	}
	return B_OK;
}


/**
	Simulates Tracker "Move to Trash".
	This is a combined entry/node operation.
*/
status_t FileAttrDialog::MoveToTrashOp(entry_ref *ref, int32 index, BMessage *message)
{
   	char trashpath[B_PATH_NAME_LENGTH];
	if (find_directory(B_TRASH_DIRECTORY, ref->device, false, trashpath, B_PATH_NAME_LENGTH) == B_OK) {
		BEntry entry(ref);		
		BDirectory trashdir(trashpath);
		if (entry.MoveTo(&trashdir) == B_OK) {
			// Tracker "Restore" needs this, but it isn't strictly necessary
			BPath path(ref);
			BNode node(&entry);
			// write _old_ path to the _new_ node.
			return node.WriteAttr("_trk/original_path", B_STRING_TYPE, 0, path.Path(), strlen(path.Path()));
		}
	}
	return B_ERROR;
}

/**
	Rename/Renumber
*/
status_t FileAttrDialog::RenameOp(entry_ref *ref, int32 index, BMessage *message)
{
	BString filename;
	if (message->FindString("name", &filename) == B_OK) {	
		filename.ReplaceFirst("*", "%d");		
		char s[B_FILE_NAME_LENGTH];
		sprintf(s, filename.String(), index);
		BEntry entry(ref);
		return entry.Rename(s);					
	}
	return B_ERROR;
}
