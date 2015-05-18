#include <Window.h>
#include <SplitView.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <StringView.h>
#include <String.h>
#include <NameValueItem.h>
#include <EditableListView.h>
#include <MenuItem.h>
#include <RegExp.h>
#include <PopUpMenu.h>
#include "MainSidebar.h"
#include "MainWindow.h"
#include "App.h"


/**
	Custom editable list view for JPEG tags.
*/
class TagList: public EditableListView
{
	public:
	
	TagList(BRect frame, uint32 resizing);
	virtual bool InitiateDrag(BPoint point, int32 index, bool wasSelected);
};


/**
	Custom editable list view for BFS attributes.
*/
class AttributeList: public EditableListView
{
	public:
	
	AttributeList(BRect frame, uint32 resizing);
	virtual void MessageReceived(BMessage *message);
	virtual void MouseDown(BPoint where);
	virtual BView* NewEditBox(int32 index);
	virtual bool ConfirmEdit(BMessage *message);

	private:

	void ShowContextMenu(BPoint where);
};





TagList::TagList(BRect frame, uint32 resizing):
	EditableListView(frame, "Tags", B_MULTIPLE_SELECTION_LIST, resizing, B_FRAME_EVENTS)
{
}

bool TagList::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	NameValueItem *item = dynamic_cast<NameValueItem*>(ItemAt(index));
	// Dragging selected items reqires two clicks!
	if (item /*&& wasSelected*/ ) {
		BMessage msg(MSG_TAG_DRAGGED);
		msg.AddString("name", item->Name());
		msg.AddString("value", item->Value());
		DragMessage(&msg, ItemFrame(index));
		return true;		
	}
	return false;
}




AttributeList::AttributeList(BRect frame, uint32 resizing):
	EditableListView(frame, "Attributes", B_MULTIPLE_SELECTION_LIST, resizing, B_FRAME_EVENTS)
{
}


void AttributeList::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case MSG_TAG_DRAGGED:
			// a bit of back'n'forth here...
			message->what = CMD_TAG_COPY;
			// makes a copy, so no worries
			Looper()->PostMessage(message);
			break;
		default:
			EditableListView::MessageReceived(message);
	}
}

void AttributeList::MouseDown(BPoint where)
{
    // This is an event hook so there must be a Looper.
    BMessage *message = Looper()->CurrentMessage();
    int32 buttons = 0;
	message->FindInt32("buttons", &buttons);    
	// Invoke the context menu, if over an item.
	int32 i = IndexOf(where);
	if (i >=0 && buttons & B_SECONDARY_MOUSE_BUTTON) {
		// Select if none selected.
		if (!ItemAt(i)->IsSelected())
			BListView::MouseDown(where);
	    ShowContextMenu(where);
    }
    else
			BListView::MouseDown(where);
    
}





/**
	Starts edit.
*/
BView* AttributeList::NewEditBox(int32 index)
{
	NameValueItem *item = dynamic_cast<NameValueItem*>(ItemAt(index));
	if (item) {
		if (item->IsReadOnly())
			return NULL;
		BRect frame = ItemFrame(index);
		frame.left += item->Divider();
		return new ListViewEditBox(frame, "EditBox", item->Value(), B_FOLLOW_LEFT_RIGHT);
	}
	else
		// not editable
		return NULL;
}

/**
	OKs an edit operation.
*/
bool AttributeList::ConfirmEdit(BMessage *message)
{
	BTextView *text = dynamic_cast<BTextView*>(EditBox());
	NameValueItem *item = dynamic_cast<NameValueItem*>(ItemAt(CurrentSelection()));
	if (item && text) {
		message->AddString("name", item->Name());
		switch (item->Type()) {
			case B_STRING_TYPE:
				message->AddString("value", text->Text());
				return true;
			default:
				return false;
		}			
	}
	return false;
}

void AttributeList::ShowContextMenu(BPoint where)
{
	    BPopUpMenu *fPopUp = new BPopUpMenu("AttributeOptions");
	    if (CurrentSelection() >= 0) {
		    //fPopUp->AddSeparatorItem();
		    fPopUp->AddItem(new BMenuItem(_("Delete"), new BMessage(CMD_ATTR_REMOVE)));
	    }
	    // Invoke the popupmenu
	    fPopUp->SetTargetForItems(this);
		// Must be in async mode for this to work:
		fPopUp->SetAsyncAutoDestruct(true);
		ConvertToScreen(&where);
		// Post messages, async mode - Go() returns ASAP.
	    fPopUp->Go(where, true, false, true);
}




