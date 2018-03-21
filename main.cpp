#include "QBookkeepingWatchdog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QBookkeepingWatchdog w;
	w.showWindow();

	return a.exec();
}
