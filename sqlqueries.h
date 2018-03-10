#ifndef SQLQUERIES_H
#define SQLQUERIES_H

#define EXEC_QUERY(x,ret_err)\
{\
	if (!query.exec(x))\
	{\
		QMessageBox::critical(0, "Ошибка выполнения SQL запроса", query.lastError().text(), QMessageBox::Ok);\
	    return ret_err;\
	}\
}

/* Таблица Settings
 * Timeout - время в секундах
 * Таблица из одной строки
 */
#define CREATE_TABLE_SETTINGS \
"\
CREATE TABLE `Settings` \
(\
    `ID`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,\
    `Timeout`       INTEGER NOT NULL UNIQUE, \
    `MinDate`       INTEGER UNIQUE \
);\
"
// ---------------------

/* Таблица Day
 * Date - юлианская дата
 * NonWorking - bool
 * Correction - время коррекции в секундах
 * WorkTotal - время (в секундах) суммарно отработанное за день, без учета коррекции
 */
#define CREATE_TABLE_DAY \
"\
CREATE TABLE `Day` \
(\
	`ID`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,\
	`Date`          INTEGER NOT NULL UNIQUE,\
	`NonWorking`    INTEGER NOT NULL,\
    `Correction`    INTEGER NOT NULL DEFAULT 0,\
    `WorkTotal`     INTEGER NOT NULL DEFAULT 0\
);\
"
// ---------------------

/* Таблица Time
 * Date  - юлианская дата
 * Time  - время в секундах от начала дня (0:0:0)
 * Event - событие (0 Вход / 1 Выход)
 */
#define CREATE_TABLE_TIME \
"\
CREATE TABLE `Time` \
(\
	`ID`            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,\
    `Date`          INTEGER NOT NULL,\
	`Time`          INTEGER NOT NULL UNIQUE,\
	`Event`         INTEGER NOT NULL\
);\
"
// ---------------------

#define INSERT_DEF_SETTINGS \
"\
INSERT INTO `Settings` \
(Timeout) \
VALUES (60); \
"
// ---------------------

/* Отметить день нерабочим.
 * %1 - юлианская дата
 */
#define FILL_LAST_DAY \
"\
INSERT INTO `Day` \
(Date, NonWorking) \
VALUES (%1, 1); \
"
// ---------------------

/* Информация за день
 * %1 - юлианская дата
 */
#define SELECT_DAY \
"\
SELECT `NonWorking`, `Correction`, `WorkTotal` \
FROM `Day` \
WHERE `Date` = %1; \
"
// ---------------------

/* Информация о времени за день
 * %1 - юлианская дата
 */
#define SELECT_TIME_OF_DAY \
"\
SELECT Time, Event \
FROM Time \
WHERE Date = %1; \
"
// ---------------------

/* Настройки
 */
#define GET_SETTINGS \
"\
SELECT Timeout, MinDate \
FROM Settings \
WHERE ID = 1; \
"
// ---------------------

/* Настройки
 * %1 - время в секундах
 */
#define UPDATE_SETTINGS \
"\
UPDATE Settings \
SET Timeout = %1 \
WHERE ID = 1; \
"
// ---------------------

/* Обновление дня
 * %1 - юлианская дата
 * %2 - нерабочий день
 * %3 - коррекция (в секундах)
 */
#define UPDATE_DAY \
"\
UPDATE Day \
SET NonWorking = %2, Correction = %3 \
WHERE Date = %1; \
"
// ---------------------

/* добавления дня
 * %1 - юлианская дата
 * %2 - нерабочий день
 * %3 - коррекция (в секундах)
 */
#define INSERT_DAY \
"\
INSERT INTO Day \
(Date, NonWorking, Correction) \
VALUES (%1, %2, %3); \
"
// ---------------------

/* Обновление или добавления дня
 * %1 - юлианская дата
 * %2 - время (в секундах от начала дня)
 * %3 - событие (0 Вход / 1 Выход)
 */
#define ADD_EVENT \
"\
INSERT INTO Time \
(Date, Time, Event) \
VALUES (%1, %2, %3); \
"
// ---------------------

/* Обновление кэша суммарного времени
 * %1 - суммарное время (в секундах)
 * %2 - юлианская дата
 */
#define UPDATE_WORK_TOTAL \
"\
UPDATE Day \
SET WorkTotal = %1 \
WHERE Date = %2; \
"
// ---------------------

/* Воремя последнего входа и текущее закэшированное время
 * %1 - юлианская дата
 */
#define SELECT_LAST_TIMES \
"\
SELECT MAX(t.Time), d.WorkTotal \
FROM Time as t, Day as d \
WHERE t.Date = %1 AND t.Event = 0 AND d.Date = %1; \
"
// ---------------------

/* Минимальная дата
 * %1 - юлианская дата
 */
#define SET_MIN_DATE \
"\
UPDATE Settings \
SET MinDate = %1 \
WHERE ID = 1; \
"
// ---------------------

#endif // SQLQUERIES_H
