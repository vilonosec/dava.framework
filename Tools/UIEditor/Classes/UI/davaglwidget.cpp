#include "DAVAEngine.h"

#include "davaglwidget.h"
#include "ui_davaglwidget.h"

#include <QApplication>
#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>

#include "controllist.h"
#include "ScreenWrapper.h"
#include "HierarchyTreeController.h"
#include "ItemsCommand.h"
#include "CommandsController.h"

#if defined (__DAVAENGINE_MACOS__)
#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
#include "Platform/Qt/Win32/QtLayerWin32.h"
#include "Platform/Qt/Win32/CorePlatformWin32.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

DavaGLWidget::DavaGLWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DavaGLWidget)
	, maxFPS(60)
	, minFrameTimeMs(0)
{
	ui->setupUi(this);

	// Widget will try to expand to maximum available size
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Init OS-specific layer
	{

#if defined (__DAVAENGINE_MACOS__)
		setMouseTracking(true);
		DAVA::QtLayerMacOS *qtLayer = (DAVA::QtLayerMacOS *) DAVA::QtLayer::Instance();
		qtLayer->InitializeGlWindow((void *)this->winId(), this->size().width(), this->size().height());
#elif defined (__DAVAENGINE_WIN32__)
		DAVA::QtLayerWin32 *qtLayer = (DAVA::QtLayerWin32 *) DAVA::QtLayer::Instance();
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		qtLayer->SetWindow(hInstance, this->winId(), this->size().width(), this->size().height());
		qtLayer->OnResume();
#else
		DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)

		DAVA::QtLayer::Instance()->SetDelegate(this);
		DAVA::QtLayer::Instance()->Resize(size().width(), size().height());

	}

	// Disable Widget blinking
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	// Setup FPS
	SetMaxFPS(maxFPS);

	// start render in 1 ms
	QTimer::singleShot(1, this, SLOT(Render()));

	// acept drops
	this->setAcceptDrops(true);	
	setMouseTracking(true);

	ScreenWrapper::Instance()->SetQtScreen(this);
}

DavaGLWidget::~DavaGLWidget()
{
	delete ui;
}

QPaintEngine *DavaGLWidget::paintEngine() const
{
	return NULL;
}

void DavaGLWidget::paintEvent(QPaintEvent *)
{
	// We have custom rendering (by timer), so here nothing to do
	return;
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);

	QPoint newPosition = mapTo(parentWidget(), QPoint(0, 0));
	DAVA::QtLayer::Instance()->Resize(e->size().width(), e->size().height());

	//YZ fix load resource
	Core::Instance()->RegisterAvailableResourceSize((int32)e->size().width(), (int32)e->size().height(), "Gfx");
	ScreenWrapper::Instance()->RequestUpdateView();
}

void DavaGLWidget::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);

	DAVA::QtLayer::Instance()->OnResume();
}

void DavaGLWidget::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);

	DAVA::QtLayer::Instance()->OnSuspend();
}

void DavaGLWidget::focusInEvent(QFocusEvent *e)
{
	QWidget::focusInEvent(e);

	DAVA::QtLayer::Instance()->LockKeyboardInput(true);
}

void DavaGLWidget::focusOutEvent(QFocusEvent *e)
{
	QWidget::focusOutEvent(e);

	DAVA::InputSystem::Instance()->GetKeyboard()->ClearAllKeys();
	DAVA::QtLayer::Instance()->LockKeyboardInput(false);
}

#if defined(Q_WS_WIN)
bool DavaGLWidget::winEvent(MSG *message, long *result)
{
	DAVA::CoreWin32Platform *core = dynamic_cast<DAVA::CoreWin32Platform *>(DAVA::CoreWin32Platform::Instance());
	if (NULL != core)
	{
		if(NULL != message && 
			(message->message == WM_LBUTTONDOWN ||
			message->message == WM_RBUTTONDOWN ||
			message->message == WM_MBUTTONDOWN))
		{
			core->SetFocused(true);
		}

		return core->WinEvent(message, result);
	}

	return false;
}
#endif //#if defined(Q_WS_WIN)

#if defined (Q_WS_MAC)
void DavaGLWidget::mouseMoveEvent(QMouseEvent *e)
{
	DAVA::QtLayerMacOS *qtLayer = dynamic_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
	if(qtLayer)
	{
        QPointF p = e->posF();
		qtLayer->MouseMoved((float32) p.x(), (float32) p.y());
	}

	QWidget::mouseMoveEvent(e);
}
#endif //#if defined (Q_WS_MAC)

void DavaGLWidget::wheelEvent(QWheelEvent *e)
{
	QWidget::wheelEvent(e);

	if (e->modifiers() & Qt::ControlModifier)
	{
		float delta = e->delta() * 0.001;
		ScreenWrapper::Instance()->UpdateScale(delta);
	}
}

void DavaGLWidget::dropEvent(QDropEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (!controlData)
		return;

	CreateControlCommand* cmd = new CreateControlCommand(controlData->GetControlName(), event->pos());
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void DavaGLWidget::dragMoveEvent(QDragMoveEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (controlData && ScreenWrapper::Instance()->IsDropEnable(event->pos()))
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void DavaGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (controlData && HierarchyTreeController::Instance()->GetActiveScreen())
		event->acceptProposedAction();
}

void DavaGLWidget::keyPressEvent(QKeyEvent *)
{
	//qDebug("DavaGLWidget::keyPressEvent");
}

void DavaGLWidget::keyReleaseEvent(QKeyEvent *)
{
	//qDebug("DavaGLWidget::keyReleaseEvent");
}

void DavaGLWidget::SetMaxFPS(int fps)
{
	maxFPS = fps;

	if(0 != fps)
	{
		minFrameTimeMs = 1000 / fps;
	}

	DAVA::RenderManager::Instance()->SetFPS(maxFPS);
}

void DavaGLWidget::Render()
{
	QElapsedTimer frameTimer;
	frameTimer.start();

	DAVA::QtLayer::Instance()->ProcessFrame();

	qint64 waitUntilNextFrameMs = (qint64) minFrameTimeMs - frameTimer.elapsed();

	if(waitUntilNextFrameMs <= 0)
	{
		// our render is too slow to reach maxFPS,
		// so we can wait a minimum time
		waitUntilNextFrameMs = 1;
	}

	QTimer::singleShot(waitUntilNextFrameMs, this, SLOT(Render()));
}

void DavaGLWidget::Quit()
{
	DAVA::Logger::Info("[QUIT]");
	exit(0);
}