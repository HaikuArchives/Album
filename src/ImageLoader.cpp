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
Failing to handle notices is a Bad Thing because a removed file
will result in an invalid reference.

Another neat trick is to combine the Node Monitor with Live Queries. 
The BeOS API makes it really simple as the message protocols are
essentially the same.  That way any client view can reflect 
a criterion set by a query, adding and removing files automatically as 
they come in and out of the scope of a query predicate.
*/
#include <Debug.h>
#include <Autolock.h>
#include <NodeMonitor.h>
#include <NodeInfo.h>
#include <Volume.h>
#include <Directory.h>
#include <fs_attr.h>
#include <TranslationKit.h>
#include <Bitmap.h>
#include <View.h>

#include "ImageLoader.h"
#include "JpegTagExtractor.h"


/**
	Creates a new node cache item.
*/
file_item::file_item(node_ref &node)
{
	nodref = node;
}


/**
	Node list compare function for binary search/insert.
*/
int file_item::node_cmp(const file_item *a, const file_item *b) 
{
	if (a->nodref.device == b->nodref.device)
		return a->nodref.node - b->nodref.node;
	else
		return a->nodref.device - b->nodref.device;
}

file_item* file_item::ref_eq(file_item *item, void *param)
{
	return item->entref == *(entry_ref*)param ? NULL : item;
}






/**
	Constructs a new message-driven image loader.
	Since it does some heavy-duty image processing it is best
	to keep it low priority to improve the overall responsivness.
*/
ImageLoader::ImageLoader(const char *name):
	BLooper(name, B_NORMAL_PRIORITY),
	// Create an owning list, 20 items per block.
	fItems(20, true),
	fRunning(false),
	fThumbWidth(130.0),
	fThumbHeight(90.0),
	fReadAttr("IPRO:thumbnail"),
	fLoadOptions(1)
{
}


ImageLoader::~ImageLoader()
{
    stop_watching(this);
	PRINT(("'%s' deleted\n", Name()));
}


/**
	Stops image processing ASAP.
*/
void ImageLoader::Stop()
{
	BAutolock lock(fLocker);
	fRunning = false;
	if (fQuery.IsLive()) {
		fQuery.Clear();
		SendNotices(MSG_LOADER_DONE);
	}
}


