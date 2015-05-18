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

\file ImageLoader.cpp
\brief Image Processor

This scheme employs the Node Monitor and the notifier-observer mechanism to
keep the client views informed on the state of the file set.

Therefore observer handlers should register with ImageLoader::StartWatchingAll(observer).

Failing to handle notices is a Bad Thing because a removed file
will result in an invalid reference.

Another neat trick is to combine the Node Monitor with Live Queries. 
The BeOS API makes it really simple as the message protocols are
essentially the same.  That way any client view can reflect 
a criterion set by a query, adding and removing files automatically as 
they come in and out of the scope of a query predicate.
*/

#define DEBUG 1
#include <Debug.h>
#include <Autolock.h>
#include <String.h>
#include <Directory.h>
#include <fs_attr.h>
#include <NodeInfo.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <NodeMonitor.h>
#include <TranslationKit.h>
#include <Bitmap.h>
#include <View.h>
#include "ImageLoader.h"
#include "JpegTagExtractor.h"

#define TRACKER_QUERY_STR_ATTR "_trk/qrystr"
#define TRACKER_QUERY_VOL_ATTR "_trk/qryvol1"

/**
	Creates a new node cache item.
*/
file_item::file_item(node_ref &node):
	nodref(node)	
{
}


/// BObjectList compare function
int node_cmp(const file_item *a, const file_item *b) 
{
	if (a->nodref.device == b->nodref.device)
		return a->nodref.node - b->nodref.node;
	else
		return a->nodref.device - b->nodref.device;
}

/// BObjectList compare function
file_item* ref_eq(file_item *item, void *param)
{
	return item->entref == *(entry_ref*)param ? item : NULL;
}


/// BObjectList search  function
BQuery* live_query_tst(BQuery *item, void *param)
{
	return item->IsLive() ? item : 0;
}


/**
	Constructs a new message-driven image loader.
	Since it does some heavy-duty image processing it is best
	to keep it low priority to improve the overall responsivness.
*/
ImageLoader::ImageLoader(const char *name):
	BLooper(name, B_NORMAL_PRIORITY),
	fItems(32, true),
	fQueries(8, true),
	fRunning(false),
	fLoadOptions(LOADER_READ_TAGS),
	fThumbWidth(64),
	fThumbHeight(64),
	fReadAttr("IPRO:thumbnail")
{
}


ImageLoader::~ImageLoader()
{
    Stop();
    stop_watching(this);
	PRINT(("%s deleted.\n", Name()));
}


/**
	Stops image processing ASAP.
	/warning Do not call from within ImageLooper handlers.
*/
void ImageLoader::Stop()
{
	BAutolock lock(fStopLocker);
	fRunning = false;
	fQueries.MakeEmpty();
	SendNotices(MSG_LOADER_DONE);
	PRINT(("Stop.\n"));
}

bool ImageLoader::IsRunning()
{
	BAutolock lock(fStopLocker);
	return fRunning;
}


void ImageLoader::SetAttrNames(const char *attrnames)
{
	fReadAttr = attrnames;
}


void ImageLoader::SetLoadOptions(uint32 flags)
{
	fLoadOptions = flags;
}



void ImageLoader::SetThumbnailSize(float width, float height)
{
	fThumbWidth = width;
	fThumbHeight = height;
}



void ImageLoader::MessageReceived(BMessage *message)
{
	
 	switch (message->what){
  		case B_SIMPLE_DATA:
  			RefsReceived(message);
   			break;
    	case B_NODE_MONITOR:
    	case B_QUERY_UPDATE:
			NodeMonitorChange(message);
	        break;
 		case CMD_LOADER_DELETE:
			DeleteReceived(message);
 			break;
   		default:
   			BLooper::MessageReceived(message);
   			break;
	}
}


/**
	Loads images and creates new items.

	Processes all files located by entry_refs in 'refs' and
	sends back a message for each file.

	After being processed, files are registered with the Node Monitor.
	Note that there is an per-application limit of 4096 monitor slots (BeOS R5).
*/
void ImageLoader::RefsReceived(BMessage *message)
{
   	entry_ref ref;
	type_code type;
	message->GetInfo("refs", &type, &fTotal);
	fDone = 0;
   	fRunning = true;
   	// May get Stop()'d at any time.
	for (int i=0; IsRunning() && message->FindRef("refs", i, &ref) == B_OK; i++)
	{
		HandleRef(&ref);
	}
	fRunning = false;
	
	BQuery* first = fQueries.EachElement(live_query_tst,NULL);
	if (first == NULL) 
		SendNotices(MSG_LOADER_DONE);
	else 
		SendNotices(MSG_LOADER_DONE_BUT_RUNNING);
	
}


