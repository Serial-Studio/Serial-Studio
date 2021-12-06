#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <QtGui>
#include "../../../qmapcontrol.h"
using namespace qmapcontrol;
class Phonebook: public QMainWindow
{
	Q_OBJECT
	public:
		Phonebook(QWidget* parent = 0);
		~Phonebook();

private:
	MapControl* mc;
	Layer* friends;
	Point* friendpoint;
      void createActions();
      void createMenus();
	
      QMenu *fileMenu;
      QMenu *editMenu;
      QMenu *helpMenu;
      QToolBar *fileToolBar;
      QToolBar *editToolBar;
      QAction *newAct;
      QAction *openAct;
      QAction *saveAct;
      QAction *saveAsAct;
      QAction *exitAct;
      QAction *cutAct;
      QAction *copyAct;
      QAction *pasteAct;
      QAction *aboutAct;
      QAction *aboutQtAct;
	
	public slots:
		void selectedName(QListWidgetItem* item);
};

#endif
