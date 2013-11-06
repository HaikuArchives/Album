#ifndef _ICONBUTTON_H_
#define _ICONBUTTON_H_

#include <Button.h>
#include <Bitmap.h>

class IconButton : public BButton
{
	public:
	
	IconButton(BRect frame, const char *name, 
					const char *label, BBitmap *iconOff, BBitmap *iconOn, BMessage *message, 
					uint32 resizing = B_FOLLOW_NONE, uint32 flags = 0);
	~IconButton();
	virtual void AttachedToWindow();
	virtual void Draw(BRect update);
	//virtual void MouseDown(BPoint where);
	//virtual void MouseUp(BPoint where);
	virtual void GetPreferredSize(float *width, float *height);
	private:
	
	BBitmap *fIconOn, *fIconOff;
};


#endif	// _ICONBUTTON_H_