/**
	Removes a set.
	Usually comes from a client view.
*/
void ImageLoader::DeleteReceived(BMessage *message)
{	
	entry_ref ref;
	for (int i=0; message->FindRef("refs", i, &ref) == B_OK; i++) {
		RemoveCacheItem(&ref);
	}
}



/**
	Handles node monitoring events.

	B_ENTRY_MOVED can mean either a file's name has changed or the file
	was moved to another folder. Moves across volumes do not happen as 
	such, but you will get a B_ENTRY_REMOVED on the monitored source volume.

	Some changes can be tracked either by watching each and every node or, 
	less wastefully, only parent directory nodes. 
	B_NODE_MONITOR messages look the same in both cases for B_ENTRY_MOVED 
	and B_ENTRY_REMOVED but you cannot detect changes to
	a particular attribute or stat that way.
	
	All this is very similar to live query updates. Hint, hint. ;)
*/
void ImageLoader::NodeMonitorChange(BMessage *message)
{
	// Common Node Monitor fields
	int32 opcode;
    node_ref noderef;
    if (message->FindInt64("node", &noderef.node) != B_OK)
    	// This is The _Node_ Monitor after all...
		return;
	message->FindInt32("device", &noderef.device); 
	message->FindInt32("opcode", &opcode);    

	// Look up cached items.
	file_item *item = const_cast<file_item*>(fItems.BinarySearch(file_item(noderef), node_cmp));
	
    if (item) {
		BMessage reply(opcode);
		reply.AddRef("ref", &item->entref);
		
		switch (opcode) {
			case B_ENTRY_MOVED: {
				// A file was renamed or moved.
				PRINT(("B_ENTRY_MOVED: %s\n", item->entref.name));
				const char *name = NULL;
				ino_t from, to;
				message->FindString("name", &name);
				message->FindInt64("from directory", &from);
				message->FindInt64("to directory", &to);
				item->entref = entry_ref(noderef.device, to, name);
				reply.AddRef("newref", &item->entref);
				SendNotices(MSG_LOADER_UPDATE, &reply);
				break;
			}
	       case B_STAT_CHANGED: {
#ifdef __HAIKU__
				int32 fields;
				message->FindInt32("fields", &fields);
				// Haiku case when only attributes are changed, but this is superfluous to B_ATTR_CHANGED 
				if (fields == B_STAT_CHANGE_TIME)
					break;
#endif		
	           	PRINT(("B_STAT_CHANGED: %s\n", item->entref.name));
				if (ReadStats(&item->entref, &reply) == B_OK) {
					ReadData(&item->entref, &reply);
			   		SendNotices(MSG_LOADER_UPDATE, &reply);
				}
			   	break;
		   }
	       case B_ATTR_CHANGED: {
	            PRINT(("B_ATTR_CHANGED: %s\n", item->entref.name));
	            BNode node(&item->entref);
			   	if (ReadAttributes(&node, &reply) == B_OK)
			   		SendNotices(MSG_LOADER_UPDATE, &reply);
	           	break;
	       }
	       case B_ENTRY_REMOVED: 
	       		PRINT(("B_ENTRY_REMOVED: %s\n", item->entref.name));
	       		if (RemoveCacheItem(&item->entref))
					SendNotices(MSG_LOADER_DELETED, &reply);
	           break;
	    }
	}	   
   	// This one can come only from a live query.
    else if (opcode == B_ENTRY_CREATED) {
    	
		entry_ref ref;
        ref.device = noderef.device;
        const char *name = NULL;
        message->FindInt64("directory", &ref.directory);
        message->FindString("name", &name);
        ref.set_name(name);
        PRINT(("B_ENTRY_CREATED: %s\n", ref.name));

		BMessage reply;
		reply.AddRef("ref", &ref);		
		if (ReadStats(&ref, &reply) != B_OK)
			return;
		if (AddCacheItem(ref, noderef)) {
			BNode node(&ref);
			ReadAttributes(&node, &reply);
			if (ReadData(&ref, &reply) == B_OK) {
				SendNotices(MSG_LOADER_UPDATE, &reply);
			}
		}			
		
    }
}



