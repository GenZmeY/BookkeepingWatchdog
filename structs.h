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
	QMap<int,Event> timeEvent;

	int workTotal()
	{
		QMapIterator<int,Event> it(timeEvent);

		int sumEnter = 0;
		int sumLeave = 0;

		for (int row = 0; it.hasNext(); row++)
		{
			it.next();
			if (it.value() == Event::Enter)
				sumEnter += it.key();
			else
				sumLeave += it.key();
		}

		if (!timeEvent.isEmpty() && timeEvent.last() == Event::Enter)
			sumLeave += QTime::currentTime().msecsSinceStartOfDay()/1000;

		return (sumLeave - sumEnter + correction);
	}
};

struct SSettings // Настройки
{
	int    timeout;
	Period period;
	bool   warnEnabled = false;
	bool   warnPayday;
	bool   warnPrepayment;
	bool   warnNkt;
	bool   warnHome;
	bool   warnWork;
	bool   warnEat;
	bool   warnDone;
};

#endif // STRUCTS_H
