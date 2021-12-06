#ifndef DIALOGS_H
#define DIALOGS_H

#include <QDialog>
#include <QTextEdit>

/**
	@author Kai Winter <kaiwinter@gmx.de>
*/
class InfoDialog : public QDialog
{
	Q_OBJECT
	public:
		InfoDialog(QWidget* parent=0);
		void setInfotext(QString text);
		
	private:
		QTextEdit* infotext;
};

#endif
