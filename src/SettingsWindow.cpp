/**
Copyright (c) 2006-2008 by Matjaz Kovac

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

\file SettingsWindow.cpp
\author Matjaž Kovač
\brief User Preferences Dialog
*/

#include <NameValueItem.h>
#include <stdio.h>
#include "App.h"
#include "SettingsWindow.h"
#include "ImageLoader.h"

SettingsWindow::SettingsWindow(BRect frame, const char *title):
        BWindow(frame, title, B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
                B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	// Container 
    BView *root = new BView(Bounds(), "", B_FOLLOW_ALL_SIDES, 0);
    root->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Font Sensitivity
    font_height fh;
    root->GetFontHeight(&fh);
    float h = 2*fh.ascent + fh.descent + fh.leading;
	BRect b = root->Bounds().InsetBySelf(10, 10);
	
    // Attributes Menu
    b.bottom = b.top + h;
    fReadAttr = new BTextControl(b, NULL, _("Thumbnail attributes"), "IPRO:thumbnail GRAFX:Thumbnail", NULL);
    fReadAttr->SetDivider(root->StringWidth(fReadAttr->Label()) + 8);
#ifdef __HAIKU__    
	fReadAttr->SetToolTip(S_THUMBNAIL_NAME_TIP);
#endif    
    root->AddChild(fReadAttr);

    // Thumbnail Size Menu
    fSizeMenu = new BPopUpMenu(_("Default Size"));
    fSizeMenu->ResizeToPreferred();
    fSizeMenu->AddItem(new BMenuItem(_("Small"), NULL));
    fSizeMenu->AddItem(new BMenuItem(_("Medium"), NULL));
    fSizeMenu->AddItem(new BMenuItem(_("Large"), NULL));
    fSizeMenu->AddItem(new BMenuItem(_("XLarge"), NULL));
    fSizeMenu->SetTargetForItems(this);
    fSizeMenu->ResizeToPreferred();
    b.OffsetBy(0, h);
	BMenuField *menuField = new BMenuField(b, NULL, _("Size for new thumbnails"), fSizeMenu);
#ifdef __HAIKU__    
	menuField->SetToolTip(S_THUMBNAIL_SIZE_TIP);
#endif
	menuField->SetDivider(root->StringWidth(menuField->Label()) + 8);
    menuField->ResizeToPreferred();
    root->AddChild(menuField);

    fFormatMenu = new BPopUpMenu(_("Default Format"));
    fFormatMenu->AddItem(new BMenuItem("JPEG", NULL));
    fFormatMenu->AddItem(new BMenuItem("PNG", NULL));
    fFormatMenu->AddItem(new BMenuItem("TGA", NULL));
    fFormatMenu->AddItem(new BMenuItem("BMP", NULL));
    fFormatMenu->AddItem(new BMenuItem("TIFF", NULL));
    fFormatMenu->AddItem(new BMenuItem("GIF", NULL));
    fFormatMenu->SetTargetForItems(this);
    fFormatMenu->ResizeToPreferred();
    b.OffsetBy(0, h);
	menuField = new BMenuField(b, NULL, _("Write thumbnails to attributes as"), fFormatMenu);
	menuField->SetDivider(root->StringWidth(menuField->Label()) + 8);
	menuField->ResizeToPreferred();
    root->AddChild(menuField);
	

	// Extract Tags
	b.OffsetBy(0, h);
    fExtractTags = new BCheckBox(b, NULL, _("Read JPEG metadata"), new BMessage(MSG_EXTRACTTAGS_CHECK));
    fExtractTags->ResizeToPreferred();
	root->AddChild(fExtractTags);

	b.OffsetBy(0, h);
    fExifThumb = new BCheckBox(b, NULL, _("Read EXIF thumbnails"), NULL);
    fExifThumb->ResizeToPreferred();
	root->AddChild(fExifThumb);

	b.OffsetBy(0, h);
    fReloadExisting = new BCheckBox(b, NULL, _("Reload existing"), NULL);
    fReloadExisting->ResizeToPreferred();
#ifdef __HAIKU__    
    fReloadExisting->SetToolTip(S_RELOAD_TIP);
#endif
	root->AddChild(fReloadExisting);

	b.OffsetBy(0, h);
    fOnlyImages = new BCheckBox(b, NULL, _("Load only images"), NULL);
    fOnlyImages->ResizeToPreferred();
	root->AddChild(fOnlyImages);

	// Display Options
	b.OffsetBy(0, h);
    fAntiFlicker = new BCheckBox(b, NULL, _("Reduce flickering"), NULL);
    fAntiFlicker->ResizeToPreferred();
#ifdef __HAIKU__    
    fAntiFlicker->SetToolTip(S_FLICKER_TIP);
#endif
	root->AddChild(fAntiFlicker);

    // Apply Button
	b.OffsetBy(0, 1.5*h);
    fApplyButton = new BButton(b, NULL, _("Save"), new BMessage(CMD_DONE));
	fApplyButton->ResizeToPreferred();
    root->AddChild(fApplyButton);
	fApplyButton->MakeDefault(true);
    AddChild(root);
    
    ResizeTo(Frame().Width(),b.bottom+10);
}


