#include "rectpropertygridwidget.h"
#include "ui_rectpropertygridwidget.h"
#include "PropertiesGridController.h"
#include "PropertyNames.h"

using namespace PropertyNames;

static const QString RECT_PROPERTY_BLOCK_NAME = "Rect";

RectPropertyGridWidget::RectPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::RectPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);
	// This event filter will prevent mouse wheel event when hovered control is not yet selected
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

RectPropertyGridWidget::~RectPropertyGridWidget()
{
    delete ui;
}

void RectPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets. TODO - understand how to re-use property names!
    RegisterSpinBoxWidgetForProperty(propertiesMap, "RelativeX", ui->relativeXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "RelativeY", ui->relativeYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "AbsoluteX", ui->absoluteXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "AbsoluteY", ui->absoluteYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "SizeX", ui->sizeXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "SizeY", ui->sizeYSpinBox);
    
    RegisterSpinBoxWidgetForProperty(propertiesMap, "PivotX", ui->pivotXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "PivotY", ui->pivotYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "Angle", ui->angleSpinBox);
	
	UpdateHorizontalWidgetsState();
	UpdateVerticalWidgetsState();
}

void RectPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterSpinBoxWidget(ui->relativeXSpinBox);
    UnregisterSpinBoxWidget(ui->relativeYSpinBox);
    UnregisterSpinBoxWidget(ui->absoluteXSpinBox);
    UnregisterSpinBoxWidget(ui->absoluteYSpinBox);
    UnregisterSpinBoxWidget(ui->sizeXSpinBox);
    UnregisterSpinBoxWidget(ui->sizeYSpinBox);
    UnregisterSpinBoxWidget(ui->pivotXSpinBox);
    UnregisterSpinBoxWidget(ui->pivotYSpinBox);
    UnregisterSpinBoxWidget(ui->angleSpinBox);
}

void RectPropertyGridWidget::OnPropertiesChangedFromExternalSource()
{
    // Re-read all the properties related to this grid.
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        UpdateWidgetWithPropertyValue(iter);
    }
}

void RectPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
	//If one of the align option state is changed we should check it and disable/enable appropriate Relative postion or size spinbox(es)
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
	
	if (propertyName == LEFT_ALIGN_ENABLED || propertyName == RIGHT_ALIGN_ENABLED || propertyName == HCENTER_ALIGN_ENABLED)
	{
		UpdateHorizontalWidgetsState();
	}

	if (propertyName == TOP_ALIGN_ENABLED || propertyName == BOTTOM_ALIGN_ENABLED || propertyName == VCENTER_ALIGN_ENABLED)
	{
		UpdateVerticalWidgetsState();
	}
}

void RectPropertyGridWidget::UpdateHorizontalWidgetsState()
{
	// Get horizontal align properties values
	bool leftAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(LEFT_ALIGN_ENABLED);
	bool hcenterAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(HCENTER_ALIGN_ENABLED);
	bool rightAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(RIGHT_ALIGN_ENABLED);
		
	// Change relative X position spinbox state according to align properties
	bool disableRelativeX = leftAlignEnabled || hcenterAlignEnabled || rightAlignEnabled;
	ui->relativeXSpinBox->setDisabled(disableRelativeX);
	ui->absoluteXSpinBox->setDisabled(disableRelativeX);
		
	// Change size X spinbox state according to align properties
	bool disableSizeX = IsTwoAlignsEnabled(leftAlignEnabled, hcenterAlignEnabled, rightAlignEnabled);
	ui->sizeXSpinBox->setDisabled(disableSizeX);
}

void RectPropertyGridWidget::UpdateVerticalWidgetsState()
{
	// Get vertical align properties values
	bool topAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(TOP_ALIGN_ENABLED);
	bool vcenterAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(VCENTER_ALIGN_ENABLED);
	bool bottomAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(BOTTOM_ALIGN_ENABLED);
		
	// Change relative Y position spinbox state according to align properties
	bool disableRelativeY = topAlignEnabled || vcenterAlignEnabled || bottomAlignEnabled;
	ui->relativeYSpinBox->setDisabled(disableRelativeY);
	ui->absoluteYSpinBox->setDisabled(disableRelativeY);
		
	// Change size Y spinbox state according to align properties
	bool disableSizeY = IsTwoAlignsEnabled(topAlignEnabled, vcenterAlignEnabled, bottomAlignEnabled);
	ui->sizeYSpinBox->setDisabled(disableSizeY);
}

bool RectPropertyGridWidget::IsTwoAlignsEnabled(bool first, bool center, bool second)
{
	return ((first && center) || (center && second) || (first && second));
}