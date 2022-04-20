//
//                 MD2 Viewer (c) 1999 by Mete Ciragan
//
// file:           GlWindow.cpp
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
#include <mx/mx.h>
#include <mx/mxMessageBox.h>
#include <mx/mxTga.h>
#include <mx/mxPcx.h>
#include <mx/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#include <ostream.h>

#include "GlWindow.h"
#include "md2.h"



#ifdef WIN32
char modelFile[256] = "";
char modelTexFile[256] = "";
char weaponFile[256] = "";
char weaponTexFile[256] = "";
char backgroundTexFile[256] = "";
char waterTexFile[256] = "";
#endif
float fps;



GlWindow::GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxGlWindow (parent, x, y, w, h, label, style)
{
	d_rotX = d_rotY = 0;
	d_transX = d_transY = 0;
	d_transZ = 50;
	d_models[0] = 0;
	d_models[1] = 0;
	d_textureNames[0] = 0;
	d_textureNames[1] = 0;
	d_textureNames[2] = 0;
	d_textureNames[3] = 0;

	setFrameInfo (0, 0);
	setRenderMode (0);
	setFlag (F_WATER, false);
	setFlag (F_LIGHT, true);
	setFlag (F_SHININESS, true);
	setFlag (F_INTERPOLATE, true);
	setFlag (F_GLCOMMANDS, true);
	setFlag (F_PAUSE, false);
	setFlag (F_BACKGROUND, false);
	
	setPitch (125.0f);
	setBGColor (0.0f, 0.0f, 0.0f);
	setFGColor (1.0f, 1.0f, 1.0f);
	setWFColor (1.0f, 1.0f, 1.0f);
	setLightColor (1.0f, 1.0f, 1.0f);
	setBrightness (5);
	setTextureLimit (256);

	loadTexture ("water.pcx", TEXTURE_WATER);

	glCullFace (GL_FRONT);

	mx::setIdleWindow (this);
	
#ifdef FULLSCREEN
	setVisible (true);
#endif
}



GlWindow::~GlWindow ()
{
	mx::setIdleWindow (0);
	loadTexture (0, TEXTURE_MODEL);
	loadTexture (0, TEXTURE_WEAPON);
	loadTexture (0, TEXTURE_BACKGROUND);
	loadTexture (0, TEXTURE_WATER);
	loadModel (0, 0);
	loadModel (0, 1);
}



int
GlWindow::handleEvent (mxEvent *event)
{
	static float oldrx = 0, oldry = 0, oldtz = 50, oldtx = 0, oldty = 0;
	static int oldx, oldy;

	switch (event->event)
	{
	case mxEvent::MouseDown:
		oldrx = d_rotX;
		oldry = d_rotY;
		oldtx = d_transX;
		oldty = d_transY;
		oldtz = d_transZ;
		oldx = event->x;
		oldy = event->y;
		//setFlag (F_PAUSE, false);

		break;

	case mxEvent::MouseDrag:
		if (event->buttons & mxEvent::MouseLeftButton)
		{
			if (event->modifiers & mxEvent::KeyShift)
			{
				d_transX = oldtx - (float) (event->x - oldx);
				d_transY = oldty + (float) (event->y - oldy);
			}
			else
			{
				d_rotX = oldrx + (float) (event->y - oldy);
				d_rotY = oldry + (float) (event->x - oldx);
			}
		}
		else if (event->buttons & mxEvent::MouseRightButton)
		{
			d_transZ = oldtz + (float) (event->y - oldy);
		}
		redraw ();
		break;

	case mxEvent::Idle:
	{
		static int timer = 0, lastTimer = 0;

		if (getFlag (F_PAUSE))
			return 0;

		lastTimer = timer;
		timer = mx::getTickCount ();

		float diff = (float) (timer - lastTimer);
		fps = 1/ (diff / 1000.0f);
		d_pol += diff / d_pitch;

		if (d_pol > 1.0f)
		{
			d_pol = 0.0f;
			d_currFrame++;
			d_currFrame2++;

			if (d_currFrame > d_endFrame)
				d_currFrame = d_startFrame;

			if (d_currFrame2 > d_endFrame)
				d_currFrame2 = d_startFrame;
		}

		redraw ();
	}
	break;

	case mxEvent::KeyDown:
	{
#ifdef FULLSCREEN
	switch (event->key)
	{
		case 27:
			delete this;
			mx::quit ();
			break;

		case 36: // Home
		{
			if (getModel (0))
			{
				float minmax[6];
				md2_getBoundingBox (getModel (0), minmax);

				// center vertically
				d_transY = (minmax[3] + minmax[2]) / 2;

				// adjust distance
				float dx = minmax[1] - minmax[0];
				float dy = minmax[3] - minmax[2];
				float dz = minmax[5] - minmax[4];

				float d = dx;
				if (dy > d)
					d = dy;

				if (dz > d)
					d = dz;

				d_transZ = d * 1.2f;
				d_transX = 0;
				d_rotX = d_rotY = 0.0f;
				redraw ();
			}
		}
		break;

		case '+':
			d_pitch -= 10.0f;
			if (d_pitch < 1.0f)
				d_pitch = 1.0f;
			break;

		case '-':
			d_pitch += 10.0f;
			break;

		case 'l':
			setFlag (F_LIGHT, !getFlag (F_LIGHT));
			redraw ();
			break;

		case 'w':
			setFlag (F_WATER, !getFlag (F_WATER));
			redraw ();
			break;

		case 's':
			setFlag (F_SHININESS, !getFlag (F_SHININESS));
			redraw ();
			break;

		case 'p':
			setFlag (F_PAUSE, !getFlag (F_PAUSE));
			break;

		case 'b':
			setFlag (F_BACKGROUND, !getFlag (F_BACKGROUND));
			redraw ();
			break;

		case '1':
		case '2':
		case '3':
		case '4':
			setRenderMode (event->key - '1');
			redraw ();
			break;
		}
#endif
	}
	break;

	}

	return 1;
}



