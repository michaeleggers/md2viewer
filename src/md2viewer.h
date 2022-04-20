//
//                 MD2 Viewer (c) 1999 by Mete Ciragan
//
// file:           md2viewer.h
// last modified:  Apr 29 1999, Mete Ciragan
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
#ifndef INCLUDED_MD2VIEWER
#define INCLUDED_MD2VIEWER



#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#ifndef INCLUDED_MD2
#include "md2.h"
#endif



#define IDC_MODEL_LOADMODEL			1001
#define IDC_MODEL_LOADWEAPON		1002
#define IDC_MODEL_UNLOADMODEL		1003
#define IDC_MODEL_UNLOADWEAPON		1004
#define IDC_MODEL_OPENPAKFILE		1005
#define IDC_MODEL_OPENPAKFILE2		1006
#define IDC_MODEL_CLOSEPAKFILE		1007
#define IDC_MODEL_RECENTMODELS1		1008
#define IDC_MODEL_RECENTMODELS2		1009
#define IDC_MODEL_RECENTMODELS3		1010
#define IDC_MODEL_RECENTMODELS4		1011
#define IDC_MODEL_RECENTPAKFILES1	1012
#define IDC_MODEL_RECENTPAKFILES2	1013
#define IDC_MODEL_RECENTPAKFILES3	1014
#define IDC_MODEL_RECENTPAKFILES4	1015
#define IDC_MODEL_EXIT				1016

#define IDC_SKIN_MODELSKIN			1021
#define IDC_SKIN_WEAPONSKIN			1022
#define IDC_SKIN_BACKGROUND			1023
#define IDC_SKIN_WATER				1024
#define IDC_SKIN_SCREENSHOT			1025

#define IDC_OPTIONS_BGCOLOR			1031
#define IDC_OPTIONS_WFCOLOR			1032
#define IDC_OPTIONS_FGCOLOR			1033
#define IDC_OPTIONS_LIGHTCOLOR		1034
#define IDC_OPTIONS_CENTERMODEL		1035
#define IDC_OPTIONS_GENERATELIGHTNORMALS	1036

#define IDC_VIEW_FILEASSOCIATIONS	1041

#define IDC_HELP_GOTOHOMEPAGE		1051
#define IDC_HELP_ABOUT				1052

#define IDC_RENDERMODE				2001
#define IDC_WATER					2002
#define IDC_LIGHT					2003
#define IDC_BRIGHTNESS				2004
#define IDC_SHININESS				2005
#define IDC_BACKGROUND				2006
#define IDC_TEXTURELIMIT			2007

#define IDC_ANIMATION				3001
#define IDC_INTERPOLATE				3002
#define IDC_GLCOMMANDS				3003
#define IDC_PITCH					3004
#define IDC_PAUSE					3005
#define IDC_FRAME					3006
#define IDC_INCFRAME				3007
#define IDC_DECFRAME				3008

#define IDC_FULLSCREEN				4001



class mxTab;
class mxMenuBar;
class mxButton;
class mxLineEdit;
class mxLabel;
class mxChoice;
class mxCheckBox;
class mxSlider;
class GlWindow;
class PAKViewer;
class PlayerModels;



class MD2Viewer : public mxWindow
{
	mxMenuBar *mb;
	mxTab *tab;

	mxChoice *cRenderMode;
	mxCheckBox *cbWater, *cbLight, *cbBackground;

	mxChoice *cAnim;
	mxButton *bPause;
	mxLineEdit *leFrame;
	mxButton *bDecFrame, *bIncFrame, *bSetFrame;

	mxLabel *lModelInfo1, *lModelInfo2;

	mxLineEdit *leWidth, *leHeight;
	mxCheckBox *cb3dfxOpenGL, *cbCDS;

	GlWindow *glw;
	PAKViewer *pakViewer;

	md2_model_t *md2Model;
	md2_model_t *md2Weapon;

	void loadRecentFiles ();
	void saveRecentFiles ();
	void initRecentFiles ();

	bool loadModel (const char *ptr, int pos);
	void setModelInfo (md2_model_t *model, int pos);
	void initAnimation (md2_model_t *model, int animation);
	void fullscreen ();

public:
	friend PAKViewer;
	friend PlayerModels;

	// CREATORS
	MD2Viewer ();
	~MD2Viewer ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	void redraw ();
	void makeScreenShot (const char *filename);
	void setRenderMode (int index);
	void centerModel ();

	// ACCESSORS
	mxMenuBar *getMenuBar () const { return mb; }
};



extern MD2Viewer *g_md2Viewer;



#endif // INCLUDED_MD2VIEWER