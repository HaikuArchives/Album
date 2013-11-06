#ifndef _FILEATTRDIALOG_H_
#define _FILEATTRDIALOG_H_


enum {
	MSG_FILEATTR_WRITE = 'fawr',
	MSG_FILEATTR_REMOVE = 'fadl',
	MSG_FILEATTR_ICONS = 'faic',
	MSG_FILEATTR_THUMBS = 'fath',
};

class BWindow;
class BButton;
class BStatusBar;

class FileAttrDialog : public BWindow
{
	public:
	
	FileAttrDialog(BRect frame, const char *title, BMessage *message);
	virtual void MessageReceived(BMessage *message);

	static void Execute(BPoint where, BMessage *message);
	
	private:
	
	status_t WriteAttributes(BMessage *targets);
	status_t WriteTrackerIcon(BMessage *targets);
	status_t WriteThumbnails(BMessage *targets);
	
	BStatusBar *fProgress;
	
};

#endif	// _FILEATTRDIALOG_H_
