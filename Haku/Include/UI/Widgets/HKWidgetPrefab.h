#if !defined(_HKWIDGET_PREFAB_H)
#define _HKWIDGET_PREFAB_H

#include "UI/Widgets/HKWidgetLayout.h"

class HKWidgetPrefab : public HKWidgetLayout
{
public:
	static HKWidget *Create(HKWidgetType *pType);
	static const char *TypeName() { return "Prefab"; }

	HKWidgetPrefab(HKWidgetType *pType);
	virtual ~HKWidgetPrefab();

	virtual void SetProperty(const char *pProperty, const char *pValue);
	virtual MFString GetProperty(const char *pProperty);

	void LoadPrefab(const char *pPrefab);

protected:
	MFString prefab;

	virtual void ArrangeChildren();
};

#endif
