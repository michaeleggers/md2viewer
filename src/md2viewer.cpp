//
//                 MD2 Viewer (c) 1999 by Mete Ciragan
//
// file:           md2viewer.cpp
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
#include <stdio.h>
#include <stdlib.h>
//#include <ostream.h>
#include <mx/mx.h>
#include <mx/gl.h>
#include <mx/mxTga.h>
#include "md2viewer.h"
#include "md2.h"
#include "GlWindow.h"
#include "pakviewer.h"
#include "FileAssociation.h"



MD2Viewer *g_md2Viewer = 0;



static char recentFiles[8][256] = { "", "", "", "", "", "", "", "" };



void
MD2Viewer::initRecentFiles ()
{
	for (int i = 0; i < 8; i++)
	{
		if (strlen (recentFiles[i]))
		{
			mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, recentFiles[i]);
		}
		else
		{
			mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, "(empty)");
			mb->setEnabled (IDC_MODEL_RECENTMODELS1 + i, false);
		}
	}
}



void
MD2Viewer::loadRecentFiles ()
{
	FILE *file = fopen ("md2.rf", "rb");
	if (file)
	{
		fread (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
}



void
MD2Viewer::saveRecentFiles ()
{
	char path[256];

	strcpy (path, mx::getApplicationPath ());
	strcat (path, "/md2.rf");

	FILE *file = fopen (path, "wb");
	if (file)
	{
		fwrite (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
}




MD2Viewer::MD2Viewer ()
: mxWindow (0, 0, 0, 0, 0, "MD2 Viewer 1.4", mxWindow::Normal)
{
	// create menu stuff
	mb = new mxMenuBar (this);
	mxMenu *menuModel = new mxMenu ();
	mxMenu *menuSkin = new mxMenu ();
	mxMenu *menuOptions = new mxMenu ();
	mxMenu *menuView = new mxMenu ();
	mxMenu *menuHelp = new mxMenu ();

	mb->addMenu ("Model", menuModel);
	mb->addMenu ("Skin", menuSkin);
	mb->addMenu ("Options", menuOptions);
	mb->addMenu ("View", menuView);
	mb->addMenu ("Help", menuHelp);

	mxMenu *menuRecentModels = new mxMenu ();
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS1);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS2);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS3);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS4);

	mxMenu *menuRecentPakFiles = new mxMenu ();
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES1);
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES2);
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES3);
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES4);


	menuModel->add ("Load Model...", IDC_MODEL_LOADMODEL);
	menuModel->add ("Load Weapon...", IDC_MODEL_LOADWEAPON);
	menuModel->addSeparator ();
	menuModel->add ("Unload Model", IDC_MODEL_UNLOADMODEL);
	menuModel->add ("Unload Weapon", IDC_MODEL_UNLOADWEAPON);
	menuModel->addSeparator ();
	menuModel->add ("Open PAK file...", IDC_MODEL_OPENPAKFILE);
	//menuModel->add ("Open Models from PAK file...", IDC_MODEL_OPENPAKFILE2);
	menuModel->add ("Close PAK file", IDC_MODEL_CLOSEPAKFILE);
	menuModel->addSeparator ();
	menuModel->addMenu ("Recent Models", menuRecentModels);
	menuModel->addMenu ("Recent PAK files", menuRecentPakFiles);
	menuModel->addSeparator ();
	menuModel->add ("Exit", IDC_MODEL_EXIT);

	menuSkin->add ("Load Model Skin...", IDC_SKIN_MODELSKIN);
	menuSkin->add ("Load Weapon Skin...", IDC_SKIN_WEAPONSKIN);
	menuSkin->addSeparator ();
	menuSkin->add ("Load Background Texture...", IDC_SKIN_BACKGROUND);
	menuSkin->add ("Load Water Texture...", IDC_SKIN_WATER);
#ifdef WIN32
	menuSkin->addSeparator ();
	menuSkin->add ("Make Screenshot...", IDC_SKIN_SCREENSHOT);
