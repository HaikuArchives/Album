/**
Copyright (c) 2009 by Matjaz Kovac

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

\file TagExtractor.cpp
*/

#include <stdio.h>
#include <TagExtractor.h>


TagExtractor::TagExtractor(BPositionIO *posio):
    fPosIO(posio),
    fTags(NULL)
{
}

TagExtractor::~TagExtractor()
{
}


/**
	Implements getch.
*/
int TagExtractor::Read()
{
	if (fPosIO) {
        int c = 0;
        if (fPosIO->Read(&c, 1) > 0)
        	return c;
    }
    return EOF;
}


/**
	Implements file read.
	Must perform a dummy read or seek if buf == NULL. 
*/
int TagExtractor::Read(void *buf, int size)
{
	if (fPosIO) {
        if (buf)
            // read block
            return fPosIO->Read(buf, size);
        else
            // skip
            return fPosIO->Seek(size, SEEK_CUR);
    }
    return 0;
}


/**
	This hook is called by Extract().
*/
void TagExtractor::TagExtracted(int category, int id, const char *name, int type, void *value, int size)
{
    if (name && value) {
        if (fTags)
        	fTags->AddData(name, type, (char*)value, size, false);
    }
}


/**
	Gets all metadata from the file.
*/
status_t TagExtractor::Extract(BMessage *tags, uint32 *features)
{
	fTags = tags;
	return B_OK;
}
