#pragma once
#include "ui_ftnoir_ftnclientcontrols.h"
#include <QUdpSocket>
#include <QThread>
#include <cmath>
#include "opentrack/plugin-support.hpp"
#include "opentrack/plugin-api.hpp"
#include "opentrack-compat/options.hpp"


using namespace options;

struct gvrs_settings : opts {
    value<int> port;
    gvrs_settings() :
        opts("gvrs-tracker"),
		port(b, "port", 4242)
    {}
};


class GVRS_Tracker : public ITracker, protected QThread
{
public:
	GVRS_Tracker();
    ~GVRS_Tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;
protected:
	void run() override;
private:
    QUdpSocket sock;
    double last_recv_pose[6];
    QMutex mutex;
    gvrs_settings s;
    volatile bool should_quit;

	mem<dylib> arucolib;
	mem<ITracker> pTracker;

};

class GVRS_TrackerControls: public ITrackerDialog
{
    Q_OBJECT
public:
	GVRS_TrackerControls();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
	Ui::UICFTNClientControls ui;
    gvrs_settings s;
private slots:
	void doOK();
	void doCancel();
};

class GVRS_TrackerDll : public Metadata
{
public:
    QString name() { return QString("GVRS sender"); }
    QIcon icon() { return QIcon(":/images/gvrs.png"); }
};
