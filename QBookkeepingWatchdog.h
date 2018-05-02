#ifndef QBOOKKEEPINGWATCHDOG_H
#define QBOOKKEEPINGWATCHDOG_H

#include <QMainWindow>
#include <QtSql>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMessageBox>
#include <QSql>
#include <QFocusEvent>
#include <QDesktopWidget>
#include <QMovie>
#include <QLineEdit>

#include "Constants.h"
#include "Structs.h"
#include "QLocalDataBase.h"

namespace Ui { class QBookkeepingWatchdog; }

class QBookkeepingWatchdog : public QMainWindow
{
	Q_OBJECT

public:
	explicit QBookkeepingWatchdog(QWidget *parent = 0);
	~QBookkeepingWatchdog();
	void showWindow();

private:
	Ui::QBookkeepingWatchdog *ui;
	QDesktopWidget desktop;
	QLocalDataBase db;
	bool trayEventBlock;
	bool hideBlock;
	QTimer leaveTimer;
	QTimer statUpdateTimer;
	int timerProgress;
	State trayState;

	bool warnPayday;
	bool warnPrepayment;
	bool warnNkt;
	bool warnHome;
	bool warnWork;
	bool warnEat;
	bool warnDone;

private:
	void setConnections();
	void setDefaults();
	void setTray();
	void updateWindow();

	void addServiceRow(QString text);
	void updateServiceRow(QString time);
	void removeServiceRow();

	void enableInterface(bool enable = true);
	void enableEnter(bool enable = true);
	void enableLeave(bool enable = true);
	void enableEdit(bool enable = true);

	void setIcon(State _state);
	void setStat(QLabel *lb, int sec, QString color = "");

	QDate paydayCorrected(int day);

private slots:
	void enter();
	void leave();
	void settings();
	void accept();
	void back();
	void dayInfoChanged();
	void updateMainWindow(QDate _date);
	void settingsChanged();
	void leaveTick();
	void statUpdateTick();
	void statCalc(QDate _date);
	void correctionSignChanged();
	void showWarning(QString res, int msec = 10000);
	void hideWarning();
	void showWarnPayday();     // -
	void showWarnPrepayment(); // -
	void showWarnNkt();        // +
	void showWarnHome();       // +
	void showWarnWork();       // +
	void showWarnDone();       // +
	void showWarnEat();        // +

private slots: // Tray
	void changeEvent(QEvent*);
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void unblockTrayEvent();

private:
	QMenu   *trayIconMenu;
	QAction *actionSettings;
	QAction *actionLeave;
	QAction *actionEnter;
	QAction *actionQuit;
	QSystemTrayIcon *trayIcon;

private:
	QMovie animation;
};

#endif // QBOOKKEEPINGWATCHDOG_H
