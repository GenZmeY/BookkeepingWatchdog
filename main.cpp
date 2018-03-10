#include "QBookkeepingWatchdog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QBookkeepingWatchdog w;
	//w.show();

	return a.exec();
}
