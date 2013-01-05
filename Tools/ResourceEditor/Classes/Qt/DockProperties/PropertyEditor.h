#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

#include "QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneData.h"

class DAVA::SceneNode;

class PropertyEditor : public QtPropertyEditor
{
	Q_OBJECT

public:
	PropertyEditor(QWidget *parent = 0);
	~PropertyEditor();

	void SetNode(DAVA::SceneNode *node);

public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);
	void sceneNodeSelected(SceneData *scene, DAVA::SceneNode *node);

protected:
	DAVA::SceneNode *curNode;
};

#endif // __QT_PROPERTY_WIDGET_H__