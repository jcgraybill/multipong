#include <Values.h>

// Constants

#define	kBaseResID			128
#define	kTicks				1L
#define	kRandomUpperLimit	32768
#define	kVisible			true
#define	kBringToFront		(WindowPtr)-1L
#define	kLeaveInBack		0
#define	kHasGoAway			true

#define kAboutWindow		kBaseResID
#define	kAboutText			kBaseResID
#define kThinkWindow		kBaseResID+1
#define kThinkText			kBaseResID+1

#define	kNumGameWindows		4
#define	kMultiPongWindow	0
#define	kTopWindow			1
#define	kBottomWindow		2
#define	kPlayerWindow		3

#define	kStartingWinBuf		30

#define	mApple				kBaseResID
#define		iAbout			1
#define	mFile				kBaseResID+1
#define 	iNew			1
#define		iStartOver		2
#define		iEndGame		3
#define		iClose			4
#define		iDifficulty		6
#define		iQuit			8
#define	mEdit				kBaseResID+2
#define	mWindow				kBaseResID+3
#define	mExtras				kBaseResID+4
#define		iThink			1
#define	mDifficulty			100
#define		iBaby			1
#define		iEasy			2
#define		iMedium			3
#define		iHard			4
#define		iNightmare		5

#define	kNotANormalMenu		-1

#define	kStartingDifficulty	iMedium
#define	kBallSize			20
#define	kMinSpeed			1
#define	kMinNightmareSpeed	3
#define	kShapeSizeDifficultyModifier	70
#define	kObjectWidth		8
#define	kOpponentSpeed		2
#define	kPaddleHeight		6
#define	kGoalHeight			2
#define	kBallColor			blackColor
#define	kWallColor			blackColor
#define	kPaddleColor		blackColor
#define	kGoalColor			greenColor
#define	kBallPattern		black
#define	kFramePattern		black
#define kGoalPattern		ltGray
#define	kPaddlePattern		black
#define	kWallPattern		dkGray
#define	kNumShapes			7
#define	kWinningScore		7

// Function prototypes

void		Bail( void );
void		ConcatStr255( StringPtr first, StringPtr second );
void		CreateGameWindows( void );
WindowPtr	CreateWindow( int whichOne );
short		DetectCollision( RectPtr shape, WindowPtr win );
void		DisplayBall( void );
short		DisplayBallInWindow( WindowPtr window );
void		DisplayResults( void );
void		DisposeGameWindows( void );
void		DoUpdate( EventRecord *event );
void		DrawPlayfield( WindowPtr window );

void 		EndGame( void );
void		EraseBall( void );
short 		EraseBallInWindow( WindowPtr window );
void		EventLoop( void );
void		GameLoop( int steps );

void		HandleAppleChoice( short item );
void		HandleCloseWindow( WindowPtr window );
void		HandleDifficultyChoice( short item );
void		HandleEditChoice( short item );
void		HandleExtrasChoice( short item );
void		HandleFileChoice( short item );
void		HandleMenuChoice( long menuChoice );
void		HandleMouseDown( EventRecord *eventPtr );
void		HandleWindowChoice( short item );

void		InitMenu( void );
void		InitOpponent( void );
void		InitShapes( void );
void		InitToolBox( void );

void		LaunchBall( void );
void		MoveBall( void );
void		MoveOpponent( void );
short		Randomize( short range );
void		RandomRect( Rect *rectPtr );
void		ScoreOpponent( void );
void		ScorePlayer( void );
void		ShowAboutWindow( void );
void		StartGame( void );
void		WriteStrPound ( int which );

// Globals

Rect 		gBall;
Rect		gShapes[ kNumShapes ];
WindowPtr	gWindows[ kNumGameWindows ];
short		gHorizontal, gVertical;
short		gQuitting, gAboutVisible, gThinkVisible = 0;
short		gameOn, gPlayerScore, gOpponentScore = 0;
short		gOpponentYPosition, gPaddleHeight;
int			gLastTick;
short		gDifficulty;

int main( void ) {
	InitToolBox();
	InitMenu();
	GetDateTime((unsigned long *)(&randSeed));
	
	ShowAboutWindow();
	EventLoop();
	return 0;
}

void Bail( void ) {
	SysBeep(10);
	ExitToShell();
}

