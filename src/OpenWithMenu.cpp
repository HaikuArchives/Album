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

\file OpenWithMenu.cpp
\author Matjaž Kovač
\brief BMenu that lists supported applications
*/

#include <NodeInfo.h>
#include <MenuItem.h>
#include <Node.h>
#include <AppFileInfo.h>
#include "OpenWithMenu.h"
#include <string.h>

OpenWithMenu::OpenWithMenu(const char *name, const entry_ref *ref, uint32 command):
        BMenu(name, B_ITEMS_IN_COLUMN)
{
    SetTo(ref, command);
}




/**
	Constructs an menu similar to the Tracker's "Open With".
	
	THe menu is divided in two parts.
	The top portion lists the apps that support the target's
	exact type, the bottom lists those that handle its supertype
	in general (all image files, for instance).
*/
status_t OpenWithMenu::SetTo(const entry_ref *ref, uint32 command)
{
    BNode node(ref);
    BNodeInfo nodeInfo(&node);
    char type[B_MIME_TYPE_LENGTH];
    if (nodeInfo.GetType(type) == B_OK) {
        BMessage msg;
        // Get the list of apps that can handle this type of file.
        BMimeType mime(type);
        mime.GetSupportingApps(&msg);
        // Preffered app
        char preferredapp[B_MIME_TYPE_LENGTH];
        mime.GetPreferredApp(preferredapp);
        BMimeType appmime;
        entry_ref appref;
        int32 subs = 0, supers = 0;
        // The list is divided in two parts 
        msg.FindInt32("be:sub", &subs);
        msg.FindInt32("be:super", &supers);
        for (int i = 0; i < subs + supers; i++) {
			// Apps that support the type directly are listed first.
            if (i == subs)
                AddSeparatorItem();
            const char *appsig;
            // Get app info for a particular signature.
            if (msg.FindString("applications", i, &appsig) == B_OK) {
                if (appmime.SetTo(appsig) != B_OK)
                    continue;
                if (appmime.GetAppHint(&appref) == B_OK) {
                    BMessage *msg = new BMessage(command);
                    msg->AddRef("refs", ref);
                    msg->AddString("app", appsig);
                    BMenuItem *item = new BMenuItem(appref.name, msg);
                    item->SetMarked(!strcmp(appsig, preferredapp));
                    AddItem(item);
                }
            }
        }
    }
    return B_OK;
}






