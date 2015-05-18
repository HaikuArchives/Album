/**
	Distributed under the MIT License
*/

#ifndef NAMEVALUEITEM_H
#define NAMEVALUEITEM_H

#include <InterfaceKit.h>
#include <TypeConstants.h>
#include <String.h>

class NameValueItem : public BListItem
{
	public:
	
	NameValueItem(const NameValueItem &source);
	NameValueItem(const char *name, const char *value=NULL, uint32 level = 0, bool expanded = true);
	NameValueItem(const char *name, type_code type, const void *value, ssize_t size, uint32 level = 0, bool expanded = true);

	virtual void DrawItem(BView *owner, BRect itemRect, bool fullUpdate = false);
	virtual void Update(BView* owner, const BFont* font);
	float Divider() const;
	bool IsReadOnly() const;
	const char* Name() const;
	void SetName(const char *name);
	const char* Label() const;
	void SetLabel(const char *label);
	uint32 Type() const;
	const char* Value() const;
	void SetReadOnly(bool enable);
	virtual void SetValue(const char *value, bool merge = false);
	status_t SetValue(uint32 type, const void *data, ssize_t size);
	void SetValueColor(rgb_color color);
	

	private:
	
	BString fName, fValue, fLabel;
	rgb_color fValueColor, fNameColor;
	float fDivider;
	bool fReadOnly;
	uint32 fType;
};



// Inlines


#endif
