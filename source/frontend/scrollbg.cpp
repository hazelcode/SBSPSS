/*=========================================================================

	scrollbg.cpp

	Author:		PKG
	Created: 
	Project:	Spongebob
	Purpose: 

	Copyright (c) 2000 Climax Development Ltd

===========================================================================*/


/*----------------------------------------------------------------------
	Includes
	-------- */

#include "frontend\scrollbg.h"

#ifndef __GFX_SPRBANK_H__
#include "gfx\sprbank.h"
#endif


/*	Std Lib
	------- */

/*	Data
	---- */

#ifndef __SPR_SPRITES_H__
#include <sprites.h>
#endif


/*----------------------------------------------------------------------
	Tyepdefs && Defines
	------------------- */

/*----------------------------------------------------------------------
	Structure defintions
	-------------------- */

/*----------------------------------------------------------------------
	Function Prototypes
	------------------- */

/*----------------------------------------------------------------------
	Vars
	---- */

/*----------------------------------------------------------------------
	Function:
	Purpose:
	Params:
	Returns:
  ---------------------------------------------------------------------- */
void CScrollyBackground::init()
{
	m_sprites=new ("Scrolly Background sprites") SpriteBank;
	m_sprites->load(SPRITES_SPRITES_SPR);
	m_xOff=m_yOff=0;

	setSpeed(DEFAULT_X_SPEED,DEFAULT_Y_SPEED);
	setSpeedScale(DEFAULT_SPEED_SCALE);
	setOt(DEFAULT_OT);
	setFrame(FRM__BG1);
	setTheDrawMode(DRAWMODE_NORMAL);
	setColour(128,128,128);
}


/*----------------------------------------------------------------------
	Function:
	Purpose:
	Params:
	Returns:
  ---------------------------------------------------------------------- */
void CScrollyBackground::shutdown()
{
	m_sprites->dump();	delete m_sprites;	m_sprites=NULL;
}


/*----------------------------------------------------------------------
	Function:
	Purpose:
	Params:
	Returns:
  ---------------------------------------------------------------------- */
void CScrollyBackground::render()
{
	int			smode;
	POLY_FT4	*ft4;
	sFrameHdr	*fh;
	int			x,y,w,h;

	switch(m_drawMode)
	{
		default:
		case DRAWMODE_NORMAL:
			smode=0;
			break;

		case DRAWMODE_ADDITIVE:
			smode=1;
			break;
	}

	fh=m_sprites->getFrameHeader(m_frame);
	w=fh->W;
	h=fh->H;
	y=(m_yOff>>m_speedScale)-h;
	do
	{
		x=(m_xOff>>m_speedScale)-w;
		do
		{
			ft4=m_sprites->printFT4(fh,x,y,0,0,m_ot);
			setShadeTex(ft4,0);
			setSemiTrans(ft4,true);
			ft4->tpage|=(smode<<5);
			setRGB0(ft4,m_r,m_g,m_b);
			x+=w;
		}
		while(x<512);
		y+=h;
	}
	while(y<256);
}


/*----------------------------------------------------------------------
	Function:
	Purpose:
	Params:
	Returns:
  ---------------------------------------------------------------------- */
void CScrollyBackground::think(int _frames)
{
	sFrameHdr	*fh;

	fh=m_sprites->getFrameHeader(m_frame);

	m_xOff=(m_xOff+(_frames*m_xSpeed))%(fh->W<<m_speedScale);
	m_yOff=(m_yOff+(_frames*m_ySpeed))%(fh->H<<m_speedScale);
}


/*===========================================================================
 end */