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
#include <Entry.h>
#include <Node.h>
#include <Locker.h>
#include <String.h>

#include <Query.h>
#include "ObjectList.h"

#define IMAGELOADER_CACHE_LIMIT 4096

enum {
	CMD_LOADER_DELETE = 'ldRm',
	// Replies.
	MSG_LOADER_UPDATE = 'ldUp',
	MSG_LOADER_DONE= 'ldDn',
	MSG_LOADER_DONE_BUT_RUNNING = 'ldDR',
	MSG_LOADER_DELETED = 'ldDl',
};


enum {
	LOADER_READ_TAGS = 1,
	LOADER_READ_EXIF_THUMB = 2,
	LOADER_RELOAD_EXISTING = 4,
	LOADER_ONLY_IMAGES = 8
};

#define MAX_QUERIES 10

/// Node Monitor Cache Item
struct file_item {
	node_ref nodref;
	entry_ref entref;
	file_item(node_ref &node);
};


class ImageLoader : public BLooper
{
	public:
	
	ImageLoader(const char *name);
	~ImageLoader();
	virtual void MessageReceived(BMessage *message);
	void Stop();
	bool IsRunning();
	void SetAttrNames(const char *attrnames);
	void SetLoadOptions(uint32 flags);
	void SetThumbnailSize(float width, float height);
	virtual void RefsReceived(BMessage *message);
	virtual void DeleteReceived(BMessage *message);	
	static BBitmap *ReadImagePreview(entry_ref *ref, float width, float height, BRect *originalBounds = NULL);
	static BBitmap *ReadThumbnail(BNode *node, const char *attrname);
	
	private:

	void NodeMonitorChange(BMessage *message);
	bool AddCacheItem(entry_ref &ref, node_ref &noderef);
	bool RemoveCacheItem(entry_ref *ref);
	status_t HandleRef(entry_ref *ref);
	status_t HandleFile(entry_ref *ref);
	status_t HandleDirectory(entry_ref *ref);
	status_t HandleTrackerQuery(BNode *node);
	//status_t LoadFile(entry_ref *ref, BMessage *reply, uint32 mode = 0xff);
	status_t ReadData(entry_ref *ref, BMessage *reply);
	status_t ReadStats(entry_ref *ref, BMessage *reply);
	status_t ReadAttributes(BNode *node, BMessage *reply);
	
	BObjectList<file_item> fItems;
	BObjectList<BQuery> fQueries;
	bool fRunning;
	BLocker fStopLocker;
	uint32 fLoadOptions;
	int32 fTotal, fDone;
	float fThumbWidth, fThumbHeight;
	BString fReadAttr, fWriteAttr;
	
};

#endif
