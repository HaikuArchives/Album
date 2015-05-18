#ifndef _MAINTOOLBAR_H_
#define _MAINTOOLBAR_H_

#include <LayoutView.h>

class BSlider;
class ProgressBar;

enum {
	MSG_TOOLBAR_ZOOM = 'zoom',
	MSG_TOOLBAR_STOP = 'stop',
};

class MainToolbar : public LayoutView 
{
	public:
	
	MainToolbar(BRect frame, uint32 resizing); 
	void UpdateProgress(float total, float done, const char *status = NULL);
	void SetCounter(int count);
	void SetSelected(int count);
	void SetAttrSelected(bool enabled);
	
	private:

	BSlider *fScaler;
	ProgressBar *fProgress;
	BControl *fStop, *fRemove, *fTagCopy, *fMark;
};

#endif
