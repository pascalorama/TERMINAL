#include <MainMenuState.h>
#include <GameState.h>
#include <Peripheral.h>
#include <Renderer.h>
#include <Menu.h>
#include <Log.h>
#include <MultiPlayerState.h>

static const Uint32 PDD_SOFT_RESET_STANDARD_GAMEPAD =
	( PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY | PDD_DGT_ST );

typedef struct _tagMAINMENU_GAMESTATE
{
	GAMESTATE	Base;
	PGLYPHSET	pGlyphSet;
	MENU		Menu;
}MAINMENU_GAMESTATE,*PMAINMENU_GAMESTATE;

static MAINMENU_GAMESTATE MainMenuState;

static int BootROM( void *p_pArgs );
static int LaunchMultiPlayer( void *p_pArgs );

static int MMS_Load( void *p_pArgs )
{
	PMAINMENU pArguments = p_pArgs;
	KMPACKEDARGB TextColour;
	KMPACKEDARGB HighlightColour;
	MENU_ITEM MenuItems[ 4 ];
	SELECTION_HIGHLIGHT_STRING SelectionHighlight;

	TextColour.dwPacked = 0xFFFFFFFF;
	HighlightColour.dwPacked = 0xFF00FF00;

	MainMenuState.pGlyphSet = GSM_GetGlyphSet(
		MainMenuState.Base.pGameStateManager, GSM_GLYPH_SET_GUI_1 );

	MenuItems[ 0 ].pName = "SINGLE PLAYER [COMING 20XX]";
	MenuItems[ 0 ].Function = NULL;

	MenuItems[ 1 ].pName = "MULTI PLAYER";
	MenuItems[ 1 ].Function = LaunchMultiPlayer;

	MenuItems[ 2 ].pName = "OPTIONS";
	MenuItems[ 2 ].Function = NULL;

	MenuItems[ 3 ].pName = "RESET TO BOOTROM";
	MenuItems[ 3 ].Function = &BootROM;

	SelectionHighlight.Base.Type = SELECTION_HIGHLIGHT_TYPE_STRING;
	SelectionHighlight.Base.HighlightColour = HighlightColour;
	SelectionHighlight.pString = "$ ";

	if( MNU_Initialise( &MainMenuState.Menu, MenuItems, 4, &SelectionHighlight,
		GSM_GetGlyphSet( MainMenuState.Base.pGameStateManager,
			GSM_GLYPH_SET_GUI_1 ), TextColour, MENU_ITEM_ALIGNMENT_LEFT,
		MainMenuState.Base.pGameStateManager->MemoryBlocks.pSystemMemory ) !=
		0 )
	{
		LOG_Debug( "MMS_Load <ERROR> Failed to initialise the menu\n" );

		return 1;
	}

	return 0;
}

static int MMS_Initialise( void *p_pArgs )
{
	return 0;
}

static int MMS_Update( void *p_pArgs )
{
	if( MainMenuState.Base.Paused == false )
	{
		/* Soft-reset */
		if( ( g_Peripherals[ 0 ].on & PDD_SOFT_RESET_STANDARD_GAMEPAD ) ==
			PDD_SOFT_RESET_STANDARD_GAMEPAD )
		{
			GSM_Quit( MainMenuState.Base.pGameStateManager );

			return 0;
		}

		if( g_Peripherals[ 0 ].press & PDD_DGT_KU )
		{
			MNU_SelectPreviousMenuItem( &MainMenuState.Menu );
		}

		if( g_Peripherals[ 0 ].press & PDD_DGT_KD )
		{
			MNU_SelectNextMenuItem( &MainMenuState.Menu );
		}

		if( g_Peripherals[ 0 ].press & PDD_DGT_TA )
		{
			PMENU_ITEM pMenuItem;

			pMenuItem = MNU_GetSelectedMenuItem( &MainMenuState.Menu );

			if( pMenuItem )
			{
				if( pMenuItem->Function )
				{
					pMenuItem->Function( NULL );
				}
			}
		}
	}

	return 0;
}

static int MMS_Render( void *p_pArgs )
{
	KMPACKEDARGB TextColour;
	float TextLength;

	if( MainMenuState.Base.Paused == false )
	{
		TextColour.dwPacked = 0xFFFFFFFF;
		REN_Clear( );

		TXT_MeasureString( MainMenuState.pGlyphSet, "[TERMINAL] //MAIN MENU",
			&TextLength );
		TXT_RenderString( MainMenuState.pGlyphSet, &TextColour,
			320.0f - ( TextLength * 0.5f ), 32.0f,
			"[TERMINAL] //MAIN MENU" );

		TXT_MeasureString( MainMenuState.pGlyphSet, "[A] select",
			&TextLength );
		TXT_RenderString( MainMenuState.pGlyphSet, &TextColour,
			640.0f - 64.0f - TextLength,
			480.0f - ( 32.0f + ( float )MainMenuState.pGlyphSet->LineHeight ),
			"[A] select" );

		MNU_Render( &MainMenuState.Menu, 1.5f, 320.0f, 240.0f );

		REN_SwapBuffers( );
	}

	return 0;
}

static int MMS_Terminate( void *p_pArgs )
{
	return 0;
}

static int MMS_Unload( void *p_pArgs )
{
	MNU_Terminate( &MainMenuState.Menu );

	return 0;
}

int MMS_RegisterWithGameStateManager(
	PGAMESTATE_MANAGER p_pGameStateManager )
{
	MainMenuState.Base.Load = &MMS_Load;
	MainMenuState.Base.Initialise = &MMS_Initialise;
	MainMenuState.Base.Update = &MMS_Update;
	MainMenuState.Base.Render = &MMS_Render;
	MainMenuState.Base.Terminate = &MMS_Terminate;
	MainMenuState.Base.Unload = &MMS_Unload;
	MainMenuState.Base.pGameStateManager = p_pGameStateManager;

	return GSM_RegisterGameState( p_pGameStateManager, GAME_STATE_MAINMENU,
		( GAMESTATE * )&MainMenuState );
}

static int LaunchMultiPlayer( void *p_pArgs )
{
	GSM_PushState( MainMenuState.Base.pGameStateManager,
		GAME_STATE_MULTIPLAYER_MAIN, NULL, NULL );

	return 0;
}

static int BootROM( void *p_pArgs )
{
	GSM_Quit( MainMenuState.Base.pGameStateManager );

	return 0;
}

