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
#include <QSettings>

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
	void readSettings();      // Чтение настроек

public:  // Get
	SDayInfo dayInfo(QDate _date); // Информация за указанный день
	SSettings settings();
	QDate minDate();           // Минимальная дата

public:  // Set
	void setSettings(SSettings _settings);
	void addTimeEvent(QDate _date, QTime _time, Event _event);
	void updateDayInfo(QDate _date, bool _nonWorking, int _correction);
	void setWarnings(bool enabled);

private:
	QSqlDatabase db;
	QSettings *conf;
	SSettings confCache;
};

#endif // QLOCALDATABASE_H