bool ImageLoader::IsBusy()
{
	BAutolock lock(fLocker);
	return fRunning;
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

void ImageLoader::SetAttrNames(const char *attrnames)
{
	BAutolock lock(fLocker);
	fReadAttr = attrnames;
}

void ImageLoader::SetLoadOptions(uint32 flags)
{
	BAutolock lock(fLocker);
	fLoadOptions = flags;
}

void ImageLoader::SetThumbnailSize(float width, float height)
{
	BAutolock lock(fLocker);
	fThumbWidth = width;
	fThumbHeight = height;
}


/**
	Handles a generic entry_ref.
*/
status_t ImageLoader::HandleRef(entry_ref *ref)
{
	

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
	node_ref noderef;
	node.GetNodeRef(&noderef);
	// MIME type is needed to proceed. 
    char s[2048];	
	if (node.ReadAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0, s, B_MIME_TYPE_LENGTH) == 0)
		return B_ERROR;
    else if (!strcmp(s, "application/x-vnd.Be-query")) {
		// Some queries work on a specific volume.
		if (node.ReadAttr("_trk/qryvol1", B_MESSAGE_TYPE, 0, s, sizeof(s)) == B_OK) {
			BMessage volmsg;
			volmsg.Unflatten(s);
			//TODO: set the right volume.
		}
		node.ReadAttr("_trk/qrystr", B_STRING_TYPE, 0, s, sizeof(s));
		// For now always use the volume the query came from.
		BVolume volume(noderef.device);
		HandleQuery(s, &volume);
    } 
    else {
		BMessage reply(MSG_LOADER_UPDATE);
		if (LoadFile(ref, &reply) == B_OK) {
			if (watch_node(&noderef, B_WATCH_ALL, this) == B_OK) {
				file_item *item = new file_item(noderef);
				item->entref = *ref;
				fItems.BinaryInsertUnique(item, file_item::node_cmp);
				PRINT(("B_WATCH_ALL '%s'\n", ref->name));
			}
			fDone++;
			reply.AddInt32("total", fTotal);
			reply.AddInt32("done", fDone);
			CurrentMessage()->SendReply(&reply);
    	}
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
	fTotal += dir.CountEntries()-1;
  	while(IsBusy() && dir.GetNextRef(&dirref) != B_ENTRY_NOT_FOUND) 
    	HandleRef(&dirref);
  	return B_OK;
}


/**
	Parses and runs a query.
*/
status_t ImageLoader::HandleQuery(const char *predicate, BVolume *volume)
{
    PRINT(("query predicate: %s\n",predicate));

    fQuery.Clear();
    fQuery.SetPredicate(predicate);
    fQuery.SetVolume(volume);

    // Make the query live.
    fQuery.SetTarget(this);
    fQuery.Fetch();

    // Get the static portion of the query.
    entry_ref ref;
    fRunning = true;
    while (IsBusy() && fQuery.GetNextRef(&ref) == B_OK)
        HandleRef(&ref);

    // Done for now, but there may be live updates later on.
    return B_OK;
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
	for (int i=0; IsBusy() && message->FindRef("refs", i, &ref) == B_OK; i++)
		HandleRef(&ref);
	fRunning = false;
	// All done.
	if (!fQuery.IsLive())
		SendNotices(MSG_LOADER_DONE);
}



/**
	Read file data.
*/
status_t ImageLoader::LoadFile(entry_ref *ref, BMessage *reply, uint32 mode)
{
	BAutolock lock(fLocker);
	
	// sanity check
	BEntry entry(ref);
	status_t status;
	if ((status = entry.InitCheck()) != B_OK)
		return status;
	reply->AddRef("ref", ref);

	uint32 flags = 0;
	
	if (mode & LOAD_STATS) {
		// Get file stat.
		struct stat st;
		if (entry.GetStat(&st) == B_OK) {
			reply->AddInt64("fsize", st.st_size); 
			reply->AddData("ctime", B_TIME_TYPE, &st.st_crtime, sizeof(time_t)); 
			reply->AddData("mtime", B_TIME_TYPE, &st.st_mtime, sizeof(time_t)); 
		}
	}

	BFile node(ref, B_READ_ONLY);
	BRect origbounds;

	if (mode & LOAD_DATA ) {
		// check file attributes for embedded thumbnails.
		BBitmap *bitmap = NULL;
		bitmap = ReadThumbnail(&node, fReadAttr.String());

		BMessage tags;
		if (fLoadOptions & 1) {
			// Reads EXIF thumbnail only if there is no embedded preview.
			JpegTagExtractor tgx(&node, !bitmap);
			int result = tgx.Extract(&tags);
			if (result >= 0) {
				flags = result;
				size_t size;
				if (fLoadOptions & 2 && !bitmap) {
					void *data;
					size = tgx.GetThumbnailData(&data, false);
					if (size > 0) {
					BMemoryIO memio(data, size);
					if (data && size > 0)
						bitmap = BTranslationUtils::GetBitmap(&memio);
					}
				}
			}
		}

		if (!bitmap) 
			// No embedded thumbnails. Make one from the actual image data.
			bitmap = MakeImagePreview(ref, fThumbWidth, fThumbHeight, &origbounds);
	    if (!bitmap) {
	    	// Still no picture. Load a Tracker-style icon.
	    	bitmap = new BBitmap(BRect(0,0,31,31), B_CMAP8);
	        BNodeInfo::GetTrackerIcon(ref, bitmap);
	   	}
	    if (bitmap) 
			// Make sure the recipient takes this bitmap's ownership!	
	    	reply->AddPointer("bitmap", bitmap);

		int16 width;
		if (tags.FindInt16("Width", &width) != B_OK && origbounds.IsValid()) {
			tags.AddInt16("Width", origbounds.IntegerWidth());
			tags.AddInt16("Height", origbounds.IntegerHeight());
		}
		if (!tags.IsEmpty())
			reply->AddMessage("tags", &tags);
	    	
	}

	// Cache attributes
	if (mode & LOAD_ATTRIBUTES) {
		if (node.InitCheck() == B_OK)  {
			BMessage attrs;
			ReadAttributes(&node, &attrs);
			if (!attrs.IsEmpty())
				reply->AddMessage("attributes", &attrs);
		}
	}
	

	if (flags)
		reply->AddInt32("flags", flags);	
	
	return B_OK;
}



/**
	Reads an embedded preview in one of the designated attributes.
*/
BBitmap* ImageLoader::ReadThumbnail(BNode *node, const char *attrname)
{
	BBitmap *bitmap = NULL;
    // Attribute name can be a list of up to 4 candidates.
   	char atrlist[1024];
   	strcpy(atrlist, attrname);
    // Split strings in tokens.
    char *pch, *prog;
    pch = strtok_r(atrlist, ", ", &prog);
    attr_info attr;
    for (int i=0; pch; i++, pch = strtok_r(NULL, " ,",&prog)) {
        if (node->GetAttrInfo(pch, &attr) == B_OK) {
			// Translate a possible bitmap. 
            char *buf = (char*)malloc(attr.size);
            int n = node->ReadAttr(pch, attr.type, 0, buf, attr.size);
            if (buf && n > 0) { 
	            BMemoryIO stream(buf, n);
				bitmap = BTranslationUtils::GetBitmap(&stream);
            }
	        free(buf);
            break;
        }
    }
    return bitmap;
}


/**
	Read all attrs into a BMessage.
*/
status_t ImageLoader::ReadAttributes(BNode *node, BMessage *attr)
{
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

		// Unfortunately, AddData does not work for 0 sizes, fake something.
		if (n == 0) {
			n += 8;
			memset(buf, 0, n);
		}
		attr->AddData(attrname, info.type, buf, n, false);
    }

    return B_OK;
}




