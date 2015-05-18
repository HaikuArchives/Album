#ifndef _FILEATTRDIALOG_H_
#define _FILEATTRDIALOG_H_


enum {
	CMD_OP_ATTR_WRITE = 'fop1',
	CMD_OP_ATTR_REMOVE = 'fop2',
	CMD_OP_ICONS = 'fop3',
	CMD_OP_THUMBS = 'fop4',
	CMD_OP_TRASH = 'fop5',
	CMD_OP_RENAME = 'fop6'	
};

class BWindow;
class BButton;
class BStatusBar;

class FileAttrDialog : public BWindow
{
	public:
	
	typedef status_t (*FileOpFunction)(entry_ref*, int32, BMessage*);
	
	
	FileAttrDialog(BRect frame, const char *title);
	static FileAttrDialog* Execute(BRect frame, BMessage *message,BHandler *replyHandler=NULL);
	virtual void MessageReceived(BMessage *message);
	virtual bool QuitRequested();
	
	private:
	
	void ProcessFiles(FileOpFunction operation, BMessage *message);
	static status_t WriteThumbnailOp(entry_ref *ref, int32 i, BMessage *message);
	static status_t WriteTrackerIconOp(entry_ref *ref, int32 i, BMessage *message);
	static status_t WriteAttributesOp(entry_ref *ref, int32 i, BMessage *message);
	static status_t RemoveAttributesOp(entry_ref *ref, int32 i, BMessage *message);
	static status_t MoveToTrashOp(entry_ref *ref, int32 i, BMessage *message);
	static status_t RenameOp(entry_ref *ref, int32 i, BMessage *message);
	
	BStatusBar *fProgress;
};

#endif	// _FILEATTRDIALOG_H_
