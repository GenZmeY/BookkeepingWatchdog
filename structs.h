#ifndef STRUCTS_H
#define STRUCTS_H

#include <QMap>
#include <QDate>
#include <QTime>

#include "Constants.h"

struct SDayInfo // Инфо за указанный день
{
	QDate date;
	bool nonWorking;
	int correction;
	int workTotal;
	QMap<QTime,Event> timeEvent;
};

#endif // STRUCTS_H
