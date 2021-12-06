#include <QApplication>
#include "mapviewer.h"
int main(int argc, char *argv[])
{
	
	QApplication app(argc, argv);
		
	Mapviewer ta;
	ta.resize(380, 565);
	ta.setWindowTitle("Mapviewer");
	ta.show();
	return app.exec();
}
