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

\file JpegTagExtractor.cpp
*/

#include <Debug.h>
#include <String.h>
#include <libiptcdata/iptc-data.h>
#include <libiptcdata/iptc-jpeg.h>
#include <JpegTagExtractor.h>

/* JPEG marker magics */
enum JPEG_MARKER {
  SOF0 = 0xc0, SOF1 = 0xc1, SOF2 = 0xc2, SOF3 = 0xc3,
  SOF5 = 0xc5, SOF6 = 0xc6, SOF7 = 0xc7, SOF9 = 0xc9,
  SOF10 = 0xca, SOF11 = 0xcb, SOF13 = 0xcd, SOF14 = 0xce,
  SOF15 = 0xcf, DHT = 0xc4, DAC = 0xcc,
  RST0 = 0xd0, RST1 = 0xd1, RST2 = 0xd2, RST3 = 0xd3,
  RST4 = 0xd4, RST5 = 0xd5, RST6 = 0xd6, RST7 = 0xd7,
  SOI = 0xd8, EOI = 0xd9, SOS = 0xda, DQT = 0xdb,
  DNL = 0xdc, DRI = 0xdd, DHP = 0xde, EXP = 0xdf,
  APP0 = 0xe0, APP1 = 0xe1, APP2 = 0xe2, APP3 = 0xe3,
  APP4 = 0xe4, APP5 = 0xe5, APP6 = 0xe6, APP7 = 0xe7,
  APP8 = 0xe8, APP9 = 0xe9, APP10 = 0xea, APP11 = 0xeb,
  APP12 = 0xec, APP13 = 0xed, APP14 = 0xee, APP15 = 0xef,
  JPG0 = 0xf0, JPG1 = 0xf1, JPG2 = 0xf2, JPG3 = 0xf3,
  JPG4 = 0xf4, JPG5 = 0xf5, JPG6 = 0xf6, SOF48 = 0xf7,
  LSE = 0xf8, JPG9 = 0xf9, JPG10 = 0xfa, JPG11 = 0xfb,
  JPG12 = 0xfc, JPG13 = 0xfd, COM = 0xfe, TEM = 0x01,
};


JpegTagExtractor::JpegTagExtractor(BPositionIO *posio, bool thumbnail):
    TagExtractor(posio),
    fThumbnailData(NULL),
    fThumbnailSize(0),
    fSkipThumbnail(!thumbnail)
{
}


JpegTagExtractor::~JpegTagExtractor()
{
	if (fThumbnailData)
		free(fThumbnailData);
}


/**
	Returns negative number if not an JPEG file or
	bitfields indicating features: HAS_IPTC, HAS_EXIF.
*/
int JpegTagExtractor::Extract(BMessage *tags)
{
	TagExtractor::Extract(tags);
    uint16 size;
    int c = 0;
	uint16 magic = 0;
	int flags = 0;
	Read(&magic, 2);
	if (ntohs(magic) != 0xffd8)
		// not a JPEG file
		return -1;
	// scan JPEG markers
    while (c != EOF) {
        if ((c = Read()) != 0xff)
            continue;
        if ((c = Read()) == 0)
            continue;
        // marker detected
        //PRINT(("MARK %X\n", c));
        switch (c) {
			case APP0:
				ReadAPP0();
				break;
            case APP1:
                if (ReadAPP1() > 0)
                	flags |= HAS_EXIF;
                break;
            case APP13:
                if (ReadAPP13() >= 0)
                	flags |= HAS_IPTC;
                break;
            case COM:
                ReadCOM();
                break;
            case DHT:
            case DQT:
                // skip a variable-length marker
                Read(&size, 2);
                Read(NULL, ntohs(size)-2);
                break;
            default:
                if (c >= SOF0 && c <= SOF15)
                    // no more metadata, image data start
                    return ReadSOF() >=0 ? flags : -1;
                if (c >= APP0 && c <= APP15) {
                    // skip unknown application segments
                    Read(&size, 2);
                    Read(NULL, ntohs(size)-2);
                }
        };
    }
    return -1;
}


/**
	Copies the pointer to raw thumbnail data and returns its size.
	If 'detach' is true the caller takes over the ownership,
	otherwise it will be freed upon destruction.
	
*/
size_t JpegTagExtractor::GetThumbnailData(void **data, bool detach)
{
	*data = fThumbnailData;
	if (detach)
		fThumbnailData = NULL;
	return fThumbnailSize;
}



