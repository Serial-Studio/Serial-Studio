#include <QApplication>
#include "multidemo.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
		
	Multidemo ta;
	ta.resize(480,640);
	ta.setWindowTitle("QMapControl Demo");
	ta.show();
	return app.exec();
}
