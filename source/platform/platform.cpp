/*=========================================================================

	nplatfrm.cpp

	Author:		CRB
	Created:
	Project:	Spongebob
	Purpose:

	Copyright (c) 2000 Climax Development Ltd

===========================================================================*/

#include "enemy\nplatfrm.h"

#ifndef __LEVEL_LEVEL_H__
#include "level\level.h"
#endif

#ifndef __FILE_EQUATES_H__
#include <biglump.h>
#endif

#ifndef __GAME_GAME_H__
#include	"game\game.h"
#endif

#ifndef	__PLAYER_PLAYER_H__
#include	"player\player.h"
#endif

#ifndef __ENEMY_NPCPATH_H__
#include	"enemy\npcpath.h"
#endif

#ifndef	__UTILS_HEADER__
#include	"utils\utils.h"
#endif

#include "Gfx\actor.h"

#ifndef __VID_HEADER_
#include "system\vid.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLayerCollision	*CNpcPlatform::m_layerCollision;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::init()
{
	CPlatformThing::init();

	
	m_modelGfx=new ("ModelGfx") CModelGfx;
	m_modelGfx->SetModel(0);

	m_animPlaying = true;
	m_animNo = m_data[m_type].initAnim;
	m_frame = 0;

	m_heading = 0;
	m_velocity = 0;
	m_rotation = 0;
	m_reversed = false;
	m_extension = 0;
	m_contact = false;
	m_timer = m_data[m_type].initTimer * GameState::getOneSecondInFrames();
	m_timerType = m_data[m_type].initTimerType;
	m_isActive = true;
	m_movementFunc = m_data[m_type].movementFunc;
	m_detectCollision = m_data[m_type].detectCollision;
	m_state = 0;
	m_tiltAngle = 0;
	m_tiltVelocity = 0;
	m_tiltable = false;

	setCollisionSize(80,40);
	//setCollisionSize( 200, 20 );

	m_layerCollision = NULL;

	m_lifetime = 0;
	m_lifetimeType = m_data[m_type].lifetimeType;

	m_npcPath.initPath();

	if ( m_type == NPC_LINEAR_PLATFORM || m_type == NPC_CART_PLATFORM )
	{
		m_npcPath.setPathType( CNpcPath::PONG_PATH );
	}
	else if ( m_type == NPC_GEYSER_PLATFORM )
	{
		m_npcPath.setPathType( CNpcPath::SINGLE_USE_PATH );
	}
	else if ( m_type == NPC_FALLING_PLATFORM )
	{
		m_npcPath.setPathType( CNpcPath::SINGLE_USE_PATH );
	}
	else
	{
		m_extension = 100;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::init( DVECTOR initPos )
{
	init();

	Pos = m_initPos = m_base = initPos;

	m_initLifetime = m_lifetime = GameState::getOneSecondInFrames() * m_data[m_type].lifetime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::init( DVECTOR initPos, s32 initLifetime )
{
	init( initPos );

	m_initLifetime = m_lifetime = GameState::getOneSecondInFrames() * initLifetime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::reinit()
{
	m_animPlaying = true;
	m_animNo = m_data[m_type].initAnim;
	m_frame = 0;

	m_heading = 0;
	m_velocity = 0;
	m_rotation = 0;
	m_reversed = false;
	m_contact = false;
	m_timer = m_data[m_type].initTimer * GameState::getOneSecondInFrames();
	m_timerType = m_data[m_type].initTimerType;
	m_isActive = true;
	m_movementFunc = m_data[m_type].movementFunc;
	m_detectCollision = m_data[m_type].detectCollision;
	m_state = 0;
	m_tiltAngle = 0;
	m_tiltVelocity = 0;

	m_lifetime = m_initLifetime;

	if ( m_type == NPC_LINEAR_PLATFORM )
	{
		Pos = m_initPos;

		m_extension = 0;
	}
	else
	{
		Pos = m_initPos;

		m_extension = 100;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::shutdown()
{
	delete m_modelGfx;
	m_npcPath.removeAllWaypoints();

	// temporary
	CPlatformThing::shutdown();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::think(int _frames)
{
	
	if ( m_isActive )
	{
		if ( m_tiltable )
		{
			processTilt( _frames );
		}

		switch( m_lifetimeType )
		{
			case NPC_PLATFORM_FINITE_LIFE:
			{
				m_lifetime -= _frames;

				if ( m_lifetime <= 0 )
				{
					shutdown();
					delete this;

					return;
				}

				break;
			}

			case NPC_PLATFORM_FINITE_LIFE_RESPAWN:
			{
				m_lifetime -= _frames;

				if ( m_lifetime <= 0 )
				{
					reinit();
				}

				break;
			}

			case NPC_PLATFORM_INFINITE_LIFE_COLLAPSIBLE:
			{
				if ( m_contact )
				{
					m_lifetime -= _frames;

					if ( m_lifetime <= 0 )
					{
						m_isActive = false;
						m_timer = 3 * GameState::getOneSecondInFrames();
						m_timerType = NPC_PLATFORM_TIMER_RESPAWN;
					}
				}

				break;
			}

			case NPC_PLATFORM_INFINITE_LIFE_FISH_HOOK:
			{
				if ( m_contact )
				{
					m_movementFunc = NPC_PLATFORM_MOVEMENT_FISH_HOOK;
				}

				break;
			}

			default:
				break;
		}

		if ( m_animPlaying )
		{
/*			int frameCount = m_actorGfx->getFrameCount(m_animNo);

			if ( frameCount - m_frame > _frames )
			{
				m_frame += _frames;
			}
			else
			{
				m_frame = frameCount - 1;
				m_animPlaying = false;
			}
*/		}

		if ( m_heading > 1024 && m_heading < 3072 )
		{
			m_reversed = true;
		}
		else
		{
			m_reversed = false;
		}

		processMovement(_frames);

		m_contact = false;
	}

	processTimer( _frames );

	CPlatformThing::think(_frames);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processTilt( int _frames )
{
	bool forceActing = false;

	if ( m_contact )
	{
		// user is touching platform, tilt accordingly

		CPlayer *player = GameScene.getPlayer();
		DVECTOR playerPos = player->getPos();

		if ( playerPos.vx > Pos.vx + 10 )
		{
			forceActing = true;

			// tilt clockwise

			if ( m_tiltVelocity < 2560 )
			{
				m_tiltVelocity += 64;
			}
		}
		else if ( playerPos.vx < Pos.vx - 10 )
		{
			forceActing = true;

			// tilt anticlockwise

			if ( m_tiltVelocity > -2560 )
			{
				m_tiltVelocity -= 64;
			}
		}
	}

	if ( !forceActing )
	{
		// no force acting, hence reduce velocity

		s32 reduction = abs( m_tiltVelocity );

		if ( reduction > 64 )
		{
			reduction = 64;
		}

		if ( m_tiltVelocity >= 0 )
		{
			reduction *= -1;
		}

		m_tiltVelocity += reduction;
	}

	m_tiltAngle += m_tiltVelocity;

	if ( m_tiltAngle > ( 512 << 8 ) )
	{
		m_tiltAngle = ( 512 << 8 );
		m_tiltVelocity = 0;
	}
	else if ( m_tiltAngle < -( 512 << 8 ) )
	{
		m_tiltAngle = -( 512 << 8 );
		m_tiltVelocity = 0;
	}

	setCollisionAngle( m_tiltAngle >> 8 );

	/*if ( getCollisionAngle() > 512 && getCollisionAngle() < 3584 )
	{
		m_detectCollision = false;
	}
	else
	{
		m_detectCollision = true;
	}*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processTimer( int _frames )
{
	switch( m_timerType )
	{
		case NPC_PLATFORM_TIMER_NONE:
			break;

		case NPC_PLATFORM_TIMER_RESPAWN:
		{
			if ( m_timer > 0 )
			{
				m_timer -= _frames;
			}
			else
			{
				reinit();
			}

			break;
		}

		case NPC_PLATFORM_TIMER_RETRACT:
		{
			if ( m_timer > 0 )
			{
				m_timer -= _frames;
			}
			else
			{
				m_timer = 4 * GameState::getOneSecondInFrames();
				m_timerType = NPC_PLATFORM_TIMER_EXTEND;
				m_detectCollision = false;
			}

			break;
		}

		case NPC_PLATFORM_TIMER_EXTEND:
		{
			if ( m_timer > 0 )
			{
				m_timer -= _frames;
			}
			else
			{
				m_timer = 4 * GameState::getOneSecondInFrames();
				m_timerType = NPC_PLATFORM_TIMER_RETRACT;
				m_detectCollision = true;
			}

			break;
		}

		case NPC_PLATFORM_TIMER_GEYSER:
		{
			if ( m_timer > 0 )
			{
				m_timer -= _frames;
			}
			else
			{
				m_movementFunc = NPC_PLATFORM_MOVEMENT_GEYSER;
			}

			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::collidedWith( CThing *_thisThing )
{
	switch(_thisThing->getThingType())
	{
		case TYPE_PLAYER:
		{
			if ( m_detectCollision && m_isActive )
			{
				CPlayer *player = (CPlayer *) _thisThing;

				if ( player->getHasPlatformCollided() )
				{
					player->setPlatform( this );

					m_contact = true;
				}
			}

			break;
		}

		default:
			ASSERT(0);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processGenericCircularPath( int _frames )
{
	m_rotation += m_data[m_type].speed;
	m_rotation &= 4095;

	Pos.vx = m_base.vx + ( ( m_extension * rcos( m_rotation ) ) >> 12 );
	Pos.vy = m_base.vy + ( ( m_extension * rsin( m_rotation ) ) >> 12 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processGenericFixedPathMove( int _frames, s32 *moveX, s32 *moveY, s32 *moveVel, s32 *moveDist )
{
	bool pathComplete;
	bool waypointChange;

	s16 headingToTarget = m_npcPath.think( Pos, &pathComplete, &waypointChange );

	if ( !pathComplete )
	{
		s16 decDir, incDir;
		s16 maxTurnRate = m_data[m_type].turnSpeed;

		decDir = m_heading - headingToTarget;

		if ( decDir < 0 )
		{
			decDir += ONE;
		}

		incDir = headingToTarget - m_heading;

		if ( incDir < 0 )
		{
			incDir += ONE;
		}

		if ( decDir < incDir )
		{
			*moveDist = -decDir;
		}
		else
		{
			*moveDist = incDir;
		}

		if ( *moveDist < -maxTurnRate )
		{
			*moveDist = -maxTurnRate;
		}
		else if ( *moveDist > maxTurnRate )
		{
			*moveDist = maxTurnRate;
		}

		m_heading += *moveDist;
		m_heading &= 4095;

		s32 preShiftX = _frames * m_data[m_type].speed * rcos( m_heading );
		s32 preShiftY = _frames * m_data[m_type].speed * rsin( m_heading );

		*moveX = preShiftX >> 12;
		if ( !(*moveX) && preShiftX )
		{
			*moveX = preShiftX / abs( preShiftX );
		}

		*moveY = preShiftY >> 12;
		if ( !(*moveY) && preShiftY )
		{
			*moveY = preShiftY / abs( preShiftY );
		}

		*moveVel = ( _frames * m_data[m_type].speed ) << 8;

		//processGroundCollisionReverse( moveX, moveY );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processGeyserMove( int _frames, s32 *moveX, s32 *moveY )
{
	s32 distX, distY, heading;
	bool pathComplete;

	m_npcPath.thinkVertical( Pos, &pathComplete, &distX, &distY, &heading );

	if ( pathComplete )
	{
		m_npcPath.resetPath();
		reinit();
	}
	else
	{
		s32 minY, maxY;

		m_npcPath.getPathYExtents( &minY, &maxY );

		*moveY = m_data[m_type].speed * _frames;

		if ( Pos.vy < ( minY + 64 ) )
		{
			s32 multiplier = Pos.vy - minY;

			*moveY = ( multiplier * (*moveY) ) >> 6;

			if ( *moveY < 1 )
			{
				*moveY = 1;
			}
		}

		if ( heading == 3072 )
		{
			*moveY = -(*moveY);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processFallingMove( int _frames, s32 *moveX, s32 *moveY )
{
	s32 distX, distY, heading;
	bool pathComplete;

	m_npcPath.thinkVertical( Pos, &pathComplete, &distX, &distY, &heading );

	if ( pathComplete )
	{
		m_isActive = false;
		m_timer = 4 * GameState::getOneSecondInFrames();
		m_timerType = NPC_PLATFORM_TIMER_RESPAWN;
	}
	else
	{
		*moveY = m_data[m_type].speed * _frames;

		if ( heading == 3072 )
		{
			*moveY = -(*moveY);
		}

		s32 groundHeight = m_layerCollision->getHeightFromGround( Pos.vx + (*moveX), Pos.vy + (*moveY), 16 );

		if ( groundHeight < *moveY )
		{
			*moveY = groundHeight;
			*moveX = 2 * _frames;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processCartMove( int _frames, s32 *moveX, s32 *moveY )
{
	s32 fallSpeed = 3;
	s8 yMovement = fallSpeed * _frames;
	s32 distX, distY, heading;
	s32 groundHeight;

	bool pathComplete;

	m_npcPath.thinkFlat( Pos, &pathComplete, &distX, &distY, &heading );

	*moveX = m_data[m_type].speed * _frames;

	if ( heading == 2048 )
	{
		*moveX = -(*moveX);
	}

	// check for vertical movement

	groundHeight = m_layerCollision->getHeightFromGround( ( Pos.vx + *moveX ), Pos.vy, yMovement + 16 );

	if ( groundHeight <= yMovement )
	{
		// groundHeight <= yMovement indicates either just above ground or on or below ground

		*moveY = groundHeight;
	}
	else
	{
		// fall

		*moveY = yMovement;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processBobMove( int _frames, s32 *moveX, s32 *moveY )
{
	if ( m_contact )
	{
		CPlayer *player = GameScene.getPlayer();
		DVECTOR playerPos = player->getPos();

		int height = player->getHeightFromGroundNoPlatform( playerPos.vx, playerPos.vy );

		// if stood on, increase velocity

		if ( m_velocity < 0 )
		{
			m_velocity = 0;
		}
		else if ( m_velocity < 4 )
		{
			if ( height <= 0 )
			{
				m_velocity = 0;
			}
			else
			{
				m_velocity += 1;
			}
		}

		m_state = NPC_BOB_MOVE;
	}
	else
	{
		if ( m_state == NPC_BOB_MOVE )
		{
			// otherwise drop velocity and ultimately reverse course

			if ( m_velocity > -2 )
			{
				m_velocity--;
			}
		}
	}

	if ( m_velocity )
	{
		*moveY = m_velocity * _frames;

		if ( Pos.vy + (*moveY) < m_initPos.vy )
		{
			Pos.vy = m_initPos.vy;
			m_velocity = 0;
			m_state = NPC_BOB_STOP;
			*moveY = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::processMovement(int _frames)
{
	VECTOR rotPos;
	DVECTOR newPos;
	DVECTOR oldPos;
	SVECTOR relPos;

	if ( _frames > 2 )
	{
		_frames = 2;
	}

	s32 moveX = 0, moveY = 0;
	s32 moveVel = 0;
	s32 moveDist = 0;

	switch( m_movementFunc )
	{
		case NPC_PLATFORM_MOVEMENT_FIXED_PATH:
		{
			processGenericFixedPathMove( _frames, &moveX, &moveY, &moveVel, &moveDist );

			break;
		}

		case NPC_PLATFORM_MOVEMENT_FIXED_CIRCULAR:
		{
			processGenericCircularPath( _frames );

			break;
		}

		case NPC_PLATFORM_MOVEMENT_FISH_HOOK:
		{
			moveY = -m_data[m_type].speed * _frames;

			if ( Pos.vx + moveY < 0 )
			{
				shutdown();
			}

			break;
		}

		case NPC_PLATFORM_MOVEMENT_BUBBLE:
		{
			moveY = -m_data[m_type].speed * _frames;

			break;
		}

		case NPC_PLATFORM_MOVEMENT_GEYSER:
		{
			processGeyserMove( _frames, &moveX, &moveY );

			break;
		}

		case NPC_PLATFORM_MOVEMENT_FALL:
		{
			processFallingMove( _frames, &moveX, &moveY );

			break;
		}

		case NPC_PLATFORM_MOVEMENT_BOB:
		{
			processBobMove( _frames, &moveX, &moveY );

			break;
		}

		case NPC_PLATFORM_MOVEMENT_CART:
		{
			processCartMove( _frames, &moveX, &moveY );

			break;
		}

		case NPC_PLATFORM_MOVEMENT_PLAYER_BUBBLE:
		case NPC_PLATFORM_MOVEMENT_STATIC:
		{
			break;
		}

		default:

			break;
	}

	int angleChange = 3;

	Pos.vx += moveX;
	Pos.vy += moveY;

	/*CThing *thisThing = Next;

	while ( thisThing )
	{
		newPos.vx = moveX;
		newPos.vy = moveY;

		thisThing->shove( newPos );

		thisThing = thisThing->getNext();
	}*/

	//setCollisionAngle( ( getCollisionAngle() + angleChange ) % 4096 );
	//setCollisionAngle( 512 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::render()
{
	if ( m_isActive )
	{
		CPlatformThing::render();

		// Render
		DVECTOR renderPos;
		DVECTOR	offset = CLevel::getCameraPos();

		renderPos.vx = Pos.vx - offset.vx;
		renderPos.vy = Pos.vy - offset.vy;

		if ( renderPos.vx >= 0 && renderPos.vx <= VidGetScrW() )
		{
			if ( renderPos.vy >= 0 && renderPos.vy <= VidGetScrH() )
			{
				m_modelGfx->Render(renderPos);
//				POLY_F4	*F4=GetPrimF4();
//				setXYWH(F4,renderPos.vx-32,renderPos.vy-32,64,16);
//				setRGB0(F4,127,127,64);
//				AddPrimToList(F4,2);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

s32	CNpcPlatform::getNewYPos(CThing *_thisThing)
{
	CRECT	thisRect;
	DVECTOR thatPos = _thisThing->getPos();

	thisRect = getCollisionArea();

	// 'render' collision box at correct angle

	SVECTOR testPointsNonRel[4];
	VECTOR testPoints[4];

	testPointsNonRel[0].vx = thisRect.x1 - Pos.vx;
	testPointsNonRel[0].vy = thisRect.y1 - Pos.vy;

	testPointsNonRel[1].vx = thisRect.x2 - Pos.vx;
	testPointsNonRel[1].vy = thisRect.y1 - Pos.vy;

	testPointsNonRel[2].vx = thisRect.x2 - Pos.vx;
	testPointsNonRel[2].vy = thisRect.y2 - Pos.vy;

	testPointsNonRel[3].vx = thisRect.x1 - Pos.vx;
	testPointsNonRel[3].vy = thisRect.y2 - Pos.vy;

	MATRIX mtx;
	SetIdentNoTrans(&mtx );
	RotMatrixZ( getCollisionAngle(), &mtx );

	int i;

	for ( i = 0 ; i < 4 ; i++ )
	{
		ApplyMatrix( &mtx, &testPointsNonRel[i], &testPoints[i] );

		testPoints[i].vx += Pos.vx;
		testPoints[i].vy += Pos.vy;
	}

	// now find the highest y pos

	// first set highestY to lowest of the four points
	
	s16 highestY = testPoints[0].vy;

	for ( i = 1 ; i < 4 ; i++ )
	{
		if ( testPoints[i].vy > highestY ) // remember y is inverted
		{
			highestY = testPoints[i].vy;
		}
	}

	for ( i = 0 ; i < 4 ; i++ )
	{
		int j = i + 1;
		j %= 4;

		VECTOR highestX, lowestX;

		if ( testPoints[i].vx < testPoints[j].vx )
		{
			lowestX = testPoints[i];
			highestX = testPoints[j];
		}
		else
		{
			lowestX = testPoints[j];
			highestX = testPoints[i];
		}

		if ( highestX.vx == lowestX.vx )
		{
			// have to compare heights of both points to get highest

			if ( lowestX.vy < highestY )
			{
				highestY = lowestX.vy;
			}

			if ( highestX.vy < highestY )
			{
				highestY = highestX.vy;
			}
		}
		else
		{
			if ( thatPos.vx >= lowestX.vx && thatPos.vx <= highestX.vx )
			{
				// current position is above or below this line

				s16 testY;

				testY = lowestX.vy + ( ( thatPos.vx - lowestX.vx ) * ( highestX.vy - lowestX.vy ) ) /
							( highestX.vx - lowestX.vx );

				if ( testY < highestY )
				{
					highestY = testY;
				}
			}
		}
	}

	return( highestY );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int	CNpcPlatform::checkCollisionAgainst(CThing *_thisThing, int _frames)
{
	DVECTOR	pos,thisThingPos;
	int		radius;
	int		collided;

	MATRIX mtx;

	pos=getCollisionCentre();
	thisThingPos=_thisThing->getCollisionCentre();

	radius=getCollisionRadius()+_thisThing->getCollisionRadius();
	collided=false;
	if(abs(pos.vx-thisThingPos.vx)<radius&&
	   abs(pos.vy-thisThingPos.vy)<radius)
	{
		CRECT	thisRect,thatRect;

		thisRect=getCollisionArea();

		// ensure user 'sticks' to platform whilst it is moving along

		thatRect=_thisThing->getCollisionArea();

		// rotate thatPos opposite way to this CThing's collision angle, so that we can regard them both as being at 0 rotation

		// get target thing's position

		DVECTOR thatPos = _thisThing->getPos();

		// get target thing's position relative to this thing's position

		SVECTOR relativePos;
		relativePos.vx = thatPos.vx - Pos.vx;
		relativePos.vy = thatPos.vy - Pos.vy;

		VECTOR newPos;

		// get target thing's collision area relative to 0

		thatRect.x1 -= thatPos.vx;
		thatRect.y1 -= thatPos.vy;
		thatRect.x2 -= thatPos.vx;
		thatRect.y2 -= thatPos.vy;

		SetIdentNoTrans(&mtx );
		RotMatrixZ( -getCollisionAngle(), &mtx );

		// rotation target relative position back to 0 by this thing's collision angle

		ApplyMatrix( &mtx, &relativePos, &newPos );

		// add on this thing's position to get new target thing's position after rotation around this thing

		newPos.vx += Pos.vx;
		newPos.vy += Pos.vy;

		// reposition target thing's collision area
		// horrible, horrible +2 shite is to deal with useless PSX innacurracies in calculations, which can cause it to
		// believe that two collision areas are not *quite* colliding, even though they are

		thatRect.x1 += newPos.vx - 2;
		thatRect.y1 += newPos.vy - 2;
		thatRect.x2 += newPos.vx + 2;
		thatRect.y2 += newPos.vy + 2;

		// check to see if bounding boxes collide

		if(((thisRect.x1>=thatRect.x1&&thisRect.x1<=thatRect.x2)||(thisRect.x2>=thatRect.x1&&thisRect.x2<=thatRect.x2)||(thisRect.x1<=thatRect.x1&&thisRect.x2>=thatRect.x2))&&
		   ((thisRect.y1>=thatRect.y1&&thisRect.y1<=thatRect.y2)||(thisRect.y2>=thatRect.y1&&thisRect.y2<=thatRect.y2)||(thisRect.y1<=thatRect.y1&&thisRect.y2>=thatRect.y2)))
		{
			collided=true;

			// check to see if centre point (i.e. where the object is standing) collides too

			if ( ( newPos.vx >= ( thisRect.x1 - 2 ) && newPos.vx <= ( thisRect.x2 + 2 ) ) &&
					( newPos.vy >= ( thisRect.y1 - 2 ) && newPos.vy <= ( thisRect.y2 + 2 ) ) )
			{
				thatPos.vy = getNewYPos( _thisThing );

				// vertical height change is the sum of the maximums of BOTH objects
				// potentially, one object could be falling down through another object that is moving up
				// hence we provide a certain leeway to compensate

				s32 thisDeltaX = abs( getPosDelta().vx );
				s32 thisDeltaY = abs( getPosDelta().vy );

				s32 thisDelta;

				if ( thisDeltaX > thisDeltaY )
				{
					thisDelta = thisDeltaX;
				}
				else
				{
					thisDelta = thisDeltaY;
				}

				s32 thatDeltaX = abs( _thisThing->getPosDelta().vx );
				s32 thatDeltaY = abs( _thisThing->getPosDelta().vy );

				s32 thatDelta;

				if ( thatDeltaX > thatDeltaY )
				{
					thatDelta = thatDeltaX;
				}
				else
				{
					thatDelta = thatDeltaY;
				}

				s32 verticalDelta = thisDelta + thatDelta;

				if ( thatPos.vy - _thisThing->getPos().vy >= -( verticalDelta ) )
				{
					if ( _thisThing->getHasPlatformCollided() )
					{
						// if this has already collided with a platform, check the current platform is
						// (a) within 10 units,
						// (b) higher

						DVECTOR oldCollidedPos = _thisThing->getNewCollidedPos();

						s32 oldY = abs( oldCollidedPos.vy - ( _thisThing->getPos().vy - verticalDelta ) );
						s32 currentY = abs( thatPos.vy - ( _thisThing->getPos().vy - verticalDelta ) );

						//if ( thatPos.vy < oldCollidedPos.vy )
						if ( currentY < oldY )
						{
							_thisThing->setNewCollidedPos( thatPos );
						}
					}
					else
					{
						_thisThing->setHasPlatformCollided( true );

						_thisThing->setCentreCollision( true );

						_thisThing->setNewCollidedPos( thatPos );
					}
				}
				else
				{
					_thisThing->setCentreCollision( false );
				}
			}
			else
			{
				_thisThing->setCentreCollision( false );
			}
		}
	}

	return collided;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::setTiltable( bool isTiltable )
{
	m_tiltable = isTiltable;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::addWaypoint( s32 xPos, s32 yPos )
{
	DVECTOR newPos;

	newPos.vx = xPos << 4;
	newPos.vy = yPos << 4;

	m_npcPath.addWaypoint( newPos );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcPlatform::setTypeFromMapEdit( u16 newType )
{
	m_type = mapEditConvertTable[newType];
}