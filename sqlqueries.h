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
 * MinDate - минимально доступная дата
 * Таблица из одной строки
 */
const QString CREATE_TABLE_SETTINGS = R"(
CREATE TABLE Settings
(
    ID      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
    MinDate INTEGER UNIQUE
);
)";
// ---------------------

/* Таблица Day
 * Date - юлианская дата
 * NonWorking - bool
 * Correction - время коррекции в секундах
 */
const QString CREATE_TABLE_DAY = R"(
CREATE TABLE Day
(
    ID            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
    Date          INTEGER NOT NULL UNIQUE,
    NonWorking    INTEGER NOT NULL,
    Correction    INTEGER NOT NULL DEFAULT 0
);
)";
// ---------------------

/* Таблица Time
 * Date  - юлианская дата
 * Time  - время в секундах от начала дня (0:0:0)
 * Event - событие (0 Вход / 1 Выход)
 */
const QString CREATE_TABLE_TIME = R"(
CREATE TABLE Time
(
    ID      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
    Date    INTEGER NOT NULL,
    Time    INTEGER NOT NULL,
    Event   INTEGER NOT NULL
);
)";
// ---------------------

/* Отметить день нерабочим.
 * %1 - юлианская дата
 */
const QString FILL_LAST_DAY = R"(
INSERT INTO
    Day
    (Date, NonWorking)
VALUES
    (%1, 1);
)";
// ---------------------

/* Информация за день
 * %1 - юлианская дата
 */
const QString SELECT_DAY = R"(
SELECT
    NonWorking,
    Correction
FROM
    Day
WHERE
    Date = %1;
)";
// ---------------------

/* Информация о времени за день
 * %1 - юлианская дата
 */
const QString SELECT_TIME_OF_DAY = R"(
SELECT
    Time,
    Event
FROM
    Time
WHERE
    Date = %1;
)";
// ---------------------

/* Настройки
 */
const QString GET_MINDATE = R"(
SELECT
    MinDate
FROM
    Settings
WHERE
    ID = 1;
)";
// ---------------------

/* Настройки
 * %1 - время в секундах
 */
const QString UPDATE_SETTINGS = R"(
UPDATE
   Settings
SET
   Timeout = %1
WHERE
   ID = 1;
)";
// ---------------------

/* Обновление дня
 * %1 - юлианская дата
 * %2 - нерабочий день
 * %3 - коррекция (в секундах)
 */
const QString UPDATE_DAY = R"(
UPDATE
    Day
SET
    NonWorking = %2,
    Correction = %3
WHERE
    Date = %1;
)";
// ---------------------

/* добавления дня
 * %1 - юлианская дата
 * %2 - нерабочий день
 * %3 - коррекция (в секундах)
 */
const QString INSERT_DAY = R"(
INSERT INTO
    Day
    (Date, NonWorking, Correction)
VALUES
    (%1, %2, %3);
)";
// ---------------------

/* Обновление или добавления дня
 * %1 - юлианская дата
 * %2 - время (в секундах от начала дня)
 * %3 - событие (0 Вход / 1 Выход)
 */
const QString ADD_EVENT = R"(
INSERT INTO
    Time
    (Date, Time, Event)
VALUES
    (%1, %2, %3);
)";
// ---------------------

/* Воремя последнего входа и текущее закэшированное время
 * %1 - юлианская дата
 */
const QString SELECT_LAST_TIMES = R"(
SELECT
    MAX(t.Time),
    d.WorkTotal
FROM
    Time as t, Day as d
WHERE
    t.Date = %1 AND
    t.Event = 0 AND
    d.Date = %1;
)";
// ---------------------

/* Минимальная дата
 * %1 - юлианская дата
 */
const QString SET_MIN_DATE = R"(
UPDATE
    Settings
SET
    MinDate = %1
WHERE
    ID = 1;
)";
// ---------------------

#endif // SQLQUERIES_H