void
GlWindow::draw ()
{
	glClearColor (d_bgColor[0], d_bgColor[1], d_bgColor[2], 0.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport (0, 0, w (), h ());

	if (getFlag (F_BACKGROUND) && d_textureNames[TEXTURE_BACKGROUND])
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f);

		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		glLoadIdentity ();

		glDisable (GL_LIGHTING);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
		glEnable (GL_TEXTURE_2D);

		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_BACKGROUND]);

		glBegin (GL_QUADS);

		glTexCoord2f (0, 0);
		glVertex2f (0, 0);

		glTexCoord2f (0, 1);
		glVertex2f (0, 1);

		glTexCoord2f (1, 1);
		glVertex2f (1, 1);

		glTexCoord2f (1, 0);
		glVertex2f (1, 0);

		glEnd ();

		glPopMatrix ();
	}

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (65.0f, (GLfloat) w () / (GLfloat) h (), 1.0f, 1024.0f);

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	if (getFlag (F_LIGHT))
	{
		GLfloat lp[4] = { 0, 0, d_transZ, 1 };
		GLfloat lc[4] = { d_lightColor[0], d_lightColor[1], d_lightColor[2], 1.0f };

		glLightfv (GL_LIGHT0, GL_POSITION, lp);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, lc);
	}

	glPixelTransferf (GL_RED_SCALE, 1.0f + 5.0f * d_bias);
	glPixelTransferf (GL_GREEN_SCALE, 1.0f + 5.0f * d_bias);
	glPixelTransferf (GL_BLUE_SCALE, 1.0f + 5.0f * d_bias);

	glTranslatef (-d_transX, -d_transY, -d_transZ);

	glRotatef (d_rotX, 1, 0, 0);
	glRotatef (d_rotY, 0, 1, 0);

	if (getFlag (F_SHININESS))
	{
		GLfloat ms[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, ms);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
	}
	else
	{
		GLfloat ms[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, ms);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
	}

	if (getFlag (F_LIGHT))
	{
		glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
	}
	else
	{
		glDisable (GL_LIGHTING);
		glDisable (GL_LIGHT0);
	}

	if (d_renderMode == RM_WIREFRAME)
	{
		GLfloat md[4] = { d_wfColor[0], d_wfColor[1], d_wfColor[2], 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
		glColor3f (d_wfColor[0], d_wfColor[1], d_wfColor[2]);

		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
	}
	else if (d_renderMode == RM_FLATSHADED ||
			d_renderMode == RM_SMOOTHSHADED)
	{
		glColor3f (d_fgColor[0], d_fgColor[1], d_fgColor[2]);
		GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);

		if (d_renderMode == RM_FLATSHADED)
			glShadeModel (GL_FLAT);
		else
			glShadeModel (GL_SMOOTH);
	}
	else if (d_renderMode == RM_TEXTURED)
	{
		glColor3f (1.0f, 1.0f, 1.0f);
		GLfloat md[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);
		glShadeModel (GL_SMOOTH);
	}

	if (d_models[0])
	{
		if (!d_textureNames[TEXTURE_MODEL] && d_renderMode == RM_TEXTURED)
		{
			GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
			glDisable (GL_TEXTURE_2D);
		}

		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_MODEL]);
		if (d_currFrame < d_models[0]->header.numFrames &&
			d_currFrame2 < d_models[0]->header.numFrames)
			md2_drawModel (d_models[0], d_currFrame, d_currFrame2, d_pol);
	}

	if (d_models[1])
	{
		if (d_renderMode == RM_TEXTURED)
		{
			if (!d_textureNames[TEXTURE_WEAPON])
			{
				GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
				glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
				glDisable (GL_TEXTURE_2D);				
			}
			else
			{
				GLfloat md[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
			}
		}

		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_WEAPON]);
		if (d_currFrame < d_models[1]->header.numFrames &&
			d_currFrame2 < d_models[1]->header.numFrames)
			md2_drawModel (d_models[1], d_currFrame, d_currFrame2, d_pol);
	}

	if (getFlag (F_WATER) && d_textureNames[TEXTURE_WATER])
	{
		glDisable (GL_LIGHTING);

		glNormal3f (0, 1, 0);
		glDisable (GL_CULL_FACE);
		glEnable (GL_BLEND);
		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_WATER]);

		glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-100.0f, 0.0f, -100.0f);
		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (100.0f, 0.0f, -100.0f);
		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (100.0f, 0.0f, 100.0f);
		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (-100.0f, 0.0f, 100.0f);
		glEnd ();
		glDisable (GL_BLEND);
	}

	glPopMatrix ();
}



