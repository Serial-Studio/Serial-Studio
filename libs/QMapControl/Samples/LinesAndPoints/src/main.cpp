#include <QApplication>
#include "linesandpoints.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
		
	LinesAndPoints ta;
	ta.resize(480,640);
	ta.setWindowTitle("QMapControl Demo");
	ta.show();
	return app.exec();
}
