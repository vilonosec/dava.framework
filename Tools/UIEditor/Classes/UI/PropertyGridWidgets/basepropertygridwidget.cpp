#include <QLayout>
#include <QColorDialog>

#include "basepropertygridwidget.h"
#include "ui_basepropertygridwidget.h"

#include "PropertiesGridController.h"
#include "HierarchyTreeController.h"
#include "WidgetSignalsBlocker.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"

BasePropertyGridWidget::BasePropertyGridWidget(QWidget *parent) :
    QWidget(parent),
    activeMetadata(NULL),
    ui(new Ui::BasePropertyGridWidget)
{
    PreparePalettes();
    ui->setupUi(this);
    //Register DAVA::Font to QT
    qRegisterMetaType<Font*>("Font");
}

BasePropertyGridWidget::~BasePropertyGridWidget()
{
    delete ui;
}

void BasePropertyGridWidget::SetPropertyBlockName(const QString& blockName)
{
    this->ui->blockNameLabel->setText(blockName);
}

void BasePropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    connect(CommandsController::Instance(), SIGNAL(UpdatePropertyValues()),
            this, SLOT(OnPropertiesChangedExternally()));
    connect(CommandsController::Instance(), SIGNAL(ChangePropertySucceeded(const QString&)),
            this, SLOT(OnChangePropertySucceeded(const QString&)));
    connect(CommandsController::Instance(), SIGNAL(ChangePropertyFailed(const QString&)),
            this, SLOT(OnChangePropertyFailed(const QString&)));
            
    this->activeMetadata = activeMetadata;
}

void BasePropertyGridWidget::Cleanup()
{
    disconnect(CommandsController::Instance(), SIGNAL(UpdatePropertyValues()),
            this, SLOT(OnPropertiesChangedExternally()));
    disconnect(CommandsController::Instance(), SIGNAL(ChangePropertySucceeded(const QString&)),
            this, SLOT(OnChangePropertySucceeded(const QString&)));
    disconnect(CommandsController::Instance(), SIGNAL(ChangePropertyFailed(const QString&)),
            this, SLOT(OnChangePropertyFailed(const QString&)));

    if (this->activeMetadata)
    {
        this->activeMetadata->CleanupParams();
        this->activeMetadata = NULL; // will be released by the owner.
    }
}

// Build the properties map in the "name - value" way to make the search faster.
BasePropertyGridWidget::PROPERTIESMAP BasePropertyGridWidget::BuildMetadataPropertiesMap()
{
    if (activeMetadata == NULL)
    {
        Logger::Error("Call to BuildMetadataPropertiesMap() without activeMetaData defined!");
        return BasePropertyGridWidget::PROPERTIESMAP();
    }
    
    BasePropertyGridWidget::PROPERTIESMAP propertiesMap;
    int32 count = activeMetadata->metaObject()->propertyCount();
    for (int32 i = 0; i < count; i ++)
    {
        const QMetaProperty& curProperty = activeMetadata->metaObject()->property(i);
        //Logger::Debug("Property name %s", curProperty.name());
        propertiesMap.insert(std::make_pair(curProperty.name(), curProperty));
    }
    
    return propertiesMap;
}

// Register grid control as state-aware.
void BasePropertyGridWidget::RegisterGridWidgetAsStateAware()
{
    connect(PropertiesGridController::Instance(),
            SIGNAL(SelectedUIControlStateChanged(UIControl::eControlState)),
            this,
            SLOT(OnSelectedUIControlStateChanged(UIControl::eControlState)));
}

void BasePropertyGridWidget::UnregisterGridWidgetAsStateAware()
{
    disconnect(PropertiesGridController::Instance(),
            SIGNAL(SelectedUIControlStateChanged(UIControl::eControlState)),
            this,
            SLOT(OnSelectedUIControlStateChanged(UIControl::eControlState)));
    
}

