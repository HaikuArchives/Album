#ifndef SETTINGSWINDOW_H_
#define SETTINGSWINDOW_H_

#include <Window.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Button.h>
#include <TextControl.h>
#include <ListView.h>
enum {
    CMD_DONE = 'done',
    MSG_EXTRACTTAGS_CHECK = 'Chk1'
};

class SettingsWindow : public BWindow
{
	public:
	
    SettingsWindow(BRect frame, const char * title);
    virtual void MessageReceived(BMessage *message);
    virtual bool QuitRequested();
	
	private:

    void Restore(BMessage *prefs);
	void Save();
		
    BButton *fApplyButton;
    BTextControl *fReadAttr;
    BPopUpMenu *fSizeMenu;
    BPopUpMenu *fFormatMenu;
    BCheckBox *fExtractTags;
    BCheckBox *fExifThumb;
    BCheckBox *fAntiFlicker;
};

#endif
