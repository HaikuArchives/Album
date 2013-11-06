#ifndef _PROGRESSBAR_H_
#define _PROGRESSBAR_H_

#include <View.h>
#include <String.h>

/**
	A lightweight BStatusBar-like progress bar.
*/
class ProgressBar: public BView
{
	public:

	ProgressBar(BRect frame, const char *name, const char *text = NULL, uint32 resizing = B_FOLLOW_NONE);
	virtual void AttachedToWindow();
	virtual void Draw(BRect update);
	virtual void MessageReceived(BMessage *message);
	inline rgb_color BarColor() const;
	void SetBarColor(rgb_color color);
	void SetStripe(float portion, float tint = B_LIGHTEN_1_TINT);

	inline border_style BorderStyle() const;
	void SetBorderStyle(border_style border);	
	
	inline float CurrentValue() const;
	void SetCurrentValue(float value);

	inline float MaxValue() const;
	void SetMaxValue(float value);

	inline const char* Text() const;
	virtual void SetText(const char *text);

	virtual void Reset(const char *text = NULL);
	virtual void Update(float delta, const char *text = NULL);
			
	private:

	BString fText;
	float fCurrent, fMaxValue;
	float fBarTop, fBarTopTint;
	rgb_color fBarColor, fBackColor;
	border_style fBorderStyle;
};


#endif
