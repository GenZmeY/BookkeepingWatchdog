#include "QLocalDataBase.h"

QLocalDataBase::QLocalDataBase(QObject *parent) : QObject(parent)
{

}

bool QLocalDataBase::connect()
{
	db = QSqlDatabase::addDatabase("QSQLITE");
	QString dbName(qApp->applicationDirPath() + "/db.sqlite");
	QFile dbFile(dbName);
	db.setDatabaseName(dbName);
	if (dbFile.exists())
	{
		if (db.open()) return true;
		else QMessageBox::critical(0, "Ошибка", QString("Не удается подключиться к локальной БД.\n%1").arg(db.lastError().text()), QMessageBox::Ok);
	}
	else
	{
		if (db.open()) { return prepare(); }
		else QMessageBox::critical(0, "Ошибка", QString("Не удается подключиться к локальной БД.\n%1").arg(db.lastError().text()), QMessageBox::Ok);
	}
	return false;
}

bool QLocalDataBase::prepare()
{
	QSqlQuery query;

	EXEC_QUERY(CREATE_TABLE_SETTINGS,false);
	EXEC_QUERY(CREATE_TABLE_DAY,false);
	EXEC_QUERY(CREATE_TABLE_TIME,false);
	EXEC_QUERY(INSERT_DEF_SETTINGS,false);

	QDate cd = QDate::currentDate();
	QDate monthBegin = QDate(cd.year(),cd.month(),1);
	QDate weekBegin  = cd.addDays(1 - cd.dayOfWeek());
	QDate min        = monthBegin < weekBegin ? monthBegin : weekBegin;

	for (QDate it = min; it < cd; it = it.addDays(1))
	{
		EXEC_QUERY(QString(FILL_LAST_DAY).arg(it.toJulianDay()),false);
	}

	EXEC_QUERY(QString(SET_MIN_DATE).arg(min.toJulianDay()),false);

	return true;
}

SDayInfo QLocalDataBase::dayInfo(QDate _date)
{
	QSqlQuery query;
	SDayInfo selectedDate;

	EXEC_QUERY(QString(SELECT_DAY).arg(_date.toJulianDay()),selectedDate);
	if (query.first()) // Есть запись об этом дне
	{
		selectedDate.nonWorking = query.value("NonWorking").toBool();
		selectedDate.correction = query.value("Correction").toInt();
		selectedDate.workTotal  = query.value("WorkTotal").toInt();

		EXEC_QUERY(QString(SELECT_TIME_OF_DAY).arg(_date.toJulianDay()),selectedDate);
		while (query.next())
		{
			int time = query.value("Time").toInt();
			Event event = (Event)query.value("Event").toInt();
			selectedDate.timeEvent.insert(QTime::fromMSecsSinceStartOfDay(time*1000),event);
		}

		if  (_date < QDate::currentDate() &&
			!selectedDate.timeEvent.empty() &&
			selectedDate.timeEvent.last() == Event::Enter
			) // Кто то забыл нажать кнопку
		{
			addTimeEvent(_date,QTime(23,59,59),Event::Leave);
			selectedDate.timeEvent.insert(QTime(23,59,59),Event::Leave);
		}
	}
	else // Нет записи об этом дне
	{
		selectedDate.nonWorking = (_date.dayOfWeek() == 6 || _date.dayOfWeek() == 7);
		selectedDate.correction = 0;
		updateDayInfo(_date,selectedDate.nonWorking,selectedDate.correction);
	}

	return selectedDate;
}

int QLocalDataBase::timeout()
{
	QSqlQuery query;

	EXEC_QUERY(GET_SETTINGS,0); query.first();

	return (query.value("Timeout").toInt());
}

QDate QLocalDataBase::minDate()
{
	QSqlQuery query;

	EXEC_QUERY(GET_SETTINGS,QDate()); query.first();

	return (QDate::fromJulianDay(query.value("MinDate").toLongLong()));
}

void QLocalDataBase::setSettings(int _timeout)
{
	QSqlQuery query;

	EXEC_QUERY(QString(UPDATE_SETTINGS).arg(_timeout),);
}

void QLocalDataBase::addTimeEvent(QDate _date, QTime _time, Event _event)
{
	QSqlQuery query;
	EXEC_QUERY(QString(SELECT_DAY).arg(_date.toJulianDay()),);
	if (!query.first()) // Записи о дне нет, надо создать
	{
		updateDayInfo(_date, (_date.dayOfWeek() == 6 || _date.dayOfWeek() == 7),0);
	}

	EXEC_QUERY(QString(ADD_EVENT).arg(_date.toJulianDay()).arg(_time.msecsSinceStartOfDay()/1000).arg((int)_event),);

	if (_event == Event::Leave)
	{
		EXEC_QUERY(QString(SELECT_LAST_TIMES).arg(_date.toJulianDay()),);
		query.first();
		int lastEnterTime = query.value("MAX(t.Time)").toInt();
		int workTotal = query.value("WorkTotal").toInt();
		workTotal += (_time.msecsSinceStartOfDay()/1000 - lastEnterTime);
		EXEC_QUERY(QString(UPDATE_WORK_TOTAL).arg(workTotal).arg(_date.toJulianDay()),);
	}
}

void QLocalDataBase::updateDayInfo(QDate _date, bool _nonWorking, int _correction)
{
	QSqlQuery query;

	EXEC_QUERY(QString(SELECT_DAY).arg(_date.toJulianDay()),);
	if (query.first()) // Есть такой
	{
		EXEC_QUERY(QString(UPDATE_DAY).arg(_date.toJulianDay()).arg((int)_nonWorking).arg(_correction),);
	}
	else // нет такого
	{
		EXEC_QUERY(QString(INSERT_DAY).arg(_date.toJulianDay()).arg((int)_nonWorking).arg(_correction),);
	}
}
