#ifndef __DAVAENGINE_RENDEROPTIONS_H__
#define __DAVAENGINE_RENDEROPTIONS_H__

#include "Base/BaseTypes.h"
#include "Base/Observable.h"

namespace DAVA
{

class RenderOptions : public Observable
{
public:
	enum RenderOption
	{
		LANDSCAPE_DRAW = 0,
		
		OPAQUE_DRAW,
		TRANSPARENT_DRAW,
		MATERIAL_DRAW,
		SPRITE_DRAW,

		IMPOSTERS_ENABLE,
		FOG_ENABLE,

		SHADOWVOLUME_DRAW,

		OPTIONS_COUNT
	};

	bool IsOptionEnabled(RenderOption option);
	void SetOption(RenderOption option, bool value);

private:
	RenderOptions();

	bool options[OPTIONS_COUNT];

	friend class RenderManager;
};

};

#endif //__DAVAENGINE_RENDEROPTIONS_H__