void BasePropertyGridWidget::RegisterLineEditWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QLineEdit* lineEditWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }

    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(lineEditWidget,
                                                 PropertyGridWidgetData(curProperty, needUpdateTree, stateAware)));

    // Update the widget with the current property value.
    UpdateLineEditWidgetWithPropertyValue(lineEditWidget, curProperty);

    // Register the signal for this widget.
    connect(lineEditWidget, SIGNAL(editingFinished()), this, SLOT(OnLineEditEditingFinished()));
}

void BasePropertyGridWidget::RegisterSpinBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                              QSpinBox* spinBoxWidget,
                                                              bool needUpdateTree, bool stateAware)
{
    spinBoxWidget->setKeyboardTracking(false);
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(spinBoxWidget,
                                                 PropertyGridWidgetData(curProperty, needUpdateTree, stateAware)));
    
    UpdateSpinBoxWidgetWithPropertyValue(spinBoxWidget, curProperty);
    
    // Register the signal for this widget.
    connect(spinBoxWidget, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
}

void BasePropertyGridWidget::RegisterCheckBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QCheckBox* checkBoxWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }

    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(checkBoxWidget, PropertyGridWidgetData(curProperty,
                                                                                        needUpdateTree, stateAware)));

    UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);

    // Register the signal for this widget.
    connect(checkBoxWidget, SIGNAL(toggled(bool)), this, SLOT(OnCheckBoxStateChanged(bool)));
}


void BasePropertyGridWidget::RegisterComboBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QComboBox *comboBoxWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(comboBoxWidget, PropertyGridWidgetData(curProperty,
                                                                                        needUpdateTree, stateAware)));
    
    UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
    
    // Register the signal for this widget.
    connect(comboBoxWidget, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnComboBoxValueChanged(QString)));
}

void BasePropertyGridWidget::RegisterPushButtonWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QPushButton *pushButtonWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }

    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(pushButtonWidget, PropertyGridWidgetData(curProperty,
                                                                                        needUpdateTree, stateAware)));

    UpdatePushButtonWidgetWithPropertyValue(pushButtonWidget, curProperty);

    // Register the signal for this widget.
    connect(pushButtonWidget, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
}

void BasePropertyGridWidget::RegisterColorButtonWidgetForProperty(const PROPERTIESMAP &propertiesMap, const char *propertyName,
                                                                  QColorButton *colorButtonWidget,
                                                                  bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(colorButtonWidget, PropertyGridWidgetData(curProperty,
                                                                                           needUpdateTree, stateAware)));
    
    UpdateColorButtonWidgetWithPropertyValue(colorButtonWidget, curProperty);
    
    // Register the signal for this widget.
    connect(colorButtonWidget, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()));
}

void BasePropertyGridWidget::UnregisterLineEditWidget(QLineEdit* lineEdit)
{
    disconnect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnLineEditEditingFinished()));
}

void BasePropertyGridWidget::UnregisterSpinBoxWidget(QSpinBox* spinBoxWidget)
{
    disconnect(spinBoxWidget, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
}

void BasePropertyGridWidget::UnregisterCheckBoxWidget(QCheckBox* checkBoxWidget)
{
    disconnect(checkBoxWidget, SIGNAL(toggled(bool)), this, SLOT(OnCheckBoxStateChanged(bool)));
}

void BasePropertyGridWidget::UnregisterComboBoxWidget(QComboBox* comboBoxWidget)
{
    disconnect(comboBoxWidget, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnComboBoxValueChanged(QString)));
}

void BasePropertyGridWidget::UnregisterPushButtonWidget(QPushButton *pushButtonWidget)
{
    disconnect(pushButtonWidget, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
}

void BasePropertyGridWidget::UnregisterColorButtonWidget(QColorButton* colorButtonWidget)
{
    disconnect(colorButtonWidget, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()));
}

void BasePropertyGridWidget::OnLineEditEditingFinished()
{
    QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnLineEditValueChanged - sender is NULL!");
        return;
    }
    
    HandleLineEditEditingFinished(senderWidget);
}