/**
   Copy 's0' to 'out', modulo middle bytes that do not match those in 's1'.
   Mismatched chars are replaced with a single '*' wildcard.
   The idea is to have a filename matching both sources - shell style.
*/
int merge_filenames(const char *s0, const char *s1, char *out)
{
    int n0 = strlen(s0);
    int n1 = strlen(s1);
    // find matching limits
    int li = 0;
    for (li = 0; li < n0 && li < n1 && s0[li] == s1[li]; li++);
    int ri0 = n0-1;
    int ri1 = n1-1;
    while (ri0 >= li && ri1 >= li && s0[ri0] == s1[ri1]) {
        if (ri1 >= 0)
            ri1--;
        if (ri0 >= 0)
            ri0--;
    }
    // copy to dest
    if (n0 == 0)
        strcpy(out, s1);
    else if (li < n0) {
        int di = 0;
        int i;
        for (i = 0; i < li; out[di++] = s0[i++]);
        out[di++] = '*';
        for (i = ri0; i < n0; out[di++] = s0[++i]);
        out[di] = 0;
    }
    else
        strcpy(out, s0);
    return 0;
}


/**
	Binary search func.
*/
int cmp_name(const NameValueItem *a, const NameValueItem *b)
{
	return strcmp(b->Name(), a->Name());
}


/**
	Copies message fields.
*/
void AddFields(BObjectList<NameValueItem> *fields, BMessage *source)
{
	char *name;
	type_code type;
	for (int i = 0; source->GetInfo(B_ANY_TYPE, i, &name, &type) == B_OK; i++) {
		const void *data;
		ssize_t size;
		for (int i = 0; source->FindData(name, type, i, &data, &size) == B_OK; i++) {
			NameValueItem newitem(name, type, data, size);
			// Only a few datatypes are supported.
			newitem.SetReadOnly(type != B_STRING_TYPE);
			NameValueItem *item = (NameValueItem*)fields->BinarySearch(newitem, cmp_name);
			if (item) 
				item->SetValue(newitem.Value(), true);
			else
				fields->BinaryInsertCopyUnique(newitem, cmp_name);
		}
	}
}



/**
	Displays stored fields.
	Optional group outline is stored in 'groups' and is preserved across updates.
*/
void UpdateList(BOutlineListView *list, BObjectList<NameValueItem> *fields, BObjectList<NameValueItem> *groups)
{
	RegExp re("(\\w+):(.*)");
	BString prefix, oldprefix;

	NameValueItem *item, *group = NULL;
	for (int i = 0; (item = fields->ItemAt(i)); i++) {
		if (groups && re.Match(item->Name()) == 0) {
			// add/find the group
			group = groups->BinaryInsertCopyUnique(NameValueItem(re.Sub(1, prefix)), cmp_name);
			// requires names to be sorted
			if (prefix != oldprefix)
				list->AddItem(group);
			list->AddUnder(item, group);
			oldprefix = prefix;
			group->SetReadOnly(true);
			item->SetLabel(re.Sub(2, prefix));
		}
		else
			list->AddItem(item, 0);		
	}
}


/**
	Packs selected list items into a message.
*/
void GetSelectedFields(BOutlineListView *list, BMessage *fields)
{
	int32 selected;
	int32 i = 0;
	while ((selected = list->FullListCurrentSelection(i++)) >= 0) {
		NameValueItem* item = dynamic_cast<NameValueItem*>(list->ItemAt(selected));
		if (item)
			fields->AddString(item->Name(), item->Value());
	}
}





MainSidebar::MainSidebar(BRect frame, AlbumView* main, uint32 resizing):
	BView(frame, "Sidebar", resizing, B_PULSE_NEEDED),
	fMain(main),
	fTags(50, true),
	fAttrs(20, true),
	fGroups(5, true)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Font Sensitivity
    font_height fh;
    GetFontHeight(&fh);
    float h = 1.25*fh.ascent + fh.descent + fh.leading;
	
	BRect rect(4,2,frame.Width()-2, h+4);
	fFName = new BTextControl(rect, "FileName", _("Name"), "", new BMessage(MSG_NAME_CHANGED), B_FOLLOW_LEFT_RIGHT);
	fFName->SetDivider(StringWidth(fFName->Label())+6);
#ifdef __HAIKU__
	fFName->SetToolTip(S_RENAME_TIP);