md2_model_t *
GlWindow::loadModel (const char *filename, int pos)
{
	if (d_models[pos] != 0)
	{
		md2_freeModel (d_models[pos]);
		d_models[pos] = 0;
#ifdef WIN32
		if (pos == 0)
			strcpy (modelFile, "");
		else // pos == 1
			strcpy (weaponFile, "");
#endif
	}

	if (!filename || !strlen (filename))
		return 0;

	d_models[pos] = md2_readModel (filename);

#ifdef WIN32
	if (pos == 0)
		strcpy (modelFile, filename);
	else // pos == 1
		strcpy (weaponFile, filename);
#endif

	return d_models[pos];
}



int
GlWindow::loadTexture (const char *filename, int name)
{
	if (!filename || !strlen (filename))
	{
		if (d_textureNames[name])
		{
			//glDeleteTextures (1, &d_textureNames[name]);
			d_textureNames[name] = 0;
		}

#ifdef WIN32
		if (name == TEXTURE_MODEL)
			strcpy (modelTexFile, "");
		else if (name == TEXTURE_WEAPON)
			strcpy (weaponTexFile, "");
		else if (name == TEXTURE_BACKGROUND)
			strcpy (backgroundTexFile, "");
		else if (name == TEXTURE_WATER)
			strcpy (waterTexFile, "");
#endif
		return 0;
	}

	mxImage *image = 0;

	if (strstr (filename, ".tga"))
		image = mxTgaRead (filename);
	else if (strstr (_strlwr ((char *) filename), ".pcx"))
	{
		mxImage *tmp = mxPcxRead (filename);
		if (tmp)
		{
			image = new mxImage ();
			if (image && image->create (tmp->width, tmp->height, 24))
			{
				byte *dataout = (byte *) image->data;
				byte *datain = (byte *) tmp->data;
				byte *palette = (byte *) tmp->palette;
				int ptr = 0;
				for (int y = 0; y < tmp->height; y++)
				{
					for (int x = 0; x < tmp->width; x++)
					{
						dataout[ptr++] = palette[datain[y * tmp->width + x] * 3 + 0];
						dataout[ptr++] = palette[datain[y * tmp->width + x] * 3 + 1];
						dataout[ptr++] = palette[datain[y * tmp->width + x] * 3 + 2];
					}
				}
			}
			else
			{
				if (image)
					delete image;
				image = 0;
			}
		}

		if (tmp)
			delete tmp;
	}

	if (image)
	{
#ifdef WIN32
		if (name == TEXTURE_MODEL)
			strcpy (modelTexFile, filename);
		else if (name == TEXTURE_WEAPON)
			strcpy (weaponTexFile, filename);
		else if (name == TEXTURE_BACKGROUND)
			strcpy (backgroundTexFile, filename);
		else if (name == TEXTURE_WATER)
			strcpy (waterTexFile, filename);
#endif
		d_textureNames[name] = name;

		// scale texture to power of 2
		int w = image->width;
		int h = image->height;
		int w2 = w, h2 = h;

		if (w2 > d_textureLimit)
			w2 = d_textureLimit;

		if (w2 > 128)
			w2 = 256;
		else if (w2 > 64)
			w2 = 128;
		else if (w2 > 32)
			w2 = 64;
		else if (w2 > 16)
			w2 = 32;
		else if (w2 > 8)
			w2 = 16;
		else if (w2 > 4)
			w2 = 8;
		else if (w2 > 2)
			w2 = 4;
		else if (w2 > 1)
			w2 = 2;

		if (h2 > d_textureLimit)
			h2 = d_textureLimit;

		if (h2 > 128)
			h2 = 256;
		else if (h2 > 64)
			h2 = 128;
		else if (h2 > 32)
			h2 = 64;
		else if (h2 > 16)
			h2 = 32;
		else if (h2 > 8)
			h2 = 16;
		else if (h2 > 4)
			h2 = 8;
		else if (h2 > 2)
			h2 = 4;
		else if (h2 > 1)
			h2 = 2;

		mxImage *image2 = new mxImage ();
		image2->create (w2, h2, 24);

		gluScaleImage(GL_RGB, w, h, GL_UNSIGNED_BYTE, image->data, w2, h2, GL_UNSIGNED_BYTE, image2->data);
		delete image;

		//glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[name]);
		//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, image2->width, image2->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image2->data);
		glBindTexture (GL_TEXTURE_2D, 0);

		delete image2;

		return name;
	}

	return 0;
}



