#ifndef _FILENAMEDIALOG_H_
#define _FILENAMEDIALOG_H_


enum {
	MSG_FILEENTRY_RENAME = 'rena',
	MSG_FILEENTRY_TRASH = 'm2tr',
};

class BWindow;
class BButton;
class BStatusBar;

class FileNameDialog : public BWindow
{
	public:
	
	FileNameDialog(BRect frame, const char *title, BMessage *message);

	virtual void MessageReceived(BMessage *message);
	void WriteName(BMessage *targets);
	void MoveToTrash(BMessage *targets);
	static void Execute(BPoint where, BMessage *message);
	
	private:
		
	BStatusBar *fProgress;

};


#endif	// _FILENAMEDIALOG_H_