/**
	Caches a node and starts watching it.
*/
bool ImageLoader::AddCacheItem(entry_ref &ref, node_ref &noderef)
{
	// don't get interrupted by Stop() and stuff
	BAutolock lock(fStopLocker);
	file_item *item = new file_item(noderef);
	if (fItems.BinaryInsertUnique(item, node_cmp)) {
		item->entref = ref;
		if (watch_node(&noderef, B_WATCH_ALL, this) == B_OK) 
			PRINT(("B_WATCH_ALL: %s\n", ref.name));
		return true;
	}
	else {
		// Don't bother with duplicates. 
		delete item;
	}
	return false;
}

bool ImageLoader::RemoveCacheItem(entry_ref *ref)
{
	// don't get interrupted by Stop() and stuff
	BAutolock lock(fStopLocker);
	file_item *item = fItems.EachElement(ref_eq, ref);
	if (item) {
		if (watch_node(&item->nodref, B_STOP_WATCHING, this) == B_OK) {
			PRINT(("B_STOP_WATCHING: %s\n", item->entref.name));
		}
		fItems.RemoveItem(item, true);
		return true;			
	}
	return false;
}

/**
	Handles a generic entry_ref.
*/
status_t ImageLoader::HandleRef(entry_ref *ref)
{
	if (fItems.CountItems() > IMAGELOADER_CACHE_LIMIT)
		return B_ERROR;
		
	
	BEntry entry(ref, false);
	if (entry.IsDirectory())
	    return HandleDirectory(ref);
	else if (entry.IsFile())
		return HandleFile(ref);	 	
	else
		return B_ERROR;  
}


/**
	Handles a real file.
*/
status_t ImageLoader::HandleFile(entry_ref *ref)
{		
    BNode node;
    if (node.SetTo(ref) != B_OK)
    	return B_ERROR;

    BString mime;
	if (node.ReadAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0, mime.LockBuffer(B_MIME_TYPE_LENGTH), B_MIME_TYPE_LENGTH) == 0)
		return B_BAD_VALUE;
	mime.UnlockBuffer();
	if (mime == "application/x-vnd.Be-query")
		HandleTrackerQuery(&node);
	else if ((fLoadOptions & LOADER_ONLY_IMAGES) && (mime.FindFirst("image/") != 0))
		return B_OK;
    else {
		BMessage reply;
		reply.AddRef("ref", ref);		
		if (ReadStats(ref, &reply) != B_OK)
			return B_ERROR;
		node_ref noderef;
		node.GetNodeRef(&noderef);			
		bool newnode = AddCacheItem(*ref, noderef);
		if (newnode || (fLoadOptions & LOADER_RELOAD_EXISTING)) {
			BNode node(ref);
			ReadAttributes(&node, &reply);
			status_t ret = ReadData(ref, &reply);
			if (ret != B_OK)
				PRINT(("ReadData(): %s\n", strerror(ret)));
		}

		fDone++;
		reply.AddInt32("total", fTotal);
		reply.AddInt32("done", fDone);
		SendNotices(MSG_LOADER_UPDATE, &reply);
	}
	return B_OK;
}


/**
	Handles a ref to a directory.
*/
status_t ImageLoader::HandleDirectory(entry_ref *ref)
{
	BDirectory dir(ref);
    entry_ref dirref;
	fTotal = +dir.CountEntries()-1;
  	while(IsRunning() && dir.GetNextRef(&dirref) != B_ENTRY_NOT_FOUND) 
    	HandleRef(&dirref);
  	return B_OK;
}
 


