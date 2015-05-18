
#define DEBUG 1
#include <Debug.h>
#include <Alert.h>
#include <Path.h>
#include <File.h>
#include <FindDirectory.h>
#include <Screen.h>
#include "App.h"
#include "MainWindow.h"

#ifdef __HAIKU__    
BDateTimeFormat App::dateTimeFmt;
BNumberFormat App::numberFmt;
#endif



/// Allocates essential resources.
App::App(): 
	BApplication(APP_SIGNATURE)
{
	BScreen screen;
	BRect r = screen.Frame().InsetByCopy(200,140);
	fMain = new MainWindow(r, "Album");
	fMain->Show();

	fPrefsDialog = new SettingsWindow(BRect(80, 80, 500, 350), _("Album Preferences"));
}

/** 
	Fires on B_ABOUT_REQUESTED.
*/
void App::AboutRequested()
{
    BAlert* box = new BAlert(_("About"), S_ABOUT_TEXT, _("Cool"));
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


bool App::QuitRequested()
{
	return true;	
}


/**
	The message loop is ready to go.
*/
void App::ReadyToRun() 
{
	BMessage prefs;
	// fail-safe defaults
	prefs.AddInt32("load_options", 3);
	prefs.AddInt32("display_options", 1);
	prefs.AddString("thumb_format","JPEG");
	prefs.AddFloat("thumb_width",64);
	prefs.AddFloat("thumb_height",64);
	RestorePreferences(&prefs, "Album_settings");

	// Dispatch to all windows
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


/**
	Localizes time, writes output to buffer 's', max n bytes.
*/
void App::FormatTimestamp(char *s, int n, time_t t)
{
#ifdef __HAIKU__		
	dateTimeFmt.Format(s, n, t, B_SHORT_DATE_FORMAT,B_SHORT_TIME_FORMAT);	
#else
	strftime(s, n, "%x %X", localtime (&t));
#endif
}


void App::FormatFileSize(char *s, int n, int64 fsize)
{
	if (fsize < 10000) {
		sprintf(s, _("%ld bytes"), (long)fsize);
	}
	else {
#ifdef __HAIKU__
		double v = (10*fsize/1024)/10.0;
		App::numberFmt.Format(s, n, (double)v);
		strcat(s, " KB");
#else
		sprintf(s, "%.1f KB", fsize/1024.0);
#endif
	}
}





/// The program run starts here.
int main(int, char**)
{	
	App app;
	app.Run();
	return 0;
}