#endif
	menuOptions->add ("Background Color...", IDC_OPTIONS_BGCOLOR);
	menuOptions->add ("Wireframe Color...", IDC_OPTIONS_WFCOLOR);
	menuOptions->add ("Shade Color...", IDC_OPTIONS_FGCOLOR);
	menuOptions->add ("Light Color...", IDC_OPTIONS_LIGHTCOLOR);
	menuOptions->addSeparator ();
	menuOptions->add ("Center Model", IDC_OPTIONS_CENTERMODEL);
	menuOptions->add ("Generate Light Normals", IDC_OPTIONS_GENERATELIGHTNORMALS);

	menuView->add ("File Associations...", IDC_VIEW_FILEASSOCIATIONS);

#ifdef WIN32
	menuHelp->add ("Goto Homepage...", IDC_HELP_GOTOHOMEPAGE);
	menuHelp->addSeparator ();
#endif
	menuHelp->add ("About", IDC_HELP_ABOUT);

	// create tabcontrol with subdialog windows
	tab = new mxTab (this, 0, 0, 0, 0);
#ifdef WIN32
	SetWindowLong ((HWND) tab->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	mxWindow *wAnim = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wInfo = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wView = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wFullScreen = new mxWindow (this, 0, 0, 0, 0);

	// and add them to the tabcontrol
	tab->add (wView, "View");
	tab->add (wAnim, "Animation");
	tab->add (wInfo, "Model Info");
#ifdef WIN32
	tab->add (wFullScreen, "Full Screen");
#endif

	// Create widgets for the View Tab
	cRenderMode = new mxChoice (wView, 5, 5, 200, 22, IDC_RENDERMODE);
	cRenderMode->add ("Wireframe");
	cRenderMode->add ("Flat shaded");
	cRenderMode->add ("Smooth Shaded");
	cRenderMode->add ("Textured");
	cRenderMode->select (0);
	cbWater = new mxCheckBox (wView, 5, 30, 100, 22, "Enable Water", IDC_WATER);
	cbLight = new mxCheckBox (wView, 5, 55, 100, 22, "Enable Light", IDC_LIGHT);
	cbLight->setChecked (true);
	mxSlider *slBrightness = new mxSlider (wView, 105, 32, 100, 16, IDC_BRIGHTNESS);
	slBrightness->setRange (0, 100);
	slBrightness->setValue (5);
	mxToolTip::add (slBrightness, "Brightness - don't forget to reload skin!");
	mxCheckBox *cbShininess = new mxCheckBox (wView, 105, 55, 100, 22, "Shininess", IDC_SHININESS);
	cbShininess->setChecked (true);
	cbBackground = new mxCheckBox (wView, 205, 55, 100, 22, "Background", IDC_BACKGROUND);
	mxChoice *cTextureLimit = new mxChoice (wView, 210, 5, 60, 22, IDC_TEXTURELIMIT);
	cTextureLimit->add ("512");
	cTextureLimit->add ("256");
	cTextureLimit->add ("128");
	cTextureLimit->select (1);
	mxToolTip::add (cTextureLimit, "Choose Texture Size Limit");

	// Create widgets for the Animation Tab
	cAnim = new mxChoice (wAnim, 5, 5, 200, 22, IDC_ANIMATION);

	mxCheckBox *cbInterp = new mxCheckBox (wAnim, 5, 30, 100, 22, "Interpolate", IDC_INTERPOLATE);
	cbInterp->setChecked (true);

	mxCheckBox *cbGlCmds = new mxCheckBox (wAnim, 105, 30, 100, 22, "Strips and Fans", IDC_GLCOMMANDS);
	cbGlCmds->setChecked (true);

	mxSlider *slPitch = new mxSlider (wAnim, 5, 55, 200, 22, IDC_PITCH);
	slPitch->setRange (1, 200);
	slPitch->setValue (125);
	mxToolTip::add (slPitch, "Frame Animation Speed (Pitch)");

	bPause = new mxButton (wAnim, 210, 5, 79, 22, "Pause", IDC_PAUSE);

	bDecFrame = new mxButton (wAnim, 211, 31, 20, 20, "<", IDC_DECFRAME);
	bDecFrame->setEnabled (false);
	mxToolTip::add (bDecFrame, "Decrease Current Frame");

	leFrame = new mxLineEdit (wAnim, 232, 30, 35, 22, "0");
	leFrame->setEnabled (false);
	mxToolTip::add (leFrame, "Current Frame");

	bSetFrame = new mxButton (wAnim, 232, 51, 35, 20, "Set", IDC_FRAME);
	bSetFrame->setEnabled (false);
	mxToolTip::add (bSetFrame, "Set Current Frame");

	bIncFrame = new mxButton (wAnim, 268, 31, 20, 20, ">", IDC_INCFRAME);
	bIncFrame->setEnabled (false);
	mxToolTip::add (bIncFrame, "Increase Current Frame");

	// Create widgets for the Model Info Tab
	lModelInfo1 = new mxLabel (wInfo, 5, 5, 150, 125, "No Model.");
	lModelInfo2 = new mxLabel (wInfo, 155, 5, 150, 125, "No Weapon.");

#ifdef WIN32
	// Create widgets for the Fullscreen Tab
	mxLabel *lResolution = new mxLabel (wFullScreen, 5, 7, 50, 18, "Resolution");
	leWidth = new mxLineEdit (wFullScreen, 5, 5, 50, 22, "800");
	mxLabel *lX = new mxLabel (wFullScreen, 65, 7, 22, 22, "x");
	leHeight = new mxLineEdit (wFullScreen, 82, 5, 50, 22, "600");
	cb3dfxOpenGL = new mxCheckBox (wFullScreen, 5, 30, 130, 22, "3Dfx OpenGL (Mesa)");
	cbCDS = new mxCheckBox (wFullScreen, 5, 55, 130, 22, "Change Display Mode");
	cbCDS->setChecked (true);
	mxButton *bView = new mxButton (wFullScreen, 140, 5, 75, 22, "Fullscreen!", IDC_FULLSCREEN);
#endif

	// create the OpenGL window
	glw = new GlWindow (this, 0, 0, 0, 0, "", mxWindow::Normal);
#ifdef WIN32
	SetWindowLong ((HWND) glw->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	// finally create the pakviewer window
	pakViewer = new PAKViewer (this);

	g_FileAssociation = new FileAssociation ();

	loadRecentFiles ();
	initRecentFiles ();

	setBounds (20, 20, 690, 550);
	setVisible (true);
}



MD2Viewer::~MD2Viewer ()
{
	saveRecentFiles ();

#ifdef WIN32
	//
	// rename opengl drivers to mesa
	//
	char szName[256], szPath[256];

	GetModuleFileName (NULL, szName, 256);
	char *ptr = strrchr (szName, '\\');
	*ptr = '\0';
	GetCurrentDirectory (256, szPath);
	SetCurrentDirectory (szName);

	CopyFile ("opengl32.dll", "mesagl.dll", FALSE);
	DeleteFile ("opengl32.dll");
	CopyFile ("glu32.dll", "mesaglu.dll", FALSE);
	DeleteFile ("glu32.dll");

	DeleteFile ("md2.cfg");

	SetCurrentDirectory (szPath);
#endif
}



int
MD2Viewer::handleEvent (mxEvent *event)
{
	switch (event->event)
	{
	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_MODEL_LOADMODEL:
		case IDC_MODEL_LOADWEAPON:
		{
			const char *ptr = mxGetOpenFileName (this, 0, "*.md2");
			if (ptr)
			{
				if (!loadModel (ptr, event->action - IDC_MODEL_LOADMODEL))
				{
					char str[256];

					sprintf (str, "Error reading model: %s", ptr);
					mxMessageBox (this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
					break;
				}

				// now update recent files list

				int i;
				char path[256];

				if (event->action == IDC_MODEL_LOADMODEL)
					strcpy (path, "[m] ");
				else
					strcpy (path, "[w] ");

				strcat (path, ptr);

				for (i = 0; i < 4; i++)
				{
					if (!mx_strcasecmp (recentFiles[i], path))
						break;
				}

				// swap existing recent file
				if (i < 4)
				{
					char tmp[256];
					strcpy (tmp, recentFiles[0]);
					strcpy (recentFiles[0], recentFiles[i]);
					strcpy (recentFiles[i], tmp);
				}

				// insert recent file
				else
				{
					for (i = 3; i > 0; i--)
						strcpy (recentFiles[i], recentFiles[i - 1]);

					strcpy (recentFiles[0], path);
				}

				initRecentFiles ();
			}
		}
		break;

		case IDC_MODEL_UNLOADMODEL:
			setModelInfo (0, 0);
			glw->loadModel (0, 0);
			glw->loadTexture (0, TEXTURE_MODEL);
			cAnim->removeAll ();
			glw->setFrameInfo (0, 0);
			glw->redraw ();
			break;

		case IDC_MODEL_UNLOADWEAPON:
			setModelInfo (0, 1);
			glw->loadModel (0, 1);
			glw->loadTexture (0, TEXTURE_WEAPON);
			glw->redraw ();
			break;

		case IDC_MODEL_OPENPAKFILE:
		case IDC_MODEL_OPENPAKFILE2:
		{
			const char *ptr = mxGetOpenFileName (this, "\\Quake2\\baseq2\\", "*.pak");
			if (ptr)
			{
				pakViewer->setLoadEntirePAK (event->action == IDC_MODEL_OPENPAKFILE);
				if (!pakViewer->openPAKFile (ptr))
				{
					mxMessageBox (this, "Error loading PAK file", "ERROR", MX_MB_OK | MX_MB_ERROR);
					break;
				}

				// update recent pak file list

				int i;

				for (i = 4; i < 8; i++)
				{
					if (!mx_strcasecmp (recentFiles[i], ptr))
						break;
				}

				// swap existing recent file
				if (i < 8)
				{
					char tmp[256];
					strcpy (tmp, recentFiles[4]);
					strcpy (recentFiles[4], recentFiles[i]);
					strcpy (recentFiles[i], tmp);
				}

				// insert recent file
				else
				{
					for (i = 7; i > 4; i--)
						strcpy (recentFiles[i], recentFiles[i - 1]);

					strcpy (recentFiles[4], ptr);
				}

				initRecentFiles ();

				redraw ();
			}
		}
		break;

		case IDC_MODEL_CLOSEPAKFILE:
		{
			pakViewer->closePAKFile ();
			redraw ();
		}
		break;

		case IDC_MODEL_RECENTMODELS1:
		case IDC_MODEL_RECENTMODELS2:
		case IDC_MODEL_RECENTMODELS3:
		case IDC_MODEL_RECENTMODELS4:
		{
			int i = event->action - IDC_MODEL_RECENTMODELS1;
			bool isModel = recentFiles[i][1] == 'm';
			char *ptr = &recentFiles[i][4];

			if (!loadModel (ptr, isModel ? 0:1))
			{
				char str[256];

				sprintf (str, "Error reading model: %s", ptr);
				mxMessageBox (this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
				break;
			}

			// update recent model list

			char tmp[256];			
			strcpy (tmp, recentFiles[0]);
			strcpy (recentFiles[0], recentFiles[i]);
			strcpy (recentFiles[i], tmp);

			initRecentFiles ();
		}
		break;

		case IDC_MODEL_RECENTPAKFILES1:
		case IDC_MODEL_RECENTPAKFILES2:
		case IDC_MODEL_RECENTPAKFILES3:
		case IDC_MODEL_RECENTPAKFILES4:
		{
			int i = event->action - IDC_MODEL_RECENTPAKFILES1 + 4;
			pakViewer->setLoadEntirePAK (true);
			pakViewer->openPAKFile (recentFiles[i]);

			char tmp[256];			
			strcpy (tmp, recentFiles[4]);
			strcpy (recentFiles[4], recentFiles[i]);
			strcpy (recentFiles[i], tmp);

			initRecentFiles ();

			redraw ();
		}
		break;

		case IDC_MODEL_EXIT:
			mx::setIdleWindow (0);
			mx::quit ();
			break;

		case IDC_SKIN_MODELSKIN:
		case IDC_SKIN_WEAPONSKIN:
		{
			const char *ptr = mxGetOpenFileName (this, 0, "*.pcx");
			if (ptr)
			{
				glw->loadTexture (ptr, event->action == IDC_SKIN_MODELSKIN ? TEXTURE_MODEL:TEXTURE_WEAPON);
				setRenderMode (3);
				glw->redraw ();
			}
		}
		break;

		case IDC_SKIN_BACKGROUND:
		case IDC_SKIN_WATER:
		{
			const char *ptr = mxGetOpenFileName (this, 0, "*.*");
			if (!ptr)
				break;

			if (glw->loadTexture (ptr, event->action == IDC_SKIN_BACKGROUND ? TEXTURE_BACKGROUND:TEXTURE_WATER))
			{
				if (event->action == IDC_SKIN_BACKGROUND)
				{
					cbBackground->setChecked (true);
					glw->setFlag (F_BACKGROUND, true);
				}
				else
				{
					cbWater->setChecked (true);
					glw->setFlag (F_WATER, true);
				}

				setRenderMode (3);
				glw->redraw ();
			}
			else
				mxMessageBox (this, "Error loading texture.", "MD2 Viewer", MX_MB_OK | MX_MB_ERROR);
		}
		break;

#ifdef WIN32
		case IDC_SKIN_SCREENSHOT:
		{
			const char *ptr = mxGetSaveFileName (this, 0, "*.tga");
			if (ptr)
				makeScreenShot (ptr);
		}
		break;
#endif

		case IDC_OPTIONS_BGCOLOR:
		case IDC_OPTIONS_FGCOLOR:
		case IDC_OPTIONS_WFCOLOR:
		case IDC_OPTIONS_LIGHTCOLOR:
		{
			float r, g, b;
			int ir, ig, ib;

			if (event->action == IDC_OPTIONS_BGCOLOR)
				glw->getBGColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_FGCOLOR)
				glw->getFGColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_WFCOLOR)
				glw->getWFColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
				glw->getLightColor (&r, &g, &b);

			ir = (int) (r * 255.0f);
			ig = (int) (g * 255.0f);
			ib = (int) (b * 255.0f);
			if (mxChooseColor (this, &ir, &ig, &ib))
			{
				if (event->action == IDC_OPTIONS_BGCOLOR)
					glw->setBGColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_FGCOLOR)
					glw->setFGColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_WFCOLOR)
					glw->setWFColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
					glw->setLightColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);

				glw->redraw ();
			}
		}
		break;

		case IDC_OPTIONS_CENTERMODEL:
		{
			centerModel ();
			glw->redraw ();
		}
		break;

		case IDC_OPTIONS_GENERATELIGHTNORMALS:
			md2_generateLightNormals (glw->getModel (0));
			md2_generateLightNormals (glw->getModel (1));
			glw->redraw ();
			break;

		case IDC_VIEW_FILEASSOCIATIONS:
			g_FileAssociation->setAssociation (0);
			g_FileAssociation->setVisible (true);
			break;