/** 
	Loads an image with the TranslationKit and scales it.
	Ratios are respected.
*/
BBitmap* ImageLoader::MakeImagePreview(entry_ref *ref, float width, float height, BRect *originalBounds)
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
			width = w0*ry;
		else if (w0*ry > width)
			height = h0*rx;
		// size in BView coords
		BRect frame(0, 0, width, height);
		if (originalBounds)
			*originalBounds = original->Bounds();
		// scale the original using an off-screen BView
		BBitmap* bitmap = new BBitmap(frame, original->ColorSpace(), true);
		BView* view = new BView(frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
		bitmap->AddChild(view);
		bitmap->Lock();
		view->DrawBitmap(original, frame);
		view->Sync();
		view->RemoveSelf();
		bitmap->Unlock();
		delete view;
    	delete original;
    	return bitmap;
    }
	return NULL;
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
    // Assume the presence of node_ref fields, this is the Node Monitor, after all.
    node_ref noderef;
    message->FindInt32("device", &noderef.device);
    message->FindInt64("node", &noderef.node);

	
	// Now see what's happened.
	int32 opcode;
	message->FindInt32("opcode", &opcode);    
	// Look up the watched node list.
	file_item *item = const_cast<file_item*>(fItems.BinarySearch(file_item(noderef), file_item::node_cmp));
    if (item) {
    	
    	
		BMessage msg(opcode);
		msg.AddRef("ref", &item->entref);
		switch (opcode) {
			case B_ENTRY_MOVED: {
				// A file was renamed or moved.
				PRINT(("B_ENTRY_MOVED '%s'\n", item->entref.name));
				const char *name = NULL;
				ino_t from, to;
				message->FindString("name", &name);
				message->FindInt64("from directory", &from);
				message->FindInt64("to directory", &to);
				item->entref = entry_ref(noderef.device, to, name);
				msg.AddRef("newref", &item->entref);
				SendNotices(MSG_LOADER_UPDATE, &msg);
				break;
	       }
	       case B_STAT_CHANGED: {
			   // A file's stat()ables changed, its data may be modified. 
	           PRINT(("B_STAT_CHANGED '%s'\n", item->entref.name));
			   if (LoadFile(&item->entref, &msg, LOAD_STATS | LOAD_DATA) == B_OK)
			   		SendNotices(MSG_LOADER_UPDATE, &msg);
			   break;
		   }
	       case B_ATTR_CHANGED:
			   // A file's attribute set changed somehow.
	           PRINT(("B_ATTR_CHANGED '%s'\n", item->entref.name));
			   if (LoadFile(&item->entref, &msg, LOAD_ATTRIBUTES) == B_OK)
			   		SendNotices(MSG_LOADER_UPDATE, &msg);
	           break;
	       case B_ENTRY_REMOVED:
	           PRINT(("B_ENTRY_REMOVED '%s'\n", item->entref.name));
			   if (fItems.RemoveItem(item, true)) {
				   SendNotices(MSG_LOADER_DELETED, &msg);
			   }
	           break;
	    }
	}	   
    // This one can come only from a live query.
    else if (opcode == B_ENTRY_CREATED) {
    	
    	
        entry_ref ref;
        const char *name = NULL;
        ref.device = noderef.device;
        message->FindInt64("directory", &ref.directory);
        message->FindString("name", &name);
        ref.set_name(name);
        PRINT(("B_ENTRY_CREATED (Query) '%s'\n", ref.name));
        // Create a new item.
		BMessage reply;
		if (LoadFile(&ref, &reply) == B_OK) {
			if (watch_node(&noderef, B_WATCH_ALL, this) == B_OK) {
				file_item *item = new file_item(noderef);
				item->entref = ref;
				fItems.BinaryInsertUnique(item, file_item::node_cmp);
				PRINT(("B_WATCH_ALL (Query)\n"));
			}
			SendNotices(MSG_LOADER_UPDATE, &reply);
    	}
    }
}


/**
	Removes a set.
	Usually comes from a client view.
*/
void ImageLoader::DeleteReceived(BMessage *message)
{
	entry_ref ref;
	for (int i=0; message->FindRef("refs", i, &ref) == B_OK; i++) {
		file_item *item = fItems.EachElement(file_item::ref_eq, &ref);
		if (item && watch_node(&item->nodref, B_STOP_WATCHING, this) == B_OK) {
	       	PRINT(("B_STOP_WATCHING: '%s'\n", ref.name));
			fItems.RemoveItem(item, true);			
		}
	}
}





