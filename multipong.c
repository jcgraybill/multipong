#include <Values.h>

#define	kBaseResID			128
#define	kTicks				1L

#define	kGameWindow			0
#define kAboutWindow		kBaseResID
#define	kAboutText			kBaseResID
#define kThinkWindow		kBaseResID+1
#define kThinkText			kBaseResID+1

#define	mApple				kBaseResID
#define	iAbout				1

#define	mFile				kBaseResID+1
#define iNew				1
#define	iStartOver			2
#define	iEndGame			3
#define	iClose				4
#define	iQuit				6

#define	mEdit				kBaseResID+2

#define	mWindow				kBaseResID+3

#define	mExtras				kBaseResID+4
#define	iThink				1

#define	kStartingWinBuf		30
#define	kVisible			true
#define	kBringToFront		(WindowPtr)-1L
#define	kLeaveInBack		0
#define	kHasGoAway			true
#define	kMenuBarHeight		20 // TODO: can Toolbox tell me this?

#define	kNumGameWindows		4
#define	kMultiPongWindow	0
#define	kTopWindow			1
#define	kBottomWindow		2
#define	kPlayerWindow		3

#define	kRandomUpperLimit	32768
#define	kBallSize			20
#define kMaxSpeed			5
#define	kMinSpeed			2

void		MenuInit( void );
void		GameWindowInit( void );
void		GameInit( void );
void		EventLoop( void );
void		HandleCloseWindow( WindowPtr window );
void		HandleMouseDown( EventRecord *eventPtr );
void		HandleMenuChoice( long menuChoice );
void		HandleAppleChoice( short item );
void		HandleFileChoice( short item );
void		HandleEditChoice( short item );
void		HandleWindowChoice( short item );
void		HandleExtrasChoice( short item );
void		RecalcBall( void );
void		DisplayBall( void );
void		EraseBall( void );
short 		EraseBallInWindow( WindowPtr window );
short		DisplayBallInWindow( WindowPtr window );
void		RandomRect( Rect *rectPtr );
short		Randomize( short range );
void		WriteStrPound ( int which );
void		DoUpdate( EventRecord *event );
void		ToolBoxInit( void );
WindowPtr	WindowInit( int whichOne );
void		bail( void );
void 		CenterPict( PicHandle picture, Rect *destRectPtr );
void		ShowAboutWindow( void );
void		StartGame( void );
void 		EndGame( void );
void		GameLoop( void );
void		DisplayResults( void );

Rect 		gBall;
WindowPtr	gWindows[kNumGameWindows];
short		gQuitting, gAboutVisible, gThinkVisible;
short		gHorizontal, gVertical;
short		gameOn, gPlayerScore, gOpponentScore = 0;


int main( void ) {
	ToolBoxInit();
	MenuInit();
	
	ShowAboutWindow();
	EventLoop();
	return 0;
}

void MenuInit( void ){
	Handle			menuBar;
	MenuHandle		menu;

	menuBar = GetNewMBar( kBaseResID );
	SetMenuBar( menuBar );
	
	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
		
	DrawMenuBar();
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
			GameLoop();
		}
	}
}

void GameLoop( void ) {
	EraseBall();
	RecalcBall();
	DisplayBall();
}

void StartGame( void ) {
	MenuHandle		menu;
	if ( gameOn ) return;

	gPlayerScore = gOpponentScore = 0;
	menu = GetMHandle( mFile );
	DisableItem( menu, iNew );
	EnableItem( menu, iStartOver );
	EnableItem( menu, iEndGame );
	
	menu = GetMHandle( mWindow );
	EnableItem( menu, 1 );
	EnableItem( menu, 2 );
	EnableItem( menu, 3 );
	EnableItem( menu, 4 );

	GameWindowInit();
	GameInit();
	
	gameOn = true;
}

void EndGame( void ) {
	MenuHandle		menu;
	short i;
	
	if ( ! gameOn ) return;

	menu = GetMHandle( mFile );
	EnableItem( menu, iNew );
	DisableItem( menu, iStartOver );
	DisableItem( menu, iEndGame );

	menu = GetMHandle( mWindow );
	DisableItem( menu, 1 );
	DisableItem( menu, 2 );
	DisableItem( menu, 3 );
	DisableItem( menu, 4 );

	for ( i = 0; i < kNumGameWindows ; i++ ) {
		DisposeWindow( gWindows[i] );
	}

	gameOn = false;

}

void DisplayResults( void ) {
	// TODO - put actual results here
	ParamText( "\p7", "\p3", "\pCongrats", "\pwon!");
	NoteAlert( kBaseResID, nil );
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
		case kGameWindow:
			SetPort( window );
			EraseBallInWindow( window );
			DisplayBallInWindow( window );
			break;
	}
	EndUpdate( window );
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
			EraseBall(); // hide the ball while you're dragging windows
			DragWindow( window, eventPtr->where, &screenBits.bounds );
			DisplayBall();
			break;
		case inGoAway:
			if ( TrackGoAway( window, eventPtr->where ) ) HandleCloseWindow( window );
			break;
	}
}