void BasePropertyGridWidget::HandleLineEditEditingFinished(QLineEdit* senderWidget)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnLineEditValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
    
    BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata, iter->second, senderWidget->text());
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void BasePropertyGridWidget::OnSpinBoxValueChanged(int value)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QWidget* senderWidget = dynamic_cast<QWidget*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnSpinBoxValueChanged - sender is NULL!");
        return;
    }

    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnSpinBoxValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

    BaseCommand* command = new ChangePropertyCommand<float>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void BasePropertyGridWidget::OnCheckBoxStateChanged(bool value)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QCheckBox* senderWidget = dynamic_cast<QCheckBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnCheckBoxStateChanged - sender is NULL!");
        return;
    }

    // After the first change don't allow the checkbox to be tristate.
    senderWidget->setTristate(false);
    
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnCheckBoxStateChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

    BaseCommand* command = new ChangePropertyCommand<bool>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void BasePropertyGridWidget::OnComboBoxValueChanged(QString value)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QComboBox* senderWidget = dynamic_cast<QComboBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnComboBoxValueChanged - sender is NULL!");
        return;
    }    
  
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnComboBoxValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
    
    ProcessComboboxValueChanged(senderWidget, iter, value);
}

void BasePropertyGridWidget::OnColorButtonClicked()
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QColorButton* senderWidget = dynamic_cast<QColorButton*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnSelectColorButtonClicked - sender is NULL!");
        return;
    }

    QColor color = QColorDialog::getColor(Qt::white, this, "Select a color",  QColorDialog::DontUseNativeDialog);
    if (color.isValid() == false)
    {
        return;
    }
        
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OpenSelectColorDialog - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
        
    BaseCommand* command = new ChangePropertyCommand<QColor>(activeMetadata, iter->second, color);
    CommandsController::Instance()->ExecuteCommand(command);
    SAFE_DELETE(command);
}

void BasePropertyGridWidget::OnPushButtonClicked()
{    
    QPushButton* senderWidget = dynamic_cast<QPushButton*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnPushButtonClicked - sender is NULL!");
        return;
    }
    
    ProcessPushButtonClicked(senderWidget);
}

void BasePropertyGridWidget::ProcessAttachedData(const PropertyGridWidgetData& attachedData)
{
    if (attachedData.IsNeedUpdateHierarchyTree())
    {
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}

void BasePropertyGridWidget::OnSelectedUIControlStateChanged(UIControl::eControlState newState)
{
    HandleSelectedUIControlStateChanged(newState);
}

void BasePropertyGridWidget::HandleSelectedUIControlStateChanged(UIControl::eControlState newState)
{
    this->activeMetadata->SetUIControlState(newState);
    
    // Look through the map of widgets and update (re-read) properties attached to state-aware
    // widgets.
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        const PropertyGridWidgetData& attachedData = iter->second;
        if (attachedData.IsStateAware() == false)
        {
            continue;
        }
        
        // The widget is state-aware. Update it according to its type.
        UpdateWidgetWithPropertyValue(iter);
    }
}

void BasePropertyGridWidget::UpdateWidgetWithPropertyValue(const PROPERTYGRIDWIDGETSITER& iter)
{
    QWidget* widget = iter->first;
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(widget);
    if (lineEdit)
    {
        UpdateLineEditWidgetWithPropertyValue(lineEdit, iter->second.getProperty());
        return;
    }

    QSpinBox* spinBoxWidget = dynamic_cast<QSpinBox*>(widget);
    if (spinBoxWidget)
    {
        UpdateSpinBoxWidgetWithPropertyValue(spinBoxWidget, iter->second.getProperty());
        return;
    }

    QCheckBox* checkBoxWidget = dynamic_cast<QCheckBox*>(widget);
    if (checkBoxWidget)
    {
        UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, iter->second.getProperty());
        return;
    }

    QComboBox* comboBoxWidget = dynamic_cast<QComboBox*>(widget);
    if (comboBoxWidget)
    {
        UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, iter->second.getProperty());
        return;
    }

    QColorButton* colorButtonWidget = dynamic_cast<QColorButton*>(widget);
    if (colorButtonWidget)
    {
        UpdateColorButtonWidgetWithPropertyValue(colorButtonWidget, iter->second.getProperty());
        return;
    }

    QPushButton *pushButtonWidget = dynamic_cast<QPushButton*>(widget);
    if (pushButtonWidget)
    {
        UpdatePushButtonWidgetWithPropertyValue(pushButtonWidget, iter->second.getProperty());
        return;
    }
   
    // Add other Widget Types here.
    Logger::Error("Unable to determine supported widget type for widget %s!", typeid(*widget).name());
}

