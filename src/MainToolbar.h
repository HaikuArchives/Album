#ifndef _MAINTOOLBAR_H_
#define _MAINTOOLBAR_H_

#include <LayoutView.h>

class BSlider;
class BControl;
class BStringView;
class ProgressBar;

enum {
	MSG_TOOLBAR_ZOOM = 'zoom',
	MSG_TOOLBAR_STOP = 'stop',
	MSG_TOOLBAR_REMOVE = 'remv',
	MSG_TOOLBAR_TAGCOPY = 'tagc',
	MSG_TOOLBAR_MARK = 'mark',
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