void HandleCloseWindow( WindowPtr window ) {
	switch ( GetWRefCon( window ) ) {
		case kGameWindow: 
			HideWindow( window );
			break;
		case kAboutWindow:
			gAboutVisible = false;
			DisposeWindow( window );
			break;
		case kThinkWindow:
			gThinkVisible = false;
			DisposeWindow( window );
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
		}
	HiliteMenu( 0 );
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

void ShowAboutWindow( void ) {
	WindowPtr	window;
	if ( !gAboutVisible ) {
		window = WindowInit( kAboutWindow );
		SetPort( window );
		WriteStrPound( kAboutText );
		gAboutVisible = true;
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
			DisplayResults();
			EndGame();
			break;
		case iClose:
			HandleCloseWindow( FrontWindow() );
			break;
		case iQuit:
			gQuitting = true;
			break;	
	}
}

void HandleEditChoice( short item ) { 
	return;
}

void HandleWindowChoice( short item ) {
	if ( item > kNumGameWindows ) return;
	ShowWindow( gWindows[ item - 1 ] );
	SelectWindow( gWindows[ item - 1 ] );
}

void HandleExtrasChoice( short item ) { 
	WindowPtr	window;

	if ( item == iThink && !gThinkVisible ) {
		window = WindowInit( kThinkWindow );
		SetPort( window );
		WriteStrPound( kThinkText );
		gThinkVisible = true;
		return;
	}
}



void GameWindowInit( void ) {
	Rect MultiPongBounds, TopBounds, BottomBounds, PlayerBounds;
	int i;
	
	MultiPongBounds.top = kMenuBarHeight + kStartingWinBuf;
	MultiPongBounds.left = kStartingWinBuf;
	MultiPongBounds.right = ( screenBits.bounds.right  / 2 ) - kStartingWinBuf;
	MultiPongBounds.bottom = screenBits.bounds.bottom - kStartingWinBuf;
	
	TopBounds.top = kMenuBarHeight + kStartingWinBuf;
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
		if ( !gWindows[i] )	bail();
	}
}

void GameInit( void ) {
    GetDateTime((unsigned long *)(&randSeed));
	RandomRect( &gBall );
	gHorizontal = Randomize( kMaxSpeed );
	if ( gHorizontal < kMinSpeed )
		gHorizontal = kMinSpeed;
	gHorizontal = -gHorizontal; // Always start with ball moving toward opponent.
	gVertical = Randomize( kMaxSpeed );
	if ( gVertical < kMinSpeed )
		gVertical = kMinSpeed;
}

void RecalcBall( void ) {
    
    gBall.top += gVertical;
    if ((gBall.top < screenBits.bounds.top) ||
        (gBall.bottom > screenBits.bounds.bottom)) {
        gVertical *= -1;
        gBall.top += 2*gVertical;
    }
    
    gBall.bottom = gBall.top + kBallSize;
    
    gBall.left += gHorizontal;
    if ((gBall.left < screenBits.bounds.left) ||
        (gBall.right > screenBits.bounds.right)) {
        gHorizontal *= -1;
        gBall.left += 2*gHorizontal;
    }

	gBall.right = gBall.left + kBallSize;

}

void DisplayBall( void ) {
	int i;
	for ( i = 0; i < kNumGameWindows; i++ ) {
		DisplayBallInWindow( gWindows[i] );
	}
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
			EraseRect( &ball );
			return true;
		} else {
			return false;
		}
}

short DisplayBallInWindow( WindowPtr window ) {
		Rect ball = gBall;
		Rect win = window->portRect;
		Rect sect;
					
		SetPort( window );
		GlobalToLocal(&topLeft(ball));
		GlobalToLocal(&botRight(ball));
			
		if ( SectRect( &ball, &win, &sect ) ) {
			PaintOval( &ball );
			return true;
		} else {
			return false;
		}
}

void RandomRect (Rect *rectPtr) {
	WindowPtr	window;
	window = FrontWindow();
	rectPtr->left   = Randomize(window->portRect.right - window->portRect.left);
	rectPtr->right  = rectPtr->left + kBallSize;
	rectPtr->top    = Randomize(window->portRect.bottom - window->portRect.top);
	rectPtr->bottom  = rectPtr->top + kBallSize;
}

short Randomize(short range) {
	long randomNumber;
	randomNumber = Random();
	if (randomNumber < 0) 
		randomNumber *= -1;
	
	return ( (randomNumber*range) / kRandomUpperLimit );
}

void ToolBoxInit( void ) {
	InitGraf( &thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
}

WindowPtr WindowInit( int whichOne ) {
	WindowPtr	window;
	Rect		windRect;
	
	window = GetNewWindow( whichOne, nil, kBringToFront );
	
	if ( window == nil )
		bail();

	SetWRefCon( window,(long)whichOne); 
	ShowWindow( window );
	return window;	
}

void bail( void ) {
	SysBeep(10);
	ExitToShell();
}

void CenterPict( PicHandle picture, Rect *destRectPtr ) {
	Rect windRect, pictRect;
	windRect = *destRectPtr;
	pictRect = (**(picture)).picFrame;
	OffsetRect(&pictRect, windRect.left-pictRect.left, windRect.top-pictRect.top);
	OffsetRect(&pictRect, (windRect.right-pictRect.right)/2, (windRect.bottom-pictRect.bottom)/2);
	*destRectPtr = pictRect;
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