/**
	Closes the window.
	Note that this window *must* be Quit() manually!
*/
bool SettingsWindow::QuitRequested()
{
	if (IsHidden()) 
		return true;
    Hide();
    return false;
}


void SettingsWindow::MessageReceived(BMessage *message)
{
    switch (message->what) {
	    case MSG_PREFS_CHANGED:
	        Restore(message);
	        break;
		case MSG_EXTRACTTAGS_CHECK:
			fExifThumb->SetEnabled(fExtractTags->Value());
			break;	        
	    case CMD_DONE:
	    	Save();
	        Hide();
	        break;
	    default:
	        BWindow::MessageReceived(message);
	        break;
    }
}



/**
	Get anything useful from the passed message
	and update controls.
*/
void SettingsWindow::Restore(BMessage *message)
{
	// Thumbnail attributes, space-separated.
    const char *val;
    if (message->FindString("thumb_attr", &val) == B_OK) {
        fReadAttr->SetText(val);
    }

    // Thumbnail size category - kind of lame.
    float width = 0, height = 0;
    if (message->FindFloat("thumb_width", &width) == B_OK && message->FindFloat("thumb_height", &height) == B_OK) {
		int i;
		if (width >= 240)
			i = 3;
		else if (width >= 160)
			i = 2;
		else if (width >= 120)
			i = 1;
		else 
			i = 0;
		BMenuItem *item = fSizeMenu->ItemAt(i);
		if (item)
			item->SetMarked(true);
    }
   	
   	// Thumbnail format 
    if (message->FindString("thumb_format", &val) == B_OK) {
        BMenuItem *item = fFormatMenu->FindItem(val);
        if (item)
            item->SetMarked(true);
    }
    
    // Read Mode
    int32 options;
    if (message->FindInt32("load_options", &options) == B_OK) {
		if (options & LOADER_READ_TAGS)
        	fExtractTags->SetValue(1);
		if (options & LOADER_READ_EXIF_THUMB)
        	fExifThumb->SetValue(1);
		if (options & LOADER_RELOAD_EXISTING)
        	fReloadExisting->SetValue(1);
		if (options & LOADER_ONLY_IMAGES)
        	fOnlyImages->SetValue(1);
    }
	fExifThumb->SetEnabled(fExtractTags->Value() == 1);
	
    if (message->FindInt32("display_options", &options) == B_OK) {
		if (options & 1)
        	fAntiFlicker->SetValue(1);
    }
        
}


/**
	Packs all settings into a message and sends it.
*/
void SettingsWindow::Save()
{
	BMessage msg(CMD_UPDATE_PREFS);

	// Thumbnail attributes, space-separated.
	msg.AddString("thumb_attr", fReadAttr->Text());
	
    // Thumbnail size category - kind of lame.
	BMenuItem *marked;
    if ((marked = fSizeMenu->FindMarked())) {
		float width, height;
    	int i = fSizeMenu->IndexOf(marked);
    	if (i >= 3)
    		width = height = 240;
    	else if (i == 2)
    		width = height = 160;
    	else if (i == 1)
    		width = height = 120;
    	else
    		width = height = 64;
		msg.AddFloat("thumb_width", width);
		msg.AddFloat("thumb_height", height);
	}
   	// Thumbnail format 
	if ((marked = fFormatMenu->FindMarked())) 
		msg.AddString("thumb_format", marked->Label());

	// Read Mode
	int32 options = 0;
	if (fExtractTags->Value())
		options |= LOADER_READ_TAGS;
	if (fExifThumb->Value())
		options |= LOADER_READ_EXIF_THUMB;
	if (fReloadExisting->Value())
		options |= LOADER_RELOAD_EXISTING;
	if (fOnlyImages->Value())
		options |= LOADER_ONLY_IMAGES;
	msg.AddInt32("load_options", options);

	options = fAntiFlicker->Value();
	msg.AddInt32("display_options", options);

    be_app->PostMessage(&msg);
}