void
GlWindow::setRenderMode (int mode)
{
	d_renderMode = mode;
}



void
GlWindow::setFrameInfo (int startFrame, int endFrame)
{
	if (d_models[0])
	{
		d_startFrame = startFrame;
		d_endFrame = endFrame;

		if (d_startFrame >= d_models[0]->header.numFrames)
			d_startFrame = d_models[0]->header.numFrames - 1;
		else if (d_startFrame < 0)
			d_startFrame = 0;

		if (d_endFrame >= d_models[0]->header.numFrames)
			d_endFrame = d_models[0]->header.numFrames - 1;
		else if (d_endFrame < 0)
			d_endFrame = 0;

		d_currFrame = d_startFrame;
		d_currFrame2 = d_startFrame + 1;

		if (d_currFrame >= d_models[0]->header.numFrames)
			d_currFrame = d_models[0]->header.numFrames - 1;

		if (d_currFrame2 >= d_models[0]->header.numFrames)
			d_currFrame2 = 0;
	}
	else
	{
		d_startFrame = d_endFrame = d_currFrame = d_currFrame2 = 0;
	}

	d_pol = 0;
}



void
GlWindow::setPitch (float pitch)
{
	d_pitch = pitch;
	if (d_pitch < 0.0f)
		d_pitch = 0.0f;
}



void
GlWindow::setBGColor (float r, float g, float b)
{
	d_bgColor[0] = r;
	d_bgColor[1] = g;
	d_bgColor[2] = b;
}



void
GlWindow::setFGColor (float r, float g, float b)
{
	d_fgColor[0] = r;
	d_fgColor[1] = g;
	d_fgColor[2] = b;
}



void
GlWindow::setWFColor (float r, float g, float b)
{
	d_wfColor[0] = r;
	d_wfColor[1] = g;
	d_wfColor[2] = b;
}



void
GlWindow::setLightColor (float r, float g, float b)
{
	d_lightColor[0] = r;
	d_lightColor[1] = g;
	d_lightColor[2] = b;
}



void
GlWindow::setFlag (int flag, bool enable)
{
	if (enable)
		d_flags |= flag;	// set flag
	else
		d_flags &= ~flag;	// clear flag

	md2_setStyle ((int) getFlag (F_GLCOMMANDS), (int) getFlag (F_INTERPOLATE));
}



void
GlWindow::setBrightness (int value)
{
	d_bias = (float) value / 100.0f;
	redraw ();
}



