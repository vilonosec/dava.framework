#ifndef __DAVAENGINE_PARTICLE_LAYER_3D_H__
#define __DAVAENGINE_PARTICLE_LAYER_3D_H__

#include "Base/BaseTypes.h"
#include "Particles/ParticleLayer.h"
#include "Math/Matrix4.h"


namespace DAVA
{

class RenderDataObject;
class Material;
class Camera;
class ParticleLayer3D : public ParticleLayer
{
public:
	ParticleLayer3D();
	virtual ~ParticleLayer3D();

	virtual void LoadFromYaml(const String & configPath, YamlNode * node);
	virtual ParticleLayer * Clone(ParticleLayer * dstLayer = 0);

	virtual void Draw(Camera * camera);

	Material * GetMaterial();
	
	virtual void SetAdditive(bool additive);

protected:
	RenderDataObject * renderData;
	Vector<float32> verts;
	Vector<float32> textures;
	Vector<uint32> colors;
    
public:
    //INTROSPECTION_EXTEND(ParticleLayer3D, ParticleLayer,
    //    MEMBER(material, "Material", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
   // );
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_3D_H__