void ConcatStr255( StringPtr first, StringPtr second ) {
	if ( first[0] + second[0] > 255 ) return;
	BlockMove( &second[1], &first[ first[0] + 1 ], second[0] );
	first[0] += second[0];
}

void CreateGameWindows( void ) {
	Rect MultiPongBounds, TopBounds, BottomBounds, PlayerBounds;
	int i;
	
	MultiPongBounds.top = MBarHeight + kStartingWinBuf;
	MultiPongBounds.left = kStartingWinBuf;
	MultiPongBounds.right = ( screenBits.bounds.right  / 2 ) - kStartingWinBuf;
	MultiPongBounds.bottom = screenBits.bounds.bottom - kStartingWinBuf;
	
	TopBounds.top = MBarHeight + kStartingWinBuf;
	TopBounds.left = MultiPongBounds.right + kStartingWinBuf;
	TopBounds.right = TopBounds.left + ( screenBits.bounds.right  / 4 );
	TopBounds.bottom = ( screenBits.bounds.bottom / 4 ) - kStartingWinBuf;
	
	BottomBounds.top = screenBits.bounds.bottom - ( screenBits.bounds.bottom / 4 ) + kStartingWinBuf;
	BottomBounds.left = MultiPongBounds.right + kStartingWinBuf;
	BottomBounds.right = BottomBounds.left + ( screenBits.bounds.right  / 4 );
	BottomBounds.bottom = screenBits.bounds.bottom - kStartingWinBuf;

	PlayerBounds.top = ( screenBits.bounds.bottom / 4 ) + kStartingWinBuf;
	PlayerBounds.left = MultiPongBounds.right + kStartingWinBuf;
	PlayerBounds.right = PlayerBounds.left + ( screenBits.bounds.right  / 4 );
	PlayerBounds.bottom = 3* ( screenBits.bounds.bottom / 4 ) - kStartingWinBuf;
		
	gWindows[kPlayerWindow] 	= NewWindow( 0L, &PlayerBounds, "\pPlayer", kVisible, noGrowDocProc, kLeaveInBack, kHasGoAway, 0);
	gWindows[kBottomWindow]		= NewWindow( 0L, &BottomBounds, "\pBottom", kVisible, noGrowDocProc, kLeaveInBack, kHasGoAway, 0);
	gWindows[kTopWindow] 		= NewWindow( 0L, &TopBounds, "\pTop", kVisible, noGrowDocProc, kLeaveInBack, kHasGoAway, 0);
	gWindows[kMultiPongWindow]	= NewWindow( 0L, &MultiPongBounds, "\pMulti pong", kVisible, noGrowDocProc, kBringToFront, kHasGoAway, 0);

	for ( i = 0; i < kNumGameWindows ; i++ ) {
		if ( !gWindows[i] )	Bail();
		SetWRefCon( gWindows[i], i );
	}
	
	InitShapes();
	InitOpponent();

	for ( i = 0; i < kNumGameWindows ; i++ ) {
		DrawPlayfield( gWindows[i] );
	}

}

WindowPtr CreateWindow( int whichOne ) {
	WindowPtr	window;
	Rect		windRect;
	
	window = GetNewWindow( whichOne, nil, kBringToFront );
	
	if ( window == nil )
		Bail();

	SetWRefCon( window,(long)whichOne); 
	ShowWindow( window );
	return window;	
}