/**
    Gets EXIF tags from an IFD block.
    'data' points to the TIFF header start.
*/
int JpegTagExtractor::ReadIFD(exif_ifd_t *ifd, uint8 *data, int ofs, bool skip)
{
    ofs = read_ifd(ifd, data, ofs);
    if (ofs) {
    	void *thumb = NULL;
    	int thumbsize = 0;
        exif_tag_t tag;
        char buf[0x1000];
        for (int i=0; i < ifd->size; i++) {
            ofs = read_ifd_tag(ifd, &tag, data, ofs);
            if (!ofs)
            	break;
            // only relevant tags
			const char *tagname = exif_descr(exif_tag_name, tag.number);
            if (!skip && tagname) {
            	BString prefix = "EXIF:";
            	prefix += tagname;
            	//PRINT(("%X %s(%d): %d\n", tag.number, tagname, tag.format, tag.value));
            	if (exif_str(buf, &tag))
            		TagExtracted('EXIF', tag.number, prefix.String(), B_STRING_TYPE, buf, strlen(buf)+1);
        	}
            if (tag.number == EXIF_JPEG_INTERCHANGE_FORMAT)
            	// thumbnail offset in 'data'
                thumb = data + tag.value;
            else if (tag.number == EXIF_JPEG_INTERCHANGE_FORMAT_LENGTH)
            	thumbsize = tag.value;
        }

		PRINT(("IFD %d %d\n", ifd->ofs, ifd->size));

        // store the thumbnail if complete and new
	    if (thumb && thumbsize && !fThumbnailData) {
	    	fThumbnailData = malloc(thumbsize);
	    	memcpy(fThumbnailData, thumb, thumbsize);
	    	fThumbnailSize = thumbsize;
	    	PRINT(("EXIF thumbnail: %d bytes\n", thumbsize));
	    }
    }
    return ofs;
}


/**
	Gets IPTC tags from an APP13 block.
*/
int JpegTagExtractor::ReadIPTC(uint8 *data, int size)
{
    unsigned int iptc_len = 0;
    int iptc_off = iptc_jpeg_ps3_find_iptc (data, size, &iptc_len);
	if (iptc_off <= 0)
        return -2;
    IptcData *iptc = iptc_data_new_from_data (data+iptc_off, iptc_len);
    if (iptc) {
        char buf[0x1000];
        int count = iptc->count;
        for (int i=0; i < count; i++) {
            IptcDataSet * e = iptc->datasets[i];
            BString name = "IPTC:";
            name += iptc_tag_get_name(e->record, e->tag);
            switch (iptc_dataset_get_format (e)) {
				//TODO: store different types
                case IPTC_FORMAT_BYTE:
                case IPTC_FORMAT_SHORT:
                case IPTC_FORMAT_LONG:
                case IPTC_FORMAT_DATE:
                case IPTC_FORMAT_TIME:
				default:
                    iptc_dataset_get_as_str(e, buf, sizeof(buf));
                    TagExtracted('IPTC', e->tag, name.String(), B_STRING_TYPE, buf, e->size+1);
                    break;
            }
        }
        iptc_data_unref(iptc);
        return count;
    }
    return -1;
}


int JpegTagExtractor::ReadAPP0()
{
    uint16 size;
    if (Read(&size, 2) < 2)
        return -1;
    size = ntohs(size)-2;
	// JFIF header
    char data[size];
    if (Read(data, size) < size)
        return -1;
	return 0;
};

/**
    Reads APP1 marker.
    Should contain EXIF data.
*/
int JpegTagExtractor::ReadAPP1()
{
    uint16 size;
    if (Read(&size, 2) < 2)
        return -1;
    size = ntohs(size) - 2;
    // EXIF identifier
    char identifier[6];
    if (Read(identifier, 6) < 6 || strcmp(identifier,"Exif") != 0)
        return -1;
    // TIFF header
    uint8 data[size-6];
    if (Read(data, size-6) < size-6)
        return -1;
    int16 ifd0magic = *(uint16*)(data+2);
    int32 ifd0ofs = *(uint32*)(data+4);
    if (data[0] == 'M') {
    	ifd0magic = ntohs(ifd0magic);
    	ifd0ofs = ntohl(ifd0ofs);
    }
    if (ifd0magic != 42)
    	return -1;
    exif_ifd_t ifd0, subifd, ifd1;
    ReadIFD(&ifd0, data, ifd0ofs);
    ReadIFD(&subifd, data, ifd0.sub);
    if (!fSkipThumbnail) {
		// Thumbnail, do not extract tags, just data
		ReadIFD(&ifd1, data, ifd0.next, true);
    }
  	return ifd0.size;
}



/**
    Reads APP13 Marker.
    Should contain IPTC tags.
*/
int JpegTagExtractor::ReadAPP13()
{
    uint16 size;
    if (Read(&size, 2) < 2)
        return -1;
    size = ntohs(size)-2;

    uint8 data[size];
    if (Read(data, size) < size)
        return -1;

    int n =  ReadIPTC(data, size);
    return n;
}



/**
    Reads a JPEG embedded comment.
*/
int JpegTagExtractor::ReadCOM()
{
    uint16 size;
    if (Read(&size, 2) < 2)
        return -1;
	// string + NUL follows
    size = ntohs(size) -2;
    char data[size];
    if (Read(data, size) < size)
    	return -1;
    // zero-terminate the comment
    data[size-1] = 0;
	TagExtracted('JPEG', 0xff, "Comment", B_STRING_TYPE, data, strlen(data)+1);
    return 0;
}



/**
	Reads a Start-of-Frame marker.
*/
int JpegTagExtractor::ReadSOF()
{
    uint16 size;
    if (Read(&size, 2) < 2)
        return -1;
    size = ntohs(size) -2;
    char data[size];
    if (Read(data, 5) < 5)
    	return -1;
	uint16 height = ntohs(*(uint16*)(data+1));
	uint16 width = ntohs(*(uint16*)(data+3));
    TagExtracted('JPEG', 2, "Width", B_INT16_TYPE, &width, 2);
    TagExtracted('JPEG', 3, "Height", B_INT16_TYPE, &height, 2);
    return 0;	
}