/**
	Query files.
*/
status_t ImageLoader::HandleTrackerQuery(BNode *node)
{
	static BVolumeRoster roster;

	attr_info attr;

	// Predicate string
	BString predicate;
	if (node->GetAttrInfo(TRACKER_QUERY_STR_ATTR, &attr) != B_OK)
		return B_ERROR;
	if (node->ReadAttr(TRACKER_QUERY_STR_ATTR, B_STRING_TYPE, 0, predicate.LockBuffer(attr.size), attr.size) < attr.size)
		return B_ERROR;
	predicate.UnlockBuffer();	
	PRINT(("Query predicate: %s\n", predicate.String()));

	// Optional volume (otherwise search all volumes)
	BString selectedVolumeName;
	BMessage volmsg;
	bool singleVolume = false;
	if (node->GetAttrInfo(TRACKER_QUERY_VOL_ATTR, &attr) == B_OK)
	{
		BString buf;
		if (node->ReadAttr(TRACKER_QUERY_VOL_ATTR, B_MESSAGE_TYPE, 0, buf.LockBuffer(attr.size), attr.size) == attr.size) 
		{
			buf.UnlockBuffer();
			volmsg.Unflatten(buf.String());
			singleVolume = true;
		}
	}

	// Calls all destructors.
	fQueries.MakeEmpty();
	
	BVolume volume;
	roster.Rewind();
	fRunning = true;
	while (IsRunning() && roster.GetNextVolume(&volume) == B_OK) {
		if (!volume.IsPersistent() || !volume.KnowsQuery())
			continue;
		BString volumeName;
		volume.GetName(volumeName.LockBuffer(B_FILE_NAME_LENGTH));
		volumeName.UnlockBuffer();
		PRINT(("Query volume: %s\n", volumeName.String()));
		if (singleVolume) {
			// Is this volume listed as a special case?
			bool found = false;
			for (int i=0; volmsg.FindString("volumeName", i, &selectedVolumeName) == B_OK; i++)
			{
				if (volumeName == selectedVolumeName) {
					found = true;
					break;
				}
			}
			// Never mind, skip this volume.
			if (!found)
				continue;
		}
		// Cache this query.
		BQuery *query = new BQuery();
		fQueries.AddItem(query);
		if (query->SetPredicate(predicate.String()) == B_OK && query->SetVolume(&volume) == B_OK) 
		{
			// Make it live.
			query->SetTarget(this);
    		// Get the static portion of the query.
		    if (query->Fetch() == B_OK) {
    			entry_ref ref;
    			while (IsRunning() && query->GetNextRef(&ref) == B_OK) {
        			HandleRef(&ref);
    			}
		    }
		}
	}
	
	return B_OK;
}



/**
	Loads the actual image payload and decodes EXIF/IPTC.
*/
status_t ImageLoader::ReadData(entry_ref *ref, BMessage *reply)
{
	BFile file(ref, B_READ_ONLY);
	if (!file.IsReadable())
		return B_ERROR;
		
	BMessage tags;		
	BRect origbounds;

	// check file attributes for embedded thumbnails.
	BBitmap *bitmap = NULL;
	bitmap = ReadThumbnail(&file, fReadAttr.String());

	uint32 flags = 0;
	
	if ((fLoadOptions & LOADER_READ_TAGS)) {
		// Read JPEG tags but skip EXIF thumbnails if we've already got one.
		bool readExifThumb = (fLoadOptions & LOADER_READ_EXIF_THUMB) && bitmap == NULL;
		JpegTagExtractor extractor(&file, readExifThumb);
		if (extractor.Extract(&tags, &flags) == B_OK) {
			size_t size;
			if (readExifThumb) {
				void *data;
				size = extractor.GetThumbnailData(&data, false);
				if (size > 0) {
					BMemoryIO memio(data, size);
					if (data && size > 0)
						bitmap = BTranslationUtils::GetBitmap(&memio);
				}
			}
		}
	}

	// No embedded thumbnails. Make one from the actual image data.
	if (!bitmap) 
		bitmap = ReadImagePreview(ref, fThumbWidth, fThumbHeight, &origbounds);

   	// Still no picture. Load a Tracker-style icon.
    if (!bitmap) {
    	bitmap = new BBitmap(BRect(0,0,31,31), B_CMAP8);
        BNodeInfo::GetTrackerIcon(ref, bitmap);
   	}
   	
	// Make sure the recipient takes this bitmap's ownership!	
    if (bitmap) 
    	reply->AddPointer("bitmap", bitmap);

	// Pseudo tags
	int16 w;
	if (origbounds.IsValid() && tags.FindInt16("Width",&w) != B_OK) {
		tags.AddInt16("Width", origbounds.IntegerWidth()+1);
		tags.AddInt16("Height", origbounds.IntegerHeight()+1);
	}

	// JPEG features
	if (flags)
		reply->AddInt32("flags", flags);	

	if (!tags.IsEmpty())
		reply->AddMessage("tags", &tags);
		
	return B_OK;
}