#ifdef WIN32
		case IDC_HELP_GOTOHOMEPAGE:
			ShellExecute (0, "open", "http://www.swissquake.ch/chumbalum-soft/index.html", 0, 0, SW_SHOW);
			break;
#endif

		case IDC_HELP_ABOUT:
			mxMessageBox (this,
				"MD2 Viewer 1.4 (c) 1999 by Mete Ciragan\n\n"
				"Left-drag to rotate.\n"
				"Right-drag to zoom.\n"
				"Shift-left-drag to x-y-pan.\n\n"
				"Build:\t" __DATE__ ".\n"
				"Email:\tmete@swissquake.ch\n"
				"Web:\thttp://www.swissquake.ch/chumbalum-soft/", "About MD2 Viewer",
				MX_MB_OK | MX_MB_INFORMATION);
			break;

		// 
		// widget actions
		//
		//

		//
		// Model Panel
		//

		case IDC_RENDERMODE:
			setRenderMode (cRenderMode->getSelectedIndex ());
			glw->redraw ();
			break;

		case IDC_WATER:
			glw->setFlag (F_WATER, cbWater->isChecked ());
			glw->redraw ();
			break;

		case IDC_LIGHT:
			glw->setFlag (F_LIGHT, cbLight->isChecked ());
			glw->redraw ();
			break;

		case IDC_BRIGHTNESS:
			glw->setBrightness (((mxSlider *) event->widget)->getValue ());
			break;

		case IDC_SHININESS:
			glw->setFlag (F_SHININESS, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;

		case IDC_BACKGROUND:
			glw->setFlag (F_BACKGROUND, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;

		case IDC_TEXTURELIMIT:
		{
			int tl[3] = { 512, 256, 128 };
			int index = ((mxChoice *) event->widget)->getSelectedIndex ();
			if (index >= 0)
				glw->setTextureLimit (tl[index]);
		}
		break;

		//
		// Animation Panel
		//
		case IDC_ANIMATION:
		{
			int index = cAnim->getSelectedIndex ();
			if (index >= 0)
			{
				// set the animation
				initAnimation (glw->getModel (0), index - 1);

				// if we pause, update current frame in leFrame
				if (glw->getFlag (F_PAUSE))
				{
					char str[32];
					int frame = glw->getCurrFrame ();
					sprintf (str, "%d", frame);
					leFrame->setLabel (str);
					glw->setFrameInfo (frame, frame);
				}

				glw->redraw ();
			}
		}
		break;

		case IDC_INTERPOLATE:
			glw->setFlag (F_INTERPOLATE, ((mxCheckBox *) event->widget)->isChecked ());
			break;

		case IDC_GLCOMMANDS:
			glw->setFlag (F_GLCOMMANDS, ((mxCheckBox *) event->widget)->isChecked ());
			break;

		case IDC_PITCH:
			glw->setPitch ((float) ((mxSlider *) event->widget)->getValue ());
			break;

		case IDC_PAUSE: // Pause/Play
		{
			bool pause = !glw->getFlag (F_PAUSE);;
			static int startFrame = 0, endFrame = 0, currFrame = 0, currFrame2 = 0;
			static float pol = 0;
			static int index;

			glw->setFlag (F_PAUSE, pause);
			bDecFrame->setEnabled (pause);
			leFrame->setEnabled (pause);
			bIncFrame->setEnabled (pause);
			bSetFrame->setEnabled (pause);

			if (pause)
			{
				char str[32];

				// store current settings
				startFrame = glw->getStartFrame ();
				endFrame = glw->getEndFrame ();
				currFrame = glw->getCurrFrame ();
				currFrame2 = glw->getCurrFrame2 ();
				pol = glw->d_pol;

				sprintf (str, "%d", glw->getCurrFrame ());
				leFrame->setLabel (str);
				bPause->setLabel ("Play");

				index = cAnim->getSelectedIndex ();
			}
			else
			{

				glw->d_startFrame = startFrame;
				glw->d_endFrame = endFrame;
				glw->d_currFrame = currFrame;
				glw->d_currFrame2 = currFrame2;
				glw->d_pol = pol;

				bPause->setLabel ("Pause");

				int index2 = cAnim->getSelectedIndex ();
				if (index != index2 && index2 >= 0)
					initAnimation (glw->getModel (0), index2 - 1);
			}
		}
		break;

		case IDC_DECFRAME:
		{
			int frame = glw->getCurrFrame () - 1;
			glw->setFrameInfo (frame, frame);

			char str[32];
			sprintf (str, "%d", glw->getCurrFrame ());
			leFrame->setLabel (str);
			glw->redraw ();
		}
		break;

		case IDC_FRAME:
		{
			const char *ptr = leFrame->getLabel ();
			if (ptr)
			{
				int frame = atoi (ptr);
				glw->setFrameInfo (frame, frame);

				char str[32];
				sprintf (str, "%d", glw->getCurrFrame ());
				leFrame->setLabel (str);
				glw->redraw ();
			}
		}
		break;

		case IDC_INCFRAME:
		{
			int frame = glw->getCurrFrame () + 1;
			glw->setFrameInfo (frame, frame);

			char str[32];
			sprintf (str, "%d", glw->getCurrFrame ());
			leFrame->setLabel (str);
			glw->redraw ();
		}
		break;

		//
		// Fullscreen Panel
		//
#ifdef WIN32		
		case IDC_FULLSCREEN:
			fullscreen ();
			break;
#endif
		}
	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		int w = event->width;
		int h = event->height;
		int y = mb->getHeight ();
#ifdef WIN32
#define HEIGHT 120
#else
#define HEIGHT 140
		h -= 40;
#endif

		if (pakViewer->isVisible ())
		{
			w -= 170;
			pakViewer->setBounds (w, y, 170, h);
		}

		glw->setBounds (0, y, w, h - HEIGHT);
		tab->setBounds (0, y + h - HEIGHT, w, HEIGHT);
	}
	break;
	} // event->event

	return 1;
}



