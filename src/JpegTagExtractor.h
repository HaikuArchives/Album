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

    virtual int Extract(BMessage *tags = NULL);
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

