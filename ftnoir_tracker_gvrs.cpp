#include "ftnoir_tracker_gvrs.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack/plugin-support.hpp"
#include <QCoreApplication>
#include <QLibrary>
#include "opentrack/selected-libraries.cpp"

#if defined(__APPLE__)
#   define SONAME "dylib"
#elif defined(_WIN32)
#   define SONAME "dll"
#else
#   define SONAME "so"
#endif

GVRS_Tracker::GVRS_Tracker() : last_recv_pose { 0,0,0, 0,0,0 }, should_quit(false) {
	// Find for aruco tracker and load it
	Modules m;
	for (auto x : m.trackers())
	{
		if(x->name == "aruco -- paper marker tracker")
		{
			arucolib = x;
			break;
		}
	}
	
	// create an instance
	if (arucolib != nullptr && arucolib->Constructor)
	{
		pTracker = make_instance<ITracker>(arucolib);
	}
	
	
}

GVRS_Tracker::~GVRS_Tracker()
{
    should_quit = true;
    wait();		
	
    if (pTracker)
    {
		pTracker = nullptr;
    }

	if (arucolib != nullptr){
        arucolib = nullptr;
	}
		
}

void GVRS_Tracker::run() {

    QByteArray datagram;
    datagram.resize(sizeof(last_recv_pose));
    (void) sock.bind(QHostAddress::Any, (int) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    for (;;) {
        if (should_quit)
            break;
        QMutexLocker foo(&mutex);
        while (sock.hasPendingDatagrams()) {
            sock.readDatagram((char * ) last_recv_pose, sizeof(double[6]));
        }
        msleep(1);
    }
}

void GVRS_Tracker::start_tracker(QFrame* videoframe)
{
	start();
	
	// start aruco tracker
	if(pTracker)
		pTracker->start_tracker(videoframe);
}

void GVRS_Tracker::data(double *data)
{
	double ardata[6] {0,0,0, 0,0,0};
    QMutexLocker foo(&mutex);
	
	// read data
	if(pTracker)
		pTracker->data(ardata);

	// Set Aruco data (x,y,z)
    for (int i = 0; i < 3; i++)
        data[i] = ardata[i];

	// set udp data (yaw,pitch,roll)
    for (int i = 3; i < 6; i++)
        data[i] = last_recv_pose[i];
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new GVRS_Tracker;
}
