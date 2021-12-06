#include <QApplication>
#include "gps.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
		
	GPS ta;
	ta.resize(480, 640);

	ta.setWindowTitle("GPS Demo");
	ta.show();
	return app.exec();
}
