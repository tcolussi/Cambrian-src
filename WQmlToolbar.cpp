#include "WQmlToolbar.h"
#include <QQuickView>
#include <QQmlContext>


WQmlToolbar::WQmlToolbar(QWidget *parent) :
	QDockWidget(parent)
	{
	// self flags
	this->setFeatures(QDockWidget::NoDockWidgetFeatures);
	setObjectName("Dashboard");

	// child qml view
	QQuickView *pQmlView = new QQuickView();
	QWidget *pContainerWidget = QWidget::createWindowContainer(pQmlView, this);
	pContainerWidget->setMinimumSize(1024, 74);


	//QQmlEngine *pQmlEngine = pQmlView->engine();
	pQmlView->setSource(QUrl(QStringLiteral("qrc:/ui/main.qml")));


	}
