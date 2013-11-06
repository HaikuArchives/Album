#ifndef _APP_H_
#define _APP_H_

#include <Application.h>
#include <SettingsWindow.h>

enum {
	CMD_SHOW_PREFS = 'shPr'
};

// Unique program identifier
#define APP_SIGNATURE "application/x-vnd.mk-album"
// String localisation placeholder
#define _(phrase) phrase

enum {
	MSG_PREFS_CHANGED = 'pref',
    CMD_UPDATE_PREFS = 'updp',
};

class App : public BApplication 
{
	public:
	
	App();
	virtual void AboutRequested();
	virtual void ArgvReceived(int32 argc, char **argv);
	virtual void MessageReceived(BMessage *message);
	virtual bool QuitRequested();	
	virtual void ReadyToRun();
	virtual void RefsReceived(BMessage *message);
	
	private:
	status_t RestorePreferences(BMessage *prefs, const char *name);
	status_t SavePreferences(BMessage *prefs, const char *name);
	void UpdatePreferences(BMessage *message);

	BWindow *fPrefsDialog;
	BWindow *fMain;
	BMessage fPreferences;
	
};

#endif

