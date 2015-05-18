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
*/

#ifndef _JPEGTAGEXTRACTOR_H_
#define _JPEGTAGEXTRACTOR_H_

#include <TagExtractor.h>
#include <exif.h>

enum {
	HAS_COMMENT = 1,
	HAS_EXIF = 2,
	HAS_IPTC = 4
};

/**
    Extracts JPEG Comment blocks and most EXIF tags.
*/
class JpegTagExtractor: public TagExtractor
{
    public:

	JpegTagExtractor(BPositionIO *posio, bool thumbnail);
	~JpegTagExtractor();

    virtual status_t Extract(BMessage *tags, uint32 *features = NULL);
    size_t GetThumbnailData(void **data, bool detach);
    
    private:

    // JPEG Application Marker processing
    int ReadIFD(exif_ifd_t *ifd, uint8 *data, int ofs, bool skip = false);
    int ReadIPTC(uint8 *data, int size);
    int ReadAPP0();
    int ReadAPP1();
    int ReadAPP13();
    int ReadCOM();
    int ReadSOF();
    
    void* fThumbnailData;
    size_t fThumbnailSize;
    bool fSkipThumbnail;
};


#endif

