#ifndef _TAGEXTRACTOR_H_
#define _TAGEXTRACTOR_H_

#include <DataIO.h>
#include <Message.h>

class TagExtractor
{
    public:

	TagExtractor(BPositionIO *posio);
    virtual ~TagExtractor();

    virtual int Read();
    virtual int Read(void *buf, int size);
    virtual void TagExtracted(int category, int id, const char *name, int type, void *value, int size);
    virtual int Extract(BMessage *tags = NULL);

    private:
    
	BPositionIO *fPosIO;
	BMessage *fTags;
};

#endif

