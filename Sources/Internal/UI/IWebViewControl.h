//
//  IWebViewControl.h
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#ifndef __DAVAENGINE_IWEBVIEWCONTROL_H__
#define __DAVAENGINE_IWEBVIEWCONTROL_H__

#include "Math/MathConstants.h"
#include "Math/Math2D.h"
#include "Math/Vector.h"
#include "Math/Rect.h"

namespace DAVA {

class UIWebView;
class IUIWebViewDelegate
{
public:
	enum eAction
	{
		PROCESS_IN_WEBVIEW = 0,
		PROCESS_IN_SYSTEM_BROWSER,
		NO_PROCESS,
		ACTIONS_COUNT
	};

	virtual eAction URLChanged(DAVA::UIWebView* webview, const String& newURL) = 0;
};


// Common interface for Web View Controls for different platforms.
class IWebViewControl
{
public:
	virtual ~IWebViewControl() {};
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect) = 0;
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen) = 0;
	
	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect) = 0;
	virtual void SetVisible(bool isVisible, bool hierarchic) = 0;
	
	virtual void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView) = 0;
};

};

#endif // __DAVAENGINE_IWEBVIEWCONTROL_H__