void BasePropertyGridWidget::UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget,
                                                                   const QMetaProperty& curProperty)
{
    // Property might have different values for different controls assigned.
    bool isPropertyValueDiffers = false;
    QString propertyValue =
        PropertiesHelper::GetAllPropertyValues<QString>(this->activeMetadata,
                                                        curProperty.name(),
                                                        isPropertyValueDiffers);
    
    {
        WidgetSignalsBlocker blocker(lineEditWidget);
        if (isPropertyValueDiffers)
        {
            lineEditWidget->setText("<multiple>"); // TODO! TO CONST!!!
        }
        else
        {
            // Get the current value.
            Logger::Debug(propertyValue.toStdString().c_str());
            lineEditWidget->setText(propertyValue);
        }
    }

    UpdateWidgetPalette(lineEditWidget, curProperty.name());
}

void BasePropertyGridWidget::UpdateSpinBoxWidgetWithPropertyValue(QSpinBox* spinBoxWidget,
                                                                  const QMetaProperty& curProperty)
{
    // Get the current value.
    bool isPropertyValueDiffers = false;
    float propertyValue =
        PropertiesHelper::GetAllPropertyValues<float>(this->activeMetadata,
                                                      curProperty.name(),
                                                      isPropertyValueDiffers);
    
    {
        // For Spin Box set some value irregardless of isPropertyValueDiffers flag.
        WidgetSignalsBlocker blocker(spinBoxWidget);
        spinBoxWidget->setValue(propertyValue);
    }
    
    UpdateWidgetPalette(spinBoxWidget, curProperty.name());
}

void BasePropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget,
                                                                   const QMetaProperty& curProperty)
{
    // Get the current value.
    bool isPropertyValueDiffers = false;
    bool value = PropertiesHelper::GetAllPropertyValues<bool>(this->activeMetadata,
                                                              curProperty.name(),
                                                              isPropertyValueDiffers);

    {
        WidgetSignalsBlocker blocker(checkBoxWidget);
        if (isPropertyValueDiffers)
        {
            // For Check Box widget we'll set it to tri-state.
            checkBoxWidget->setTristate();
            checkBoxWidget->setCheckState(Qt::PartiallyChecked);
        }
        else
        {
            checkBoxWidget->setTristate(false);
            checkBoxWidget->setChecked(value);
        }
    }
}

void BasePropertyGridWidget::UpdateColorButtonWidgetWithPropertyValue(QColorButton *colorButtonWidget, const QMetaProperty &curProperty)
{
    // Get the current value.
    bool isPropertyValueDiffers = false;
    QColor propertyValue = PropertiesHelper::GetPropertyValue<QColor>(this->activeMetadata,
                                                          curProperty.name(), isPropertyValueDiffers);
    {
        //Set background color
        WidgetSignalsBlocker blocker(colorButtonWidget);
        colorButtonWidget->SetBackgroundColor(propertyValue);

        //Set background image if property is not equal to property for normal state
        colorButtonWidget->SetDisplayMultipleColors(IsActiveStatePropertyDirty(curProperty.name()));
    }
}

void BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox*, const QMetaProperty &)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue is called - you've forgot to create custom handler for some combo!!!");
}

