//
//                 MD2 Viewer (c) 1999 by Mete Ciragan
//
// file:           PlayerModels.h
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.4
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#ifndef INCLUDED_PLAYERMODELS
#define INCLUDED_PLAYERMODELS



#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif



#define IDC_PLAYERMODELS		1001
#define IDC_REFRESH				1002



class mxTreeView;
class mxLineEdit;
class mxButton;



class PlayerModels : public mxWindow
{
	mxTreeView *tvModels;
	mxLineEdit *leDir;
	mxButton *bRefresh;

public:
	// CREATORS
	PlayerModels (mxWindow *window);
	~PlayerModels ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);

	void refresh ();
};



#endif // INCLUDED_PLAYERMODELS