void
MD2Viewer::redraw ()
{
	mxEvent event;
	event.event = mxEvent::Size;
	event.width = w2 ();
	event.height = h2 ();
	handleEvent (&event);
}



void
MD2Viewer::makeScreenShot (const char *filename)
{
#ifdef WIN32
	glw->redraw ();
	int w = glw->w2 ();
	int h = glw->h2 ();

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
#if 0
		glReadBuffer (GL_FRONT);
		glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image->data);
#else
		HDC hdc = GetDC ((HWND) glw->getHandle ());
		byte *data = (byte *) image->data;
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				COLORREF cref = GetPixel (hdc, x, y);
				data[i++] = (byte) ((cref >> 0)& 0xff);
				data[i++] = (byte) ((cref >> 8) & 0xff);
				data[i++] = (byte) ((cref >> 16) & 0xff);
			}
		}
		ReleaseDC ((HWND) glw->getHandle (), hdc);
#endif
		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", "MD2 Viewer", MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
#endif
}



void
MD2Viewer::setRenderMode (int mode)
{
	if (mode >= 0)
	{
		cRenderMode->select (mode);
		glw->setRenderMode (mode);

		// disable light, if not needed
		glw->setFlag (F_LIGHT, mode != 0);
		cbLight->setChecked (mode != 0);
	}
}



