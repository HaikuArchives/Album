#ifndef _APP_H_
#define _APP_H_

#include <Application.h>
#include <SettingsWindow.h>
#ifdef __HAIKU__
#include <DateTimeFormat.h>
#include <NumberFormat.h>
#endif

// string constants
#include "phrase.h"

// Localization placeholder, gettext-style
// for now this just identifies strings that should be translated sometime
#define _(phrase) phrase

// Unique program identifier
#define APP_SIGNATURE "application/x-vnd.mk-album"

// Program-wide commands
enum {
	MSG_PREFS_CHANGED = 'pref',
    CMD_UPDATE_PREFS = 'updp',
	CMD_SHOW_PREFS = 'shPr'
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

	static void FormatTimestamp(char *buf, int bufsize, time_t t);
	static void FormatFileSize(char *buf, int bufsize, int64 filesize);
#ifdef __HAIKU__
	static BDateTimeFormat dateTimeFmt;
	static BNumberFormat numberFmt;
#endif

	private:
	
	status_t RestorePreferences(BMessage *prefs, const char *name);
	status_t SavePreferences(BMessage *prefs, const char *name);
	void UpdatePreferences(BMessage *message);

	BWindow *fPrefsDialog;
	BWindow *fMain;
	BMessage fPreferences;
	
};



#endif

