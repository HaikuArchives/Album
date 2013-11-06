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
*/
#ifndef IMAGELOADER_H_
#define IMAGELOADER_H_

#include <Looper.h>
#include <Locker.h>
#include <Entry.h>
#include <Node.h>
#include <Query.h>
#include <String.h>
#include "ObjectList.h"

class BMessage;

enum {
	CMD_LOADER_DELETE = 'ldRm',
	// Replies.
	MSG_LOADER_UPDATE = 'ldUp',
	MSG_LOADER_DONE= 'ldEn',
	MSG_LOADER_DELETED = 'ldDl',
};

enum {
	LOAD_STATS = 1,
	LOAD_ATTRIBUTES = 2,
	LOAD_DATA = 4,
};


/// Node Monitor Cache Item
struct file_item {
	node_ref nodref;
	entry_ref entref;
	file_item(node_ref &node);
	static int node_cmp(const file_item *a, const file_item *b);
	static file_item *ref_eq(file_item *item, void *param);
};


class ImageLoader : public BLooper
{
	public:
	
	ImageLoader(const char *name);
	~ImageLoader();
	virtual void MessageReceived(BMessage *message);
	void Stop();
	bool IsBusy();
	void SetAttrNames(const char *attrnames);
	void SetLoadOptions(uint32 flags);
	void SetThumbnailSize(float width, float height);
	
	virtual status_t HandleRef(entry_ref *ref);
	virtual status_t HandleFile(entry_ref *ref);
	virtual status_t HandleDirectory(entry_ref *ref);
    virtual status_t HandleQuery(const char *predicate, BVolume *volume);
	
	private:

	virtual void RefsReceived(BMessage *message);
	virtual status_t LoadFile(entry_ref *ref, BMessage *reply, uint32 mode = 0xff);
	status_t ReadAttributes(BNode *node, BMessage *attr);
	BBitmap *ReadThumbnail(BNode *node, const char *attrname);
	BBitmap *MakeImagePreview(entry_ref *ref, float width, float height, BRect *originalBounds = NULL);
	void NodeMonitorChange(BMessage *message);
	void DeleteReceived(BMessage *message);
	
	BObjectList<file_item> fItems;
	BLocker fLocker;
	bool fRunning;
	BQuery fQuery;
	int32 fTotal, fDone;
	float fThumbWidth, fThumbHeight;
	BString fReadAttr, fWriteAttr;
	uint32 fLoadOptions;
};

#endif
