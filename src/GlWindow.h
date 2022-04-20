//
//                 MD2 Viewer (c) 1999 by Mete Ciragan
//
// file:           GlWindow.h
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
#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW



#ifndef INCLUDED_MXGLWINDOW
#include <mx/mxGlWindow.h>
#endif

#ifndef INCLUDED_MD2VIEWER
#include "md2viewer.h"
#endif

#ifndef INCLUDED_MD2
#include "md2.h"
#endif



enum // GlWindow Flags
{
	F_WATER = 1,
	F_LIGHT = 2,
	F_SHININESS = 4,
	F_INTERPOLATE = 8,
	F_GLCOMMANDS = 16,
	F_PAUSE = 32,
	F_BACKGROUND = 64
};


enum // texture names
{
	TEXTURE_MODEL = 1,
	TEXTURE_WEAPON = 2,
	TEXTURE_BACKGROUND = 3,
	TEXTURE_WATER = 4
};



enum // render modes
{
	RM_WIREFRAME,
	RM_FLATSHADED,
	RM_SMOOTHSHADED,
	RM_TEXTURED
};



class GlWindow : public mxGlWindow
{
	float d_rotX, d_rotY;
	float d_transX, d_transY, d_transZ;
	md2_model_t *d_models[2];
	unsigned int d_textureNames[5]; // 0 = none, 1 = model, 2 = weapon, 3 = water, 4 = font
	int d_textureLimit;

	int d_renderMode;

	float d_pol; // interpolate value 0.0f - 1.0f
	int d_currFrame, d_currFrame2, d_startFrame, d_endFrame;
	float d_pitch;

	float d_bgColor[3];
	float d_fgColor[3];
	float d_wfColor[3];
	float d_lightColor[3];

	float d_bias;

	int d_flags;

public:
	friend MD2Viewer;

	// CREATORS
	GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style);
	~GlWindow ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	virtual void draw ();

	md2_model_t *loadModel (const char *filename, int pos);
	int loadTexture (const char *filename, int name);
	void setRenderMode (int mode);
	void setFrameInfo (int startFrame, int endFrame);
	void setPitch (float pitch);
	void setBGColor (float r, float g, float b);
	void setFGColor (float r, float g, float b);
	void setWFColor (float r, float g, float b);
	void setLightColor (float r, float g, float b);
	void setFlag (int flag, bool enable);
	void setBrightness (int value);
	void setTextureLimit (int limit) { d_textureLimit = limit; }

#ifdef FULLSCREEN
	void setRot (float x, float y) { d_rotX = x; d_rotY = y; }
	void setTrans (float x, float y, float z) { d_transX = x; d_transY = y; d_transZ = z; }
	void setInterpolate (float pol) { d_pol = pol; }
	void setCurrFrame (int currFrame) { d_currFrame = currFrame; }
	void setCurrFrame2 (int currFrame2) { d_currFrame2 = currFrame2; }
	void setStartFrame (int startFrame) { d_startFrame = startFrame; }	
	void setEndFrame (int endFrame) { d_endFrame = endFrame; }
	void setFlags (int flags) { d_flags = flags; }
#endif

	// ACCESSORS
	md2_model_t *getModel (int pos) const { return d_models[pos]; }
	int getRenderMode () const { return d_renderMode; }
	int getCurrFrame () const { return d_currFrame; }
	int getCurrFrame2 () const { return d_currFrame2; }
	int getStartFrame () const { return d_startFrame; }
	int getEndFrame () const { return d_endFrame; }
	void getBGColor (float *r, float *g, float *b) { *r = d_bgColor[0]; *g = d_bgColor[1]; *b = d_bgColor[2]; }
	void getFGColor (float *r, float *g, float *b) { *r = d_fgColor[0]; *g = d_fgColor[1]; *b = d_fgColor[2]; }
	void getWFColor (float *r, float *g, float *b) { *r = d_wfColor[0]; *g = d_wfColor[1]; *b = d_wfColor[2]; }
	void getLightColor (float *r, float *g, float *b) { *r = d_lightColor[0]; *g = d_lightColor[1]; *b = d_lightColor[2]; }
	bool getFlag (int flag) const { return ((d_flags & flag) == flag); }
	int getFlags () const { return d_flags; }
	int getTextureLimit () const { return d_textureLimit; }
};



#ifdef WIN32
extern char modelFile[];
extern char modelTexFile[];
extern char weaponFile[];
extern char weaponTexFile[];
extern char backgroundTexFile[];
extern char waterTexFile[];
#endif



#endif // INCLUDED_GLWINDOW