void BasePropertyGridWidget::ProcessComboboxValueChanged(QComboBox*, const PROPERTYGRIDWIDGETSITER&, const QString&)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::ProcessComboboxValueChanged is called - you've forgot to create custom handler for some combo!!!");
}

void BasePropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue(QPushButton *, const QMetaProperty &)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue is called - you've forgot to create custom handler for button!!!");
}

void BasePropertyGridWidget::ProcessPushButtonClicked(QPushButton *)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::ProcessPushButtonClicked is called - you've forgot to create custom handler for this button!!!");
}

void BasePropertyGridWidget::SetComboboxSelectedItem(QComboBox* comboBoxWidget, const QString& selectedItemText)
{
    int index = comboBoxWidget->findText(selectedItemText);
    if ( index != -1 )
    {
        comboBoxWidget->setCurrentIndex(index);
    }
}

void BasePropertyGridWidget::OnPropertiesChangedExternally()
{
    OnPropertiesChangedFromExternalSource();
}

void BasePropertyGridWidget::OnChangePropertySucceeded(const QString& propertyName)
{
    HandleChangePropertySucceeded(propertyName);
}

void BasePropertyGridWidget::OnChangePropertyFailed(const QString& propertyName)
{
    HandleChangePropertyFailed(propertyName);
}

void BasePropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    UpdateWidgetWithPropertyValue(propertyName);
}

void BasePropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    UpdateWidgetWithPropertyValue(propertyName);
}

void BasePropertyGridWidget::UpdateWidgetWithPropertyValue(const QString& propertyName)
{
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        const PropertyGridWidgetData& attachedData = iter->second;
        if (attachedData.getProperty().name() == propertyName)
        {
            UpdateWidgetWithPropertyValue(iter);
            
            // Also do the postprocessing.
            const PropertyGridWidgetData& attachedData = iter->second;
            ProcessAttachedData(attachedData);
            break;
        }
    }
}

bool BasePropertyGridWidget::IsWidgetBoundToProperty(QWidget* widget, const QString& propertyName) const
{
    BasePropertyGridWidget::PROPERTYGRIDWIDGETSCONSTITER iter = this->propertyGridWidgetsMap.find(widget);
    if (iter == this->propertyGridWidgetsMap.end())
    {
        return false;
    }
    
    return (iter->second.getProperty().name() == propertyName);
}

bool BasePropertyGridWidget::IsActiveStatePropertyDirty(const QString& propertyName) const
{
    if (activeMetadata == NULL)
    {
        return false;
    }

    return IsPropertyDirty(propertyName, this->activeMetadata->GetUIControlState());
}

bool BasePropertyGridWidget::IsPropertyDirty(const QString& propertyName, UIControl::eControlState state) const
{
    if (activeMetadata == NULL)
    {
        return false;
    }
    
    // Check for all attached Params.
    int paramsCount = this->activeMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = this->activeMetadata->GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }
        
        if (treeNode->GetExtraData().IsStatePropertyDirty(state, propertyName))
        {
            return true;
        }
    }
    
    // No "Dirty" flag for all attached Params.
    return false;
}

void BasePropertyGridWidget::UpdateWidgetPalette(QWidget* widget, const QString& propertyName)
{
    const QPalette& palette = IsActiveStatePropertyDirty(propertyName) ?
        GetWidgetPaletteForDirtyProperty() :
        GetWidgetPaletteForClearProperty();
    
    widget->setPalette(palette);
}

void BasePropertyGridWidget::PreparePalettes()
{
    dirtyPropertyPalette.setColor(QPalette::Text, Qt::red);
    clearPropertyPalette.setColor(QPalette::Text, Qt::black);
}

const QPalette& BasePropertyGridWidget::GetWidgetPaletteForDirtyProperty() const
{
    return this->dirtyPropertyPalette;
}

const QPalette& BasePropertyGridWidget::GetWidgetPaletteForClearProperty() const
{
    return this->clearPropertyPalette;
}