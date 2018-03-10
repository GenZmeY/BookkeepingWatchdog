#ifndef QLOCALDATABASE_H
#define QLOCALDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QCoreApplication>
#include <QMessageBox>
#include <QSqlQuery>
#include <QFile>
#include <QString>
#include <QDate>
#include <QVariant>

#include "Structs.h"
#include "SqlQueries.h"

class QLocalDataBase : public QObject
{
	Q_OBJECT
public:
	explicit QLocalDataBase(QObject *parent = nullptr);

public:  // Init
	bool connect();           // Подключение к БД

private: // Init
	bool prepare();           // Инициализация БД

public:  // Get
	SDayInfo dayInfo(QDate _date); // Информация за указанный день
	int timeout();           // Время между отрыванием задницы от стула и проходом через турникет (в секундах)
	QDate minDate();           // Минимальная дата

public:  // Set
	void setSettings(int _timeout);
	void addTimeEvent(QDate _date, QTime _time, Event _event);
	void updateDayInfo(QDate _date, bool _nonWorking, int _correction);

private:
	QSqlDatabase db;
};

#endif // QLOCALDATABASE_H
