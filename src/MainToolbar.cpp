#include <IconButton.h>
#include <ProgressBar.h>
#include <Slider.h>
#include <Control.h>
#include <StringView.h>
#include <TranslationKit.h>
#include <stdio.h>
#include "MainToolbar.h"
#include "App.h"
#include "MainWindow.h"

MainToolbar::MainToolbar(BRect frame, uint32 resizing):
	LayoutView(frame, "Toolbar", new StripesLayout(B_HORIZONTAL), resizing)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	Layout().SetSpacing(5,5);
	Layout().Frame().OffsetBy(5,0);
	SetDefaultHint(LAYOUT_HINT_VCENTER | LAYOUT_HINT_NO_RESIZE);
	SetPadding(2,0,0,0);
	
	BBitmap *icon0 = BTranslationUtils::GetBitmap('PNG ', "tb-stop0");
	BBitmap *icon1 = BTranslationUtils::GetBitmap('PNG ', "tb-stop1");
	fStop = new IconButton(BRect(0,0,40,24), NULL, S_STOP_SUB, icon0, icon1, new BMessage(MSG_TOOLBAR_STOP),B_FOLLOW_NONE);
	fStop->SetEnabled(false);
#ifdef __HAIKU__    
	fStop->SetToolTip(S_STOP_TIP);
#endif
	AddChild(fStop);

	fProgress = new ProgressBar(BRect(0,0,50,24), NULL);
#ifdef __HAIKU__    
	fProgress->SetToolTip(S_PROGRESS_TIP);
#endif
	AddChild(fProgress);

	icon0 = BTranslationUtils::GetBitmap('PNG ', "tb-remove0");
	icon1 = BTranslationUtils::GetBitmap('PNG ', "tb-remove1");
	fRemove = new IconButton(BRect(0,0,100,24), NULL, S_REMOVE_SUB, icon0, icon1, new BMessage(CMD_ITEM_REMOVE), B_FOLLOW_NONE);
	fRemove->SetEnabled(false);
#ifdef __HAIKU__    
	fRemove->SetToolTip(S_REMOVE_TIP);
#endif
	AddChild(fRemove);

	icon0 = BTranslationUtils::GetBitmap('PNG ', "tb-tagcopy0");
	icon1 = BTranslationUtils::GetBitmap('PNG ', "tb-tagcopy1");
	fTagCopy = new IconButton(BRect(0,0,100,24), NULL, S_COPYTAG_SUB, icon0, icon1, new BMessage(CMD_TAG_COPY),B_FOLLOW_NONE);
	fTagCopy->SetEnabled(false);
#ifdef __HAIKU__    
	fTagCopy->SetToolTip(S_COPYTAG_TIP);
#endif
	AddChild(fTagCopy);

	icon0 = BTranslationUtils::GetBitmap('PNG ', "tb-mark0");
	icon1 = BTranslationUtils::GetBitmap('PNG ', "tb-mark1");	
	fMark = new IconButton(BRect(0,0,100,24), NULL, S_MARK_SUB, icon0, icon1, new BMessage(CMD_ITEM_MARK),B_FOLLOW_NONE);
	fMark->SetEnabled(false);
#ifdef __HAIKU__    
	fMark->SetToolTip(S_MARK_TIP);
#endif
	AddChild(fMark);

    // Magnifier
    fScaler = new BSlider(BRect(0, 0, 100, 24), "Magnify", NULL, new BMessage(MSG_TOOLBAR_ZOOM), 6, 20); 
	fScaler->SetModificationMessage(new BMessage('zoom'));
	fScaler->SetHashMarks(B_HASH_MARKS_BOTH);
	fScaler->SetHashMarkCount(3);
    fScaler->SetValue(20);
    fScaler->SetSnoozeAmount(10000);
#ifdef __HAIKU__    
	fScaler->SetToolTip(S_ZOOM_TIP);
#endif
	AddChild(fScaler);
}




void MainToolbar::UpdateProgress(float total, float done, const char *status)
{
	char s[8];
	sprintf(s, "%.0f", done);
	fProgress->SetMaxValue(total);
	fProgress->Update(done - fProgress->CurrentValue());
	fProgress->Invalidate();
	fStop->SetEnabled(done > 0 || status != NULL);
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