/**
	Reads statable properties into 'reply'.
*/
status_t ImageLoader::ReadStats(entry_ref *ref, BMessage *reply)
{
	BEntry entry(ref);
	if (!entry.Exists())
		return B_ENTRY_NOT_FOUND;

	struct stat st;
	if (entry.GetStat(&st) == B_OK) {
		reply->AddInt64("fsize", st.st_size); 
		reply->AddData("ctime", B_TIME_TYPE, &st.st_crtime, sizeof(time_t)); 
		reply->AddData("mtime", B_TIME_TYPE, &st.st_mtime, sizeof(time_t)); 
		return B_OK;
	}
	return B_ERROR;
}



/**
	Reads the first embedded BFS thumbnail.
*/
BBitmap* ImageLoader::ReadThumbnail(BNode *node, const char *attrnames)
{
	BBitmap *bitmap = NULL;
    // Can be a list of up to 4 candidates.
   	char atrlist[B_ATTR_NAME_LENGTH*4];
    // Split in tokens.
   	strncpy(atrlist, attrnames, sizeof(atrlist));
    char *pch, *prog;
    pch = strtok_r(atrlist, ", ", &prog);
    for (int i=0; pch && i < 4; i++) {
	    attr_info attr;
        if (node->GetAttrInfo(pch, &attr) == B_OK) {
			// In-memory bitmap translation.
            char *buf = (char*)malloc(attr.size);
            int n = node->ReadAttr(pch, attr.type, 0, buf, attr.size);
            if (buf && n > 0) { 
	            BMemoryIO stream(buf, n);
				bitmap = BTranslationUtils::GetBitmap(&stream);
            }
	        free(buf);
            break;
        }
        // next token
    	pch = strtok_r(NULL, " ,",&prog);        
    }
    return bitmap;
}


/**
	Read all attrs into a BMessage.
*/
status_t ImageLoader::ReadAttributes(BNode *node, BMessage *reply)
{
	BMessage msg;
	char attrname[B_ATTR_NAME_LENGTH];
	// We'll be reading previews only.
	char buf[256];
    while (node->GetNextAttrName(attrname) == B_OK) {
   		attr_info info;
   		if (node->GetAttrInfo(attrname, &info) != B_OK)
   			continue;
   			
		//TODO: waste less space for attrs which can't be displayed.
		size_t size = sizeof(buf);
		ssize_t n = node->ReadAttr(attrname, info.type, 0, buf, size);

		// AddData does not work for 0 sizes, fake something.
		if (n == 0) {
			n += 1;
			buf[0] = 0;
		}
		msg.AddData(attrname, info.type, buf, n, false);
    }
    if (!msg.IsEmpty())
    	reply->AddMessage("attributes", &msg);		
    return B_OK;
}




/** 
	Loads an image with the TranslationKit and scales it.
	Ratios are respected.
*/
BBitmap* ImageLoader::ReadImagePreview(entry_ref *ref, float width, float height, BRect *originalBounds)
{	
    BBitmap *original = BTranslationUtils::GetBitmap(ref);
    if (original) {
    	float w0 = original->Bounds().Width();
    	float h0 = original->Bounds().Height();
		// ratios
    	float rx = width/w0;
    	float ry = height/h0;
    	// fit to frame
    	if (h0*rx > height)
			width = ceil(w0*ry);
		else if (w0*ry > width)
			height = ceil(h0*rx);

		PRINT(("Creating %.0fx%.0f thumbnail for '%s'.\n", width, height, ref->name ));
			
		// save original bounds
		BRect frame(0, 0, width, height);
		if (originalBounds)
			*originalBounds = original->Bounds();
			
		// scale the original using an off-screen BView
		BBitmap* bitmap = new BBitmap(frame, original->ColorSpace(), true);
		BView* view = new BView(frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
		bitmap->Lock();
		bitmap->AddChild(view);
#ifdef __HAIKU__
		// by hey68you@gmail.com
		view->SetDrawingMode( B_OP_ALPHA );
		view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_COMPOSITE);
		view->DrawBitmap(original, original->Bounds(), frame, B_FILTER_BITMAP_BILINEAR);
#else
		view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		view->DrawBitmap(original, frame);
#endif
		view->RemoveSelf();
		bitmap->Unlock();
		delete view;
    	delete original;
    	return bitmap;
    }
	return NULL;
}