short DetectCollision( RectPtr shape, WindowPtr win ) {	
	Rect ball = gBall;
	Rect winRect = win->portRect;
	Rect winSect, shapeSect;
		
	if ( ! ((WindowPeek)win)->visible ) return false;
	
	SetPort( win );
	GlobalToLocal(&topLeft(ball));
	GlobalToLocal(&botRight(ball));
		
	if ( SectRect( &ball, &winRect, &winSect ) ) {
		if ( SectRect( &ball, shape, &shapeSect ) ) {
			RgnHandle shapeRegion, overlapRegion, visibleRegion;

			// Opponent/goal are active even if occluded by other windows.
			if ( GetWRefCon( win ) == kMultiPongWindow ) 
				return true;

			shapeRegion = NewRgn();
			overlapRegion = NewRgn();
			visibleRegion = ((WindowPeek)win)->port.visRgn;

			RectRgn(shapeRegion, &shapeSect);
			SectRgn( shapeRegion, visibleRegion, overlapRegion );
			if ( EmptyRgn(overlapRegion) ) 
				return false;
			else
				return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void DisplayBall( void ) {
	short i, ballIsVisible;
	
	ballIsVisible = false;
	for ( i = 0; i < kNumGameWindows; i++ ) {
		if ( DisplayBallInWindow( gWindows[i] ) ) 
			ballIsVisible = true;
	}
	
	if ( !ballIsVisible ) {
		ScoreOpponent();
	}
}

short DisplayBallInWindow( WindowPtr window ) {
		Rect ball = gBall;
		Rect win = window->portRect;
		Rect sect;
		
		if ( ! ((WindowPeek)window)->visible ) return false;
		
		SetPort( window );
		GlobalToLocal(&topLeft(ball));
		GlobalToLocal(&botRight(ball));
			
		if ( SectRect( &ball, &win, &sect ) ) {
			PenPat( kBallPattern );
			ForeColor( kBallColor );
			PaintOval( &ball );
			return true;
		} else {
			return false;
		}
}

void DisplayResults( void ) {
	Str255 opponentScore, playerScore;
	NumToString(gOpponentScore, opponentScore);
	NumToString(gPlayerScore, playerScore);
	
	if ( gPlayerScore == gOpponentScore ) {
		ParamText( playerScore, opponentScore, "\pNice", "\ptied.");	
	} else if ( gPlayerScore > gOpponentScore ) {
		ParamText( playerScore, opponentScore, "\pCongrats", "\pwon!");	
	} else {
		ParamText( playerScore, opponentScore, "\pBummer", "\plost.");		
	}
	NoteAlert( kBaseResID, nil );
}

void DisplayScore( void ) {
	Str255 score = "\pPlayer ";
	Str255 opponent = "\p | Opponent ";
	Str255 pScore, oScore;
	NumToString( gPlayerScore, pScore );
	NumToString( gOpponentScore, oScore );
	
	ConcatStr255( score, pScore );
	ConcatStr255( score, opponent );
	ConcatStr255( score, oScore );
	
	SetWTitle( gWindows[ kMultiPongWindow ], score );
}

void DisposeGameWindows( void ) {
	short i;
	for ( i = 0; i < kNumGameWindows ; i++ ) {
		DisposeWindow( gWindows[i] );
	}
}

void DoUpdate( EventRecord *event ) {
	WindowPtr		window;
	Rect sect;
	Rect ball = gBall;
			
	window = (WindowPtr) event->message;
	BeginUpdate( window );
	switch ( GetWRefCon( window ) ) {
		case kAboutWindow:
			SetPort( window );
			WriteStrPound( kAboutWindow );
			break;
		case kThinkWindow:
			SetPort( window );
			WriteStrPound( kThinkWindow );
			break;
		default:
			SetPort( window );
			EraseBallInWindow( window );
			DisplayBallInWindow( window );
			DrawPlayfield( window );
			break;
	}
	EndUpdate( window );
}

void DrawPlayfield( WindowPtr window ) {

	SetPort( window );
	PenSize( 1, 1 );
	switch ( GetWRefCon( window ) ) {
		case kMultiPongWindow:
			PenPat( kWallPattern );
			ForeColor( kWallColor );		
			PaintRect( &gShapes[0] );
			PaintRect( &gShapes[1] );
			PenPat( kFramePattern );
			FrameRect( &gShapes[0] );
			FrameRect( &gShapes[1] );
			PenPat( kGoalPattern );
			ForeColor( kGoalColor );
			PaintRect( &gShapes[2] );
			break;
		case kTopWindow:
			PenPat( kWallPattern );
			ForeColor( kWallColor );
			PaintRect( &gShapes[4] );
			PenPat( kFramePattern );
			FrameRect( &gShapes[4] );
			break;
		case kPlayerWindow:
			PenPat( kPaddlePattern );
			ForeColor( kPaddleColor );
			PaintRect( &gShapes[5] );
			break;
		case kBottomWindow:
			PenPat( kWallPattern );
			ForeColor( kWallColor );
			PaintRect( &gShapes[6] );
			PenPat( kFramePattern );
			FrameRect( &gShapes[6] );
			break;
	}
}

void EndGame( void ) {
	MenuHandle		menu;
	
	if ( ! gameOn ) return;
	gameOn = false;

	menu = GetMHandle( mFile );
	EnableItem( menu, iNew );
	EnableItem( menu, iDifficulty );
	DisableItem( menu, iStartOver );
	DisableItem( menu, iEndGame );

	menu = GetMHandle( mWindow );
	DisableItem( menu, 1 );
	DisableItem( menu, 2 );
	DisableItem( menu, 3 );
	DisableItem( menu, 4 );

	DisposeGameWindows();
}

void EraseBall( void ) {
	int i;
	for ( i = 0; i < kNumGameWindows; i++ ) {
		EraseBallInWindow( gWindows[i] );
	}
}

short EraseBallInWindow( WindowPtr window ) {
		Rect ball = gBall;
		Rect win = window->portRect;
		Rect sect;
					
		SetPort( window );
		GlobalToLocal(&topLeft(ball));
		GlobalToLocal(&botRight(ball));
			
		if ( SectRect( &ball, &win, &sect ) ) {
			EraseOval( &ball );
			return true;
		} else {
			return false;
		}
}

void EventLoop( void ) {

	while ( !gQuitting ) {
		char theChar;
		EventRecord event;
	
		if ( WaitNextEvent( everyEvent, &event, kTicks, nil ) ) {
			switch ( event.what ) {
				case mouseDown:
					HandleMouseDown( &event );
					break;
				case keyDown:
				case autoKey:
					theChar = event.message & charCodeMask;
					if ( (event.modifiers & cmdKey) != 0 ) HandleMenuChoice( MenuKey( theChar ) );
					break;
				case updateEvt:
					DoUpdate( &event );
					break;
			}
		} else if ( gameOn ) {
			GameLoop(TickCount() - gLastTick);
			gLastTick = TickCount();
		}
	}
}

void GameLoop( int steps ) {
	EraseBall();
	if ( gameOn ) {
		int i;
		for (i = 0; i < steps; i++) {
			MoveBall();
			MoveOpponent();
		}
		DisplayBall();
	}
}

void HandleAppleChoice( short item ) { 
	MenuHandle	appleMenu;
	Str255 		accName;
	short		accNumber;
	
	if ( item == iAbout ) {
		ShowAboutWindow();
		return;
	} 
	
	appleMenu = GetMHandle( mApple );
	GetItem( appleMenu, item, accName );
	accNumber = OpenDeskAcc( accName );

}

void HandleCloseWindow( WindowPtr window ) {
	switch ( GetWRefCon( window ) ) {
		case kAboutWindow:
			gAboutVisible = false;
			DisposeWindow( window );
			break;
		case kThinkWindow:
			gThinkVisible = false;
			DisposeWindow( window );
			break;
		default: 
			HideWindow( window );
			break;
	}
}

void HandleDifficultyChoice( short item ) { 
	MenuHandle menuHandle;

	menuHandle = GetMHandle( mDifficulty );
	gDifficulty = item;

	CheckItem( menuHandle, iBaby, gDifficulty == iBaby );
	CheckItem( menuHandle, iEasy, gDifficulty == iEasy );
	CheckItem( menuHandle, iMedium, gDifficulty == iMedium );
	CheckItem( menuHandle, iHard, gDifficulty == iHard );
	CheckItem( menuHandle, iNightmare, gDifficulty == iNightmare );
	
	return;
}

void HandleEditChoice( short item ) { 
	return;
}

void HandleExtrasChoice( short item ) { 
	WindowPtr	window;

	if ( item == iThink && !gThinkVisible ) {
		window = CreateWindow( kThinkWindow );
		SetPort( window );
		WriteStrPound( kThinkText );
		gThinkVisible = true;
		return;
	}
}

void HandleFileChoice( short item ) {
	switch ( item ) {
		case iNew:
			StartGame();
			break;
		case iStartOver:
			EndGame();
			StartGame();
			break;
		case iEndGame:
			EndGame();
			DisplayResults();
			break;
		case iClose:
			HandleCloseWindow( FrontWindow() );
			break;
		case iQuit:
			gQuitting = true;
			break;	
	}
}

void HandleMenuChoice( long menuChoice ) {
	short menu;
	short item;
	
	if ( menuChoice != 0 ) {
		menu = HiWord( menuChoice );
		item = LoWord( menuChoice );
		
		switch ( menu ) {
			case mApple:
				HandleAppleChoice( item );
				break;
			case mFile:
				HandleFileChoice( item );
				break;
			case mEdit:
				HandleEditChoice( item );
				break;
			case mWindow:
				HandleWindowChoice( item );
				break;
			case mExtras:
				HandleExtrasChoice( item );
				break;
			case mDifficulty:
				HandleDifficultyChoice( item );
		}
	HiliteMenu( 0 );
	}
}

void HandleMouseDown( EventRecord *eventPtr ) {
	WindowPtr	window;
	short int	thePart;
	long			menuChoice;

	thePart = FindWindow( eventPtr->where, &window );
	
	switch (thePart) {
		case inMenuBar:
			menuChoice = MenuSelect( eventPtr->where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow:
			SystemClick( eventPtr, window );
			break;
		case inContent:
			SelectWindow( window );
			break;
		case inDrag:
			if ( gameOn ) EraseBall(); // hide the ball while player is dragging windows
			if ( GetWRefCon( window ) == kMultiPongWindow ) { 
				SelectWindow( window ) ;
			} else {
				DragWindow( window, eventPtr->where, &screenBits.bounds );
			}
			if ( gameOn ) { 
				DrawPlayfield( window );
			}
			break;
		case inGoAway:
			if ( TrackGoAway( window, eventPtr->where ) ) HandleCloseWindow( window );
			break;
	}
}

void HandleWindowChoice( short item ) {
	if ( item > kNumGameWindows ) return;
	ShowWindow( gWindows[ item - 1 ] );
	SelectWindow( gWindows[ item - 1 ] );
}

void InitMenu( void ){
	Handle			menuBar;
	MenuHandle		menu;

	menuBar = GetNewMBar( kBaseResID );
	SetMenuBar( menuBar );
	
	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
	
	menu = GetMenu( mDifficulty );
	InsertMenu( menu, kNotANormalMenu );
	CheckItem( menu, kStartingDifficulty, true );
	gDifficulty = kStartingDifficulty;
		
	DrawMenuBar();
}

void InitOpponent( void ) {
	gOpponentYPosition = Randomize(gWindows[ kMultiPongWindow ]->portRect.bottom / kGoalHeight); 
	gOpponentYPosition += gWindows[ kMultiPongWindow ]->portRect.bottom / (kGoalHeight * 2);
}

void InitShapes( void ) {
	short playerPaddleHeight;
	short difficulty = kShapeSizeDifficultyModifier + gDifficulty * 10;

	short goalHeight = screenBits.bounds.bottom / kGoalHeight;
	goalHeight = goalHeight * 100 / difficulty;
	
	gPaddleHeight = screenBits.bounds.bottom / kPaddleHeight;
	playerPaddleHeight = gPaddleHeight;
	gPaddleHeight = gPaddleHeight * difficulty / 100;

	gShapes[0].top = 1;
	gShapes[0].left = 1;
	gShapes[0].right = kObjectWidth;
	gShapes[0].bottom = ( gWindows[ kMultiPongWindow ]->portRect.bottom - goalHeight ) / 2;

	gShapes[1].top = ( ( gWindows[ kMultiPongWindow ]->portRect.bottom - goalHeight ) / 2 ) + goalHeight + 2;
	gShapes[1].left = 1;
	gShapes[1].right = kObjectWidth;
	gShapes[1].bottom = gWindows[ kMultiPongWindow ]->portRect.bottom - 1;

	gShapes[2].top = ( ( gWindows[ kMultiPongWindow ]->portRect.bottom - goalHeight ) / 2 ) + 1;
	gShapes[2].left = 1;
	gShapes[2].right = kObjectWidth;
	gShapes[2].bottom = ( ( gWindows[ kMultiPongWindow ]->portRect.bottom - goalHeight ) / 2 ) + goalHeight;
	
	gShapes[4].top = 1;
	gShapes[4].left = 1;
	gShapes[4].right = gWindows[ kTopWindow ]->portRect.right - 1;
	gShapes[4].bottom = kObjectWidth;
	
	gShapes[5].top = ( gWindows[ kPlayerWindow ]->portRect.bottom - playerPaddleHeight ) / 2;
	gShapes[5].left = gWindows[ kPlayerWindow ]->portRect.right - kObjectWidth;
	gShapes[5].right = gWindows[ kPlayerWindow ]->portRect.right - 1;
	gShapes[5].bottom = gShapes[5].top + playerPaddleHeight;
	
	gShapes[6].top = gWindows[ kBottomWindow ]->portRect.bottom - kObjectWidth;
	gShapes[6].left = 1;
	gShapes[6].right = gWindows[ kBottomWindow ]->portRect.right - 1;
	gShapes[6].bottom = gWindows[ kBottomWindow ]->portRect.bottom - 1;

}

void InitToolBox( void ) {
	InitGraf( &thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
}

void LaunchBall( void ) {
	
	RandomRect( &gBall );
	gHorizontal = Randomize( gDifficulty );
	if ( gHorizontal < kMinSpeed  )
		gHorizontal = kMinSpeed;
	if ( gDifficulty == iNightmare && gHorizontal < kMinNightmareSpeed ) 
		gHorizontal = kMinNightmareSpeed;
	if ( gPlayerScore + gOpponentScore > kWinningScore ) gHorizontal++;

	gHorizontal = -gHorizontal; // Always start with ball moving toward opponent.

	gVertical = Randomize( gDifficulty );
	if ( gVertical < kMinSpeed )
		gVertical = kMinSpeed;
	if ( gDifficulty == iNightmare && gVertical < kMinNightmareSpeed ) 
		gVertical = kMinNightmareSpeed;
	if ( gPlayerScore + gOpponentScore > kWinningScore ) gVertical++;

	
	if ( gBall.top > screenBits.bounds.bottom / 2 ) 
		gVertical = -gVertical;
}

void MoveBall( void ) {
    int i;
    
    gBall.top += gVertical;
    gBall.left += gHorizontal;
    
	for ( i = 0; i < kNumGameWindows; i++ ) {
		switch( i ) {
			case kMultiPongWindow:
				if ( DetectCollision( &gShapes[ 0 ], gWindows[ kMultiPongWindow ] ) ) {
					gHorizontal = -gHorizontal;
					gBall.left += gHorizontal*2;
					gBall.left += 1;
				}
				if ( DetectCollision( &gShapes[ 1 ], gWindows[ kMultiPongWindow ] ) ) {
					gHorizontal = -gHorizontal;
					gBall.left += gHorizontal*2;
					gBall.left += 1;
				}
				if ( DetectCollision( &gShapes[ 2 ], gWindows[ kMultiPongWindow ] ) ) {
					ScorePlayer();
				}
				if ( DetectCollision( &gShapes[ 3 ], gWindows[ kMultiPongWindow ] ) ) {
					gHorizontal = -gHorizontal;
					gBall.left += gHorizontal*2;
					gBall.left += 1;
				}
				break;
			case kTopWindow:
				if ( DetectCollision( &gShapes[ 4 ], gWindows[ kTopWindow ] ) ) {
					gVertical = -gVertical;
					gBall.top += gVertical*2;
					gBall.top += 1;
				}
				break;
			case kBottomWindow:
				if ( DetectCollision( &gShapes[ 6 ], gWindows[ kBottomWindow ] ) ) {
					gVertical = -gVertical;
					gBall.top += gVertical*2;
					gBall.top -= 1;
					DrawPlayfield( gWindows[ kBottomWindow ] );
				}
				break;
			case kPlayerWindow:
				if ( DetectCollision( &gShapes[ 5 ], gWindows[ kPlayerWindow ] ) ) {
					gHorizontal = -gHorizontal;
					gBall.left += gHorizontal*2;
					gBall.left -= 1;
					DrawPlayfield( gWindows[ kPlayerWindow ] );
				}
				break;
		}
	}
    
    gBall.bottom = gBall.top + kBallSize;
	gBall.right = gBall.left + kBallSize;

}

void MoveOpponent( void ) {
	short ballYPosition;
	Rect ball = gBall;
	Rect eraseMe;

	if ( ( TickCount() % ( gDifficulty + 1 ) ) == 0 ) return;
	
	SetPort( gWindows[ kMultiPongWindow ] );

	GlobalToLocal(&topLeft(ball));
	GlobalToLocal(&botRight(ball));
	
	ballYPosition = ball.bottom - ( kBallSize / 2 );
	
	if ( ballYPosition > gOpponentYPosition ) {
		eraseMe.top = gOpponentYPosition - gPaddleHeight / 2;
		eraseMe.bottom = eraseMe.top + 1;
		gOpponentYPosition++;
	} else if ( ballYPosition < gOpponentYPosition ) {
		eraseMe.top = ( gOpponentYPosition + gPaddleHeight / 2 ) - 1;
		eraseMe.bottom = eraseMe.top + 1;
		gOpponentYPosition--;
	}
	
	gShapes[3].top = gOpponentYPosition - gPaddleHeight / 2;
	gShapes[3].bottom = gShapes[3].top + gPaddleHeight;
	gShapes[3].left = 1 + kObjectWidth + 1;
	gShapes[3].right = gShapes[3].left + kObjectWidth;
	
	PenPat( kPaddlePattern );
	ForeColor( kPaddleColor );
	PaintRect( &gShapes[3] );
	
	eraseMe.left = gShapes[3].left;
	eraseMe.right = gShapes[3].right;
	EraseRect( &eraseMe );
}

short Randomize(short range) {
	long randomNumber;
	randomNumber = Random();
	if (randomNumber < 0) 
		randomNumber *= -1;
	
	return ( (randomNumber*range) / kRandomUpperLimit );
}

void RandomRect (Rect *rectPtr) {
	WindowPtr	window;
	window = gWindows[ kMultiPongWindow ];
	rectPtr->left   = Randomize(( window->portRect.right / 2) ) + (window->portRect.right / 2) - ( kBallSize ) ;
	rectPtr->right  = rectPtr->left + kBallSize;
	rectPtr->top    = Randomize(window->portRect.bottom - - kBallSize * 2) + kBallSize;
	rectPtr->bottom  = rectPtr->top + kBallSize;
}

void ScoreOpponent( void ) {
	gOpponentScore++;
	SysBeep( 10 );
	
	if ( gPlayerScore < kWinningScore && gOpponentScore < kWinningScore ) {
		DisposeGameWindows();
		CreateGameWindows();
		DisplayScore();
		LaunchBall();
	} else { 
		EndGame();
		DisplayResults();
	}
}

void ScorePlayer( void ) {
	gPlayerScore++;
	SysBeep( 10 );
	
	if ( gPlayerScore < kWinningScore && gOpponentScore < kWinningScore ) {
		DisposeGameWindows();
		CreateGameWindows();
		DisplayScore();
		LaunchBall();
	} else { 
		EndGame();
		DisplayResults();
	}
}

void ShowAboutWindow( void ) {
	WindowPtr	window;
	if ( !gAboutVisible ) {
		window = CreateWindow( kAboutWindow );
		SetPort( window );
		WriteStrPound( kAboutText );
		gAboutVisible = true;
	}
}

void StartGame( void ) {
	MenuHandle		menu;
	if ( gameOn ) return;

	gPlayerScore = gOpponentScore = 0;
	menu = GetMHandle( mFile );
	DisableItem( menu, iNew );
	DisableItem( menu, iDifficulty );
	EnableItem( menu, iStartOver );
	EnableItem( menu, iEndGame );
	
	menu = GetMHandle( mWindow );
	EnableItem( menu, 1 );
	EnableItem( menu, 2 );
	EnableItem( menu, 3 );
	EnableItem( menu, 4 );

	CreateGameWindows();
	LaunchBall();
	
	gameOn = true;
	gLastTick = TickCount();
}

void WriteStrPound ( int which ) {
	short			fontNum;
	short 			pos;
	int 			index = 0;
	StringHandle	theString;
	FontInfo		fInfo;
	short			lineHeight;
	
	switch ( which ) {
		case kAboutWindow:
			GetFNum( "\pGeneva", &fontNum);	
			TextSize(9);
			break;
		case kThinkWindow:
			GetFNum( "\pPalatino", &fontNum);	
			TextSize(12);
			break;	
	}

	if ( fontNum != 0 ) 
		TextFont(fontNum);
	
	ForeColor( blackColor );
	GetFontInfo(&fInfo);

	lineHeight = fInfo.ascent + fInfo.descent + fInfo.leading;

	while ( true ) {
		GetIndString(*theString, which, ++index);
		if ( *theString[0] == 0 ) return;
		
		if ( which == kAboutWindow ) {
			if ( index == 1 ) TextFace( bold );
			pos = (thePort->portRect.right / 2) - ( StringWidth(*theString) / 2 );
			MoveTo( pos , index*lineHeight);		
			DrawString(*theString);			
			if ( index == 1 ) TextFace( 0 );
		} else {
			pos = StringWidth( "\pX" );
			MoveTo( pos, index*lineHeight);		
			DrawString(*theString);
		}
	}

}