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

#include "App.h"
#include "SettingsWindow.h"

#include <stdio.h>
#include <NameValueItem.h>

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
    float h = fh.ascent + fh.descent + fh.leading;
	BRect b = root->Bounds().InsetBySelf(12, 12);
	
    // Attributes Menu
    b.bottom = b.top + h;
    fReadAttr = new BTextControl(b, "Attributes", _("Thumbnail attributes"), "IPRO:thumbnail GRAFX:Thumbnail", NULL);
    fReadAttr->SetDivider(root->StringWidth(fReadAttr->Label()) + 8);
    root->AddChild(fReadAttr);

    // Thumbnail Size Menu
    fSizeMenu = new BPopUpMenu(_("Sizes"));
    fSizeMenu->AddItem(new BMenuItem("32x32", NULL));
    fSizeMenu->AddItem(new BMenuItem("56x48", NULL));
    fSizeMenu->AddItem(new BMenuItem("64x64", NULL));
    fSizeMenu->AddItem(new BMenuItem("96x64", NULL));
    fSizeMenu->AddItem(new BMenuItem("128x96", NULL));
    fSizeMenu->AddItem(new BMenuItem("200x160", NULL));
    fSizeMenu->SetTargetForItems(this);
    b.OffsetBy(0, 2*h);
	BMenuField *menuField = new BMenuField(b, "Dimensions", _("Size for new thumbnails"), fSizeMenu);
	menuField->SetDivider(root->StringWidth(menuField->Label()) + 8);
    root->AddChild(menuField);

    fFormatMenu = new BPopUpMenu(_("Format"));
    fFormatMenu->AddItem(new BMenuItem("GIF", NULL));
    fFormatMenu->AddItem(new BMenuItem("JPEG", NULL));
    fFormatMenu->AddItem(new BMenuItem("PNG", NULL));
    fFormatMenu->AddItem(new BMenuItem("PPM", NULL));
    fFormatMenu->AddItem(new BMenuItem("TGA", NULL));
    fFormatMenu->AddItem(new BMenuItem("BMP", NULL));
    fFormatMenu->AddItem(new BMenuItem("TIFF", NULL));
    fFormatMenu->SetTargetForItems(this);
    b.OffsetBy(0, 2*h);
	menuField = new BMenuField(b, "Format", _("Write thumbnails to attributes as"), fFormatMenu);
	menuField->SetDivider(root->StringWidth(menuField->Label()) + 8);
    root->AddChild(menuField);
	

	// Extract Tags
	b.OffsetBy(0, 2 * h);
    fExtractTags = new BCheckBox(b, "ExtractTags", _("Read JPEG metadata"), new BMessage(MSG_EXTRACTTAGS_CHECK));
	root->AddChild(fExtractTags);

	b.OffsetBy(0, 2 * h);
    fExifThumb = new BCheckBox(b, "ExifThumb", _("Read EXIF thumbnail"), NULL);
	root->AddChild(fExifThumb);

	// Display Options
	b.OffsetBy(0, 2 * h);
    fAntiFlicker = new BCheckBox(b, "AntiFlicker", _("Reduce flickering"), NULL);
	root->AddChild(fAntiFlicker);

    // Apply Button
	b.OffsetTo(b.right-80, root->Bounds().Height() - 3 * h);
    fApplyButton = new BButton(b, "OkButton", _("Save"), new BMessage(CMD_DONE));
	fApplyButton->ResizeToPreferred();
    root->AddChild(fApplyButton);
	fApplyButton->MakeDefault(true);

    AddChild(root);
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
	message->PrintToStream();
    // Attribute names.
    const char *val;
    if (message->FindString("thumb_attr", &val) == B_OK) {
        fReadAttr->SetText(val);
    }
    // Mark an item with the right label - kind of lame.
    float width = 0, height = 0;
    if (message->FindFloat("thumb_width", &width) == B_OK && message->FindFloat("thumb_height", &height) == B_OK) {
        char label[16] = "";
        sprintf(label, "%0.0fx%0.0f", width, height);
        BMenuItem *item = fSizeMenu->FindItem(label);
        if (item)
            item->SetMarked(true);
    }
    if (message->FindString("thumb_format", &val) == B_OK) {
        BMenuItem *item = fFormatMenu->FindItem(val);
        if (item)
            item->SetMarked(true);
    }
    
    // Read Mode
    int32 options = 0;
    if (message->FindInt32("load_options", &options) == B_OK) {
		if (options & 1)
        	fExtractTags->SetValue(1);
		if (options & 2)
        	fExifThumb->SetValue(1);
    }
	fExifThumb->SetEnabled(fExtractTags->Value());
	
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
	// Read Attributes
	msg.AddString("thumb_attr", fReadAttr->Text());
	// Thumbnail Dimensions (from labels, kind of lame).
	float width = 0, height = 0;
	if (fSizeMenu->FindMarked()) {
	    if (sscanf(fSizeMenu->FindMarked()->Label(), "%fx%f", &width, &height) == 2) {
	        msg.AddFloat("thumb_width", width);
	        msg.AddFloat("thumb_height", height);
	    }
	}
	if (fFormatMenu->FindMarked()) {
	        msg.AddString("thumb_format", fFormatMenu->FindMarked()->Label());
	}

	// Read Mode
	int32 options = 0;
	if (fExtractTags->Value())
		options += 1;
	if (fExifThumb->Value())
		options += 2;
	msg.AddInt32("load_options", options);

	options = fAntiFlicker->Value();
	msg.AddInt32("display_options", options);

    be_app->PostMessage(&msg);
}


