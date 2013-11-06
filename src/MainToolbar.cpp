#include "App.h"
#include "MainToolbar.h"
#include <IconButton.h>
#include "iconbitmaps.h"
#include <ProgressBar.h>
#include <Slider.h>
#include <Control.h>
#include <StringView.h>
#include <stdio.h>

MainToolbar::MainToolbar(BRect frame, uint32 resizing):
	LayoutView(frame, "Toolbar", new StripesLayout(B_HORIZONTAL), resizing, B_NAVIGABLE_JUMP)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetFontSize(11);
	
	Layout().SetSpacing(5,5);
	Layout().Frame().OffsetBy(5,0);
	SetDefaultHint(LAYOUT_HINT_VCENTER | LAYOUT_HINT_NO_RESIZE);
	SetPadding(2,0,0,0);
	
	// Children will be automatically laid out	

	BBitmap *icon0 = new BBitmap(BRect(0,0,stop0_png_width,stop0_png_height), B_RGBA32);
	memcpy(icon0->Bits(), stop0_png_bits, stop0_png_size);
	BBitmap *icon1 = new BBitmap(BRect(0,0,stop1_png_width,stop1_png_height), B_RGBA32);
	memcpy(icon1->Bits(), stop1_png_bits, stop1_png_size);
	fStop = new IconButton(BRect(0,0,40,24), "Stop", NULL ,icon0, icon1, new BMessage(MSG_TOOLBAR_STOP),B_FOLLOW_NONE, B_NAVIGABLE);
	fStop->SetEnabled(false);
	AddChild(fStop);
	fProgress = new ProgressBar(BRect(0,0,50,24),"Progress");
	AddChild(fProgress);


	icon0 = new BBitmap(BRect(0,0,remove0_png_width,remove0_png_height), B_RGBA32);
	memcpy(icon0->Bits(), remove0_png_bits, remove0_png_size);
	icon1 = new BBitmap(BRect(0,0,remove1_png_width,remove1_png_height), B_RGBA32);
	memcpy(icon1->Bits(), remove1_png_bits, remove1_png_size);
	fRemove = new IconButton(BRect(0,0,100,24), "Remove", NULL ,icon0, icon1, new BMessage(MSG_TOOLBAR_REMOVE), B_FOLLOW_NONE, B_NAVIGABLE);
	fRemove->SetEnabled(false);
	AddChild(fRemove);

	icon0 = new BBitmap(BRect(0,0,tagcopy0_png_width,tagcopy0_png_height), B_RGBA32);
	memcpy(icon0->Bits(), tagcopy0_png_bits, tagcopy0_png_size);
	icon1 = new BBitmap(BRect(0,0,tagcopy1_png_width,tagcopy1_png_height), B_RGBA32);
	memcpy(icon1->Bits(), tagcopy1_png_bits, tagcopy1_png_size);
	fTagCopy = new IconButton(BRect(0,0,100,24), "Remove", NULL ,icon0, icon1, new BMessage(MSG_TOOLBAR_TAGCOPY),B_FOLLOW_NONE, B_NAVIGABLE);
	fTagCopy->SetEnabled(false);
	AddChild(fTagCopy);

	icon0 = new BBitmap(BRect(0,0,mark0_png_width,mark0_png_height), B_RGBA32);
	memcpy(icon0->Bits(), mark0_png_bits, mark0_png_size);
	icon1 = new BBitmap(BRect(0,0,mark1_png_width,mark1_png_height), B_RGBA32);
	memcpy(icon1->Bits(), mark1_png_bits, mark1_png_size);
	fMark = new IconButton(BRect(0,0,100,24), "Remove", NULL ,icon0, icon1, new BMessage(MSG_TOOLBAR_MARK),B_FOLLOW_NONE, B_NAVIGABLE);
	fMark->SetEnabled(false);
	AddChild(fMark);



    // Magnifier
    fScaler = new BSlider(BRect(0, 0, 100, 24), "Magnify", NULL, new BMessage(MSG_TOOLBAR_ZOOM), 6, 20); 
	fScaler->SetModificationMessage(new BMessage('zoom'));
	fScaler->SetHashMarks(B_HASH_MARKS_BOTH);
	fScaler->SetHashMarkCount(3);
    fScaler->SetValue(20);
    fScaler->SetSnoozeAmount(10000);
	AddChild(fScaler);
}




void MainToolbar::UpdateProgress(float total, float done, const char *status)
{
	char s[8];
	sprintf(s, "%.0f", done);
	fProgress->SetMaxValue(total);
	fProgress->Update(done - fProgress->CurrentValue());
	fProgress->Invalidate();
	fStop->SetEnabled(done);
}


void MainToolbar::SetCounter(int count)
{
	char s[8];
	sprintf(s, "%d", count);
	fProgress->SetText(count > 0 ? s : "");
	fProgress->Invalidate();
}


void MainToolbar::SetSelected(int count)
{
	fRemove->SetEnabled(count);
	if (!count)
		fTagCopy->SetEnabled(count);
	fMark->SetEnabled(count);
}

void MainToolbar::SetAttrSelected(bool enabled)
{
	fTagCopy->SetEnabled(enabled);
}

