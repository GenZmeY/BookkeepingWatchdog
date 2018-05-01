#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QTime>

const int trayBlockInterval       = 250;
const int leaveTimerInterval      = 1  * 1000;
const int statUpdateTimerInterval = 60 * 1000;
const int warningTimerInterval    = 15 * 60 * 1000;

const int defWidth                = 330;
const int defHeight               = 475;

const int workDayRequiredStandart = 8 * 60 * 60;
const int payDayRequiredStandart  = 6 * 60 * 60;

const int payday                  = 7;
const int prepayment              = 22;

const QTime warnTimePayday        = QTime(16,0);

const QString resIcon       = ":/Icon/Res/Icon.png";
const QString resTimeBad    = ":/Tray/Res/TimeBad.png";
const QString resTimeNormal = ":/Tray/Res/TimeNormal.png";
const QString resTimeGood   = ":/Tray/Res/TimeGood.png";
const QString resTimeAFK    = ":/Tray/Res/TimeAFK.png";
const QString resNkt        = ":/Warnings/Res/15minutes.gif";
const QString resPayDay1    = ":/Warnings/Res/PayDay.gif";
const QString resPayDay2    = ":/Warnings/Res/PayDay2.gif";
const QString resWork       = ":/Warnings/Res/Umaru.gif";
const QString resDone       = ":/Warnings/Res/Applo.gif";
const QString resHome       = ":/Warnings/Res/GoHome.png";
const QString resPrepayment = ":/Warnings/Res/Prepayment.gif";
const QString resEat        = ":/Warnings/Res/Eat.gif";

enum class Event : int
{
	Enter      = 0,
	Leave      = 1
};

enum class Page : int
{
	Main       = 0,
	Settings   = 1,
	Warnings   = 2
};

enum class State : int
{
	Nan        = 0,
	Bad        = 1,
	Normal     = 2,
	Good       = 3,
	AFK        = 4
};

enum class Period : int
{
	Month      = 0,
	Week       = 1,
	Day        = 2
};

// PushButtons Style Sheet

const QString pbEnterStyle = "\
QPushButton\
{\
    background-image: url(:/Button/Res/Enter.png);\
    background-position: center;\
    border: none;\
    width: 150px;\
    height: 40px;\
}\
\
QPushButton:hover\
{\
    background-image:url(:/Button/Res/EnterActive.png);\
}\
QPushButton:disabled\
{\
    background-image:url(:/Button/Res/EnterDisabled.png);\
}\
";

const QString pbLeaveStyle = "\
QPushButton\
{\
    background-image: url(:/Button/Res/Leave.png);\
    background-position: center;\
    border: none;\
    width: 150px;\
    height: 40px;\
}\
\
QPushButton:hover\
{\
    background-image:url(:/Button/Res/LeaveActive.png);\
}\
QPushButton:disabled\
{\
    background-image:url(:/Button/Res/LeaveDisabled.png);\
}\
";

#endif // CONSTANTS_H
