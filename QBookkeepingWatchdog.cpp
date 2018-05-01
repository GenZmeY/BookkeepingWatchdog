#include "QBookkeepingWatchdog.h"
#include "ui_QBookkeepingWatchdog.h"

QBookkeepingWatchdog::QBookkeepingWatchdog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QBookkeepingWatchdog)
{
	ui->setupUi(this);

	if (qApp->arguments().size() >= 2)
		warningsEnabled = (qApp->arguments().at(1) == "--with-warnings");

	setTray();
	setConnections();
	setDefaults();

	if (!db.connect())
	{
		qApp->quit(); // TODO: не работает, заменить
		enableInterface(false);
	}

	statUpdateTimer.start(statUpdateTimerInterval);
	updateMainWindow(QDate::currentDate());
}

QBookkeepingWatchdog::~QBookkeepingWatchdog()
{
	statUpdateTimer.stop();
	leaveTimer.stop();
	blockTimer.stop();

	delete trayIcon;
	delete trayIconMenu;
	delete actionSettings;
	delete actionLeave;
	delete actionEnter;
	delete actionQuit;
	delete ui;
}

void QBookkeepingWatchdog::setConnections()
{
	// GUI
	connect(ui->pbEnter     , SIGNAL(clicked     (bool )), this, SLOT(enter           (     )));
	connect(ui->pbLeave     , SIGNAL(clicked     (bool )), this, SLOT(leave           (     )));
	connect(ui->pbBack      , SIGNAL(clicked     (bool )), this, SLOT(back            (     )));
	connect(ui->pbAccept    , SIGNAL(clicked     (bool )), this, SLOT(accept          (     )));
	connect(ui->calendar    , SIGNAL(clicked     (QDate)), this, SLOT(updateMainWindow(QDate)));
	connect(ui->correction  , SIGNAL(timeChanged (QTime)), this, SLOT(dayInfoChanged  (     )));
	connect(ui->nonWorking  , SIGNAL(clicked     (bool )), this, SLOT(dayInfoChanged  (     )));
	connect(ui->timeout     , SIGNAL(timeChanged (QTime)), this, SLOT(settingsChanged (QTime)));
	connect(ui->correctionSign, SIGNAL(clicked   (bool )), this, SLOT(correctionSignChanged()));

	// Timers
	connect(&blockTimer     , SIGNAL(timeout     (     )), this, SLOT(blockTick       (     )));
	connect(&leaveTimer     , SIGNAL(timeout     (     )), this, SLOT(leaveTick       (     )));
	connect(&statUpdateTimer, SIGNAL(timeout     (     )), this, SLOT(statUpdateTick  (     )));

	// Tray
	connect(actionQuit      , SIGNAL(triggered   (     )), qApp, SLOT(quit            (     )));
	connect(actionEnter     , SIGNAL(triggered   (bool )), this, SLOT(enter           (     )));
	connect(actionLeave     , SIGNAL(triggered   (bool )), this, SLOT(leave           (     )));
	connect(actionSettings  , SIGNAL(triggered   (bool )), this, SLOT(settings        (     )));
	connect(trayIcon        , SIGNAL(activated   (QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

void QBookkeepingWatchdog::showWarnPayday    () { showWarning(QTime::currentTime().msecsSinceStartOfDay() % 2 == 0 ? resPayDay1 : resPayDay2); warnPayday = true; }
void QBookkeepingWatchdog::showWarnNkt       () { showWarning(resNkt);        warnNkt        = true; }
void QBookkeepingWatchdog::showWarnPrepayment() { showWarning(resPrepayment); warnPrepayment = true; }
void QBookkeepingWatchdog::showWarnHome      () { showWarning(resHome);       warnHome       = true; }
void QBookkeepingWatchdog::showWarnWork      () { showWarning(resWork,7500);  warnWork       = true; }
void QBookkeepingWatchdog::showWarnDone      () { showWarning(resDone);       warnDone       = true; }
void QBookkeepingWatchdog::showWarnEat       () { showWarning(resEat,6600);   warnEat        = true; }

void QBookkeepingWatchdog::setDefaults()
{
	trayEventBlock = false;
	hideBlock = false;
	this->setWindowFlags(Qt::CustomizeWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);// | Qt::FramelessWindowHint);
	ui->pbEnter->setCursor(QCursor(Qt::PointingHandCursor));
	ui->pbLeave->setCursor(QCursor(Qt::PointingHandCursor));
	ui->stackedWidget->setCurrentIndex((int)Page::Main);
	ui->pbEnter->setStyleSheet(pbEnterStyle);
	ui->pbLeave->setStyleSheet(pbLeaveStyle);
	ui->pbAccept->setEnabled(false);
	timerProgress = 0;
	warnPayday     = false;
	warnPrepayment = false;
	warnNkt        = false;
	warnHome       = false;
	warnWork       = false;
	warnEat        = false;
	warnDone       = false;
}

void QBookkeepingWatchdog::setTray()
{
	actionEnter    = new QAction("Я пришёл" , this);
	actionLeave    = new QAction("Я ушёл"   , this);
	actionSettings = new QAction("Настройки", this);
	actionQuit     = new QAction("Выход"    , this);

	trayIconMenu   = new QMenu(this);
	trayIconMenu->addAction(actionEnter);
	trayIconMenu->addAction(actionLeave);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(actionSettings);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(actionQuit);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(QIcon(resIcon));
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->show();
}

void QBookkeepingWatchdog::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	    case QSystemTrayIcon::Trigger:
	    case QSystemTrayIcon::DoubleClick:
		    if (isHidden() && !trayEventBlock)
				showWindow();
				ui->calendar->setSelectedDate(QDate::currentDate());
				updateMainWindow(QDate::currentDate());
		    break;
	    default:
		    break;
	}
}

void QBookkeepingWatchdog::unblockTrayEvent()
{
	trayEventBlock = false;
}

void QBookkeepingWatchdog::changeEvent(QEvent *event)
{
	QMainWindow::changeEvent(event);
	qApp->processEvents(); // Для того чтобы trayIconActivated отработал раньше. В этом случае не будет повторного разворачивания окна
	if (event->type() == QEvent::WindowStateChange && isMinimized() && !hideBlock)
		hide(); // Скрытие при сворачивании (не нужно, но пусть будет)
	if (event->type() == QEvent::ActivationChange && !isActiveWindow() && !hideBlock)
		hide();
	trayEventBlock = true; // костыль в пару
	QTimer::singleShot(trayBlockInterval,this,SLOT(unblockTrayEvent()));
}

void QBookkeepingWatchdog::enter()
{
	QDate cd = QDate::currentDate();

	trayState = State::Nan;
	hideBlock = false;

	if (!leaveTimer.isActive())
	{
		int lastLeave = 0;
		if (!db.dayInfo(cd).timeEvent.isEmpty())
			lastLeave = db.dayInfo(cd).timeEvent.lastKey();

		db.addTimeEvent(cd, QTime::currentTime().addSecs(db.settings().timeout*(-1)),Event::Enter);
		updateMainWindow(cd);
		ui->tableDay->scrollToBottom();

		SDayInfo scd = db.dayInfo(cd);

		if (warningsEnabled)
		{
			if (scd.timeEvent.size() == 1)
			{
				warnPayday     = false;
				warnPrepayment = false;
				warnNkt        = false;
				warnHome       = false;
				warnWork       = false;
				warnEat        = false;
				warnDone       = false;

				if (trayState == State::Bad) QTimer::singleShot(3000,this,SLOT(showWarnWork()));
			}


			int diff = 0;
			if (!db.dayInfo(cd).timeEvent.isEmpty())
				diff = db.dayInfo(cd).timeEvent.lastKey() - lastLeave;

			if (!warnEat && diff >= 30*60 && diff <= 75*60 && lastLeave >= 11*60*60 && lastLeave <= 15*60*60)
				QTimer::singleShot(3000,this,SLOT(showWarnEat()));

			QTime ct = QTime::currentTime();
			if (!warnPayday && cd == paydayCorrected(payday))
				QTimer::singleShot(ct.msecsTo(QTime(16,0)),this,SLOT(showWarnPayday()));

			if (!warnPrepayment && cd == paydayCorrected(prepayment))
				QTimer::singleShot(ct.msecsTo(QTime(16,0)),this,SLOT(showWarnPrepayment()));
		}
	}
	else
	{
		leaveTimer.stop();
		timerProgress = 0;

		enableInterface();
		enableLeave();
		enableEnter(false);

		removeServiceRow();
	}
}

void QBookkeepingWatchdog::leave()
{
	enableInterface(false);
	enableLeave(false);
	enableEnter();
	hideBlock = true;
	if (db.settings().timeout)
	{
		addServiceRow("Ожидание выхода");
		leaveTimer.start(leaveTimerInterval);
		ui->tableDay->scrollToBottom();
	}
	else
	{
		db.addTimeEvent(QDate::currentDate(),QTime::currentTime(),Event::Leave);
		updateMainWindow(QDate::currentDate());
		enableInterface();
		enableEnter();
		ui->tableDay->scrollToBottom();
		trayState = State::AFK;
	}
}

void QBookkeepingWatchdog::leaveTick()
{
	timerProgress += 1; // секунды
	if (timerProgress >= db.settings().timeout)
	{
		leaveTimer.stop();
		timerProgress = 0;
		db.addTimeEvent(QDate::currentDate(),QTime::currentTime(),Event::Leave);
		removeServiceRow();
		addServiceRow("Блокировка входа");
		enableEnter(false);
		blockTimer.start(leaveTimerInterval);
		trayState = State::AFK;
	}
	else
	{
		updateServiceRow(QTime::fromMSecsSinceStartOfDay(db.settings().timeout*1000).addSecs((-1)*timerProgress).toString());
	}
}

void QBookkeepingWatchdog::blockTick()
{
	timerProgress += 1; // секунды
	if (timerProgress >= db.settings().timeout)
	{
		blockTimer.stop();
		timerProgress = 0;
		removeServiceRow();
		updateMainWindow(QDate::currentDate());
		enableInterface();
		enableEnter();
		ui->tableDay->scrollToBottom();
	}
	else
	{
		updateServiceRow(QTime::fromMSecsSinceStartOfDay(db.settings().timeout*1000).addSecs((-1)*timerProgress).toString());
	}
}

void QBookkeepingWatchdog::settings()
{
	ui->timeout->setTime(QTime::fromMSecsSinceStartOfDay(db.settings().timeout*1000));
	ui->stackedWidget->setCurrentIndex((int)Page::Settings);
	showWindow();
}

void QBookkeepingWatchdog::accept()
{
	SSettings s;

	s.timeout = ui->timeout->time().msecsSinceStartOfDay()/1000;

	// TODO

	db.setSettings(s);
	ui->pbAccept->setEnabled(false);
}

void QBookkeepingWatchdog::back()
{
	ui->stackedWidget->setCurrentIndex((int)Page::Main);
}

// TODO: В трёх функциях ниже состояние кнопки со знаком не только используется, но и несколько раз меняется!
// И это работает! Всё потому что я сверхразум.
// Правда, я теперь и сам не понимаю как это работает.
// Надо переписать для простых смертных.
void QBookkeepingWatchdog::correctionSignChanged()
{
	ui->correctionSign->setText(ui->correctionSign->text() == "+" ? "-" : "+");
	dayInfoChanged();
}

void QBookkeepingWatchdog::dayInfoChanged()
{
	int signFactor = ui->correctionSign->text() == "+" ? 1 : -1;
	db.updateDayInfo(ui->calendar->selectedDate(),ui->nonWorking->isChecked(),ui->correction->time().msecsSinceStartOfDay()*signFactor/1000);
	updateMainWindow(ui->calendar->selectedDate());
}

void QBookkeepingWatchdog::updateMainWindow(QDate _date)
{
	if (blockTimer.isActive() || leaveTimer.isActive()) return;
	enableEdit(_date <= QDate::currentDate() && _date >= db.minDate());

	if (_date >= QDate::currentDate()) // немного рушит логику функций...
		ui->nonWorking->setEnabled(true); // но не страшно

	SDayInfo selected = db.dayInfo(_date);
	ui->nonWorking->setChecked(selected.nonWorking);

	int signFactor = selected.correction >= 0 ? 1 : -1;
	ui->correctionSign->setText(selected.correction >= 0 ? "+" : "-");
	ui->correction->setTime(QTime::fromMSecsSinceStartOfDay(selected.correction*signFactor*1000));

	enableEnter(_date == QDate::currentDate() && ( selected.timeEvent.empty() || selected.timeEvent.last() == Event::Leave));
	enableLeave(_date == QDate::currentDate() && (!selected.timeEvent.empty() && selected.timeEvent.last() == Event::Enter));

	// Обновить таблицу
	ui->tableDay->clear();
	ui->tableDay->setColumnCount(2);
	ui->tableDay->setRowCount(selected.timeEvent.size());
	ui->tableDay->setHorizontalHeaderLabels(QStringList() << "Время" << "Событие");

	// Очищать память не нужно, это делает QTableWidget::clear()
	QMapIterator<int, Event> it(selected.timeEvent);
	for (int row = 0; it.hasNext(); row++)
	{
		it.next();
		QTableWidgetItem *cTimeItem = new QTableWidgetItem(QTime::fromMSecsSinceStartOfDay(it.key()*1000).toString());
		QTableWidgetItem *cEventItem = new QTableWidgetItem(it.value() == Event::Enter ? "Вход" : "Выход");
		cTimeItem->setTextAlignment(Qt::AlignCenter);
		cEventItem->setTextAlignment(Qt::AlignCenter);
		ui->tableDay->setItem(row, 0, cTimeItem);
		ui->tableDay->setItem(row, 1, cEventItem);
	}

	if (_date <= QDate::currentDate() && _date >= db.minDate())
	{
		statCalc(_date);
	}
	else
	{
		ui->lbBonusCount->setText(QString("-"));
		ui->lbDayStat->setText(QString("<font color='Black'>--:--</font>"));
		ui->lbWeekStat->setText(QString("<font color='Black'>--:--</font>"));
		ui->lbMonthStat->setText(QString("<font color='Black'>--:--</font>"));
	}
}

void QBookkeepingWatchdog::settingsChanged(QTime _timeout)
{
	ui->pbAccept->setEnabled(db.settings().timeout != _timeout.msecsSinceStartOfDay()/1000);
}

void QBookkeepingWatchdog::showWindow()
{
	int dx = desktop.availableGeometry(-1).x();
	int dy = desktop.availableGeometry(-1).y();
	int dw = desktop.availableGeometry(-1).width ();
	int dh = desktop.availableGeometry(-1).height();
	//int fw = frameGeometry().width();
	//int fh = frameGeometry().height();

	int w = geometry().width ();
	int h = geometry().height();
	int y = dy ? dy : dh - h;
	int x = dx ? dx : dw - w;

	setGeometry(x,y,w,h);
	showNormal();
	setFocus();
	activateWindow();
}

void QBookkeepingWatchdog::addServiceRow(QString text)
{
	ui->tableDay->insertRow(ui->tableDay->rowCount());
	QTableWidgetItem *cTimeItem = new QTableWidgetItem(QTime::fromMSecsSinceStartOfDay(db.settings().timeout*1000).toString());
	QTableWidgetItem *cEventItem = new QTableWidgetItem(text);
	cTimeItem->setTextAlignment(Qt::AlignCenter);
	cEventItem->setTextAlignment(Qt::AlignCenter);
	ui->tableDay->setItem(ui->tableDay->rowCount()-1, 0, cTimeItem);
	ui->tableDay->setItem(ui->tableDay->rowCount()-1, 1, cEventItem);
}

void QBookkeepingWatchdog::updateServiceRow(QString time)
{
	ui->tableDay->item(ui->tableDay->rowCount()-1,0)->setText(time);
}

void QBookkeepingWatchdog::removeServiceRow()
{
	ui->tableDay->removeRow(ui->tableDay->rowCount()-1);
}

void QBookkeepingWatchdog::enableInterface(bool enable)
{
	ui->calendar->setEnabled(enable);
	actionSettings->setEnabled(enable);
	enableEdit(enable);
}

void QBookkeepingWatchdog::enableEnter(bool enable)
{
	ui->pbEnter->setEnabled(enable);
	actionEnter->setEnabled(enable);
}

void QBookkeepingWatchdog::enableLeave(bool enable)
{
	ui->pbLeave->setEnabled(enable);
	actionLeave->setEnabled(enable);
}

void QBookkeepingWatchdog::enableEdit(bool enable)
{
	ui->correction->setEnabled(enable);
	ui->lbCorrection->setEnabled(enable);
	ui->nonWorking->setEnabled(enable);
}

void QBookkeepingWatchdog::statUpdateTick()
{
	statCalc(QDate::currentDate());
}

void QBookkeepingWatchdog::statCalc(QDate _date)
{
	QDate cd = QDate::currentDate();
	QDate monthBegin = QDate(_date.year(),_date.month(),1);
	QDate monthEnd   = QDate(_date.year(),_date.month(), _date.daysInMonth());
	QDate weekBegin  = _date.addDays(1 - _date.dayOfWeek());
	QDate weekEnd    = _date.addDays(7 - _date.dayOfWeek());
	QDate min        = monthBegin < weekBegin ? monthBegin : weekBegin;
	QDate max        = monthEnd   > weekEnd   ? monthEnd   : weekEnd;
		  max        = max        < cd ? max : cd;

	int workMonthRequired = 0; // Необходимый объем работа за месяц
	int workMonthComplete = 0; // Время, отработанное в будни, за месяц
	int workMonthWeekend  = 0; // Время, отработанное в выходные за месяц
	int paydayWeekend     = 0; // Оплачиваемые выходные

	int workWeekRequired  = 0; // Необходимый объем работа за неделю
	int workWeekComplete  = 0; // Время, отработанное в будни, за неделю
	int workWeekWeekend   = 0; // Время, отработанное в выходные за неделю

	int workDayRequired   = 0; // Необходимый объем работа за день
	int workDayComplete   = 0; // Время отработанное за день

	for (QDate dateIt = min; dateIt <= max; dateIt = dateIt.addDays(1))
	{
		SDayInfo date = db.dayInfo(dateIt);

		int workToday = date.workTotal();

		// Текущая наработка
		if (dateIt == cd && !date.timeEvent.isEmpty() && date.timeEvent.last() == Event::Enter)
			workToday += QTime::currentTime().msecsSinceStartOfDay()/1000 - date.timeEvent.lastKey();

		if (dateIt == _date) // За день
		{
			workDayRequired = date.nonWorking ? 0 : workDayRequiredStandart;
			workDayComplete = workToday;

			if (_date == cd && (date.timeEvent.isEmpty() || (!date.timeEvent.isEmpty() && date.timeEvent.last() == Event::Leave)))
			{
				trayState = State::AFK;
				hideBlock = true;
			}
		}
		if (dateIt >= weekBegin && dateIt <= weekEnd) // За неделю
		{
			workWeekRequired += date.nonWorking ? 0 : workDayRequiredStandart;
			workWeekComplete += date.nonWorking ? 0 : workToday;
			workWeekWeekend  += date.nonWorking ? workToday : 0;
		}
		if (dateIt >= monthBegin && dateIt <= monthEnd) // За месяц
		{
			workMonthRequired += date.nonWorking ? 0 : workDayRequiredStandart;
			workMonthComplete += date.nonWorking ? 0 : workToday;
			workMonthWeekend  += date.nonWorking ? workToday : 0;
			paydayWeekend     += (date.nonWorking && workToday > payDayRequiredStandart) ? 1 : 0;
		}
	}

	int diffMonth = workMonthComplete - workMonthRequired;
	if (diffMonth < 0) // есть недоработка
	{
		workMonthWeekend += diffMonth; // компенсируем из выходных (вычитаем разницу из выходных)
		diffMonth = workMonthWeekend;  // Результат - то что осталось по времени.
		if (workMonthWeekend >= 0) // если хватило
		{
			// Пересчитываем чего хватает на оплату
			paydayWeekend = workMonthWeekend/payDayRequiredStandart;
		}
	}

	int diffWeek = workWeekComplete - workWeekRequired;
	if (diffWeek < 0) // есть недоработка
	{
		workWeekWeekend += diffWeek; // компенсируем из выходных (вычитаем разницу из выходных)
		diffWeek = workWeekWeekend;  // Результат - то что осталось по времени.
	}

	int diffDay = workDayComplete - workDayRequired;

	ui->lbBonusCount->setText(QString("%1").arg(paydayWeekend));
	if (cd >= monthBegin && cd <= monthEnd)
	{
		if (diffMonth + workDayRequiredStandart < 0)
			setStat(ui->lbMonthStat,diffMonth,"Red");
		else if (diffMonth + workDayRequiredStandart >= 0 && diffMonth < 0)
			setStat(ui->lbMonthStat,diffMonth,"Black");
		else
			setStat(ui->lbMonthStat,diffMonth,"Green");
	}
	else
	{
		setStat(ui->lbMonthStat,diffMonth);
	}

	if (cd >= weekBegin && cd <= weekEnd)
	{
		if (diffWeek + workDayRequiredStandart < 0)
		{
			setIcon(resTimeBad);
			setStat(ui->lbWeekStat,diffWeek,"Red");
			trayState = State::Bad;
		}
		else if (diffWeek + workDayRequiredStandart >= 0 && diffWeek < 0)
		{
			setIcon(resTimeNormal);
			setStat(ui->lbWeekStat,diffWeek,"Black");
			if (warningsEnabled && !warnDone && trayState == State::Bad)
				QTimer::singleShot(1000,this,SLOT(showWarnDone()));
			trayState = State::Normal;
		}
		else
		{
			setIcon(resTimeGood);
			setStat(ui->lbWeekStat,diffWeek,"Green");
			if (!warnHome && warningsEnabled && trayState == State::Normal) QTimer::singleShot(1000,this,SLOT(showWarnHome()));
			if (!warnNkt  && warningsEnabled && trayState == State::Normal) QTimer::singleShot(warningTimerInterval,this,SLOT(showWarnNkt()));
			trayState = State::Good;
		}
	}
	else
	{
		setStat(ui->lbWeekStat,diffWeek);
	}

	if (ui->calendar->selectedDate() == cd && _date == cd)
	{
		if (diffDay < 0)
			setStat(ui->lbDayStat,diffDay,"Black");
		else
			setStat(ui->lbDayStat,diffDay,"Green");
	}
	else if (ui->calendar->selectedDate() == _date)
	{	
		setStat(ui->lbDayStat,diffDay);
	}
}

void QBookkeepingWatchdog::setIcon(QString ico)
{
	trayIcon->setIcon(QIcon(ico));
	trayIcon->show();
}

void QBookkeepingWatchdog::setStat(QLabel *lb, int sec, QString color)
{
	QString sign = "";
	if      (sec > 0) { sign = "+"; if(color.isEmpty()) color = "Green"; }
	else if (sec < 0) { sign = "-"; if(color.isEmpty()) color = "Red"  ; }
	else              {                                 color = "Black"; }
	QString time = QTime::fromMSecsSinceStartOfDay(sec < 0 ? sec*(-1000) : sec*1000).toString("hh:mm");
	lb->setText(QString("<font color='%1'>%2%3</font>").arg(color).arg(sign).arg(time));
}

void QBookkeepingWatchdog::showWarning(QString res, int msec)
{
	if (hideBlock) return;
	// Пояснение к строке выше:
	// Использование QTimer::singleShot, по факту, вызывает функцию два раза, вместо одного
	// Это - гарантия того что функция не выполнится лишний раз.
	// Почему это происходит - большая загадка для меня
	// Кстати баг плавающий

	hideBlock = true;
	enableInterface(false);
	QPixmap pic(res);
	pic = pic.scaled(width(),height(),Qt::KeepAspectRatio);
	setFixedHeight(pic.height()); // Проблема димана в этой строчке

	if (res.endsWith("gif"))
	{
		animation.setFileName(res);
		ui->lbWarning->setMovie(&animation);
		animation.setScaledSize(QSize(pic.width(),pic.height()));
		animation.start();
	}
	else
	{
		ui->lbWarning->setPixmap(pic);
	}
	ui->stackedWidget->setCurrentIndex((int)Page::Warnings);
	QTimer::singleShot(msec,this,SLOT(hideWarning()));
	showWindow();
}

void QBookkeepingWatchdog::hideWarning()
{
	setFixedWidth(defWidth);
	setFixedHeight(defHeight);
	ui->stackedWidget->setCurrentIndex((int)Page::Main);
	animation.stop();
	hide();
	enableInterface(true);
	hideBlock = false;
}

QDate QBookkeepingWatchdog::paydayCorrected(int day)
{
	QDate cd = QDate::currentDate();
	QDate payday = QDate(cd.year(),cd.month(),day);

	for (int count = 0; ; count--)
		if (!db.dayInfo(payday.addDays(count)).nonWorking)
			return payday.addDays(count);
}
