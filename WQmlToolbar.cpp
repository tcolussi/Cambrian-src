#include "WQmlToolbar.h"
#include <QQuickView>
#include <QQmlContext>


WQmlToolbar::WQmlToolbar(QWidget *parent) :
	QDockWidget(parent)
	{
	// self flags
	setFeatures(QDockWidget::NoDockWidgetFeatures);
	setObjectName("Toolbar");

	QWidget* pwWidgetLayout = new QWidget();	// Since the QDockWidget can handle only one widget, we create a widget with a layout inside

	OLayoutHorizontal *pLayout = new OLayoutHorizontal(pwWidgetLayout);
	pLayout->setMargin(0);

	pwWidgetLayout->setStyleSheet("background-color: yellow");
	pwWidgetLayout->setLayout(pLayout);

	QQuickView *pQmlView = new QQuickView();
	//QQmlEngine *pQmlEngine = pQmlView->engine();
	pQmlView->setSource(QUrl(QStringLiteral("qrc:/ui/main.qml")));

	QWidget *pContainerWidget = QWidget::createWindowContainer(pQmlView);
	pLayout->addWidget(pContainerWidget);
	setWidget(pwWidgetLayout);


	#if 0
	OLayoutVertical * pLayoutVertical = new OLayoutVertical(pwWidgetLayout);		// Vertical layout to stack the profile switcher, search, tree view and the status.

	pwWidgetLayout->setStyleSheet("background-color: yellow");
	setWidget(pwWidgetLayout);


	/*QWidget *pDummy = new QWidget();
	pLayout->addWidget(pDummy);
	pDummy->setStyleSheet("background-color: yellow");
	this->setWidget(pDummy);
	*/

	// child qml view


	pContainerWidget->setMinimumSize(1024, 74);
	pLayoutVertical->addWidget(pContainerWidget);

	#endif


	}