void
MD2Viewer::centerModel ()
{
	if (glw->getModel (0))
	{
		float minmax[6];
		md2_getBoundingBox (glw->getModel (0), minmax);

		// center vertically
		glw->d_transY = (minmax[3] + minmax[2]) / 2;

		// adjust distance
		float dx = minmax[1] - minmax[0];
		float dy = minmax[3] - minmax[2];
		float dz = minmax[5] - minmax[4];

		float d = dx;
		if (dy > d)
			d = dy;

		if (dz > d)
			d = dz;

		glw->d_transZ = d * 1.2f;
		glw->d_transX = 0;
		glw->d_rotX = glw->d_rotY = 0.0f;
	}
}



bool
MD2Viewer::loadModel (const char *ptr, int pos)
{
	md2_model_t *model = glw->loadModel (ptr,  pos);
	if (!model)
		return false;

	char path[256];
	strcpy (path, mx_getpath (ptr));
	mx_setcwd (path);

	if (pos == 0)
	{	
		initAnimation (model, -1);
		setModelInfo (model, 0);
		glw->loadTexture (0, TEXTURE_MODEL);
		setRenderMode (0);
		centerModel ();					
	}
	else
	{
		setModelInfo (model, 1);
		glw->loadTexture (0, TEXTURE_WEAPON);
	}

	strcat (path, "/skin.pcx");
	if (glw->loadTexture (path, pos == 0 ? TEXTURE_MODEL:TEXTURE_WEAPON))
		setRenderMode (3);

	glw->redraw ();

	return true;
}