#ifdef FULLSCREEN
int
main (int argc, char *argv[])
{
	mx::init (argc, argv);

	FILE *file = fopen ("md2.cfg", "rb");
	if (!file)
	{
		mxMessageBox (0, "Error loading configuration", "MD2 Viewer", MX_MB_OK | MX_MB_ERROR);
		return 0;
	}

	int width, height;
	bool cds;
	float d_rotX, d_rotY;
	float d_transX, d_transY, d_transZ;
	int d_renderMode;
	float d_pol;
	int d_currFrame, d_currFrame2, d_startFrame, d_endFrame;
	float d_pitch;
	float d_bgColor[3];
	float d_fgColor[3];
	float d_wfColor[3];
	float d_lightColor[3];
	float d_bias;
	int d_flags;
	int d_textureLimit;


	fread (&width, sizeof (int), 1, file);
	fread (&height, sizeof (int), 1, file);
	fread (&cds, sizeof (bool), 1, file);

	fread (modelFile, 256, 1, file);
	fread (modelTexFile, 256, 1, file);
	fread (weaponFile, 256, 1, file);
	fread (weaponTexFile, 256, 1, file);
	fread (backgroundTexFile, 256, 1, file);
	fread (waterTexFile, 256, 1, file);

	fread (&d_rotX, sizeof (float), 1, file);
	fread (&d_rotY, sizeof (float), 1, file);
	fread (&d_transX, sizeof (float), 1, file);
	fread (&d_transY, sizeof (float), 1, file);
	fread (&d_transZ, sizeof (float), 1, file);

	fread (&d_renderMode, sizeof (int), 1, file);

	fread (&d_pol, sizeof (float), 1, file);
	fread (&d_currFrame, sizeof (int), 1, file);
	fread (&d_currFrame2, sizeof (int), 1, file);
	fread (&d_startFrame, sizeof (int), 1, file);
	fread (&d_endFrame, sizeof (int), 1, file);
	fread (&d_pitch, sizeof (float), 1, file);

	fread (d_bgColor, 3 * sizeof (float), 1, file);
	fread (d_fgColor, 3 * sizeof (float), 1, file);
	fread (d_wfColor, 3 * sizeof (float), 1, file);
	fread (d_lightColor, 3 * sizeof (float), 1, file);
	fread (&d_bias, sizeof (int), 1, file);
	fread (&d_flags, sizeof (int), 1, file);
	fread (&d_textureLimit, sizeof (int), 1, file);

	fclose (file);

	if (cds)
		mx::setDisplayMode (width, height, 16);

	mxGlWindow::setFormat (mxGlWindow::FormatDouble, 16, 16);

	GlWindow *glw = new GlWindow (0, 0, 0, width, height, "", mxWindow::Popup);

	glw->loadModel (modelFile, 0);
	glw->loadModel (weaponFile, 1);
	glw->loadTexture (modelTexFile, TEXTURE_MODEL);
	glw->loadTexture (weaponTexFile, TEXTURE_WEAPON);
	glw->loadTexture (backgroundTexFile, TEXTURE_BACKGROUND);
	glw->loadTexture (waterTexFile, TEXTURE_WATER);

	glw->setRot (d_rotX, d_rotY);
	glw->setTrans (d_transX, d_transY, d_transZ);
	glw->setRenderMode (d_renderMode);
	glw->setInterpolate (d_pol);
	glw->setCurrFrame (d_currFrame);
	glw->setCurrFrame2 (d_currFrame2);
	glw->setStartFrame (d_startFrame);
	glw->setEndFrame (d_endFrame);
	glw->setPitch (d_pitch);

	glw->setBGColor (d_bgColor[0], d_bgColor[1], d_bgColor[2]);
	glw->setFGColor (d_fgColor[0], d_fgColor[1], d_fgColor[2]);
	glw->setWFColor (d_wfColor[0], d_wfColor[1], d_wfColor[2]);
	glw->setLightColor (d_lightColor[0], d_lightColor[1], d_lightColor[2]);

	glw->setBrightness ((int) d_bias);
	glw->setFlags (d_flags);
	glw->setFlag (F_LIGHT, false);
	glw->setTextureLimit (d_textureLimit);

	int ret = mx::run ();

	if (cds)
		mx::setDisplayMode (0, 0, 0);

	delete glw;

	return ret;
}
#endif // FULLSCREEN
