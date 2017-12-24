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

\file ProgressBar.cpp
\author Matjaž Kovač
\brief A BStatusBar-like view
*/

#include <Message.h>
#include <ProgressBar.h>


/**
	Inits a new instance.
*/
ProgressBar::ProgressBar(BRect frame, const char *name, const char *text, uint32 resizing):
	BView(frame, name, resizing, B_WILL_DRAW),
	fText(text),
	fCurrent(0),
	fMaxValue(100.0),
	fBarTop(0.33),
	fBarTopTint(B_LIGHTEN_1_TINT),
	fBarColor((rgb_color){50, 150, 255}),
	fBackColor((rgb_color){210, 210, 210}),
	fBorderStyle(B_FANCY_BORDER)
{
}


/**
	The view is ready for action.
	Sets ViewColor() to B_TRANSPARENT_COLOR.
*/
void ProgressBar::AttachedToWindow()
{
	SetViewColor(B_TRANSPARENT_COLOR);
}


/**
	Renders the progress bar and labels.
	Touches every pixel within Bounds().
*/
void ProgressBar::Draw(BRect update)
{
	BRect bar = Bounds();

	// Border
	if (fBorderStyle != B_NO_BORDER) {
		rgb_color light = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_2_TINT);
		rgb_color dark = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT);
		BeginLineArray(fBorderStyle == B_FANCY_BORDER ? 8 : 4);
		AddLine(bar.RightTop(), bar.LeftTop(), light);
		AddLine(bar.LeftTop(), bar.LeftBottom(), light);
		AddLine(bar.RightBottom(), bar.LeftBottom(), dark);
		AddLine(bar.RightBottom(), bar.RightTop(), dark);
		if (fBorderStyle == B_FANCY_BORDER) {
			bar.InsetBy(1,1);
			AddLine(bar.RightTop(), bar.LeftTop(), dark);
			AddLine(bar.LeftTop(), bar.LeftBottom(), dark);
			AddLine(bar.RightBottom(), bar.LeftBottom(), light);
			AddLine(bar.RightBottom(), bar.RightTop(), light);
		}
		EndLineArray();
		bar.InsetBy(1,1);
	}

	// Value
	SetDrawingMode(B_OP_COPY);
	float xs = bar.left;
	if (MaxValue() != 0)
		xs += -1 + (bar.Width()+1) * CurrentValue() / MaxValue();
	float ys = bar.top + bar.Height() * fBarTop;
	SetLowColor(tint_color(fBarColor, fBarTopTint));
	FillRect(BRect(bar.left, bar.top, xs, ys-1), B_SOLID_LOW);
	SetLowColor(fBarColor);
	FillRect(BRect(bar.left, ys, xs, bar.bottom), B_SOLID_LOW);
	// Remainder
	SetLowColor(tint_color(fBackColor, fBarTopTint));
	FillRect(BRect(xs+1, bar.top, bar.right, ys-1), B_SOLID_LOW);
	SetLowColor(fBackColor);
	FillRect(BRect(xs+1, ys, bar.right, bar.bottom), B_SOLID_LOW);

	// Text
	if (Text()) {
		SetDrawingMode(B_OP_OVER);
		font_height fh;
		GetFontHeight(&fh);
 		BPoint p(bar.left + (bar.Width() - StringWidth(Text())) / 2.0, bar.bottom - (bar.Height() - fh.ascent)/2.0);
		DrawString(Text(), p);
	}
}

/**
	Augments BView behaviour with BStatusBar-like messages.
	Basically just unpacks a message and calls Reset() or Update().
	Understands the same message format as BStatusBar, but the behaviour is not the same.
	B_RESET_STATUS_BAR only sets the current value to 0. MaxValue()
	is not set to 100.0 but it is instead set to whatever is stored in "max_value" field
	or left unchanged if that field is absent.
	B_UPDATE_STATUS_BAR also recognises "max_value" the rest is the same as with BStatusBar.

	Commands and fields:
	- B_UPDATE_STATUS_BAR: Increments CurrentValue() by "delta" (float) and
	sets Text() to "text" (string) if present. Also sets MaxValue() to "max_value"
	if present.
	- B_RESET_STATUS_BAR: Sets CurrentValue() to 0.
	Sets Text() to "label" if present and MaxValue() to "max_value", if present.

*/
void ProgressBar::MessageReceived(BMessage *message)
{
	float delta = 0;
	const char *text = NULL;
	switch (message->what) {
		case B_UPDATE_STATUS_BAR:
			message->FindFloat("delta", &delta);
			message->FindString("text", &text);
			Update(delta, text);
			break;
		case B_RESET_STATUS_BAR:
			message->FindString("label", &text);
			Reset(text);
			break;
		default:
			BView::MessageReceived(message);
	}
}


/**
	Sets the base bar color.
*/
void ProgressBar::SetBarColor(rgb_color color)
{
	fBarColor = color;
}

/**
	Sets the top stripe width and tint.
*/
void ProgressBar::SetStripe(float portion, float tint)
{
	fBarTop = portion;
	fBarTopTint = tint;
}


/**
	Sets the border style.
	- B_NO_BORDER means no border, which leaves more room for the progress bar.
	- B_PLAIN_BORDER is a one pixel wide bevel.
	- B_FANCY_BORDER is a two pixels wide ridge-like border.
*/
void ProgressBar::SetBorderStyle(border_style border)
{
	fBorderStyle = border;
}


/**
	Sets the current bar value without redrawing.
*/
void ProgressBar::SetCurrentValue(float value)
{
	if (value <= 0)
		fCurrent = 0;
	else if (value <= fMaxValue)
		fCurrent = value;
	else
		fCurrent = fMaxValue;
}


/**
	Sets the maximum value.
*/
void ProgressBar::SetMaxValue(float value)
{
	fMaxValue = value;
}


/**
	Sets the overlaid bar text.
*/
void ProgressBar::SetText(const char *text)
{
	fText = text;
}


/**
	Sets CurrentValue() to 0 and optionally sets Text().
	Unlike BStatusBar::Reset() this does nothing to MaxValue().
*/
void ProgressBar::Reset(const char *text)
{
	SetCurrentValue(0);
	if (text)
		SetText(text);
	Invalidate();
}


/**
	Bumps CurrentValue() by 'delta' and optionally sets Text().
*/
void ProgressBar::Update(float delta, const char *text)
{
	SetCurrentValue(fCurrent + delta);
	if (text)
		SetText(text);
	Invalidate();
}