void
MD2Viewer::setModelInfo (md2_model_t *model, int pos)
{
	static char str[1024];

	if (model)
	{
		sprintf (str,
			"Skins: %d\n"
			"Vertices: %d\n"
			"TexCoords: %d\n"
			"Triangles: %d\n"
			"GlCommands: %d\n"
			"Frames: %d\n",
			model->header.numSkins,
			model->header.numVertices,
			model->header.numTexCoords,
			model->header.numTriangles,
			model->header.numGlCommands,
			model->header.numFrames
			);
	}
	else
	{
		if (pos == 0)
			strcpy (str, "No Model.");
		else
			strcpy (str, "No Weapon.");
	}

	if (pos == 0)
		lModelInfo1->setLabel (str);
	else
		lModelInfo2->setLabel (str);
}



void
MD2Viewer::initAnimation (md2_model_t *model, int animation)
{
	cAnim->removeAll ();

	if (!model)
		return;

	int count = md2_getAnimationCount (model);

	cAnim->add ("<all animations>");

	for (int i = 0; i < count; i++)
		cAnim->add (md2_getAnimationName (model, i));

	int startFrame, endFrame;
	md2_getAnimationFrames (model, animation, &startFrame, &endFrame);
	glw->setFrameInfo (startFrame, endFrame);

	if (animation == -1)
		glw->setFrameInfo (0, model->header.numFrames - 1);

	cAnim->select (animation + 1);
}



