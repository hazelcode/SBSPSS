/*=========================================================================

	pmbubble.h

	Author:		PKG
	Created:	
	Project:	Spongebob
	Purpose:	

	Copyright (c) 2001 Climax Development Ltd

===========================================================================*/

#ifndef	__PLAYER_PMBUBBLE_H__
#define __PLAYER_PMBUBBLE_H__

/*----------------------------------------------------------------------
	Includes
	-------- */

#ifndef __PLAYER_PMODES_H__
#include "player\pmodes.h"
#endif


/*	Std Lib
	------- */

/*----------------------------------------------------------------------
	Tyepdefs && Defines
	------------------- */

/*----------------------------------------------------------------------
	Structure defintions
	-------------------- */

class CPlayerModeBubbleMixture : public CPlayerModeBase
{
public:
	virtual void				enter();
	virtual void				think();

	virtual void				setAnimNo(int _animNo);
	virtual void				setAnimFrame(int _animFrame);

	void						giveBubbleAmmo(int _amount)			{m_bubbleCount+=_amount;}

private:
	enum
	{
		BUBBLE_DELAY=30*1,
	};

	int							canBlowBubbleFromThisState();

	int							m_savedAnimNo,m_savedAnimFrame;
	int							m_blowFrame;
	int							m_blowing;
	int							m_bubbleDelay;

	int							m_bubbleCount;

};


/*----------------------------------------------------------------------
	Globals
	------- */

/*----------------------------------------------------------------------
	Functions
	--------- */

/*---------------------------------------------------------------------- */

#endif	/* __PLAYER_PMBUBBLE_H__ */

/*===========================================================================
 end */



