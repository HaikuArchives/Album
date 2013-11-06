#include <Debug.h>
#include <Window.h>
#include <Window.h>
#include <Button.h>
#include <StatusBar.h>
#include "FileNameDialog.h"
#include <Entry.h>
#include <Path.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <String.h>
#include <stdio.h>

FileNameDialog::FileNameDialog(BRect frame, const char *title, BMessage *message):
	BWindow(frame, title, B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	ResizeTo(400,70);
	
	BView *view = new BView(Bounds(), NULL, B_FOLLOW_ALL, 0);	
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = view->Bounds();
	rect.InsetBy(10,10);
	fProgress = new BStatusBar(rect, "Progress", "Start", "Done");
	view->AddChild(fProgress);
	AddChild(view);
}


void FileNameDialog::MessageReceived(BMessage *message) 
{
	switch (message->what) {
		case MSG_FILEENTRY_RENAME:
			WriteName(message);
			Quit();
			break;	
		case MSG_FILEENTRY_TRASH:
			MoveToTrash(message);
			Quit();
			break;	
		default:
			BWindow::MessageReceived(message);
	}	
}



void FileNameDialog::WriteName(BMessage *targets)
{
	fProgress->Reset();	
	ssize_t count;
	type_code type;
	targets->GetInfo("refs", &type, &count);
	fProgress->SetMaxValue(count);
	
	BString filename;
	if (targets->FindString("name", &filename) == B_OK) {	
		filename.ReplaceFirst("*", "%d");		
		BEntry entry;
		entry_ref ref;
		int serial = 1;
		for (int i = 0; targets->FindRef("refs", i, &ref) == B_OK; i++) {
			fProgress->Update(1, ref.name);
        	UpdateIfNeeded();
			if (entry.SetTo(&ref) == B_OK) {
				char s[B_FILE_NAME_LENGTH];
				sprintf(s, filename.String(), serial++);
				entry.Rename(s);					
			}	
		}
	}
}



void FileNameDialog::MoveToTrash(BMessage *targets)
{
   	char trashpath[B_PATH_NAME_LENGTH];
	BEntry entry;
	entry_ref ref;
	for (int i = 0; targets->FindRef("refs", i, &ref) == B_OK; i++) {
		if (entry.SetTo(&ref) == B_OK) {
  			if (find_directory(B_TRASH_DIRECTORY, ref.device, false, trashpath, B_PATH_NAME_LENGTH) == B_OK) {
	 	 		BDirectory dir(trashpath);
				entry.MoveTo(&dir);
  			}
		}
	}
}



/**
	Class factory.
	Call this from another thread!
*/
void FileNameDialog::Execute(BPoint where, BMessage *message)
{
	BRect frame(0,0,200,60);
	frame.OffsetBy(where);
	FileNameDialog *window = new FileNameDialog(frame, "Updating ...", message); 
	
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

