#include <QApplication>
#include "phonebook.h"

int main(int argc, char *argv[])
{
	
	QApplication app(argc, argv);
		
	Phonebook pb;
	pb.resize(480, 500);
	pb.setWindowTitle("Phonebook Demo");
	pb.show();
	return app.exec();
}