#endif	
	AddChild(fFName);
	
	rect.OffsetBy(0,h+6);
	fFSize = new BStringView(rect,"FileSize", NULL);
	AddChild(fFSize);

	rect.OffsetBy(0,h);
	fCTime = new BStringView(rect,"CTime", NULL);
	AddChild(fCTime);

	rect.OffsetBy(0,h);
	fMTime = new BStringView(rect,"MTime", NULL);
	AddChild(fMTime);
	
	BRect r = Bounds();
	r.SetLeftTop(BPoint(0,rect.bottom + 2));
	SplitView* view = new SplitView(r, NULL, B_VERTICAL, B_FOLLOW_ALL_SIDES);
		
	rect = view->Bounds();
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	rect.bottom *= 0.6;

	fTagList = new TagList(rect, B_FOLLOW_ALL);
	fTagList->SetSelectionMessage(new BMessage(MSG_TAG_SELECTED));
#ifdef __HAIKU__    
	fTagList->SetToolTip(S_TAGS_TIP);
#endif	
	view->AddChild(new BScrollView(NULL, fTagList, B_FOLLOW_LEFT_RIGHT, 0 , false, true, B_PLAIN_BORDER));

	fAttrList = new AttributeList(rect, B_FOLLOW_ALL);
	fAttrList->SetInvocationMessage(new BMessage(MSG_ATTR_CHANGED));
#ifdef __HAIKU__    
	fAttrList->SetToolTip(S_ATTRS_TIP);
#endif
	view->AddChild(new BScrollView(NULL, fAttrList, B_FOLLOW_ALL, 0 , false, true, B_PLAIN_BORDER));
	
	AddChild(view);
	
	Update();
}




/**
	Shows pending updates.
*/
void MainSidebar::Pulse()
{
	if (fUpdateStats || fUpdateTags || fUpdateAttrs) {
		Window()->DisableUpdates();
		ShowSummary();
		Window()->EnableUpdates();
	}
}



/**
	Removes all content.
*/
void MainSidebar::Clear()
{
	if (fUpdateStats) {
		fFName->SetEnabled(false);
	}
	if (fUpdateTags) {
		fTagList->MakeEmpty();
		fTags.MakeEmpty();
	}
	if (fUpdateAttrs) {
		fAttrList->MakeEmpty();	
		fAttrs.MakeEmpty();
	}
}



/**
	Calculates selection summary.
*/
int MainSidebar::ShowSummary()
{

	char fname[B_FILE_NAME_LENGTH] = "";	
	off_t fsize = 0;
	time_t ctime = 0, mtime = 0;
		
	Clear();
	int count = 0;
	for (int i = 0; i < fMain->CountItems(); i++) {
		AlbumFileItem *item = dynamic_cast<AlbumFileItem*>(fMain->ItemAt(i));
		if (!fMain->IsItemVisible(item) || !item->IsSelected())
			continue;
		if (fUpdateStats) {
			BString s = fname;
			merge_filenames(s.String(), item->Ref().name, fname);
			// min,max times
			fsize += item->fFSize;
			if (item->fCTime > ctime)
				ctime = item->fCTime;
			if (item->fMTime > mtime)
				mtime = item->fMTime;
		}
		if (fUpdateTags)
			AddFields(&fTags, &item->fTags);
		if (fUpdateAttrs)
			AddFields(&fAttrs, &item->fAttributes);
		count++;
	}
	
	if (fUpdateStats) {
		char s[32];

		fFName->SetText(fname);
		fFName->SetEnabled(count > 0);
		
		BString sizeText(_("Size "));	
		App::FormatFileSize(s, sizeof(s)-1, (long)fsize);
		sizeText += s;
		fFSize->SetText(sizeText.String());

		BString createdText(_("Created "));
		if (ctime) {
			App::FormatTimestamp(s, sizeof(s)-1, ctime);
			createdText += s;
		}
		fCTime->SetText(createdText.String());
		
		BString modifiedText(_("Modified "));
		if (mtime) {
			App::FormatTimestamp(s, sizeof(s)-1, mtime);
			modifiedText += s;
		}
		fMTime->SetText(modifiedText.String());

		fUpdateStats = false;
		
	}

	if (fUpdateTags) {
		UpdateList(fTagList, &fTags, &fGroups);
		fUpdateTags = false;
	}

	if (fUpdateAttrs) {
		UpdateList(fAttrList, &fAttrs, NULL);
		fUpdateAttrs = false;
	}

	return count;
}



void MainSidebar::Update(uint32 mask)
{
	if (mask & UPDATE_STATS)
		fUpdateStats = true;
	if (mask & UPDATE_TAGS)
		fUpdateTags = true;
	if (mask & UPDATE_ATTRS)
		fUpdateAttrs = true;
}


void MainSidebar::GetSelectedTags(BMessage *tags)
{
	GetSelectedFields(fTagList, tags);
}


void MainSidebar::GetSelectedAttrs(BMessage *attrs)
{
	GetSelectedFields(fAttrList, attrs);
}


