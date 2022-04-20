//
//                 MD2 Viewer (c) 1999 by Mete Ciragan
//
// file:           PlayerModels.cpp
// last modified:  May 05 1999, Mete Ciragan
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
#include <string.h>
#include <mx/mx.h>
#include "PlayerModels.h"
#include "GlWindow.h"
#include "md2viewer.h"


char g_qdir[256] = "\\quake2\\baseq2\\players\\";



PlayerModels::PlayerModels (mxWindow *window)
: mxWindow (window, 0, 0, 0, 0)
{
	tvModels = new mxTreeView (this, 0, 0, 0, 0, IDC_PLAYERMODELS);
	leDir = new mxLineEdit (this, 0, 0, 0, 0, g_qdir);
	leDir->setLabel (g_qdir);
	bRefresh = new mxButton (this, 0, 0, 0, 0, "Refresh", IDC_REFRESH);
	refresh ();
}



PlayerModels::~PlayerModels ()
{
	tvModels->remove (0);
}



int
PlayerModels::handleEvent (mxEvent *event)
{
	switch (event->event)
	{
	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_PLAYERMODELS:
		{
			if (event->flags & mxEvent::DoubleClicked)
			{
				mxTreeViewItem *tvItem = tvModels->getSelectedItem ();
				if (tvItem)
				{
					char label[256], dir[256];

					strcpy (label, tvModels->getLabel (tvItem));
					if (strstr (label, ".pcx"))
					{
						const char *ptr = tvModels->getLabel (tvModels->getParent (tvItem));
						if (ptr)
						{
							strcpy (dir, g_qdir);
							strcat (dir, ptr);
							strcat (dir, "\\");
							strcat (dir, label);
							g_md2Viewer->glw->loadTexture (dir, TEXTURE_MODEL);
						}
					}
					else
					{
						md2_model_t *model;

						strcpy (dir, g_qdir);
						strcat (dir, label);
						SetCurrentDirectory (dir);
						strcat (dir, "\\tris.md2");
						model = g_md2Viewer->glw->loadModel (dir, 0);
						if (model)
						{
							g_md2Viewer->initAnimation (model, -1);
							g_md2Viewer->setModelInfo (model, 0);
							g_md2Viewer->centerModel ();
						}

						strcpy (dir, g_qdir);
						strcat (dir, label);
						strcat (dir, "\\weapon.md2");
						model = g_md2Viewer->glw->loadModel (dir, 1);
						if (model)
							g_md2Viewer->setModelInfo (model, 1);

						strcpy (dir, g_qdir);
						strcat (dir, label);
						strcat (dir, "\\weapon.pcx");
						g_md2Viewer->glw->loadTexture (dir, TEXTURE_WEAPON);

						const char *ptr = tvModels->getLabel (tvModels->getFirstChild (tvItem));
						if (ptr)
						{
							char label2[256];
							strcpy (label2, ptr);
							strcpy (dir, g_qdir);
							strcat (dir, label);
							strcat (dir, "\\");
							strcat (dir, label2);
							g_md2Viewer->glw->loadTexture (dir, TEXTURE_MODEL);
							g_md2Viewer->setRenderMode (3);
						}

						g_md2Viewer->glw->redraw ();
					}
				}
			}
		}
		break;

		case IDC_REFRESH:
			refresh ();

		} // event->action
	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		tvModels->setBounds (0, 0, event->width, event->height - 44);
		leDir->setBounds (0, event->height - 44, event->width, 22);
		bRefresh->setBounds (0, event->height - 22, event->width, 22);
	} // mxEvent::Size
	break;

	} // event->event

	return 1;
}



void
PlayerModels::refresh ()
{

	strcpy (g_qdir, leDir->getLabel ());
	strcat (g_qdir, "\\");
	tvModels->remove (0);

#ifdef WIN32
	WIN32_FIND_DATA fd;
	HANDLE handle;
	BOOL bDone = FALSE;
	char dir[256];
	mxTreeViewItem *tvItem;

	strcpy (dir, g_qdir);
	strcat (dir, "*.*");
	memset (&fd, sizeof fd, 0);
	fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	handle = FindFirstFile (dir, &fd);
	if (handle)
	{
		while (!bDone)
		{
			if (strcmp (fd.cFileName, ".") &&
				strcmp (fd.cFileName, ".."))
				tvItem = tvModels->add (0, fd.cFileName);

			HANDLE handle2;
			WIN32_FIND_DATA wfd2;
			BOOL bDone2 = FALSE;
			strcpy (dir, g_qdir);
			strcat (dir, fd.cFileName);
			strcat (dir, "\\*.*");
			memset (&wfd2, sizeof wfd2, 0);
			wfd2.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
			handle2 = FindFirstFile (dir, &wfd2);
			if (handle2)
			{
				while (!bDone2)
				{
					char *filename = _strlwr (wfd2.cFileName);
					if (strstr (filename, ".pcx") &&
						!strstr (wfd2.cFileName, "_i.pcx") &&
						!strstr (wfd2.cFileName, "weapon.pcx"))
						tvModels->add (tvItem, filename);
					bDone2 = !FindNextFile (handle2, &wfd2);
				}
			}

			bDone = !FindNextFile (handle, &fd);
		}

		FindClose (handle);
	}

#endif
}