#ifdef WIN32
void
MD2Viewer::fullscreen ()
{
	char szName[256], szPath[256];

	GetModuleFileName (NULL, szName, 256);
	char *ptr = strrchr (szName, '\\');
	*ptr = '\0';
	GetCurrentDirectory (256, szPath);
	SetCurrentDirectory (szName);

	if (cb3dfxOpenGL->isChecked ())
	{
		MoveFile ("mesagl.dll", "opengl32.dll");
		MoveFile ("mesaglu.dll", "glu32.dll");
	}
	else
	{
		MoveFile ("opengl32.dll", "mesagl.dll");
		MoveFile ("glu32.dll", "mesaglu.dll");
	}

	FILE *file = fopen ("md2.cfg", "wb");
	if (file)
	{
		int width = atoi (leWidth->getLabel ());
		int height = atoi (leHeight->getLabel ());
		bool cds = cbCDS->isChecked ();

		fwrite (&width, sizeof (int), 1, file);
		fwrite (&height, sizeof (int), 1, file);
		fwrite (&cds, sizeof (bool), 1, file);

		fwrite (modelFile, 256, 1, file);
		fwrite (modelTexFile, 256, 1, file);
		fwrite (weaponFile, 256, 1, file);
		fwrite (weaponTexFile, 256, 1, file);
		fwrite (backgroundTexFile, 256, 1, file);
		fwrite (waterTexFile, 256, 1, file);

		fwrite (&glw->d_rotX, sizeof (float), 1, file);
		fwrite (&glw->d_rotY, sizeof (float), 1, file);
		fwrite (&glw->d_transX, sizeof (float), 1, file);
		fwrite (&glw->d_transY, sizeof (float), 1, file);
		fwrite (&glw->d_transZ, sizeof (float), 1, file);

		fwrite (&glw->d_renderMode, sizeof (int), 1, file);

		fwrite (&glw->d_pol, sizeof (float), 1, file);
		fwrite (&glw->d_currFrame, sizeof (int), 1, file);
		fwrite (&glw->d_currFrame2, sizeof (int), 1, file);
		fwrite (&glw->d_startFrame, sizeof (int), 1, file);
		fwrite (&glw->d_endFrame, sizeof (int), 1, file);
		fwrite (&glw->d_pitch, sizeof (float), 1, file);

		fwrite (glw->d_bgColor, 3 * sizeof (float), 1, file);
		fwrite (glw->d_fgColor, 3 * sizeof (float), 1, file);
		fwrite (glw->d_wfColor, 3 * sizeof (float), 1, file);
		fwrite (glw->d_lightColor, 3 * sizeof (float), 1, file);

		fwrite (&glw->d_bias, sizeof (float), 1, file);
		fwrite (&glw->d_flags, sizeof (int), 1, file);
		fwrite (&glw->d_textureLimit, sizeof (int), 1, file);

		fclose (file);

		//glw->setFlag (F_PAUSE, true);

#if 1

		// pause idle window
		if (!glw->getFlag (F_PAUSE))
		{
			mxEvent event;
			event.event = mxEvent::Action;
			event.action = IDC_PAUSE;
			handleEvent (&event);
		}
#endif

		WinExec ("md2_fs.exe", SW_SHOW);
	}

	SetCurrentDirectory (szPath);
}
#endif



int
main (int argc, char *argv[])
{
	//
	// make sure, we start in the right directory
	//
	mx_setcwd (mx::getApplicationPath ());

	mx::init (argc, argv);

	g_md2Viewer = new MD2Viewer ();
	g_md2Viewer->setMenuBar (g_md2Viewer->getMenuBar ());

	return mx::run ();
}
