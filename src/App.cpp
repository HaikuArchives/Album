#include <Debug.h>
#include "MainWindow.h"
#include "App.h"
#include <Alert.h>
#include <Path.h>
#include <File.h>
#include <FindDirectory.h>

#define ABOUT_TEXT "Album (Code Name Aberration) v0.9.2\n\nCopyright "B_UTF8_COPYRIGHT" 2006-2010 by Matjaž Kovač\n""This program is distributed under the terms of the MIT License (see LICENSE).\n""Includes libiptcdata, copyright "B_UTF8_COPYRIGHT" 2005 by David Moore under the terms of the GNU LGPL.\n""Recompiled on "__DATE__


/// Allocates essential resources.
App::App(): 
	BApplication(APP_SIGNATURE)
{
	fMain = new MainWindow(BRect(20,30,640,530), "Album");
	fMain->Show();
	fPrefsDialog = new SettingsWindow(BRect(40, 40, 440, 260), _("Preferences"));
}

/** 
	Fires on B_ABOUT_REQUESTED.
*/
void App::AboutRequested()
{
    BAlert* box = new BAlert(_("About"), ABOUT_TEXT, _("Cool"));
    box->Go();
    // BAlert deleted itself.
}


void App::ArgvReceived(int32 argc, char **argv)
{
	//TODO: Handle program arguments
	
}

void App::MessageReceived(BMessage *message)
{
    switch (message->what) {
		case CMD_SHOW_PREFS:
			if (fPrefsDialog->IsHidden())
				fPrefsDialog->Show();
			else
				fPrefsDialog->Activate();
			break;
		case CMD_UPDATE_PREFS:
			SavePreferences(message, "Album_settings");
			UpdatePreferences(message);
			break;
    	default:
    		BApplication::MessageReceived(message);
    }
}

/**
	Closes all windows and calls it a run.
*/
bool App::QuitRequested()
{
	// This better work...
	BWindow *window = NULL;
	while ((window = WindowAt(0))) {
		if (window->Lock())
			window->Quit();
		else break;
	}
	return true;
}


/**
	The message loop is ready to go.
*/
void App::ReadyToRun() 
{
	BMessage prefs;
	// defaults
	prefs.AddInt32("load_options", 3);
	prefs.AddInt32("display_options", 1);
	RestorePreferences(&prefs, "Album_settings");
	UpdatePreferences(&prefs);
}

void App::RefsReceived(BMessage *message)
{
	if (fMain) {
		message->what = B_SIMPLE_DATA;
		fMain->PostMessage(message);
	}
}



/**
	Restores user preferences.
*/
status_t App::RestorePreferences(BMessage *prefs, const char *name)
{
	BPath path;
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    if (status == B_OK) {
    	path.Append(name);
    	BFile file(path.Path(), B_READ_ONLY);
    	status = prefs->Unflatten(&file);
    	
    }
    return status;
}

/**
	Stores the preferences in the User Settings Directory.
*/
status_t App::SavePreferences(BMessage *prefs, const char *name)
{
	BPath path;
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    if (status == B_OK) {
    	path.Append(name);
    	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
    	status = prefs->Flatten(&file);
    }
    return status;
}

/**
	Broadcasts new preferences to all windows.
*/
void App::UpdatePreferences(BMessage *message)
{
	message->what = MSG_PREFS_CHANGED;
	BWindow *window = NULL;
	for (int32 i = 0; (window = WindowAt(i)); i++) {
		window->PostMessage(message);		
	}
}




/// The program run starts here.
int main(int, char**)
{	
	App app;
	app.Run();
	return 0;
}
