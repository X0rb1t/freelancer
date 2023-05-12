#include "stdafx.h"



#include <string.h>
#include <TCHAR.h>
#include <direct.h>
#include "main.h"
#include "basicbutton.h"
#include "headers.h"
#include "ItemRender.h"
#include "ItemProto.h"
#include "Nk2DFrame.h"
#include "UIMgr.h"
#include "tcpipcon.h"
#include "textoutbox.h"
#include "controlbottom.h"
#include "controlinven.h"
#include "controlexchange.h"
#include "msgpopup.h"
#include "g_stringmanager.h"
#include "NkCharacter.h"
#include "DirtSnd.h"
#include "Country.h"

#include "Sprite.h"
#include "ControlUpgrade.h"
#include "Mouse.h"


#include "LHItemSystem_Manager.h"
#include "LHParam_MakeItem.h"
#include "LHParam_SeperateItem.h"
#include "LHParam_GMagicStone.h"

#include "LHParam_ConfusionItem.h"
#include "LHParam_EnchantItem.h"



#define IS_2011_ITEM_ARMOR(vnum) ((2295<=vnum && vnum<=2316) || (2942<=vnum && vnum<=2953) || (3152<=vnum && vnum<=3155) || (3227<=vnum && vnum<=3230) ? TRUE : FALSE )
#define IS_2011_ITEM_WEAPON(vnum) ((2743<=vnum && vnum<=2749) || (2939<=vnum && vnum<=2941) || (3151==vnum) || (3231==vnum) ? TRUE : FALSE ) // System Upgrade 360 Weapon = 01

#define IS_G_LEVEL_ITEM(vnum) ( (2173<=vnum && vnum<=2179) || (2743<=vnum && vnum<=2749) )

#ifndef GET_MY_SLOT_X
#define GET_MY_SLOT_X(point_x)		((point_x - m_my_slotstart_x) / SLOT_WIDTH)
#endif

#ifndef GET_MY_SLOT_Y
#define GET_MY_SLOT_Y(point_y)		((point_y - m_my_slotstart_y) / SLOT_HEIGHT)
#endif

extern DWORD				g_dwLangType;

extern int					g_EffectSound[];
extern DWORD				g_dwClientCountry;



extern BOOL CheckCanNpcUpgradeItem( CItem *pItem ); // For checking whether an item can be strengthened.
extern BOOL CheckCanConversionItem( CItem *pItem ); // For checking whether an item can be converted.
extern BOOL CheckIsConversionItem( CItem *pItem, int type = 0 ,int sub_type=0); // Check if it is a converted item. (Used to unlock Manastone restrictions, etc.)
extern BOOL ChechIs261LvWeponItem(CItem *pItem); // Check if it is 261 True Mugiru
extern BOOL CheckIsGodItem(CItem *pItem);


// Comments for temporary Hangul strings => // han_msg

const __int64 NeedLaim_GodPower = 200000000; // System Unlock Money


CControlUpgrade::CControlUpgrade()
	: m_hWnd(NULL)
	, m_pNkCha(NULL)
	, m_pItemSystemMgr(NULL)
{
	memset(m_iNeedGemVnum, 0, sizeof(m_iNeedGemVnum));
	memset(m_iNeedGemCount, 0, sizeof(m_iNeedGemCount));
	memset(m_need_gem, 0, sizeof(m_need_gem));

	m_pBackSur = NULL;
	m_pBack_Title = NULL;
	m_pTextInfo = NULL;
	m_pEnableSlot = NULL;
	m_pDisableSlot = NULL;
	m_pExistSlot  = NULL;
	m_pDiaGem = NULL;
	m_pRegentDiaGem = NULL;
	m_pRegOrGrtDiaGem = NULL;

	for( int i = 0 ; i < MAX_UPGRADE_ANI ; ++i )
		m_pAniSound[i] = NULL;




	Init(); // Initialize variables.


}

CControlUpgrade::~CControlUpgrade()
{
	DeleteRes();

	m_pNkCha = NULL;


}

void CControlUpgrade::Init()
{
	int i = 0;

	m_iType = 0; // A flag whether it is reinforced or processed. If it is currently 0, it is strengthened. If it is 1, it is processed.
	m_my_slotstart_x = 0;
	m_my_slotstart_y = 0;

	m_NeedGemText.SetString( _T(" ")/*atoi(m_strMoney1)*/, true);
	m_NeedGemNumText.SetString( _T(" ")/*atoi(m_strMoney1)*/, true);
	m_NeedMoneyText1.SetString( _T(" ")/*atoi(m_strMoney1)*/, true);
	m_NeedMoneyText2.SetString( _T("0")/*atoi(m_strMoney1)*/, true);
	m_NeedMoneyText3.SetString( _T("")/*atoi(m_strMoney1)*/, true);
	m_HaveMoneyText.SetString( _T("0")/*atoi(m_strMoney1)*/, true);

	m_bSlotLock = FALSE; // A flag to prevent it from being pulled out of the slot when replacing.
	m_NowState = UPGRADE_ANI_NON;

	m_dwLastTickTime = 0;
	m_dwStartTickTime = 0;
	m_dwMinAniTime = 0;
	m_dwMaxAniTime = 0;
	m_iAniChangeFrame = 0;

	for( i = 0 ; i < MAX_NEED_ITEM_IN_NPC_UPGRADE ; ++i )
	{
		NeedItem[i] = 0; // The number required for each item type when strengthening or processing.
	}
	NeedLaim = 0; //Need lime.

	m_iOldMoney = 0; // previous holdings

	m_Result = 0;

	m_iTextInfoIndex = 200; // During conversion, the text info is loaded differently depending on the situation and used. A variable that remembers which image is loaded and the number index of the previous image.
    m_iOldTextInfoIndex = 0; // During conversion, the text info is loaded differently depending on the situation and used. A variable that remembers which image is loaded and the number index of the previous image.
	m_pItemRender = NULL;
	m_pOverItem = NULL;
	m_dwMouseClickTickTime = timeGetTime();
}

void CControlUpgrade::DeleteRes()
{
	m_UpgradeBtn.DeleteRes();
	m_BtnCancel.DeleteRes();
	m_NeedGemText.DeleteRes();
	m_NeedGemNumText.DeleteRes();
	m_NeedMoneyText1.DeleteRes();
	m_NeedMoneyText2.DeleteRes();
	m_HaveMoneyText.DeleteRes();

	for( int i = 0 ; i < MAX_UPGRADE_ANI ; ++i )
	{
		m_SpriteAni[i].DeleteRes();
		SAFE_RELEASE( m_pAniSound[i] );
	}


	SAFE_DELETE(m_pBackSur);
	SAFE_DELETE(m_pBack_Title);
	SAFE_DELETE(m_pTextInfo);

	SAFE_DELETE(m_pEnableSlot);
	SAFE_DELETE(m_pDisableSlot);
	SAFE_DELETE(m_pExistSlot);

	SAFE_DELETE(m_pDiaGem);
	SAFE_DELETE(m_pRegentDiaGem);
	SAFE_DELETE(m_pRegOrGrtDiaGem);
}

void CControlUpgrade::LoadRes( int Type, CItemRender *pItemRender )
{
	if( !g_pDisplay || !pItemRender )
		return;

	int i = 0;

	Init(); //Initialize variables once.

	m_pItemRender = pItemRender;

	m_iType = Type;

	m_hWnd =  g_pDisplay->GetHWnd();
	GetClientRect( m_hWnd, &m_ClientRc );

	if( FAILED( g_pDisplay->CreateSurfaceFromBitmap( &m_pBackSur, "interface/npcupgrade/NPC_upgrade.BMP" ) ) )
		return;

	g_pDisplay->CreateSurfaceFromBitmap( &m_pEnableSlot, "interface/slot/slot_enable.bmp" );
	g_pDisplay->CreateSurfaceFromBitmap( &m_pDisableSlot, "interface/slot/slot_disable.bmp" );
	g_pDisplay->CreateSurfaceFromBitmap( &m_pExistSlot, "interface/slot/slot_exist.bmp" );

	g_pDisplay->CreateSurfaceFromBitmap( &m_pDiaGem, "interface/NpcUpgrade/NormalDia.bmp" );
	g_pDisplay->CreateSurfaceFromBitmap( &m_pRegentDiaGem, "interface/NpcUpgrade/RegentDia.bmp" );
	g_pDisplay->CreateSurfaceFromBitmap( &m_pRegOrGrtDiaGem, "interface/NpcUpgrade/RegOrGrtDia.bmp" );


	m_BtnCancel.SetFileName( "common/btn_close_01" );	  //cancel button

	m_NeedGemText.Init( 12, RGB(255,255,0), 0, 0 );
	m_NeedGemNumText.Init( 12, RGB(255,255,0), 0, 0 );
	m_NeedMoneyText1.Init( 12, RGB(255,255,0), 0, 0 );
	m_NeedMoneyText2.Init( 12, RGB(255,255,0), 0, 0 );
	m_NeedMoneyText3.Init( 12, RGB(255,255,0), 0, 0 );
	m_HaveMoneyText.Init( 12, RGB(255,255,0), 0, 0 );


	// Guys that load differently depending on whether they are reinforced or processed...
        switch( m_iType )
{
        case INTERFACE_TYPE_UPARMOR: // Strengthen Armor
        case INTERFACE_TYPE_UPWEAPON: // Enhance weapon
	    case INTERFACE_TYPE_UPARMOR_400:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/NpcUpgrade/NpcUpgrade_info.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_enhancement" );	  // cancel button

			if( g_dwClientCountry == CTRY_KOR ) // Korea decided to omit the anime because of some XXX called Remy Bonyaski.
			{
				// load sprite animation
				m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type0_ing", 13, 110 );
				m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type0_success", 25, 110 );
				m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes( "interface/NpcUpgrade/effect/type0_fail", 16, 110 );
				m_SpriteAni[UPGRADE_ANI_BROKEN].LoadRes( "interface/NpcUpgrade/effect/type0_broken", 18, 120 );

				// Input the values ​​required for other animations.
                m_dwMinAniTime = 2000; // Even if a packet arrives, it is switched after the specified time.
                m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
                m_iAniChangeFrame = 8; // transition at frame 8 of 'ing'
			}
			else // Let's put other countries.
			{
				// load sprite animation
				m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type0_ing", 13, 110 );
				m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type0_success", 25, 110 );
				m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes( "interface/NpcUpgrade/effect/type0_fail", 16, 110 );
				m_SpriteAni[UPGRADE_ANI_BROKEN].LoadRes( "interface/NpcUpgrade/effect/type0_broken", 18, 120 );

				// Input the values ​​required for other animations.
                m_dwMinAniTime = 2000; // Even if a packet arrives, it is switched after the specified time.
                m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
                m_iAniChangeFrame = 8; // transition at frame 8 of 'ing'


				//Loading sound effects.
				for( i = 0 ; i < MAX_UPGRADE_ANI ; ++i )
				{
					SAFE_RELEASE( m_pAniSound[i] );
				}

				if( g_pDSound )
				{
					m_pAniSound[UPGRADE_ANI_ING] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_ing.wav" );
					m_pAniSound[UPGRADE_ANI_SUCCESS] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_success.wav" );
					m_pAniSound[UPGRADE_ANI_FAIL] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_fail.wav" );
					m_pAniSound[UPGRADE_ANI_BROKEN] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_broken.wav" );
				}
			}
		}
		break;

	case INTERFACE_TYPE_CONVERSION: // conversion
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/Conversion_info200.bmp" );	// ÆÄÀÏ¸í ¹øÈ£ÀÇ ÀÇ¹Ì´Â 2(º¯È­)ÀÇ 00¹ø(±âº»¸àÆ®) ÀÌ¹ÌÁö ¶ó´Â ¶æ.
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_conversion" );

			if( g_dwClientCountry == CTRY_KOR ) // Korea decided to omit animation because of some XXX called Remy Bonyaski.
{
                // Don't load animation at all.

                // Other values ​​are set to 0.
                m_dwMinAniTime = 0; // Even if a packet arrives, it is switched after the specified time.
                m_dwMaxAniTime = 0; // Even if there is no packet, it is switched after the specified time.
                m_iAniChangeFrame = 0; // transition at frame 8 of 'ing'
			}
			else // Let's put other countries.
			{
				// load sprite animation
				m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type2_ing", 14, 110 );
				m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type2_success", 25, 110 );
				m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes( "interface/NpcUpgrade/effect/type2_fail", 24, 100 );

				// Enter the values ​​required for other animations.
				m_dwMinAniTime = 3000; // Even if a packet arrives, it is switched after the specified time.
                m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
                m_iAniChangeFrame = 13; // transition at frame 8 of 'ing'

				// Loading sound effects.
				for( i = 0 ; i < MAX_UPGRADE_ANI ; ++i )
				{
					SAFE_RELEASE( m_pAniSound[i] );
				}

				if( g_pDSound )
				{
					m_pAniSound[UPGRADE_ANI_ING] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_ing.wav" );
					m_pAniSound[UPGRADE_ANI_SUCCESS] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_success.wav" );
					m_pAniSound[UPGRADE_ANI_FAIL] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_broken.wav" );
					//			m_pAniSound[UPGRADE_ANI_BROKEN] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_broken.wav" );
				}
			}
		}
		break;

	case INTERFACE_TYPE_CONFUSION: // Random rewind.
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/Confusion_info.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_confusion" );

			if( g_dwClientCountry == CTRY_KOR ) // Korea decided to omit the anime because of some XXX called Remy Bonyaski.
			{
				// Don't load animation at all.

                // Other values ​​are set to 0.
               m_dwMinAniTime = 0; // Even if a packet arrives, it is switched after the specified time.
               m_dwMaxAniTime = 0; // Even if there is no packet, it is switched after the specified time.
               m_iAniChangeFrame = 0; // transition at frame 8 of 'ing'
			}
			else // Let's put other countries.
			{
				// Load sprite animation // This guy is an unconditional success.
               m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type2_ing", 14, 110 );
               m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type2_success", 25, 110 );

                // Input the values ​​required for other animations.
               m_dwMinAniTime = 3000; // Even if a packet arrives, it is switched after the specified time.
               m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
               m_iAniChangeFrame = 13; // transition at frame 8 of 'ing'

                // Load the sound effect.
				for( i = 0 ; i < MAX_UPGRADE_ANI ; ++i )
				{
					SAFE_RELEASE( m_pAniSound[i] );
				}

				if( g_pDSound )
				{
					m_pAniSound[UPGRADE_ANI_ING] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_ing.wav" );
					m_pAniSound[UPGRADE_ANI_SUCCESS] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_success.wav" );
				}
			}
		}
		break;

	case INTERFACE_TYPE_PROCESS:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/Process_info200.bmp" );	// The meaning of the file name number is the 00 (basic comment) image of 2 (change).
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_refining" );

			if( g_dwClientCountry == CTRY_KOR ) // Korea decided to omit animation because of some XXX called Remy Bonyaski.
{
                // Don't load animation at all.

                // Other values ​​are set to 0.
            m_dwMinAniTime = 0; // Even if a packet arrives, it is switched after the specified time.
            m_dwMaxAniTime = 0; // Even if there is no packet, it is switched after the specified time.
            m_iAniChangeFrame = 0; // transition at frame 8 of 'ing'
			}
			else // Let's put other countries.
			{
				m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type2_ing", 14, 110 );
				m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type2_success", 25, 110 );
				m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes(  "interface/NpcUpgrade/effect/type2_fail", 24, 100 );

				// Input the values ​​required for other animations.
                   m_dwMinAniTime = 3000; // Even if a packet arrives, it is switched after the specified time.
                   m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
                   m_iAniChangeFrame = 13; // transition at frame 8 of 'ing'

				// Loading sound effects.
				for( i = 0 ; i < MAX_UPGRADE_ANI ; ++i )
				{
					SAFE_RELEASE( m_pAniSound[i] );
				}

				if( g_pDSound )
				{
					m_pAniSound[UPGRADE_ANI_ING] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_ing.wav" );
					m_pAniSound[UPGRADE_ANI_SUCCESS] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_success.wav" );
					m_pAniSound[UPGRADE_ANI_FAIL] = g_pDSound->LoadWavToOutside( "data/sound/NpcUp_broken.wav" );
				}
			}
		}
		break;
	case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/info_Conversion2_50.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_conversion" );
			m_iTextInfoIndex = 50;
		// System Enable Effect Upgrade = 01
		//============================================================================================
		// load sprite animation
			m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type0_ing", 13, 110 );
			m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type0_success", 25, 110 );
			m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes( "interface/NpcUpgrade/effect/type0_fail", 16, 110 );
			m_SpriteAni[UPGRADE_ANI_BROKEN].LoadRes( "interface/NpcUpgrade/effect/type0_broken", 18, 120 );

			// Input the values ​​required for other animations.
            m_dwMinAniTime = 2000; // Even if a packet arrives, it is switched after the specified time.
            m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
            m_iAniChangeFrame = 8; // transition at frame 8 of 'ing'
		//============================================================================================
				m_iTextInfoIndex = 50;
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_UPGRADE:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/info_upgrade.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_enhancement" );
		// System Enable Effect Upgrade = 01
		//============================================================================================
			m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type2_ing", 14, 110 );
			m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type2_success", 25, 110 );
			m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes(  "interface/NpcUpgrade/effect/type2_fail", 24, 100 );

		// Input the values ​​required for other animations.
            m_dwMinAniTime = 3000; // Even if a packet arrives, it is switched after the specified time.
            m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
            m_iAniChangeFrame = 13; // transition at frame 8 of 'ing'
		//============================================================================================
		}
		break;

	case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/info_upgrade.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_enhancement" );

			m_gem_image.Init("npcupgrade/jewellist",5,5);

		// System Enable Effect Upgrade = 01
		//============================================================================================
			m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type2_ing", 14, 110 );
			m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type2_success", 25, 110 );
			m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes(  "interface/NpcUpgrade/effect/type2_fail", 24, 100 );

		// Input the values ​​required for other animations.
            m_dwMinAniTime = 3000; // Even if a packet arrives, it is switched after the specified time.
            m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
            m_iAniChangeFrame = 13; // transition at frame 8 of 'ing'
		//============================================================================================
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/info_godpow.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_Godpower" );

		// System Enable Effect Upgrade = 01
		//============================================================================================
			m_SpriteAni[UPGRADE_ANI_ING].LoadRes( "interface/NpcUpgrade/effect/type2_ing", 14, 110 );
			m_SpriteAni[UPGRADE_ANI_SUCCESS].LoadRes( "interface/NpcUpgrade/effect/type2_success", 25, 110 );
			m_SpriteAni[UPGRADE_ANI_FAIL].LoadRes(  "interface/NpcUpgrade/effect/type2_fail", 24, 100 );

		// Input the values ​​required for other animations.
            m_dwMinAniTime = 3000; // Even if a packet arrives, it is switched after the specified time.
            m_dwMaxAniTime = 5000; // Even if there is no packet, it is switched after the specified time.
            m_iAniChangeFrame = 13; // transition at frame 8 of 'ing'
		//============================================================================================
		}
		break;


	case INTERFACE_TYPE_ITEMMAKE:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pBack_Title, "interface/npcupgrade/TITLE_makeitem.bmp" );
			if( m_pBack_Title )
				m_pBack_Title->SetColorKey(TRANS_COLOR_NEW);
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/itemmake.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_production" );

			m_dwMinAniTime = 0;
			m_dwMaxAniTime = 0;
			m_iAniChangeFrame = 0;
			break;
		}

	case INTERFACE_TYPE_BUFFMAKE_MAKE:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pBack_Title, "interface/npcupgrade/TITLE_makeitem.bmp" );
			if( m_pBack_Title )
				m_pBack_Title->SetColorKey(TRANS_COLOR_NEW);
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/buffmake_make.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_production" );

			m_dwMinAniTime = 0;
			m_dwMaxAniTime = 0;
			m_iAniChangeFrame = 0;
		}
		break;

	case INTERFACE_TYPE_BUFFMAKE_GATCHA:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pBack_Title, "interface/npcupgrade/TITLE_gatcha.bmp" );
			if( m_pBack_Title )
				m_pBack_Title->SetColorKey(TRANS_COLOR_NEW);
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/buffmake_gatcha.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_gatcha" );

			m_dwMinAniTime = 0;
			m_dwMaxAniTime = 0;
			m_iAniChangeFrame = 0;
		}
		break;

	case INTERFACE_TYPE_BUFFMAKE_MEDAL:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pBack_Title, "interface/npcupgrade/TITLE_medal.bmp" );
			if( m_pBack_Title )
				m_pBack_Title->SetColorKey(TRANS_COLOR_NEW);
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/buffmake_medal.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_medal" );

			m_dwMinAniTime = 0;
			m_dwMaxAniTime = 0;
			m_iAniChangeFrame = 0;
		}
		break;

	case INTERFACE_TYPE_ITEMSEPARATE:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pBack_Title, "interface/npcupgrade/TITLE_disjointing.bmp" );
			if( m_pBack_Title )
				m_pBack_Title->SetColorKey(TRANS_COLOR_NEW);
			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/itemseparate.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/btn_disassamble" );

			m_dwMinAniTime = 0;
			m_dwMaxAniTime = 0;
			m_iAniChangeFrame = 0;
			break;
		}

	case INTERFACE_TYPE_GMAGICSTONE:
		{
			g_pDisplay->CreateSurfaceFromBitmap( &m_pBack_Title, "interface/REFINE/Refine_title.bmp" );
			if( m_pBack_Title )
				m_pBack_Title->SetColorKey(TRANS_COLOR_NEW);

			g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, "interface/description/npcupgrade/info_G_Magicstone.bmp" );
			m_UpgradeBtn.SetFileName( "NpcUpgrade/BTN_REFINING" );

			m_dwMinAniTime = 0;
			m_dwMaxAniTime = 0;
			m_iAniChangeFrame = 0;
			break;
		}




	default:
		return;
	}

	m_UpgradeBtn.LoadRes();
	m_BtnCancel.LoadRes();

	m_pBackSur->SetColorKey( TRANS_COLOR_NEW );

	m_pBackSur->Xpos = g_pNk2DFrame->GetClientWidth() - m_pBackSur->GetWidth() - 291; // System Windows Position = 0
	m_pBackSur->Ypos = 0;

	m_pDiaGem->Xpos = m_pBackSur->Xpos + 105;
	m_pDiaGem->Ypos = m_pBackSur->Ypos + 198;
	m_pRegentDiaGem->Xpos = m_pBackSur->Xpos + 105;
	m_pRegentDiaGem->Ypos = m_pBackSur->Ypos + 198;

	m_pRegOrGrtDiaGem->Xpos = m_pBackSur->Xpos + 105;
	m_pRegOrGrtDiaGem->Ypos = m_pBackSur->Ypos + 198;

	m_my_slotstart_x = m_pBackSur->Xpos + 34;
	m_my_slotstart_y = m_pBackSur->Ypos + 251;


	m_UpgradeBtn.SetPosition( m_pBackSur->Xpos+47, m_pBackSur->Ypos+199 );

	m_BtnCancel.SetPosition( m_pBackSur->Xpos+178, m_pBackSur->Ypos+17); // System windows Button = 0

	m_NeedGemText.SetPos( m_pBackSur->Xpos+35, m_pBackSur->Ypos+201+5 );
	m_NeedGemNumText.SetPos( m_pBackSur->Xpos+35, m_pBackSur->Ypos+201+5 );
	m_NeedMoneyText1.SetPos( m_pBackSur->Xpos+39, m_pBackSur->Ypos+170 ); // Place the item on it.
	m_NeedMoneyText2.SetPos( m_pBackSur->Xpos+35, m_pBackSur->Ypos+383 );

	m_HaveMoneyText.SetPos( m_pBackSur->Xpos+35, m_pBackSur->Ypos+423 ); // second space


	if( m_iType == INTERFACE_TYPE_CONVERSION  // Conversion and Refinement slightly change the position of the "Necessary Gem" character.
			|| m_iType == INTERFACE_TYPE_PROCESS )
	{
		m_NeedGemText.m_PosY -= 64;
	}

	m_HaveMoneyText.SetString( g_pRoh->m_Money, TRUE ); // Set when you open your own holding amount interface. Don't forget to update the part when you use it due to reinforcement
	m_iOldMoney = g_pRoh->m_Money;

	CheckUpgradeNeed(); // There are cases where you put it in the exchange window at the time of loading, so you have to calculate the necessary lime at this time as well.

	if( m_iType == INTERFACE_TYPE_CONVERSION  // Conversion and Refinement slightly change the position of the "Necessary Gem" character.
			|| m_iType == INTERFACE_TYPE_PROCESS
			|| m_iType == INTERFACE_TYPE_320LV_WEAPON_CONVERSION
	  )
	{
		UpdateTextInfo( m_iType, m_iTextInfoIndex ); // Update if changed.
	}
}

CItem* CControlUpgrade::Draw()
{
	if( !g_pDisplay )
		return NULL;

	m_pOverItem = NULL; // It is initialized here and is set in DrawMySlot() below.

	g_pDisplay->Blt( m_pBackSur->Xpos, m_pBackSur->Ypos, m_pBackSur );

	if ( NULL != m_pBack_Title )
	{
		g_pDisplay->Blt( m_pBackSur->Xpos, m_pBackSur->Ypos, m_pBack_Title );
	}

	DrawNeedInfo(); // Outputs the gems and amount required to enhance the item.

	m_UpgradeBtn.Draw();
	m_BtnCancel.Draw();

	// Check if the money has changed and update it.
	if( g_pRoh && m_iOldMoney != g_pRoh->m_Money )
	{
		m_HaveMoneyText.SetString( g_pRoh->m_Money, TRUE );
		m_iOldMoney = g_pRoh->m_Money;
	}

	m_HaveMoneyText.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+423 , 163);


	// State transition handling (note that this is state change handling, not animation of states)
	ProcessState();

	if( m_NowState == UPGRADE_ANI_NON ) // When not in animation.
	{
		DrawMySlot(); // Draw a slot.
	}
	else if( m_NowState >= 0 && m_NowState < MAX_UPGRADE_ANI ) // Check the abnormal range.
	{
		m_SpriteAni[m_NowState].Update( timeGetTime()-m_dwLastTickTime );
		m_SpriteAni[m_NowState].Draw( m_my_slotstart_x, m_my_slotstart_y );
	}



	m_dwLastTickTime = timeGetTime(); // Keep ticks updated.

	return m_pOverItem; //The reason for this return seems to be to update and display the item information displayed in the DrawItemInfo() function...
}

void CControlUpgrade::DrawNeedInfo() // Outputs the gems and amount required to enhance the item.
{
	int i = 0;

	if( m_pTextInfo )
		g_pDisplay->Blt( m_pBackSur->Xpos + 34, m_pBackSur->Ypos + 80, m_pTextInfo );

	switch(m_iType)
	{
	case INTERFACE_TYPE_UPARMOR_400:
		{
			if( CheckUpgradeNeed() == -1 )
			{
				break;
			}

			m_NeedGemText.Draw(m_pBackSur->Xpos+59, m_pBackSur->Ypos+145);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);

			const int gemGap = 16;

			int gemPosX = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2 + 30;
			int gemCntPosX = gemPosX + gemGap + m_NeedGemText.m_FontWidth / 2;
			int offsetY = m_pDiaGem->Ypos-74;

			char buf[MAX_PATH] = {0,};
			g_pNk2DFrame->GetItemRender()->BeginRenderLolo();
			for(int i = 0; i < NEEDGEM_MAX; i++)
			{
				if( m_iNeedGemCount[i] == 0 )
					continue;

				sprintf(buf, "(%d)", m_iNeedGemCount[i]);
				g_pNk2DFrame->RenderItemInUIWithSize(m_iNeedGemVnum[i], gemPosX, offsetY, 27, 54);
				if( m_iNeedGemVnum[i] == 3331 ) // Shinseok
				{						
					g_pNk2DFrame->RenderItemInUIWithSize(3332, gemPosX + 29, offsetY, 27, 54);

					sprintf(buf, "OR   (%d)", m_iNeedGemCount[i]);
				}
				m_NeedGemText.SetString(buf, FALSE);
				m_NeedGemText.Draw(gemPosX + 20, offsetY + 21);

				offsetY += gemGap;
			}
			g_pNk2DFrame->GetItemRender()->EndRenderLolo();
		}
		break;
	case INTERFACE_TYPE_UPARMOR:
	case INTERFACE_TYPE_UPWEAPON:
		{
			m_NeedGemText.Draw(m_pBackSur->Xpos+59, m_pBackSur->Ypos+160);
			m_NeedGemNumText.Draw(m_pBackSur->Xpos+150, m_pBackSur->Ypos+160);
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+175,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);

			int mx = 0;

			// Gem Drawing
			for( i = 0 ; i < MAX_NEED_ITEM_IN_NPC_UPGRADE ; ++i )
			{
				if( NeedItem[i] ) // If there is a guy with the required number...
				{
					switch( i )
					{
					case UPGRADE_NEED_DIA:
						mx = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2 + 30;
						g_pDisplay->Blt( mx, m_pDiaGem->Ypos-42, m_pDiaGem );
						mx = mx + m_pDiaGem->GetWidth() + m_NeedGemText.m_FontWidth / 2;
						break;
					case UPGRADE_NEED_REGENT_DIA:
						mx = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2 + 30;
						g_pDisplay->Blt( mx, m_pRegentDiaGem->Ypos-42, m_pRegentDiaGem );
						mx = mx + m_pRegentDiaGem->GetWidth() + m_NeedGemText.m_FontWidth / 2;
						break;
					}

					// Currently, there is only one type, so if there is one with the required number, take it and escape the loop.
                    // i = MAX_NEED_ITEM_IN_NPC_UPGRADE; // A trick to escape the loop. Currently, there is no need to escape.
				}
			}

			if( mx !=0 )
			{
				m_NeedGemNumText.SetPos(mx, m_NeedGemText.m_PosY );
			}
			m_NeedGemNumText.Draw();
		}
		break;

	case INTERFACE_TYPE_CONVERSION : // In the case of a conversion interface
		{
			if( m_iTextInfoIndex >= 203 ) // 203 The above is the requirement drawing condition.
				m_NeedGemText.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+206,163);

			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
		}
		break;

	case INTERFACE_TYPE_CONFUSION : // random redo
		{
			//m_NeedGemText.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+206,163);
			////m_NeedGemNumText.Draw();
			//m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			//m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);

			//int mx = 0;

			//// Gem drawing // This guy is definitely a diamond.
			//if( NeedLaim )
			//{
			//	mx = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2;
			//	g_pDisplay->Blt( mx, m_pDiaGem->Ypos, m_pDiaGem );
			//	mx = mx + m_pDiaGem->GetWidth() + m_NeedGemText.m_FontWidth / 2;
			//}

			m_NeedGemText.Draw(m_pBackSur->Xpos+59, m_pBackSur->Ypos+160);
			m_NeedGemNumText.Draw(m_pBackSur->Xpos+150, m_pBackSur->Ypos+160);
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+175,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);

			int mx = 0;


			if( NeedLaim )
			{
				mx = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2 + 30;
				g_pDisplay->Blt( mx, m_pDiaGem->Ypos-42, m_pDiaGem );
				mx = mx + m_pDiaGem->GetWidth() + m_NeedGemText.m_FontWidth / 2;
			}


			if( mx !=0 )
			{
				m_NeedGemNumText.SetPos(mx, m_NeedGemText.m_PosY );
			}
			m_NeedGemNumText.Draw();

		}
		break;

	case INTERFACE_TYPE_PROCESS: // In the case of a conversion interface
		{
			if( m_iTextInfoIndex >= 203 ) // 203 The above is the requirement drawing condition.
				m_NeedGemText.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+206,163);

			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
		}
		break;
	case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
		{
			if(m_iTextInfoIndex == 53)
			{
				m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			}
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_UPGRADE:
		{
			int mx = 0;

			if( NeedItem[UPGRADE_NEED_REGENT_DIA] )
			{
				mx = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2 + 30;
				g_pDisplay->Blt( mx, m_pRegOrGrtDiaGem->Ypos-46, m_pRegOrGrtDiaGem );
				mx = mx + m_pRegOrGrtDiaGem->GetWidth() + m_NeedGemText.m_FontWidth / 2;
			}

			m_NeedGemText.Draw(m_pBackSur->Xpos+59, m_pBackSur->Ypos+154);
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
			if( mx != 0 )
				m_NeedGemNumText.SetPos(mx, m_NeedGemText.m_PosY );
			m_NeedGemNumText.Draw();
		}
		break;

	case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
		{
			int mx = m_NeedGemText.m_PosX + m_NeedGemText.m_Width + m_NeedGemText.m_FontWidth / 2 + 30;

			int gem1 = GetGemToIndex(m_need_gem[0]);
			int gem2 = GetGemToIndex(m_need_gem[1]);
			int gem3 = GetGemToIndex(m_need_gem[2]);

			if( gem1 != -1 )
				m_gem_image.Draw(mx , m_pBackSur->Ypos+150 ,gem1);

			if( gem2 != -1 )
			{
				// or
				m_gem_image.Draw(mx + 16 , m_pBackSur->Ypos+150 ,24);
				m_gem_image.Draw(mx + 32 , m_pBackSur->Ypos+150 ,gem2);
			}
			if( gem3 != -1 )
			{
				m_NeedMoneyText1.SetString((char*)G_STRING(IDS_LHSTRING1954));
				m_NeedMoneyText1.Draw(m_pBackSur->Xpos+59, m_pBackSur->Ypos+170);
				m_gem_image.Draw(mx  , m_pBackSur->Ypos+168 ,gem3);
			}
			else
			{
				m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			}

			if( gem1 != -1 || gem2 != -1 )
				m_NeedGemText.Draw(m_pBackSur->Xpos+59, m_pBackSur->Ypos+154);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER:
		{
			if( NeedItem[UPGRADE_NEED_REGENT_DIA] )
			{
			}
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
		}
		break;

	case INTERFACE_TYPE_ITEMMAKE:  // Display Lime on NPC
		{
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
			m_NeedMoneyText3.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+405,163);
		}
		break;
	case INTERFACE_TYPE_BUFFMAKE_MAKE:
	case INTERFACE_TYPE_BUFFMAKE_GATCHA:	
		{
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
		}
		break;

	case INTERFACE_TYPE_ITEMSEPARATE:
		{

		}
		break;

	case INTERFACE_TYPE_GMAGICSTONE:
		{
			m_NeedMoneyText1.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+170,163);
			m_NeedMoneyText2.Draw(m_pBackSur->Xpos+35, m_pBackSur->Ypos+383,163);
		}
		break;


	}
}

void CControlUpgrade::DrawMySlot()
{
	if( !g_pDisplay )
		return;

	RECT rcRect;
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(g_pDisplay->GetHWnd(), &point);

	if (pCMyApp->m_pMouse && IsInside(point.x, point.y))
		pCMyApp->m_pMouse->SetMouseType(M_NORMAL);

	// Show the slot where the item is located
	CItem *pItem = NULL;
	if( g_pRoh )
		pItem = g_pRoh->m_ExgInven;

	while (pItem)
	{
		rcRect.left = 0;
		rcRect.right = SLOT_WIDTH * GET_ITEM_WIDTH(pItem);
		rcRect.top = 0;
		rcRect.bottom = SLOT_HEIGHT * GET_ITEM_HEIGHT(pItem);
		g_pDisplay->Blt( m_my_slotstart_x + (SLOT_WIDTH*pItem->m_SlotX),
						 m_my_slotstart_y + (SLOT_HEIGHT*pItem->m_SlotY),
						 m_pExistSlot, &rcRect);
		pItem = pItem->m_Next;
	}

	// Extra Slot indicate where items in the
	int slot_x_num = 0, slot_y_num = 0;
	if (g_pRoh && g_pRoh->m_ExtraSlot)
	{
		if (point.x >= m_my_slotstart_x
				&& point.x < m_my_slotstart_x + (SLOT_WIDTH * EXG_SLOT_X_NUM )
				&& point.y >= m_my_slotstart_y
				&& point.y < m_my_slotstart_y + (SLOT_HEIGHT * EXG_SLOT_Y_NUM) )
		{
			if (GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot) % 2)
				slot_x_num = (point.x - m_my_slotstart_x) / SLOT_WIDTH - (GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot)/2);
			else
				slot_x_num = (point.x - m_my_slotstart_x + (SLOT_WIDTH/2)) / SLOT_WIDTH - (GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot)/2);

			if (GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot) % 2)
				slot_y_num = (point.y - m_my_slotstart_y) / SLOT_HEIGHT - (GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot)/2);
			else
				slot_y_num = (point.y - m_my_slotstart_y + (SLOT_HEIGHT/2)) / SLOT_HEIGHT - (GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot)/2);

			if (slot_x_num >= 0
					&& slot_x_num + GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot) - 1 < EXG_SLOT_X_NUM
					&& slot_y_num >= 0
					&& slot_y_num + GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot) - 1 < EXG_SLOT_Y_NUM)
			{
				rcRect.left = 0;
				rcRect.right = SLOT_WIDTH * GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot);
				rcRect.top = 0;
				rcRect.bottom = SLOT_HEIGHT * GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot);
				if (g_pRoh->GetItemIndexInExgSlot(slot_x_num, slot_y_num,
												  GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot), GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot))
						>= -1)
				{
					g_pDisplay->Blt( m_my_slotstart_x + (SLOT_WIDTH*slot_x_num),
									 m_my_slotstart_y + (SLOT_HEIGHT*slot_y_num),
									 m_pEnableSlot, &rcRect);
				}
				else
				{
					g_pDisplay->Blt( m_my_slotstart_x + (SLOT_WIDTH*slot_x_num),
									 m_my_slotstart_y + (SLOT_HEIGHT*slot_y_num),
									 m_pDisableSlot, &rcRect);
				}
			}
		}
	}

	//Drawing my exchange...
	if (point.x >= m_my_slotstart_x && point.y >= m_my_slotstart_y)
	{
		slot_x_num = GET_MY_SLOT_X(point.x);
		slot_y_num = GET_MY_SLOT_Y(point.y);
	}
	else
	{
		slot_x_num = -1;
		slot_y_num = -1;
	}
	int index = 0;
	if( g_pRoh )
		index = g_pRoh->GetExgSlotIndex(slot_x_num, slot_y_num);

	if( g_pRoh )
		pItem = g_pRoh->m_ExgInven;

	if( m_pItemRender )
	{
		m_pItemRender->BeginRenderLolo();		// Important.. affects the parameters of m_pItemRender->RenderLolo.
		while (pItem)
		{
			if (pItem->m_Index == index)
			{
				g_pNk2DFrame->RenderItemInUI(m_my_slotstart_x + pItem->m_SlotX * SLOT_WIDTH,
											 m_my_slotstart_y + pItem->m_SlotY * SLOT_HEIGHT,
											 pItem , TRUE , FALSE , FALSE, TRUE );
				m_pOverItem = pItem;
			}
			else
			{
				DWORD ambient = 0x00555555;
				if (g_pNk2DFrame->IsScroll(pItem) )
					ambient = 0x00cccccc;
				else if (pItem->m_PlusNum > 0)
					ambient = UPGRADE_ITEM_COLOR;

				g_pNk2DFrame->RenderItemInUI(m_my_slotstart_x + pItem->m_SlotX * SLOT_WIDTH,
											 m_my_slotstart_y + pItem->m_SlotY * SLOT_HEIGHT,
											 pItem , FALSE , FALSE , FALSE, TRUE );
			}
			pItem = pItem->m_Next;
		}
		m_pItemRender->EndRenderLolo();
	}
}

LRESULT CControlUpgrade::MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char commOutBuf[512];
	int hr = -1;

	int x, y;
	x = LOWORD( lParam );
	y = HIWORD( lParam );

	if( m_bSlotLock ) // This is to prevent extras at the moment of exchange. lock
	{
		// How about forcibly releasing the lock after a certain period of time?
		return 1;
	}

	switch (msg)
	{
	case WM_RBUTTONDBLCLK:

		// Cannot move large equipment
        if( pCMyApp->m_bEquipLock == 1 )
            return 1;
        // Ring cannot be moved
        if( pCMyApp->m_bRingLock == 1 )
            return 1;
        if(pCMyApp->m_bBrLock ==1)
            return 1;
        if(pCMyApp->m_bNecklaceLock==1) ///Fairy
			return 1;

		// ExtraSlot If there is an item in
		if( g_pRoh->m_ExtraSlot )
			return 1;

		if( IsInside( x, y ) )
		{
			// When the Manastone Upgrade window is displayed
			if( g_pNk2DFrame
					&& g_pNk2DFrame->GetControlInven()
					&& g_pNk2DFrame->GetControlInven()->m_bAct_Masuk )
			{
				g_pNk2DFrame->GetControlInven()->Close_Masuk();
				return 1;
			}

			// area inspection
			if( g_pRoh
					&& ( x >= m_my_slotstart_x )
					&& ( x < m_my_slotstart_x + ( SLOT_WIDTH * EXG_SLOT_X_NUM ) )
					&& ( y >= m_my_slotstart_y )
					&& ( y < m_my_slotstart_y + ( SLOT_HEIGHT * EXG_SLOT_Y_NUM ) ) )
			{
				int nSlot_X_Num = 0;
				int nSlot_Y_Num = 0;

				//slot count
				nSlot_X_Num = ( x - m_my_slotstart_x ) / SLOT_WIDTH;
				nSlot_Y_Num = ( y - m_my_slotstart_y ) / SLOT_HEIGHT;

				int nItemIndex = g_pRoh->GetExgSlotIndex( nSlot_X_Num, nSlot_Y_Num );
				if( nItemIndex < 0 )
					return 1;

				int nVNum = 0;
				int nPlusNum = 0;
				int nSpecialNum = 0;

				CItem* pItem = g_pRoh->m_ExgInven;
				CItem* pTempItem = pItem;

				// Extract wnum of eyeite using index value
				while( pItem )
				{
					if( pItem->m_Index == nItemIndex )
					{
						int nWidth = GET_ITEM_WIDTH( pItem );
						int nHeight = GET_ITEM_HEIGHT( pItem );

						if( nWidth != 1 || nHeight != 1 )
							return 0;

						nVNum = pItem->m_Vnum;
						nPlusNum = pItem->m_PlusNum;
						nSpecialNum = pItem->m_Special;

						break;
					}
					pItem = pItem->m_Next;
				}

				// const variable declaration
				char szTmp[ 10 ] = {0,};
				char szStr[ 100 ] = {0,};

				// Delay time of 1.5 seconds during operation
                // minimum safety
				if( g_pNk2DFrame && g_pNk2DFrame->GetControlExchange() )
				{
					if( g_pNk2DFrame->GetControlExchange()->m_byExchLock )
					{
						DWORD curTime = timeGetTime();

						if (curTime > g_pNk2DFrame->GetControlExchange()->m_timeExchLock + 1500) // if seconds have passed
							g_pNk2DFrame->GetControlExchange()->m_byExchLock = 0;
						else
							return 1;
					}

					// When the delay is over, the flag is removed.
					g_pNk2DFrame->GetControlExchange()->SetExchLock( 1 );
				}

				// Extract the same item as the selected item
				while( pTempItem )
				{
					if( ( pTempItem->m_Vnum == nVNum )
							&& ( pTempItem->m_PlusNum == nPlusNum )
							&& ( pTempItem->m_Special == nSpecialNum ) )
					{
						sprintf( szTmp, "%d %d ", pTempItem->m_SlotX, pTempItem->m_SlotY );
						strcat( szStr, szTmp );
					}
					pTempItem = pTempItem->m_Next;
				}

				CheckUpgradeNeed(); // It does not only check, but also sets text.

				sprintf( commOutBuf, "exch_get %d %s\n", g_pRoh->m_CurPackNum, szStr );

				if( g_pTcpIp )
					g_pTcpIp->SendNetMessage( commOutBuf );

			}

		}

		break;

	case WM_LBUTTONDOWN:

		if( IsInside(x,y) ) // When you click inside the interface.
		{
			m_UpgradeBtn.MsgProc(hWnd, msg, wParam, lParam);
			m_BtnCancel.MsgProc( hWnd, msg, wParam, lParam );

			if (x >= m_my_slotstart_x && x < m_my_slotstart_x + (SLOT_WIDTH * EXG_SLOT_X_NUM)
					&& y >= m_my_slotstart_y && y < m_my_slotstart_y + (SLOT_HEIGHT * EXG_SLOT_Y_NUM) ) // When you click the slot part of the exchange window...
			{

				if( timeGetTime() - m_dwMouseClickTickTime < 300 )  // 0.3candle
					return 1;

				m_dwMouseClickTickTime = timeGetTime();
				int slot_x_num, slot_y_num;
				if( g_pNk2DFrame && g_pNk2DFrame->GetControlExchange() )
				{
					if( g_pNk2DFrame->GetControlExchange()->m_byExchLock )
					{
						DWORD curTime = timeGetTime();

						if (curTime > g_pNk2DFrame->GetControlExchange()->m_timeExchLock + 1500) // 1.5 if seconds have passed
							g_pNk2DFrame->GetControlExchange()->m_byExchLock = 0;
						else
							return 1;
					}
				}

				if( g_pRoh && g_pRoh->m_ExtraSlot )
				{
					if( GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot) % 2 )
						slot_x_num = (x - m_my_slotstart_x) / SLOT_WIDTH - (GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot)/2);
					else
						slot_x_num = (x - m_my_slotstart_x + (SLOT_WIDTH/2)) / SLOT_WIDTH - (GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot)/2);

					if( GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot) % 2 )
						slot_y_num = (y - m_my_slotstart_y) / SLOT_HEIGHT - (GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot)/2);
					else
						slot_y_num = (y - m_my_slotstart_y + (SLOT_HEIGHT/2)) / SLOT_HEIGHT - (GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot)/2);



					if( slot_x_num >= 0 && slot_x_num + GET_ITEM_WIDTH(g_pRoh->m_ExtraSlot) - 1 < EXG_SLOT_X_NUM
							&& slot_y_num >= 0 && slot_y_num + GET_ITEM_HEIGHT(g_pRoh->m_ExtraSlot) - 1 < EXG_SLOT_Y_NUM ) // If the clicked position is a valid position... (check only the range)
					{
						if( g_pRoh->AddToExgInven(slot_x_num,slot_y_num) )
						{
							sprintf( commOutBuf, "exch inven %d %d\n", slot_x_num, slot_y_num );
							if( g_pTcpIp )
								g_pTcpIp->SendNetMessage( commOutBuf );
						}
					}
				}
				else
				{
					// If you're in action, don't pick it up...

					if(    g_pRoh && g_pRoh->AddToExgInven( (x-m_my_slotstart_x)/SLOT_WIDTH, (y-m_my_slotstart_y)/SLOT_HEIGHT )     )
					{
						sprintf( commOutBuf, "exch inven %d %d\n", (x-m_my_slotstart_x) / SLOT_WIDTH, (y-m_my_slotstart_y) / SLOT_HEIGHT );
						if( g_pTcpIp )
							g_pTcpIp->SendNetMessage(commOutBuf);
					}
				}

				//When an item is moved, the item is inspected and the necessary conditions are output.
				if( m_iType == INTERFACE_TYPE_CONVERSION || m_iType == INTERFACE_TYPE_PROCESS )
				{
					if( m_iTextInfoIndex == 201 || m_iTextInfoIndex == 202 ) // In the result state, TextInfo is not changed until the inventory window is clicked. (If a new click is made, check the condition display again..)
					{
						m_iTextInfoIndex = 200; // If you click it, the default message... (If there is a conversion item, it will be changed to the condition immediately.)
					}
				}
				
				// In the case of conversion interface, TextInfo changes the surface itself.
                // Modify TextInfo by responding only when clicked.
				if( m_iTextInfoIndex != 201 && m_iTextInfoIndex != 202 ) // TextInfo is not changed even if there is an item in the result message state. (It is updated when clicking on the inventory window.)
				{
					m_iTextInfoIndex = 200;	// Initializes the part that outputs TextInfo to 0 during conversion. (0 is the default message.)
				}

				if( m_iType == INTERFACE_TYPE_320LV_WEAPON_CONVERSION )
				{
					if( m_iTextInfoIndex != 50 )
						m_iTextInfoIndex = 50;
				}
				CheckUpgradeNeed(); // It does not only check, but also sets text.

				if( m_iType == INTERFACE_TYPE_CONVERSION || m_iType == INTERFACE_TYPE_PROCESS
						|| m_iType == INTERFACE_TYPE_320LV_WEAPON_CONVERSION
				  )
				{
					UpdateTextInfo( m_iType, m_iTextInfoIndex ); // Update if changed.
				}



			}
		}
		else if( IsPlayArea(x,y) ) // When clicking on the ground outside the interface.
		{
			g_pNk2DFrame->ToggleUpgradeWindow( m_iType, TRUE ); // Toggle Use a function to force quit.
		}
		else
			return 0;

		return 1;

	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:

		if( IsInside(x,y) ) // When you click inside the interface.
		{
			m_UpgradeBtn.MsgProc(hWnd, msg, wParam, lParam);
			m_BtnCancel.MsgProc( hWnd, msg, wParam, lParam );


			if( g_pNk2DFrame && m_BtnCancel.GetState()==BTN_ACTION )
			{
				g_pNk2DFrame->ToggleUpgradeWindow( m_iType, TRUE ); // Force termination using the Toggle function.
                        m_BtnCancel.SetState(BTN_NORMAL); // Guess I have to release the button's status myself.
			}


			if( m_UpgradeBtn.GetState() == BTN_ACTION ) // When you click the upgrade button.
			{

				if( g_pRoh->m_ExtraSlot )
					return 1;


				if( CheckSlotPure() != -1 ) // If there were no inappropriate items in the exchange window...
				{
					if( !CheckBrokenItem() ) //When there is a risk of breakage, a warning window is displayed once more.
					{
						m_Result = 0; //reset.

						switch( m_iType ) // Packets are different depending on the type.
						{
						case INTERFACE_TYPE_UPARMOR_400:
							sprintf( commOutBuf, "npc_up armor_400\n" );
							break;
						case INTERFACE_TYPE_UPARMOR: // For the enhanced interface (armor).
							sprintf( commOutBuf, "npc_up armor\n" );
							break;
						case INTERFACE_TYPE_UPWEAPON: // For enhanced interface (weapon)
							sprintf( commOutBuf, "npc_up weapon\n" );
							break;
						case INTERFACE_TYPE_CONVERSION: //In the case of a conversion interface
							sprintf( commOutBuf, "npc_up conv\n" );
							break;
						case INTERFACE_TYPE_CONFUSION: // random redo
							sprintf( commOutBuf, "npc_up reload\n" );
							break;
						case INTERFACE_TYPE_PROCESS: // Processing
							sprintf( commOutBuf, "npc_up refine\n" );
							break;
						case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
							sprintf( commOutBuf, "npc_up conv\n" );
							break;
						case INTERFACE_TYPE_320LV_WEAPON_UPGRADE:
							sprintf( commOutBuf, "npc_up weapon\n" );
							break;
						case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
							sprintf( commOutBuf, "npc_up newup\n" );
							break;
						case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER:
							sprintf( commOutBuf, "npc_up godpower\n" );
							break;

						case INTERFACE_TYPE_ITEMMAKE:
							{
								sprintf( commOutBuf, "makes\n" );
							}
							break;

						case INTERFACE_TYPE_BUFFMAKE_MAKE:
							{
								sprintf( commOutBuf, "buffmake_make\n");
							}
							break;

						case INTERFACE_TYPE_BUFFMAKE_GATCHA:
							{
								sprintf( commOutBuf, "buffmake_gatcha\n");
							}
							break;

						case INTERFACE_TYPE_BUFFMAKE_MEDAL:
							{
								sprintf( commOutBuf, "buffmake_medal\n");
							}
							break;

						case INTERFACE_TYPE_ITEMSEPARATE:
							{
								sprintf( commOutBuf, "seperate\n" );
							}
							break;

						case INTERFACE_TYPE_GMAGICSTONE:
							{
								sprintf( commOutBuf, "ex_item ma_stone\n" );
							}
							break;


						}

						if( g_pTcpIp )
						{
							g_pTcpIp->SendNetMessage( commOutBuf );
							// send the packet (Do not wait for the end of the animation, send it immediately.)
                                                        m_bSlotLock = TRUE; // so that it cannot be removed after the replacement has started....
						}
					}


				}

				m_UpgradeBtn.SetState(BTN_NORMAL);	// I guess I have to release the button's status myself.
			}

			return 1;
		}

	case WM_RBUTTONDOWN:

		x = LOWORD( lParam );
		y = HIWORD( lParam );
		if( IsInside(x,y) )
			return 1;
		break;
	}
	return 0;
}

HRESULT CControlUpgrade::BrokenPopupMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, int nMsg)
{
	char commOutBuf[512];

	if(nMsg == 1)
	{
		/// send the packet (Do not wait for the end of the animation, send it immediately.)
                    m_bSlotLock = TRUE; // so that it cannot be removed after the replacement has started....
                    m_Result = 0; // reset.


		// Packets are different depending on the type.
		switch(m_iType )
		{
		case INTERFACE_TYPE_UPARMOR_400:
			sprintf( commOutBuf, "npc_up armor_400\n" );
			break;
		case INTERFACE_TYPE_UPARMOR: // In case of enhanced interface (armor).
                        sprintf( commOutBuf, "npc_up armor\n" );
                        break;
                case INTERFACE_TYPE_UPWEAPON: // In case of enhanced interface (weapon)
                        sprintf( commOutBuf, "npc_up weapon\n" );
                        break;
                case INTERFACE_TYPE_CONVERSION : // In case of conversion interface
                        sprintf( commOutBuf, "npc_up conv\n" );
                        break;
                case INTERFACE_TYPE_CONFUSION : // Random rerun
                        sprintf( commOutBuf, "npc_up reload\n" );
                        break;
                case INTERFACE_TYPE_PROCESS: // processing
                        sprintf( commOutBuf, "npc_up refine\n" );
                        break;
                case INTERFACE_TYPE_320LV_WEAPON_CONVERSION: // processing
                        sprintf( commOutBuf, "npc_up conv\n" );
                        break;
                case INTERFACE_TYPE_320LV_WEAPON_UPGRADE: // machining
                        sprintf( commOutBuf, "npc_up weapon\n" );
                        break;
                case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
                        sprintf( commOutBuf, "npc_up newup\n" );
                        break;
                case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER: // machining
			sprintf( commOutBuf, "npc_up godpower\n" );
			break;
		case INTERFACE_TYPE_ITEMMAKE:
			sprintf( commOutBuf, "makes\n" );
			break;
		case INTERFACE_TYPE_BUFFMAKE_MAKE:
			sprintf( commOutBuf, "buffmake_make\n");			
			break;
		case INTERFACE_TYPE_BUFFMAKE_GATCHA:
			sprintf( commOutBuf, "buffmake_gatcha\n");			
			break;
		case INTERFACE_TYPE_BUFFMAKE_MEDAL:
			sprintf( commOutBuf, "buffmake_medal\n");
			break;
		case INTERFACE_TYPE_ITEMSEPARATE:
			sprintf( commOutBuf, "seperate\n" );
			break;
		case INTERFACE_TYPE_GMAGICSTONE:
			sprintf( commOutBuf, "ex_item ma_stone\n" );
			break;
		}
		if( g_pTcpIp )
			g_pTcpIp->SendNetMessage( commOutBuf );
	}

	return 0L;
}

BOOL CControlUpgrade::IsInside( int x, int y )
{
	if( !m_pBackSur )
		return FALSE;

	if (x >= m_pBackSur->Xpos
			&& x < m_pBackSur->Xpos + m_pBackSur->GetWidth()
			&& y >= m_pBackSur->Ypos
			&& y < m_pBackSur->Ypos + m_pBackSur->GetHeight() )
		return TRUE;

	return FALSE;
}

BOOL CControlUpgrade::IsPlayArea( int x, int y ) //Check to see if you clicked the ground outside the interface.
{
	if( !IsInside(x,y) && !g_pNk2DFrame->IsInPopup(x,y) )
		return true;

	return false;
}

int CControlUpgrade::CheckUpgradeNeed() // Obtain the gems (type, number) and limes required for enhancement. Returns a pointer to the measured item. (If multiple items are mounted at the same time, NULL is returned. Only returns properly when only one piece of armor is mounted. If NULL, the user uploaded it incorrectly.)
{
	CItem* pItem = NULL;
	CItem* pCheckItem = NULL;
	CItem* pCheckItem2 = NULL;
	//	CItem* pGem = NULL; // Pointer of the jewels that are included. (If there are multiple jewels, only the last one is checked.)
	int ItemCount = 0, ItemCount2 = 0; //For checking the number of items included. Just check the armor.
	int i = 0;
	int special_level = -1, special_level1 = -1, special_level2 = -1, special_level3 = -1, special_level4 = -1;
	char strTemp[256];
	char strTemp2[30];

	ZeroMemory( m_need_gem , sizeof(m_need_gem));

	// Initialization of prerequisites. (Note that this function must be done first.)
	for( i = 0 ; i < MAX_NEED_ITEM_IN_NPC_UPGRADE ; ++i )
	{
		NeedItem[i] = 0; // The number required for each item type when strengthening or processing.
	}
	NeedLaim = 0; //Need lime.


	if( g_pRoh )
		pItem = g_pRoh->m_ExgInven;

	if(!pItem)
	{
		m_NeedGemText.SetString( "", TRUE ); // If you do this, you won't see it.
                m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
		m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );
		m_NeedMoneyText2.SetString( "", TRUE );

		return -1;
	}

	// Initialize the text as well
      //-- IDS_NEED_GEM : Required Gems:
        sprintf( strTemp, (char*)G_STRING(IDS_NEED_GEM) );
        m_NeedGemText.SetString( strTemp, FALSE ); // If the first argument is a string, the second argument is the font color.
        sprintf( strTemp, "(%d)", NeedItem[UPGRADE_NEED_REGENT_DIA] );
        m_NeedGemNumText.SetString( strTemp, FALSE ); // If the first argument is a string, the second argument is the font color.
        m_NeedMoneyText1.SetString( "", TRUE ); // If the first argument is a number, the second argument is a comma.
        m_NeedMoneyText2.SetString( "", TRUE ); // Initialize it so that it is invisible. (You can think of it as output in black.)

	memset(&m_iNeedGemVnum, 0, sizeof(m_iNeedGemVnum));
	memset(&m_iNeedGemCount, 0, sizeof(m_iNeedGemCount));

	switch(m_iType )
	{
	case INTERFACE_TYPE_UPARMOR_400:
		{
			DefenseValMgr* defValMgr = DefenseValMgr::GetInstance();
			if( defValMgr == NULL )
				break;

			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_ARMOR || GET_TYPE(pItem)==ITYPE_WEAPON )
				{
					if( defValMgr->IsExist(pItem->m_Vnum) )
					{
						++ItemCount;
						pCheckItem = pItem;
					}
				}

				pItem = pItem->m_Next;
			}

			m_NeedGemText.SetString( "", TRUE );
			m_NeedGemNumText.SetString( "", TRUE );

			if( ItemCount != 1 )
			{
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}

			if( pCheckItem )
			{
				int vnum = pCheckItem->m_Vnum;
				int itemPlus = pCheckItem->m_PlusNum + 1;
				NeedLaim = defValMgr->GetLaim(vnum, itemPlus);
				sprintf( strTemp2, "%d", NeedLaim );
				sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) );
				m_NeedMoneyText1.SetString("", TRUE);
				m_NeedMoneyText2.SetString(NeedLaim, TRUE);

				m_NeedGemText.SetString( (char*)G_STRING(IDS_NPCUPGRADE_NEEDGEM), FALSE );

				for(int i = 0; i < NEEDGEM_MAX; i++)
				{
					m_iNeedGemVnum[i] = defValMgr->GetGemVnum(vnum, itemPlus, i);
					m_iNeedGemCount[i] = defValMgr->GetGemCount(vnum, itemPlus, i);
				}

				return 1;
			}
			else
			{
				return -1;
			}
		}
		break;
	case INTERFACE_TYPE_UPARMOR:
	case INTERFACE_TYPE_UPWEAPON:
		{
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_ARMOR || GET_TYPE(pItem)==ITYPE_WEAPON ) // Weapons have also been added. Only weapons and armor are checked.
				{

					++ItemCount;
					pCheckItem = pItem; // Remember the pointer. If there is normally only one item, this pointer is used to get what you need.

					// When strengthening an item, a warning message is displayed under conditions that can be destroyed.
					if( g_SvrType == ST_ADULT_ONLY )
					{
						if( GET_TYPE(pItem) == ITYPE_WEAPON ) // && ChechIs261LvWeponItem(pItem) )
						{
							if( ChechIs261LvWeponItem(pItem)
									|| ( pItem->m_Vnum >= 1313 && pItem->m_Vnum <= 1317 || pItem->m_Vnum == 1324)
									&& !( pItem->m_MinLevel_Org >= 300 || IS_2011_ITEM_WEAPON(pItem->m_Vnum) )
							  )
							{
								switch( pItem->m_PlusNum)
								{
								case 3:
								case 7:
								case 11:
									g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG_01), TYPE_NOR_OK);
									break;
								}
							}
						}
						else
						{
							if( GET_TYPE(pItem) == ITYPE_ARMOR )
							{
								switch( pItem->m_PlusNum)
								{
								case 3:
								case 8:
									{
										g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG_01), TYPE_NOR_OK);
										break;
									}
								case 11:
									{
										if( pItem->m_MinLevel_Org > 300 || IS_2011_ITEM_ARMOR(pItem->m_Vnum) )
										{
											g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG_01), TYPE_NOR_OK);
											break;
										}
									}
								}
							}
						}
					}
				}
				pItem = pItem->m_Next;
			}

			if( ItemCount != 1 ) // It is an error if there is not one weapon or armor.
			{
				m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}
			else if( GET_TYPE(pCheckItem)==ITYPE_WEAPON && ChechIs261LvWeponItem(pCheckItem) && pCheckItem->m_PlusNum >= 10 )
			{
				m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                //-- IDS_NPCUPGRADE_MAXLEVEL : This item can no longer be enhanced.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), FALSE );

				return -1;
			}
			else if( (GET_TYPE(pCheckItem)==ITYPE_ARMOR && pCheckItem->m_PlusNum >= 15) ||  // Armor can only be upgraded up to level 15. (05-11-10 Changed from Step 20 to Step 15)
					 (GET_TYPE(pCheckItem)==ITYPE_ARMOR && pCheckItem->m_MinLevel_Org >= 360 && pCheckItem->m_PlusNum >= 10) )
			{
				m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                //-- IDS_NPCUPGRADE_MAXLEVEL : This item can no longer be enhanced.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), TYPE_NOR_OK, 1 );
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), FALSE );

				return -1;
			}
			else // If you normally have only one weapon or armor, you can get the lime you need.
			{
				if( pCheckItem ) // you should check this
				{
					if( !CheckCanNpcUpgradeItem(pCheckItem) ) // Only certain items can be enhanced.
					{
						m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                        m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                        //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

						return -1;
					}

					// [2008/7/7 Theodoric]  Rare armor enhancement added
					if( (pCheckItem->m_Vnum >= 1560 && pCheckItem->m_Vnum <= 1579) || (pCheckItem->m_Vnum >= 1797 && pCheckItem->m_Vnum <= 1816)

							|| ( TRUE == IS_2011_ITEM_ARMOR( pCheckItem->m_Vnum ) )

					  )
					{
						++NeedItem[UPGRADE_NEED_REGENT_DIA]; // 300·The level is always one Regent.
					}
					else if( (pCheckItem->m_Vnum >= 1190 && pCheckItem->m_Vnum <= 1229) || (pCheckItem->m_Vnum >= 1313 && pCheckItem->m_Vnum <= 1326) ) // ±âÁ¸ ¹«±â,¹æ¾î±¸
					{
						if( (pCheckItem->m_PlusNum+1) > NPC_UPGRADE_NEED_REGENT_LEVEL ) // +9Abnormal reinforcement is regent...
						{
							++NeedItem[UPGRADE_NEED_REGENT_DIA];
						}
						else
						{
							++NeedItem[UPGRADE_NEED_DIA];
						}
					}
					else if( ( pCheckItem->m_Vnum >= 2580 && pCheckItem->m_Vnum <= 2583 )
							 || ( pCheckItem->m_Vnum >= 2604 && pCheckItem->m_Vnum <= 2607 )
							 || ( pCheckItem->m_Vnum >= 2612 && pCheckItem->m_Vnum <= 2615 )
							 || ( pCheckItem->m_Vnum >= 2620 && pCheckItem->m_Vnum <= 2623 )
							 || ( pCheckItem->m_Vnum >= 2628 && pCheckItem->m_Vnum <= 2631 )
						   )
					{
						if( (pCheckItem->m_PlusNum+1) > NPC_UPGRADE_NEED_REGENT_LEVEL ) // +9ÀÌ»ó °­È­´Â ¸®Á¨Æ®·Î...
						{
							++NeedItem[UPGRADE_NEED_REGENT_DIA];
						}
						else
						{
							++NeedItem[UPGRADE_NEED_DIA];
						}
					}
					else if( ( pCheckItem->m_Vnum >= 2636 && pCheckItem->m_Vnum <= 2639 )
							 || ( pCheckItem->m_Vnum >= 2644 && pCheckItem->m_Vnum <= 2647 )
							 || ( pCheckItem->m_Vnum >= 2652 && pCheckItem->m_Vnum <= 2655 )
							 || ( pCheckItem->m_Vnum >= 2660 && pCheckItem->m_Vnum <= 2663 )
							 || ( pCheckItem->m_Vnum >= 2668 && pCheckItem->m_Vnum <= 2671 )
						   )
					{
						if( (pCheckItem->m_PlusNum+1) > NPC_UPGRADE_NEED_REGENT_LEVEL ) // +9Abnormal reinforcement is regent...
						{
							++NeedItem[UPGRADE_NEED_REGENT_DIA];
						}
						else
						{
							++NeedItem[UPGRADE_NEED_DIA];
						}
					}

					//-- IDS_NPCUPGRADE_NEEDGEM : Required Gems:
                    m_NeedGemText.SetString( (char*)G_STRING(IDS_NPCUPGRADE_NEEDGEM), FALSE ); // The reason I'm doing this here is to fix it with a different message if the gem type is wrong...
                    sprintf( strTemp, "(%d)", 1 ); // First of all, the number is unconditionally required, so set it to 1 forcibly. If the number changes later, please fix it.
                    m_NeedGemNumText.SetString( strTemp, FALSE ); // The reason for doing this here is to fix it with a different message if the gem type is wrong.

					if ( pCheckItem->m_Vnum >= 1797 && pCheckItem->m_Vnum <= 1816 ) // [2008/7/7 Theodoric]  ·¹¾î ¹æ¾î±¸ °­È­ Ãß°¡
						NeedLaim = 450000 * (pCheckItem->m_PlusNum+1);
					else if( pCheckItem->m_Vnum >= 1560 && pCheckItem->m_Vnum <= 1579
							 || ( TRUE == IS_2011_ITEM_ARMOR( pCheckItem->m_Vnum ) )
						   ) // 300 Added new level armor.
					{
						if ( GET_TYPE(pCheckItem)==ITYPE_ARMOR )
						{
							NeedLaim = 450000 * (pCheckItem->m_PlusNum+1);
						}
						else
						{
							int	newMoney[15] = { 4500000,5000000,5500000,6000000,6500000,7000000,7500000,8000000,8500000,9000000,9500000,10000000,10500000,11000000,12000000 };
							NeedLaim = newMoney[pCheckItem->m_PlusNum];
						}
					}
					else if( (pCheckItem->m_Vnum >= 1190 && pCheckItem->m_Vnum <= 1229 ) || (pCheckItem->m_Vnum >= 1313 && pCheckItem->m_Vnum <= 1326) ) // ±âÁ¸ ¹«±â,¹æ¾î±¸
						NeedLaim = 300000 * (pCheckItem->m_PlusNum+1);
					else if( ( pCheckItem->m_Vnum >= 2580 && pCheckItem->m_Vnum <= 2583 )
							 || ( pCheckItem->m_Vnum >= 2604 && pCheckItem->m_Vnum <= 2607 )
							 || ( pCheckItem->m_Vnum >= 2612 && pCheckItem->m_Vnum <= 2615 )
							 || ( pCheckItem->m_Vnum >= 2620 && pCheckItem->m_Vnum <= 2623 )
							 || ( pCheckItem->m_Vnum >= 2628 && pCheckItem->m_Vnum <= 2631 )
						   )
						NeedLaim = 300000 * (pCheckItem->m_PlusNum+1);
					else if( ( pCheckItem->m_Vnum >= 2636 && pCheckItem->m_Vnum <= 2639 )
							 || ( pCheckItem->m_Vnum >= 2644 && pCheckItem->m_Vnum <= 2647 )
							 || ( pCheckItem->m_Vnum >= 2652 && pCheckItem->m_Vnum <= 2655 )
							 || ( pCheckItem->m_Vnum >= 2660 && pCheckItem->m_Vnum <= 2663 )
							 || ( pCheckItem->m_Vnum >= 2668 && pCheckItem->m_Vnum <= 2671 )
						   )
						NeedLaim = 300000 * (pCheckItem->m_PlusNum+1);

					// Changes whether to strengthen weapons or armors internally in the client.
					if( GET_TYPE(pCheckItem)==ITYPE_ARMOR )
					{
						m_iType = INTERFACE_TYPE_UPARMOR;
					}
					else // Even if it is neither armor nor weapon, it is just treated as a weapon.
					{
						m_iType = INTERFACE_TYPE_UPWEAPON;
					}
				}
			}
		}
		break;

	case INTERFACE_TYPE_CONVERSION:
		{
			// The conversion interface displays the necessary conditions by changing the image for each condition...
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON || GET_TYPE(pItem)==ITYPE_ARMOR ) // Check weapons or armor.
				{
					++ItemCount;
					pCheckItem = pItem; // Remember the pointer. If there is normally only one item, this pointer is used to get what you need.
				}

				// No gem check.

				pItem = pItem->m_Next;
			}

			if( ItemCount != 1 ) // It is an error if there is not one weapon or armor.
			{
				m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}
			else if( pCheckItem->m_PlusNum != 15 ) // Currently, only +15 items can be converted.
			{
				m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}
			else // If you normally have only one weapon or armor, you can get the lime you need.
			{
				if( pCheckItem ) // you should check this
				{
					if( !CheckCanConversionItem(pCheckItem) ) // Only certain items can be enhanced.
                    {
                        m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                        m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                        //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

						return -1;
					}

					// If the message file to be drawn has changed, reload it.
					if( m_iTextInfoIndex != 201 && m_iTextInfoIndex != 202 ) // TextInfo is not changed even if there is an item in the result message state. (It is updated when clicking on the inventory window.)
					{
						if( GET_ITEM_LEVEL(pCheckItem) == 195 || GET_ITEM_LEVEL(pCheckItem) == 200 ) // 3 types of requirements. (Equivalent weapons for hybrids are limited to level 200, so check them additionally)
						{
							m_iTextInfoIndex = 203; // 0,1,are the comments...
						}
						else if( GET_ITEM_LEVEL(pCheckItem) == 210 ) // 4 types of requirements.
						{
							m_iTextInfoIndex = 204; // 0,1,2are the comments...
						}
						else if( GET_ITEM_LEVEL(pCheckItem) == 261 ) // 5 types of requirements.
						{
							m_iTextInfoIndex = 205; // 0,1,2are the comments...
						}
					}

					NeedLaim = 10000000; // Currently fixed at 10 million.
				}
			}
		}
		break;

	case INTERFACE_TYPE_CONFUSION:
		{
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON || GET_TYPE(pItem)==ITYPE_ARMOR ) // Check weapons or armor.
				{
					++ItemCount;
					pCheckItem = pItem; // Remember the pointer. If there is normally only one item, this pointer is used to get what you need.
				}

				//No gem check.

				pItem = pItem->m_Next;
			}

			if( ItemCount != 1 ) // If there is not 1 weapon or armor, an error.
             {
                 m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                 m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                 //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}
			else //If you normally have only one weapon or armor, you can get the lime you need.
			{
				if( pCheckItem ) //you should check this
				{
					if( !CheckIsConversionItem(pCheckItem,0,2) ) // Only certain items (converted ones only) can be strengthened.
					{
						m_NeedGemText.SetString( "", TRUE ); // This will make it invisible.
                        m_NeedGemNumText.SetString( "", TRUE ); // This will make it invisible.
                        //-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

						return -1;
					}

					NeedItem[UPGRADE_NEED_DIA] = (pCheckItem->m_PlusNum-1) / 4;

					if( NeedItem[UPGRADE_NEED_DIA] < 0 )
						NeedItem[UPGRADE_NEED_DIA] = 0;

					//-- IDS_NPCUPGRADE_NEEDGEM : ÇÊ¿ä º¸¼®:
					m_NeedGemText.SetString( (char*)G_STRING(IDS_NPCUPGRADE_NEEDGEM), FALSE ); // The reason I'm doing this here is to fix it with a different message if the type of gem is wrong...
					sprintf( strTemp, "(%d)", NeedItem[UPGRADE_NEED_DIA] ); // Is currently
					m_NeedGemNumText.SetString( strTemp, FALSE ); // The reason I'm doing this here is to fix it with a different message if the type of gem is wrong...

					int TempInt = pCheckItem->m_PlusNum-5;
					if( TempInt < 0 )
						TempInt = 0;

					NeedLaim = 2000000 + ( TempInt*400000 );
				}
			}
		}
		break;


	case INTERFACE_TYPE_PROCESS:
		{
			while (pItem)
			{
				if( GET_TYPE(pItem) == ITYPE_ARMOR ) // Armor check.
				{
					++ItemCount;
					pCheckItem = pItem;
				}

				// No gem check.
				pItem = pItem->m_Next;
			}

			if( ItemCount != 1 ) //It's an error if it's not armor.
			{
				m_NeedGemText.SetString( "", TRUE );
				m_NeedGemNumText.SetString( "", TRUE );
				//-- IDS_NPCUPGRADE_PLZUPITEM : ÇØ´ç ¾ÆÀÌÅÛÀ» ¿Ã·ÁÁÖ¼¼¿ä.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}
			else
			{
				if( pCheckItem )
				{
					if( !(GET_ATT_FLAG(pCheckItem) & IATT_REFINE) )
					{
						m_NeedGemText.SetString( "", TRUE );
						m_NeedGemNumText.SetString( "", TRUE );
						//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

						return -1;
					}
				}
			}

			pItem = g_pRoh->m_ExgInven;

			while (pItem)
			{
				if( GET_TYPE(pItem) == ITYPE_ETC && GET_SHAPE(pItem) == IETC_ETC) //Refinery check.
				{
					pCheckItem2 = pItem; // Remember the pointer. If there is normally only one item, this pointer is used to get what you need.
					++ItemCount2;
				}
				pItem = pItem->m_Next;
			}

			if( ItemCount2 == 0 ) // If it's not a refinement book, it's an error.
			{
				m_NeedGemText.SetString( "", TRUE );
				m_NeedGemNumText.SetString( "", TRUE );
				//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
				m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );

				return -1;
			}

			if( pCheckItem2 )
			{
				if( pCheckItem2->m_Vnum == 1362 )
					special_level = (pCheckItem->m_Special2 & 0x1C00) >> 10;
				else if( pCheckItem2->m_Vnum == 1363 )
					special_level1 = (pCheckItem->m_Special2 & 0xE000) >> 13;
				else if( pCheckItem2->m_Vnum == 1364 )
					special_level2 = (pCheckItem->m_Special2 & 0x70000) >> 16;
				else if( pCheckItem2->m_Vnum == 1365 )
					special_level3 = (pCheckItem->m_Special2 & 0x380000) >> 19;
				else if( pCheckItem2->m_Vnum == 1366 )
					special_level4 = (pCheckItem->m_Special2 & 0x1C00000) >> 22;
			}

			if( m_iTextInfoIndex != 201 && m_iTextInfoIndex != 202 ) // TextInfo is not changed even if there is an item in the result message state. (It is updated when clicking on the inventory window.)
			{
				if( special_level == 0 || special_level1 == 0 || special_level2 == 0 || special_level3 == 0 || special_level4 == 0)
				{
					m_iTextInfoIndex = 203;
					NeedLaim = 261000;
				}
				else if( special_level == 1 || special_level1 == 1 || special_level2 == 1 || special_level3 == 1 || special_level4 == 1 )
				{
					m_iTextInfoIndex = 204;
					NeedLaim = 522000;
				}
				else if( special_level == 2 || special_level1 == 2 || special_level2 == 2 || special_level3 == 2 || special_level4 == 2 )
				{
					m_iTextInfoIndex = 205;
					NeedLaim = 783000;
				}
				else if( special_level == 3 || special_level1 == 3 || special_level2 == 3 || special_level3 == 3 || special_level4 == 3 )
				{
					m_iTextInfoIndex = 206;
					NeedLaim = 1044000;
				}
				else if( special_level == 4 || special_level1 == 4 || special_level2 == 4 || special_level3 == 4 || special_level4 == 4 )
				{
					m_iTextInfoIndex = 207;
					NeedLaim = 1305000;
				}
				else if( special_level >= 5 || special_level1 >= 5 || special_level2 >= 5 || special_level3 >= 5 || special_level4 >= 5 )
				{
					//-- IDS_MAX_OPTION : Current options are maximum.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_MAX_OPTION), TYPE_NOR_OK, 1 );
					return -1;
				}
				/*else if( special_level == 5 || special_level1 == 5 || special_level2 == 5 || special_level3 == 5 || special_level4 == 5 )
				{
					m_iTextInfoIndex = 207;
					NeedLaim = 1500000;
				}
				else if( special_level == 6 || special_level1 == 6 || special_level2 == 6 || special_level3 == 6 || special_level4 == 6 )
				{
					//-- IDS_MAX_OPTION : Current options are maximum.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_MAX_OPTION), TYPE_NOR_OK, 1 );
					return -1;
				}*/
			}
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
		{
			CItem * targetItem = NULL;
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON )
				{
					if( !ChechIs261LvWeponItem(pItem) || pItem->m_PlusNum < 10 || pItem->m_LimitTime )
					{
						//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );
						return -1;
					}
					if( targetItem == NULL )
						targetItem = pItem;
				}
				else if (GET_TYPE(pItem)==ITYPE_ARMOR )
				{
					//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
					m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );
					return -1;
				}
				pItem = pItem->m_Next;
			}

			NeedItem[UPGRADE_NEED_DIA] = 3;
			NeedItem[UPGRADE_NEED_REGENT_DIA] = 8;

			// Japan wrongly requested rhyme
			if( g_dwClientCountry == CTRY_JPN ) NeedLaim = 50000000;
			else								NeedLaim = 10000000;

			if( targetItem != NULL)
			{
				if( targetItem->m_PlusNum > 10)
					m_iTextInfoIndex = 54;
				else
					m_iTextInfoIndex = 53;
			}

			targetItem = NULL;
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_UPGRADE:
		{
			int GMRDia = 0;

			pCheckItem = NULL;
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON )
				{
					pCheckItem = pItem;

					if( !( (pItem->m_Vnum >= 2049 && pItem->m_Vnum <= 2055) || (pItem->m_Vnum >= 2743 && pItem->m_Vnum <= 2749 ) // System Upgrade 360 Weapon = 02
							//|| (pItem->m_Vnum >= 2939 && pItem->m_Vnum <= 2941)
							|| ( TRUE == IS_2011_ITEM_WEAPON( pItem->m_Vnum ) )
						 )
							|| pItem->m_LimitTime )
					{
						ResetText();
						return -1;
					}

					if( pItem->m_PlusNum >= 15)
					{
						ResetText();
						return -1;
					}
				}
				else if( pItem->m_Vnum == 1967) // great magic regent diamond
				{
					GMRDia ++;
				}
				else if( pItem->m_Vnum == 1068 )
				{		}
				else
				{
					ResetText();
					return -1;
				}

				pItem = pItem->m_Next;
			}

			if( pCheckItem )
			{
				if( GMRDia == 0)
				{
					// Print a warning message.
					if( g_SvrType == ST_ADULT_ONLY)
					{
						if( GET_TYPE(pCheckItem) == ITYPE_WEAPON )
						{
							switch( pCheckItem->m_PlusNum )
							{
							case 3:
							case 7:
							case 11:
								g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG_01), TYPE_NOR_OK);
								break;
								break;
							}
						}
					}
					else
					{
						switch( pCheckItem->m_PlusNum )
						{
						case 3:
						case 7:
							// IDS_UPGRADE_MSG1 It may be initialized when the upgrade fails.
							g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG1), TYPE_NOR_OK, 1 );
							break;
						case 11:
							// IDS_UPGRADE_MSG2 It may be downgraded if the upgrade fails.
							g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG2), TYPE_NOR_OK, 1 );
							break;
						}
					}
				}

				// Japan wrongly requested rhyme
				if( g_dwClientCountry == CTRY_JPN )
				{
					switch( pCheckItem->m_PlusNum )
					{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
						NeedLaim = 3000000 + ( pCheckItem->m_PlusNum * 500000 );
						break;
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
						NeedLaim = 6000000 + ( (pCheckItem->m_PlusNum-5) * 1000000 );
						break;
					default:
						NeedLaim = 12000000 + ( (pCheckItem->m_PlusNum-10) * 2000000 );
						break;
					}
				}
				else
				{
					// No problem, let's count the rhyme.
					switch( pCheckItem->m_PlusNum )
					{
					case 13:
					case 14:
						if ( TRUE == IS_2011_ITEM_WEAPON( pCheckItem->m_Vnum ) )
						{
							int	newMoney[15] = { 4500000,5000000,5500000,6000000,6500000,7000000,7500000,8000000,8500000,9000000,9500000,10000000,10500000,11000000,12000000 };
							NeedLaim = newMoney[pCheckItem->m_PlusNum];
						}
						else
						{
							NeedLaim = 11000000 + ((pCheckItem->m_PlusNum-12)*1000000);
						}
						break;
					default:
						NeedLaim = 4500000 + (pCheckItem->m_PlusNum * 500000);
						break;
					}
				}


				NeedItem[UPGRADE_NEED_REGENT_DIA]++;
				m_NeedGemText.SetString( (char*)G_STRING(IDS_NPCUPGRADE_NEEDGEM), FALSE );
				sprintf( strTemp, "(%d)", 1 );
				m_NeedGemNumText.SetString( strTemp, FALSE );
			}
		}
		break;

	case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
		{
			if( GET_TYPE(pItem) == ITYPE_WEAPON )
			{
				if( CheckIsGodItem(pItem) && pItem->m_PlusNum >= 10 )
				{
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), TYPE_NOR_OK, 1 );
					m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), FALSE );
					return -1;
				}
			}

			if( NULL == LHIS_ENCHANT() )
			{
				NeedLaim = 0;
				return -1;
			}

			CItem * current_item = NULL;
			int upgrade_type = LHIS_ENCHANT()->CheckUpgrade_Weapon(&current_item);
			if( upgrade_type == -1 || !current_item )
			{
				NeedLaim = 0;
				return -1;
			}

			if( NULL == LHIS_ENCHANT_EX() )
			{
				NeedLaim = 0;
				return -1;
			}

			const LHParam_EnchantItem_Ex::sData * param_ex = LHIS_ENCHANT_EX()->GetParam(upgrade_type, current_item->m_PlusNum);

			if( !param_ex )
				return -1;

			CopyMemory( m_need_gem, param_ex->m_gem, sizeof(m_need_gem) );
			NeedLaim = LHIS_ENCHANT_EX()->GetNeedLaim(upgrade_type,current_item->m_PlusNum);
			m_NeedMoneyText2.SetString( NeedLaim, TRUE );
			return -1;
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER:
		{
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON )
				{
					if( !(pItem->m_Vnum >= 2049 && pItem->m_Vnum <= 2055)
							//|| ( pItem->m_Vnum >= 2939 && pItem->m_Vnum <= 2941 )
							|| ( TRUE == IS_2011_ITEM_WEAPON( pItem->m_Vnum ) )
							|| pItem->m_LimitTime
							|| pItem->m_PlusNum != 15 )
					{
						//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );
						return -1;
					}
				}
				else if( GET_TYPE(pItem)==ITYPE_ARMOR )
				{
					if( !(pItem->m_Vnum >= 2942 && pItem->m_Vnum <= 2953)
							|| pItem->m_LimitTime
							|| pItem->m_PlusNum != 15 )
					{
						//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );
						return -1;
					}
				}

				pItem = pItem->m_Next;
			}

			NeedItem[UPGRADE_NEED_REGENT_DIA]++;
			//		m_NeedGemText.SetString("Dragon Seal 1set");
                        // Japan requested the wrong rhyme
			NeedLaim = NeedLaim_GodPower;
		}
		break;


	case INTERFACE_TYPE_ITEMMAKE:		
		{
			if ( false == LHIS_MAKE()->CheckNeedItem( NeedLaim ) ) // Display Lime on NPC
			{
				sprintf( strTemp2, "%d", NeedLaim );
				sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) );
				m_NeedMoneyText1.SetString( strTemp, FALSE );
				m_NeedMoneyText2.SetString( NeedLaim, TRUE );
				m_NeedMoneyText3.SetString("Você Precisa do item ",TRUE);
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_BUFFMAKE_MAKE:
		{
			if ( false == LHIS_MAKE()->CheckNeedItemBuff( NeedLaim ) )
			{
				sprintf( strTemp2, "%d", NeedLaim );
				sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) );
				m_NeedMoneyText1.SetString( strTemp, FALSE );
				m_NeedMoneyText2.SetString( NeedLaim, TRUE );
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_BUFFMAKE_GATCHA:
		{
			NeedLaim = 1000000;
			sprintf( strTemp2, "%d", NeedLaim );
			sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) );
			m_NeedMoneyText1.SetString( strTemp, FALSE );
			m_NeedMoneyText2.SetString( NeedLaim, TRUE );
		}
		break;	

	case INTERFACE_TYPE_ITEMSEPARATE:
		{
			if ( false == LHIS_SEPERATE()->CheckNeedItem() )
			{
				sprintf( strTemp2, "%d", NeedLaim );
				sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) ); // Temporarily use a buffer of m_NeedMoneyText1 to insert a comma.
				m_NeedMoneyText1.SetString( strTemp, FALSE );
				m_NeedMoneyText2.SetString( NeedLaim, TRUE );
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_GMAGICSTONE:
		{
			if ( false == LHIS_GMAGICSTONE()->CheckNeedItem( NeedLaim ) )
			{
				sprintf( strTemp2, "%d", NeedLaim );
				sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) ); // Temporarily use a buffer of m_NeedMoneyText1 to insert a comma.
				m_NeedMoneyText1.SetString( strTemp, FALSE );
				m_NeedMoneyText2.SetString( NeedLaim, TRUE );
				return -1;
			}
		}
		break;


	}

	sprintf( strTemp2, "%d", NeedLaim );
	sprintf( strTemp, (char*)G_STRING(IDS_NPCUPGRADE_NEEDLAIM), m_NeedMoneyText1.TransComma(strTemp2) ); // Temporarily use a buffer of m_NeedMoneyText1 to insert a comma.
	m_NeedMoneyText1.SetString( strTemp, FALSE );
	m_NeedMoneyText2.SetString( NeedLaim, TRUE );

	return 1;
}

int CControlUpgrade::CheckSlotPure() //Check to see if the upgrade attempt contains inappropriate items.
{
	CItem* pItem = NULL;
	CItem* pCheckItem = NULL;
	int GemCount = 0; // number of gems
	int special_level = -1, special_level1 = -1, special_level2 = -1, special_level3 = -1, special_level4 = -1;

	if( !g_pRoh )
		return -1;

	pItem = g_pRoh->m_ExgInven;

	if(!pItem)
		return -1;

	if( NeedLaim > g_pRoh->m_Money )
	{
		g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_INTERCHA_NOT_ENOUGH_MONEY), TYPE_NOR_OK, 1 );
		return -1;
	}

	switch( m_iType )
	{
	case INTERFACE_TYPE_UPARMOR_400:
		break;
	case INTERFACE_TYPE_UPARMOR:
	case INTERFACE_TYPE_UPWEAPON: //For hardened interfaces. Check them one by one and make the necessary phrases.
		{
			while (pItem)
			{
				if( GET_TYPE(pItem)!=ITYPE_ARMOR && GET_TYPE(pItem)!=ITYPE_WEAPON
						&& ( pItem->m_Vnum != 228 && pItem->m_Vnum != 1966 )
						&& ( pItem->m_Vnum != 1068 && pItem->m_Vnum != 1967 )
				  ) // Currently, it does not support anything other than armor, weapon, diamond, and regent diamond.
				{
					// If the wrong item is included...
					//-- IDS_QUEST_ITEM_NOT_MATCH : Different types of items are mixed.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				else if( GET_TYPE(pItem)==ITYPE_ARMOR || GET_TYPE(pItem)==ITYPE_WEAPON ) // When it comes to armor... as a weapon...
				{
					// by evilkiki 2010.03.16 Time system applied only to weapons/armor
                                        if( pItem->m_bTimeLimit ) // Time-limited items are not allowed. addition. (06-02-24 Gemstone)
					{
						//-- IDS_LIMITED_CANT : Restricted items are not available.
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_LIMITED_CANT), TYPE_NOR_OK, 1 );
						return -1;
					}

					if( ChechIs261LvWeponItem(pItem) && pItem->m_PlusNum >= 10)
					{
						//-- IDS_NPCUPGRADE_MAXLEVEL : It is an item that can no longer be strengthened.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), TYPE_NOR_OK, 1 );
						return -1;
					}
					else if( !CheckCanNpcUpgradeItem(pItem) ) // Only certain items can be enhanced.
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH :A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}

				if( pItem->m_Vnum == 228 || pItem->m_Vnum == 1068
						|| pItem->m_Vnum == 1966 || pItem->m_Vnum == 1967
						//|| pItem->m_Vnum ==2939
				  ) // number of gems.
				{
					++GemCount;
				}

				pItem = pItem->m_Next;
			}

			if( GemCount != 1 ) // The current gem requirement is unconditionally 1.
			{
				//-- IDS_CHECK_ITEM_COUNT : Please check the number of items.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}

			if( CheckUpgradeNeed() == -1 ) // This function also has a function to check if there is only one type of armor.
			{
				//If you have 2 or more armor...
				return -1;
			}

			// Now that you have the required gem, check that the gem is in the correct place.
			pItem = g_pRoh->m_ExgInven;

			while (pItem)
			{
				if( GET_TYPE(pItem)!=ITYPE_ARMOR && GET_TYPE(pItem)!=ITYPE_WEAPON ) // Take off the armor and check. Weapons too...
				{
					if( NeedItem[UPGRADE_NEED_DIA]
							&& ( pItem->m_Vnum != 228 && pItem->m_Vnum != 1966 )
					  ) // If you ask for diamonds and have anything other than diamonds...
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH : ´A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
					else if( NeedItem[UPGRADE_NEED_REGENT_DIA]
							 && ( pItem->m_Vnum != 1068 && pItem->m_Vnum != 1967 )
						   ) // If you ask for a Regent Diamond, but have anything else...
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}
				else // Armor checks durability.
				{
					if( pItem->m_MaxUpgradeEndurance && pItem->m_UpgradeEndurance <= 0 ) // If reinforcement durability runs out... // 07-04-17 Fixed to check durability only when Max Chi is present.
					{
						//-- IDS_NPCUPGRADE_EDRLOW : Insufficient reinforcement durability.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCUPGRADE_EDRLOW), TYPE_NOR_OK, 1 );
						return -1;
					}
				}

				pItem = pItem->m_Next;
			}
		}
		break;

	case INTERFACE_TYPE_CONVERSION: // In the case of a conversion interface
		{
			//No need gem flags.
			int Amethyst = 0, Topaz = 0, Opal = 0, Sapphire = 0, Ruby = 0, Emerald = 0, Diamond = 0, RegentDia = 0, Platinum = 0;

			// Necessary Gem Inspection.
			while (pItem)
			{
				if( pItem->m_bTimeLimit ) // Non-processing of time-based items. addition. (06-02-24 Gemstone)
				{
					//-- IDS_LIMITED_CANT :Restricted items are not available.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_LIMITED_CANT), TYPE_NOR_OK, 1 );
					return -1;
				}

				if( GET_TYPE(pItem)!=ITYPE_ARMOR && GET_TYPE(pItem)!=ITYPE_WEAPON && GET_TYPE(pItem)!=IETC_GEM && GET_TYPE(pItem)!=IETC_GEMCHIP ) // If there is a type of item that should not be...
				{
					// If the wrong item is included...
                                       //-- IDS_QUEST_ITEM_NOT_MATCH : Different types of items are mixed.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				else if( GET_TYPE(pItem)==ITYPE_ARMOR || GET_TYPE(pItem)==ITYPE_WEAPON ) // When it's armor...or when it's a weapon.
				{
					if( !CheckCanConversionItem(pItem) ) // Only certain items can be changed.
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH :A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}

				if( g_dwLangType == LANG_KOR)
				{
					switch(pItem->m_Vnum)
					{
					case 222:
					case 1960:	// Magic Amethyst, Great
						Amethyst++;
						break;
					case 223:
					case 1961:	// Magic Topaz, Great
						Topaz++;
						break;
					case 224:
					case 1962:	// Magic Opal, Great
						Opal++;
						break;
					case 225:
					case 1963:	// Magic Sapphire, Great
						Sapphire++;
						break;
					case 226:
					case 1964:	// Magic Ruby, Great
						Ruby++;
						break;
					case 227:
					case 1965:	//Magic Emerald, Great
						Emerald++;
						break;
					case 228:
					case 1966:	// Magic Diamond, Great
						Diamond++;
						break;
					case 1068:
					case 1967:	// Magic Regent Diamond, Great
						RegentDia++;
						break;
					case 978:				// Magic Platinum
						Platinum++;
						break;
					}
				}
				else
				{
					if( pItem->m_Vnum == 222 ) // magic amethyst
						++Amethyst;
					if( pItem->m_Vnum == 223 ) // Magic Topaz
						++Topaz;
					if( pItem->m_Vnum == 224 ) // ¸ÅÁ÷¿ÀÆÈ
						++Opal;
					if( pItem->m_Vnum == 225 ) // magic opal
						++Sapphire;
					if( pItem->m_Vnum == 226 ) // magic sapphire
						++Ruby;
					if( pItem->m_Vnum == 227 ) // magic ruby
						++Emerald;
					if( pItem->m_Vnum == 228 ) //magic emerald
						++Diamond;
					if( pItem->m_Vnum == 1068 ) // Magic Regent Diamond
						++RegentDia;
					if( pItem->m_Vnum == 978 ) //Magic Platinum
						++Platinum;
				}

				pItem = pItem->m_Next;
			}

			if( CheckUpgradeNeed() == -1 ) // This function also has a function to check if there is only one type of armor or weapon.
			{
				// If you have 2 or more armor or weapons...
				return -1;
			}
			
			// Check if the number of gems is correct. (The reason for going through the above CheckUpgradeNeed first is to prove that there is only one weapon or armor item.)
			pItem = g_pRoh->m_ExgInven; // Prepare for inspection again from scratch.

			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON || GET_TYPE(pItem)==ITYPE_ARMOR ) // Check weapons or armor.
				{
					break; // Prepare for inspection again from scratch.
				}

				pItem = pItem->m_Next;
			}

			if( pItem ) // OK kill.
			{
				// Check the requirements for each item level class.
				if( GET_ITEM_LEVEL(pItem) == 195 ) // 3 types of requirements.
				{
					if( Amethyst != 1 || Topaz != 1 || Opal != 1 || Sapphire != 1
							|| Ruby != 1 || Emerald != 1 || Diamond != 10 || RegentDia != 0 || Platinum != 2 )
					{
						//-- IDS_CHECK_ITEM_COUNT :Please check the number of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
						return -1;
					}
				}
				else if( GET_ITEM_LEVEL(pItem) == 210 ) // 4 types of requirements.
				{
					if( Amethyst != 1 || Topaz != 1 || Opal != 1 || Sapphire != 1
							|| Ruby != 1 || Emerald != 1 || Diamond != 5 || RegentDia != 0  || Platinum != 7 )
					{
						//-- IDS_CHECK_ITEM_COUNT :Please check the number of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
						return -1;
					}
				}
				else if( GET_ITEM_LEVEL(pItem) == 261 ) // 5 types of requirements.
				{
					if( Amethyst != 1 || Topaz != 1 || Opal != 1 || Sapphire != 1
							|| Ruby != 1 || Emerald != 1 || Diamond != 1 || RegentDia != 1  || Platinum != 10 )
					{
						//-- IDS_CHECK_ITEM_COUNT :Please check the number of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
						return -1;
					}
				}
			}
		}
		break;

	case INTERFACE_TYPE_CONFUSION: // random redo
		{
			while (pItem)
			{
				if( pItem->m_bTimeLimit ) // Time-limited items cannot be processed. addition. (06-02-24 Gemstone)
				{
					//-- IDS_LIMITED_CANT : Restricted items are not available.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_LIMITED_CANT), TYPE_NOR_OK, 1 );
					return -1;
				}

				if( GET_TYPE(pItem)!=ITYPE_ARMOR && GET_TYPE(pItem)!=ITYPE_WEAPON&& pItem->m_Vnum != 228) // Currently, it does not support anything other than armor, weapons, and diamonds.
				{
					// If the wrong item is included...
                                      //-- IDS_QUEST_ITEM_NOT_MATCH : Different types of items are mixed.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				else if( GET_TYPE(pItem)==ITYPE_ARMOR || GET_TYPE(pItem)==ITYPE_WEAPON ) // When it comes to armor... as a weapon...
				{
					if( !CheckIsConversionItem(pItem,0,2) ) // Only certain items (only converted items) can be strengthened.
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}

				if( pItem->m_Vnum == 228 ) //number of gems.
					++GemCount;

				pItem = pItem->m_Next;
			}

			if( GemCount != NeedItem[UPGRADE_NEED_DIA] )
			{
				//-- IDS_CHECK_ITEM_COUNT : ¾ÆÀÌÅÛ °³¼ö¸¦ È®ÀÎÇØÁÖ¼¼¿ä.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}

			if( CheckUpgradeNeed() == -1 ) // This function also has a function to check if there is only one type of armor.
			{
				// If you have 2 or more armor...
				return -1;
			}

			// Now that you have the required gem, check that the gem is in the correct place.
			pItem = g_pRoh->m_ExgInven;

			while (pItem)
			{
				if( GET_TYPE(pItem)!=ITYPE_ARMOR && GET_TYPE(pItem)!=ITYPE_WEAPON ) // Take off the armor and check. Weapons too...
				{
					if( NeedItem[UPGRADE_NEED_DIA] && pItem->m_Vnum != 228) // If you ask for diamonds and have anything other than diamonds...
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}

				pItem = pItem->m_Next;
			}
		}
		break;

	case INTERFACE_TYPE_PROCESS: // process
		{
			// Required gem flags.
			int Amethyst = 0, Topaz = 0, Opal = 0, Sapphire = 0, Ruby = 0,
				Emerald = 0, Diamond = 0, RegentDia = 0, Platinum = 0,
				ProcessFire = 0, ProcessWater = 0, ProcessLight = 0, ProcessEarth = 0, ProcessWind = 0;

			// Necessary Gem Inspection.
			while (pItem)
			{
				if( pItem->m_bTimeLimit ) // Time-limited items cannot be processed. addition. (06-02-24 Gemstone)
				{
					//-- IDS_LIMITED_CANT : Restricted items are not available.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_LIMITED_CANT), TYPE_NOR_OK, 1 );
					return -1;
				}

				if( GET_TYPE(pItem)!=ITYPE_ARMOR && GET_TYPE(pItem)!=IETC_GEM && GET_TYPE(pItem)!=IETC_GEMCHIP ) // If there is a type of item that should not be...
				{
					//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				else if( GET_TYPE(pItem) == ITYPE_ARMOR) // When it comes to defense
				{
					if( !(GET_ATT_FLAG(pItem) & IATT_REFINE) ) // Only certain items can be changed.
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH :A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}

				if( g_dwLangType == LANG_KOR )
				{
					switch(pItem->m_Vnum)
					{
					case 222:
					case 1960:	// Magic Amethyst, Great
						Amethyst++;
						break;
					case 223:
					case 1961:	// Magic Topaz, Great
						Topaz++;
						break;
					case 224:
					case 1962:	// Magic Opal, Great
						Opal++;
						break;
					case 225:
					case 1963:	// Magic Sapphire, Great
						Sapphire++;
						break;
					case 226:
					case 1964:	// Magic Ruby, Great
						Ruby++;
						break;
					case 227:
					case 1965:	// ¸Magic Emerald, Great
						Emerald++;
						break;
					case 228:
					case 1966:	//Magic Diamond, Great
						Diamond++;
						break;
					case 1068:
					case 1967:	// Magic Regent Diamond, Great
						RegentDia++;
						break;
					case 978:				// Magic Platinum
						Platinum++;
						break;
					}
				}
				else
				{
					if( pItem->m_Vnum == 222 ) // Magic Amethyst
                                                   ++Amethyst;
                                        if( pItem->m_Vnum == 223 ) // Magic Topaz
                                                   ++Topaz;
                                        if( pItem->m_Vnum == 224 ) // Magic Opal
                                                   ++Opal;
                                        if( pItem->m_Vnum == 225 ) // Magic Sapphire
                                                   ++Sapphire;
                                        if( pItem->m_Vnum == 226 ) // Magic Ruby
                                                   ++Ruby;
                                        if( pItem->m_Vnum == 227 ) // Magic Emerald
                                                   ++Emerald;
                                        if( pItem->m_Vnum == 228 ) // Magic Diamond
                                                   ++Diamond;
                                        if( pItem->m_Vnum == 1068 ) // Magic Regent Diamond
                                                   ++RegentDia;
                                        if( pItem->m_Vnum == 978 ) // Magic Platinum
						++Platinum;
				}
				pItem = pItem->m_Next;
			}

			if( CheckUpgradeNeed() == -1 ) // This function also has a function to check if there is only one type of armor or weapon.
			{
				// If you have 2 or more armor or weapons...
				return -1;
			}

			//Check if the number of gems is correct. (The reason for going through the above CheckUpgradeNeed first is to prove that there is only one weapon or armor item.)
			if( g_pRoh )
				pItem = g_pRoh->m_ExgInven; // 

			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_ARMOR ) // Check weapons or armor.
				{
					break; // There is only one, so escape immediately.
				}

				pItem = pItem->m_Next;
			}

			pItem = g_pRoh->m_ExgInven;

			while (pItem)
			{
				if( GET_TYPE(pItem) == ITYPE_ETC && GET_SHAPE(pItem) == IETC_ETC) // Refinery check.
				{
					pCheckItem = pItem; // Remember the pointer. If there is normally only one item, this pointer is used to get what you need.
				}
				pItem = pItem->m_Next;
			}

			if( pCheckItem )
			{
				if( pCheckItem->m_Vnum == 1362 )
					special_level = (pCheckItem->m_Special2 & 0x1C00) >> 10;
				else if( pCheckItem->m_Vnum == 1363 )
					special_level1 = (pCheckItem->m_Special2 & 0xE000) >> 13;
				else if( pCheckItem->m_Vnum == 1364 )
					special_level2 = (pCheckItem->m_Special2 & 0x70000) >> 16;
				else if( pCheckItem->m_Vnum == 1365 )
					special_level3 = (pCheckItem->m_Special2 & 0x380000) >> 19;
				else if( pCheckItem->m_Vnum == 1366 )
					special_level4 = (pCheckItem->m_Special2 & 0x1C00000) >> 22;
			}
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
		{
			int Gem[7] = {0,};
			int MRDia = 0;
			int GMRDia = 0;
			int	GMDia = 0;
			int Item = 0;
			int offset = 0;
			int count = 0;
			int	trash = 0;
			CItem* pCheckItem = NULL;

			while (pItem)
			{
				// Check by jewel type
				offset = pItem->m_Vnum - 222;
				if( offset >= 0 && offset < 7 ) // 222(Magic Amethyst) ~ 228 (Magic Diamond)
				{
					Gem[offset] ++;
				}
				else if( ChechIs261LvWeponItem(pItem) )
				{
					if( pItem->m_PlusNum < 10 || pItem->m_LimitTime != 0 )
					{
						//-- IDS_NPCUPGRADE_PLZUPITEM : Please upload the item.
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), TYPE_NOR_OK, 1 );
						return -1;
					}
					count++;
					pCheckItem = pItem;
				}
				else if( pItem->m_Vnum == 1068 )// Magic Regent Diamond
				{
					MRDia++;
				}
				else if( pItem->m_Vnum == 1966 )// Great Magic Diamond.
				{
					GMDia++;
				}
				else if( pItem->m_Vnum == 1967 )// Great Magic Regent Diamond.
				{
					GMRDia++;
				}
				else
				{
					//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				pItem = pItem->m_Next;
			}

			int totalGems = 0;

			if( count == 1 )
			{
				if( pCheckItem->m_PlusNum > 10 )
				{
					totalGems = Gem[0] + Gem[1] + Gem[2] + Gem[3] + Gem[4] + Gem [5] + Gem[6] + GMDia + GMRDia + MRDia ;
					if( totalGems != 0 )
					{
						//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
						return -1;
					}
				}
				else if( pCheckItem->m_PlusNum == 10 )
				{
					totalGems = Gem[0] + Gem[1] + Gem[2] + Gem[3] + Gem[4] + Gem [5] + Gem[6];
					if( !(totalGems == 7 && GMDia == 1 && GMRDia == 1 && MRDia == 3 && count == 1) )
					{
						//-- IDS_CHECK_ITEM_COUNT : Please check the number of items.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
						return -1;
					}
				}
			}
			else if( count > 1)
			{
				//-- IDS_CHECK_ITEM_COUNT : Please check the number of items.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
			else
			{
				//-- IDS_CHECK_ITEM_COUNT : Please check the number of items.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_UPGRADE:
		{
			//			int MRDia = 0;
			//			int GMRDia = 0;
			int Dia = 0;
			int count = 0;
			int	trash = 0;

			CItem *pItemTemp = NULL;

			while (pItem)
			{
				if( ( pItem->m_Vnum >= 2049 && pItem->m_Vnum <= 2055 )
						|| ( TRUE == IS_2011_ITEM_WEAPON( pItem->m_Vnum ) )
						|| ( pItem->m_Vnum >= 2743 && pItem->m_Vnum <= 2749 ) ) // System Upgrade 360 Weapon = 03
				{
					if( pItem->m_PlusNum == 15 )
					{
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_MAXLEVEL), FALSE );
						return -1;
					}

					if( GET_TYPE(pItem) == ITYPE_WEAPON  )
					{
						if( pItem->m_LimitTime != 0  )
						{
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_LIMITED_CANT), TYPE_NOR_OK, 1 );
							return -1;
						}
					}

					pItemTemp = pItem;
					count++;
				}
				else if( pItem->m_Vnum == 1068 || pItem->m_Vnum == 1967 ) // Either one is unconditional
				{
					Dia++;
				}
				else
				{
					//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				pItem = pItem->m_Next;
			}

			if( !(Dia == 1 && count == 1) )
			{
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
		{
			if( NULL == LHIS_ENCHANT() )
				return -1;

			CItem * current_item = NULL;
			int upgrade_type = LHIS_ENCHANT()->CheckUpgrade(&current_item);
			if( upgrade_type == -1 || !current_item )
			{
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}

			LHParam_EnchantItem_Ex * enchant_ex_system = LHIS_ENCHANT_EX();
			if( NULL == LHIS_ENCHANT_EX() )
			{
				return -1;
			}
			if( false == LHIS_ENCHANT_EX()->CheckGem(upgrade_type, current_item->m_PlusNum) )
			{
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
			if( false == LHIS_ENCHANT_EX()->CheckEtc(upgrade_type, current_item->m_PlusNum) )
			{
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER:
		{
			int count = 0;
			int	trash = 0;

			while (pItem)
			{
				// º¸¼® Á¾·ù¹ú·Î Ã¼Å©
				if( ( pItem->m_Vnum >= 2049 && pItem->m_Vnum <= 2055 )
						//|| ( pItem->m_Vnum >= 2939 && pItem->m_Vnum <= 2941 )
						//|| ( pItem->m_Vnum >= 2942 && pItem->m_Vnum <= 2953 )
						|| ( TRUE == IS_2011_ITEM_WEAPON( pItem->m_Vnum ) )

				  )
				{
					if( pItem->m_PlusNum != 15 || pItem->m_LimitTime != 0 )
					{
						//-- IDS_NPCUPGRADE_MAXLEVEL : This item can no longer be enhanced.
                                                //m_NeedMoneyText1.SetString( "This item cannot be exchanged.", FALSE );

						//-- IDS_UPGRADE_GODMSG : This item cannot be exchanged.
						m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_UPGRADE_GODMSG), FALSE );
						return -1;
					}
					count++;
				}
				else
				{
					//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
					return -1;
				}
				pItem = pItem->m_Next;
			}

			if( count != 1)
			{
				//-- IDS_CHECK_ITEM_COUNT : Please check the number of items.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;


	case INTERFACE_TYPE_ITEMMAKE:	
		{
			__int64 nNeedLaim = 0; // System Unlock Money
			if ( false == LHIS_MAKE()->CheckNeedItem( nNeedLaim ) )  // Display Lime on NPC
			{
				//-- IDS_QUEST_ITEM_NOT_MATCH : A mix of different types of items.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
				//g_pNk2DFrame->InsertPopup( "Check the materials required to craft the item.", TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;	

	case INTERFACE_TYPE_ITEMSEPARATE:
		{
			if ( false == LHIS_SEPERATE()->CheckNeedItem() )
			{
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_QUEST_ITEM_NOT_MATCH), TYPE_NOR_OK, 1 );
				//g_pNk2DFrame->InsertPopup( "Item cannot be disassembled.", TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;

	case INTERFACE_TYPE_GMAGICSTONE:
		{
			__int64 nNeedLaim = 0; // System Unlock Money
			if ( false == LHIS_GMAGICSTONE()->CheckNeedItem( nNeedLaim ) )
			{
				//-- IDS_CHECK_ITEM_COUNT : Please check the number of items.
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CHECK_ITEM_COUNT), TYPE_NOR_OK, 1 );
				return -1;
			}
		}
		break;


	}
	return 1;
}

BOOL CControlUpgrade::CheckBrokenItem()
{
	if( m_iType == INTERFACE_TYPE_GOD_WEAPON_UPGRADE )
	{
		if( NULL == LHIS_ENCHANT() )
			return FALSE;

		CItem * current_item = NULL;
		int upgrade_type = LHIS_ENCHANT()->CheckUpgrade_Weapon(&current_item);
		if( upgrade_type == -1 || !current_item )
			return FALSE;

		if( NULL == LHIS_ENCHANT_EX() )
			return FALSE;

		const LHParam_EnchantItem_Ex::sData * param_ex = LHIS_ENCHANT_EX()->GetParam(upgrade_type, current_item->m_PlusNum);


		if( !param_ex )
			return FALSE;

		int msg_index = -1;

		switch( param_ex->m_penalty_type )
		{
//		case 0:		msg_index = IDS_LHSTRING1946;		break;
		case 1:
			msg_index = IDS_LHSTRING1947;
			break;
		case 2:
			msg_index = IDS_LHSTRING1948;
			break;
		case 3:
			msg_index = IDS_LHSTRING1949;
			break;
		case 4:
			msg_index = IDS_LHSTRING1950;
			break;
		case 5:
			msg_index = IDS_LHSTRING1951;
			break;
		case 6:
			msg_index = IDS_LHSTRING1952;
			break;
		case 7:
			msg_index = IDS_LHSTRING1953;
			break;
		}

		if( msg_index != -1 )
		{
			g_pNk2DFrame->InsertPopup((char*)G_STRING(msg_index),TYPE_NOR_OKCANCLE, 30);
			return TRUE;
		}
	}
	else if( m_iType == INTERFACE_TYPE_UPARMOR || m_iType == INTERFACE_TYPE_UPWEAPON  || m_iType == INTERFACE_TYPE_320LV_WEAPON_UPGRADE )
	{
		CItem* pItem = NULL;

		pItem = g_pRoh->m_ExgInven;

		if(!pItem)
			return FALSE;

		if( g_SvrType != ST_ADULT_ONLY )
		{
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_ARMOR )
				{
					if( pItem->m_PlusNum == 3 || pItem->m_PlusNum == 8 )
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_11_WARNING),TYPE_NOR_OKCANCLE, 30);
						return TRUE;
					}
					else if( pItem->m_PlusNum == 11 && (pItem->m_MinLevel_Org > 300 || IS_2011_ITEM_ARMOR(pItem->m_Vnum )) )
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_LHSTRING1955),TYPE_NOR_OKCANCLE, 30);
						return TRUE;
					}
				}
				pItem = pItem->m_Next;
			}

			pItem = g_pRoh->m_ExgInven;

			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON )
				{
					if( pItem->m_PlusNum == 3 || pItem->m_PlusNum == 7 )
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG1), TYPE_NOR_OKCANCLE, 30 );
						return TRUE;
					}
					else if(pItem->m_PlusNum == 11 )
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_MSG2), TYPE_NOR_OKCANCLE, 30 );
						return TRUE;
					}
				}
				pItem = pItem->m_Next;
			}
		}
		else
		{
			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_ARMOR )
				{
					if( pItem->m_PlusNum == 3 || pItem->m_PlusNum == 8 )
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_11_WARNING),TYPE_NOR_OKCANCLE, 30);
						return TRUE;
					}
					else if(pItem->m_PlusNum == 11 && pItem->m_MinLevel_Org > 300 )
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_11_WARNING),TYPE_NOR_OKCANCLE, 30);
						return TRUE;
					}
				}
				pItem = pItem->m_Next;
			}

			pItem = g_pRoh->m_ExgInven;

			while (pItem)
			{
				if( GET_TYPE(pItem)==ITYPE_WEAPON )
				{
					if( pItem->m_PlusNum == 3 || pItem->m_PlusNum == 7 || pItem->m_PlusNum == 11)
					{
						g_pNk2DFrame->InsertPopup((char*)G_STRING(IDS_UPGRADE_11_WARNING), TYPE_NOR_OKCANCLE, 30 );
						return TRUE;
					}
				}
				pItem = pItem->m_Next;
			}
		}
	}


	return FALSE;
}

void CControlUpgrade::ProcessState()
{
	switch( m_NowState )
	{
	case UPGRADE_ANI_ING:

		if( m_SpriteAni[m_NowState].m_NowFrame == 0 ) // In the first frame...
		{
			if( g_pDSound && m_pAniSound[m_NowState] )
			{
				g_pDSound->PlayToOutside( m_pAniSound[m_NowState] );
			}
		}

		if( timeGetTime() - m_dwStartTickTime > m_dwMaxAniTime )
		{
			if( m_Result > 0 && m_Result < MAX_UPGRADE_ANI )
			{
				m_NowState = m_Result;

				m_SpriteAni[m_NowState].m_AniType = SPRITE_ANITYPE_ONEPLAY;
				m_SpriteAni[m_NowState].SetStart();

				if( g_pDSound && m_pAniSound[m_NowState] )
				{
					g_pDSound->PlayToOutside( m_pAniSound[m_NowState] );
				}
			}
			else
			{
				g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCUPGRADE_ERROR), TYPE_NOR_OK, 1 );

				m_NowState = UPGRADE_ANI_NON; // Restore state.
                                m_bSlotLock = FALSE; // Release the lock.
                                m_Result = 0; // Initialize the result...

				return;
			}
		}
		else if( timeGetTime() - m_dwStartTickTime > m_dwMinAniTime ) // If a reasonable amount of time has elapsed...wait for a specific frame.
		{
			if( m_SpriteAni[m_NowState].m_NowFrame == m_iAniChangeFrame ) // To advance to the next animation at a specific frame.. (Frame number is hard-coded.)
			{
				if( m_Result > 0 && m_Result < MAX_UPGRADE_ANI ) // range check
				{
					m_NowState = m_Result;

					m_SpriteAni[m_NowState].m_AniType = SPRITE_ANITYPE_ONEPLAY; // This animation is set to repeat once.
					m_SpriteAni[m_NowState].SetStart(); // Start animation.

					if( g_pDSound && m_pAniSound[m_NowState] )
					{
						g_pDSound->PlayToOutside( m_pAniSound[m_NowState] ); // Play sound effect when changing state.
					}
				}
				else // In this case, something is wrong.
				{
					g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCUPGRADE_ERROR), TYPE_NOR_OK, 1 );

					m_NowState = UPGRADE_ANI_NON; // Restore state.
                                        m_bSlotLock = FALSE; // Release the lock.
                                        m_Result = 0; // Initialize the result...

					return;
				}
			}
		}
		break;

	case UPGRADE_ANI_SUCCESS: // When the packet is delivered with success...
	case UPGRADE_ANI_FAIL:
	case UPGRADE_ANI_BROKEN:

		if( m_SpriteAni[m_NowState].IsAniEnd() ) // Once the animation has been played...
		{
			if( g_pNk2DFrame )
			{
				switch( m_iType )
				{
				case INTERFACE_TYPE_UPARMOR:
				case INTERFACE_TYPE_UPWEAPON: // The message should change according to the type.
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//-- IDS_UIMGR_SUCCEED_UPGRADE : Equipment upgrade was successful
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_SUCCEED_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_SUCCEED_UPGRADE), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//-- IDS_UIMGR_FAIL_UPGRADE : Equipment upgrade failed
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_FAIL_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_FAIL_UPGRADE), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_BROKEN:
							//-- IDS_UIMGR_REMOVE_UPGRADE : Equipment has been destroyed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_REMOVE_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_REMOVE_UPGRADE), -1); // It is also left in the chat window.

							break;
						}
					}
					break;

				case INTERFACE_TYPE_CONVERSION : // The message should change according to the type.
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//-- IDS_CONVERSION_SUCC : Conversion was successful.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CONVERSION_SUCC), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_CONVERSION_SUCC), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//-- IDS_CONVERSION_FAIL : Conversion failed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CONVERSION_FAIL), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_CONVERSION_FAIL), -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_CONFUSION: // random redo
					{
						// This guy is unconditionally successful.
                                              //-- IDS_EQUIP_STATUS_RESET : The new equipment value has been applied.
						g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_EQUIP_STATUS_RESET), TYPE_NOR_OK, 32 );
						g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_EQUIP_STATUS_RESET), -1); // It is also left in the chat window.
					}
					break;

				case INTERFACE_TYPE_PROCESS: // The message should change according to the type.
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//-- IDS_NPCPROCESS_SUCC : Refining was successful.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCPROCESS_SUCC), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( (char*)G_STRING(IDS_NPCPROCESS_SUCC), -1); //It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//-- IDS_NPCPROCESS_FAIL : Refining failed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_NPCPROCESS_FAIL), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( (char*)G_STRING(IDS_NPCPROCESS_FAIL), -1); // It is also left in the chat window.
							break;
						}
					}
					break;
				case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//-- IDS_CONVERSION_SUCC : Conversion was successful.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CONVERSION_SUCC), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_CONVERSION_SUCC), -1); //It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//-- IDS_CONVERSION_FAIL : Conversion failed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_CONVERSION_FAIL), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_CONVERSION_FAIL), -1); //It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_320LV_WEAPON_UPGRADE:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//-- IDS_UIMGR_SUCCEED_UPGRADE : Equipment upgrade was successful
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_SUCCEED_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_SUCCEED_UPGRADE), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//-- IDS_UIMGR_FAIL_UPGRADE : Equipment upgrade failed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_FAIL_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_FAIL_UPGRADE), -1); //It is also left in the chat window.
							break;

						case UPGRADE_ANI_BROKEN:
							//-- IDS_UIMGR_REMOVE_UPGRADE :Equipment has been destroyed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_REMOVE_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_REMOVE_UPGRADE), -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_GOD_WEAPON_UPGRADE:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//-- IDS_UIMGR_SUCCEED_UPGRADE : Equipment upgrade was successful
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_SUCCEED_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_SUCCEED_UPGRADE), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//-- IDS_UIMGR_FAIL_UPGRADE : Equipment upgrade failed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_FAIL_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_FAIL_UPGRADE), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_BROKEN:
							//-- IDS_UIMGR_REMOVE_UPGRADE : Equipment has been destroyed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UIMGR_REMOVE_UPGRADE), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UIMGR_REMOVE_UPGRADE), -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_320LV_WEAPON_GODWPOWER:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UPGRADE_MSG3), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UPGRADE_MSG3), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_UPGRADE_MSG4), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_UPGRADE_MSG4), -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_ITEMMAKE:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							g_pNk2DFrame->InsertPopup( "Item crafting success", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Item crafting success", -1); // left in chat
							break;

						case UPGRADE_ANI_FAIL:
							g_pNk2DFrame->InsertPopup( "Failed to create item", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Failed to create item", -1); // left in chat
							break;
						}
					}
					break;

				case INTERFACE_TYPE_BUFFMAKE_MAKE:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							g_pNk2DFrame->InsertPopup( "Successfully crafting a buff item", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Successfully crafting a buff item", -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							g_pNk2DFrame->InsertPopup( "Failed to craft buff item", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Failed to craft buff item", -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_BUFFMAKE_GATCHA:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							g_pNk2DFrame->InsertPopup( "Buff Item Gotcha Success", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Buff Item Gotcha Success", -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							g_pNk2DFrame->InsertPopup( "Buff Item Gatcha Failed", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Buff Item Gatcha Failed", -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_BUFFMAKE_MEDAL:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							g_pNk2DFrame->InsertPopup( "medal exchange success", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "medal exchange success", -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							g_pNk2DFrame->InsertPopup( "Failed to exchange medals", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Failed to exchange medals", -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_ITEMSEPARATE:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							g_pNk2DFrame->InsertPopup( "Item disassembly success", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Item disassembly success", -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							g_pNk2DFrame->InsertPopup( "Item disassembly failed", TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr( "Item disassembly failed", -1); // It is also left in the chat window.
							break;
						}
					}
					break;

				case INTERFACE_TYPE_GMAGICSTONE:
					{
						switch( m_Result )
						{
						case UPGRADE_ANI_SUCCESS:
							//Purification was successful.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_ITEM_GMASUK_SUCCESS), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_ITEM_GMASUK_SUCCESS), -1); // It is also left in the chat window.
							break;

						case UPGRADE_ANI_FAIL:
							//Purification failed.
							g_pNk2DFrame->InsertPopup( (char*)G_STRING(IDS_ITEM_GMASUK_FAILED), TYPE_NOR_OK, 32 );
							g_pNk2DFrame->AddChatStr((char*)G_STRING(IDS_ITEM_GMASUK_FAILED), -1); // It is also left in the chat window.
							break;
						}
					}
					break;


				}
			}

			pCMyApp->m_bInverseReturn = TRUE; // set to prevent return

			m_NowState = UPGRADE_ANI_NON; // Restore state.
                        m_bSlotLock = FALSE; // Release the lock.
                        m_Result = 0; // Initialize the result...

			CheckUpgradeNeed(); // Renew the requirements.
		}

		break;
	}
}

void CControlUpgrade::UpdateTextInfo( int nType, int TextInfoIndex ) // TextInfo The guy who changes the surface and loads it.
{
	char strTemp[256];

	if( m_iTextInfoIndex != m_iOldTextInfoIndex ) // When the surface needs to be changed.
	{
		SAFE_DELETE(m_pTextInfo); // Delete it first...

		switch( nType)
		{
		case INTERFACE_TYPE_CONVERSION:
			sprintf( strTemp, "interface/description/NpcUpgrade/Conversion_info%03d.bmp", TextInfoIndex ); //Read info by number.
			break;

		case INTERFACE_TYPE_PROCESS:
			sprintf( strTemp, "interface/description/NpcUpgrade/Process_info%03d.bmp", TextInfoIndex ); // Read info by number.
			break;
		case INTERFACE_TYPE_320LV_WEAPON_CONVERSION:
			sprintf( strTemp, "interface/description/npcupgrade/info_Conversion2_%2d.bmp", TextInfoIndex ); // Read info by number.
			break;
		}

		g_pDisplay->CreateSurfaceFromBitmap( &m_pTextInfo, strTemp );  // reload.

		m_hWnd =  g_pDisplay->GetHWnd();
		GetClientRect( m_hWnd, &m_ClientRc );

		if( m_pTextInfo )
		{
			m_pTextInfo->Xpos = m_pBackSur->Xpos + 34;
			m_pTextInfo->Ypos = m_pBackSur->Ypos + 80;
		}
	}

	m_iOldTextInfoIndex = m_iTextInfoIndex; // renewal.
}

BOOL CheckCanNpcUpgradeItem( CItem *pItem ) //For checking whether an item can be strengthened.
{
	if( !pItem )
		return FALSE;

	if( (GET_ATT_FLAG(pItem) & IATT_NO_GEM) )
		return FALSE;

	// m_Vnum : Item DB number
	if( (pItem->m_Vnum >= 1190 && pItem->m_Vnum <= 1229) || (pItem->m_Vnum >= 1313 && pItem->m_Vnum <= 1326) ) // Currently, only items in this range are available.
		return TRUE;

	if( pItem->m_Vnum >= 1560 && pItem->m_Vnum <= 1579 ) // 300·Added new level armor.
		return TRUE;

	if( pItem->m_Vnum >= 1797 && pItem->m_Vnum <= 1816 ) // [2008/7/7 Theodoric]  Rare armor enhancement added
		return TRUE;

	if( ( pItem->m_Vnum >= 2580 && pItem->m_Vnum <= 2583 )
			|| ( pItem->m_Vnum >= 2604 && pItem->m_Vnum <= 2607 )
			|| ( pItem->m_Vnum >= 2612 && pItem->m_Vnum <= 2615 )
			|| ( pItem->m_Vnum >= 2620 && pItem->m_Vnum <= 2623 )
			|| ( pItem->m_Vnum >= 2628 && pItem->m_Vnum <= 2631 )
	  )
		return TRUE;
	if(    ( pItem->m_Vnum >= 2636 && pItem->m_Vnum <= 2639 )
			|| ( pItem->m_Vnum >= 2644 && pItem->m_Vnum <= 2647 )
			|| ( pItem->m_Vnum >= 2652 && pItem->m_Vnum <= 2655 )
			|| ( pItem->m_Vnum >= 2660 && pItem->m_Vnum <= 2663 )
			|| ( pItem->m_Vnum >= 2668 && pItem->m_Vnum <= 2671 )
	  )
		return TRUE;
	// Fancy of Jonas Defined for Upgrade in NPC Laglamia - Jonas
	if(    ( pItem->m_Vnum >= 2855 && pItem->m_Vnum <= 2878 )	// Moonshadow
			|| ( pItem->m_Vnum >= 6181 && pItem->m_Vnum <= 6184 )	// Ghost Moonshadow
			|| ( pItem->m_Vnum >= 2822 && pItem->m_Vnum <= 2841 )	// 290 Set
			|| ( pItem->m_Vnum >= 2648 && pItem->m_Vnum <= 2651 )	// Ecricks
			|| ( pItem->m_Vnum >= 2624 && pItem->m_Vnum <= 2627 )	// Demios 
			|| ( pItem->m_Vnum >= 3124 && pItem->m_Vnum <= 3127 )	// Gold Mantis
			|| ( pItem->m_Vnum >= 2660 && pItem->m_Vnum <= 2663 )	// Demius (G)
	  )
		return TRUE;



	if ( TRUE == IS_2011_ITEM_ARMOR( pItem->m_Vnum ) )
	{
		return TRUE;
	}
	if ( TRUE == IS_2011_ITEM_WEAPON( pItem->m_Vnum ) && pCMyApp->GetCurWorld() == 7 )
	{
		return TRUE;
	}

	if( pItem->m_Vnum >= 3227 && pItem->m_Vnum <= 3230 )
		return TRUE;

	return FALSE;

}

BOOL CheckCanConversionItem( CItem *pItem ) // For checking whether an item can be converted.
{
	if( !pItem )
		return FALSE;

	// Let's jump for faster processing.
	switch( pItem->m_Vnum )
	{
	case 641:
	case 642:
	case 643:
	case 644:
	case 930:
	case 979:
	case 980:
	case 981:
	case 982:
	case 983:
	case 1062:
	case 1063:
	case 1064:
	case 1065:
	case 1066:
	case 1142:
	case 1146:
	case 1147:
	case 1150:
	case 1151:
	case 1152:
		return TRUE;
	}
	return FALSE;
}

BOOL CheckIsConversionItem( CItem *pItem, int type ,int sub_type) // Check whether the item is converted or not. (Used for unlocking Manastone restrictions, etc.)
{
	if( !pItem )
		return FALSE;

	switch( type )
	{
	case 0:
		{
			if( sub_type == 1 )
			{
				if( pItem->m_Vnum >= 2694 && pItem->m_Vnum <= 2700)
					return TRUE;
				if( pItem->m_Vnum >= 1318 && pItem->m_Vnum <= 1321 )
					return TRUE;
				if( pItem->m_Vnum == 1323
						|| pItem->m_Vnum == 1325
						|| pItem->m_Vnum == 1326 )
					return TRUE;
			}
			else if( sub_type == 2 )
			{
				if( (pItem->m_Vnum >= 1313 && pItem->m_Vnum <= 1326)
						|| (pItem->m_Vnum >= 1703 && pItem->m_Vnum <= 1707)
				  )
					return TRUE;
			}
			else if( sub_type == 0 )
			{
				if( (pItem->m_Vnum >= 1313 && pItem->m_Vnum <= 1326)
						|| (pItem->m_Vnum >= 1703 && pItem->m_Vnum <= 1707)
						|| (pItem->m_Vnum >= 2694 && pItem->m_Vnum <= 2700)
				  )
					return TRUE;
			}
		}
		break;
	case 1: // 320 weapon
		if( pItem->m_Vnum >= 2049 && pItem->m_Vnum <= 2055 )
			return TRUE;


	case 2:	// System Upgrade 360 Weapon = 04
		{
			if( pItem->m_Vnum >= 2743 && pItem->m_Vnum <= 2749 )
				return TRUE;
			if( pItem->m_Vnum == 3231 )
				return TRUE;
		}
		break;
	}

	return FALSE;
}
BOOL ChechIs261LvWeponItem(CItem *pItem)
{
	if( pItem == NULL )
		return  FALSE;

	if( pItem->m_Vnum >= 1318 && pItem->m_Vnum <= 1321)
		return TRUE;

	switch( pItem->m_Vnum )
	{
	case 1323:
	case 1325:
	case 1326:
		return TRUE;
	}

	return FALSE;
}

BOOL CheckIsGodItem(CItem * pItem)
{
	if( !pItem )
		return FALSE;

	switch(pItem->m_Vnum)
	{
	case 2173: // God Ascalon
	case 2174: // God Heimdallrs Axe
	case 2175: // God Gungnir
	case 2176: // God Brionac
	case 2177: // God Sarnga
	case 2178: // God Vjaya
	case 2179: // God Mjornir
	case 3323: // God of Perom Spear

	// System Weapon Bulkan = 0
	case 6000: // 400 - Abandoned Sword
	case 6001: // 450 - War Nomi Sword
	case 6002: // 500 - Jadeite  Sword
	case 6003: // 550 - Dragonlord Sword
	case 6004: // 600 - Dark Overlord Sword
	case 6005: // 650 - Black Phoenix Sword
	case 6006: // 700 - Dark Overlord Sword
	case 6007: // 750 - Brave Sword
	case 6008: // 800 - Dragonfire Sword
	case 6009: // 850 - Titan Sword
	case 6010: // 900 - Bloody Angel Sword
	case 6011: // 950 - Judge Angel Sword
	case 6012: // 1000 - Suffering Angel Sword

	// System Weapon Bulkan = 0
	case 6013: // 400 - Abandoned Axe
	case 6014: // 450 - War Nomi Axe
	case 6015: // 500 - Jadeite  Axe
	case 6016: // 550 - Dragonlord Axe
	case 6017: // 600 - Dark Overlord Axe
	case 6018: // 650 - Black Phoenix Axe
	case 6019: // 700 - Dark Overlord Axe
	case 6020: // 750 - Brave Axe
	case 6021: // 800 - Dragonfire Axe
	case 6022: // 850 - Titan Axe
	case 6023: // 900 - Bloody Angel Axe
	case 6024: // 950 - Judge Angel Axe
	case 6025: // 1000 - Suffering Angel Axe

	// System Weapon Kailipton = 0
	case 6026: // 400 - Abandoned Staff
	case 6027: // 450 - S Chan Staff
	case 6028: // 500 - Demon Lord Staff
	case 6029: // 550 - Eclipse Staff
	case 6030: // 600 - Legendary Staff
	case 6031: // 650 - Fire Kirin Staff
	case 6032: // 700 - Blackfire Staff
	case 6033: // 750 - Archmage Soul Staff
	case 6034: // 800 - Black Demon Staff
	case 6035: // 850 - Hades Staff
	case 6036: // 900 - Bloody Angel Staff
	case 6037: // 950 - Dark Angel Staff
	case 6038: // 1000 - Suffering Angel Staff

	// System Weapon Human = 0
	case 6039: // 400 - Abandoned Gun
	case 6040: // 450 - Chen Guinea Gun
	case 6041: // 500 - Sci Machine V5 Gun
	case 6042: // 550 - Sci Machine V6 Gun
	case 6043: // 600 - Sci Machine V7 Gun
	case 6044: // 650 - Sci Machine V8 Gun
	case 6045: // 700 - Sci Machine V9 Gun
	case 6046: // 750 - Sci Machine V10 Gun
	case 6047: // 800 - Sci Machine V11 Gun
	case 6048: // 850 - Sci Machine V12 Gun
	case 6049: // 900 - Sci Machine V13 Gun
	case 6050: // 950 - Sci Machine V15 Gun
	case 6051: // 1000 - Sci Machine V16 Gun

	// System Weapon Aidia = 0
	case 6052: // 400 - Abandoned Ring
	case 6053: // 450 - Sedna Ring
	case 6054: // 500 - Lilium Ring
	case 6055: // 550 - Mistery Ring
	case 6056: // 600 - Queen Ring
	case 6057: // 650 - Aura Ring
	case 6058: // 700 - Golden Rose Ring
	case 6059: // 750 - Holyangel Ring
	case 6060: // 800 - Crimson Ring
	case 6061: // 850 - Frost Ring
	case 6062: // 900 - Bloody Angel Ring
	case 6063: // 950 - Phoenix Angel Ring
	case 6064: // 1000 - Suffering Angel Ring

	// System Weapon Hybrider = 0
	case 6065: // 400 - Abandoned Dual Sword
	case 6066: // 450 - Car Itty Dual Sword
	case 6067: // 500 - Atlans Dual Sword
	case 6068: // 550 - Rolling Thunder Dual Sword
	case 6069: // 600 - Raging Fire Dual Sword
	case 6070: // 650 - Destruction Dual Sword
	case 6071: // 700 - Royal Dual Sword
	case 6072: // 750 - Ambition Dual Sword
	case 6073: // 800 - Robust Dual Sword
	case 6074: // 850 - Grace Atlans Dual Sword
	case 6075: // 900 - Bloody Angel Dual Sword
	case 6076: // 950 - Judge Angel Dual Sword
	case 6077: // 1000 - Suffering Angel Dual Sword

	// System Weapon Hybrider = 0
	case 6078: // 400 - Abandoned Hammer
	case 6079: // 450 - Car Itty Hammer
	case 6080: // 500 - Atlans Hammer
	case 6081: // 550 - Rolling Thunder Hammer
	case 6082: // 600 - Raging Fire Hammer
	case 6083: // 650 - Destruction Hammer
	case 6084: // 700 - Royal Hammer
	case 6085: // 750 - Ambition Hammer
	case 6086: // 800 - Robust Hammer
	case 6087: // 850 - Grace Atlans Hammer
	case 6088: // 900 - Bloody Angel Dual Hammer
	case 6089: // 950 - Judge Angel Hammer
	case 6090: // 1000 - Suffering Angel Hammer

	// System Weapon Spear = 0
	case 6091: // 400 - PS-40 Abandoned Spear
	case 6092: // 450 - PS-45 Kling Spear
	case 6093: // 500 - PS-50 Gruhill Spear
	case 6094: // 550 - PS-40 Princie Spear
	case 6095: // 600 - PS-40 Hirat Spear
	case 6096: // 650 - PS-40 Constant Spear
	case 6097: // 700 - PS-40 Grace Spear
	case 6098: // 750 - PS-40 Holyangel Spear
	case 6099: // 800 - PS-40 Kenaz Spear
	case 6100: // 850 - PS-40 Sate Spear
	case 6101: // 900 - PS-40 Frere Spear
	case 6102: // 950 - PS-40 Awakening Spear
	case 6103: // 1000 - PS-40 Manticore Spear
		return TRUE;
	}

	return FALSE;
}

void CControlUpgrade::ResetText()
{
	m_NeedGemText.SetString( "", TRUE );
	m_NeedGemNumText.SetString( "", TRUE );
	m_NeedMoneyText1.SetString( (char*)G_STRING(IDS_NPCUPGRADE_PLZUPITEM), FALSE );
}

int CControlUpgrade::GetGemToIndex(int item_index)
{
	switch( item_index )
	{
	case 222: // Amethyst
		return 0;
	case 223: // Topaz
		return 1;
	case 224: // Opal
		return 2;
	case 225: // Sapphire
		return 3;
	case 226: // Ruby
		return 4;
	case 227: // Emerald
		return 5;
	case 228: // Diamond
		return 6;
	case 1068: // Regent Diamond
		return 7;
	case 1944: // Test Item
		return 8;
	case 1960: // Great Amethyst
		return 9;
	case 1961: // Great Topaz
		return 10;
	case 1962: // Great Opal
		return 11;
	case 1963: // Great Sapphire
		return 12;
	case 1964: // Great Ruby
		return 13;
	case 1965: // Great Emerald
		return 14;
	case 1967: // Great Regent Diamond
		return 15;
	case 3331: // God Stone
		return 16;
	case 3332: // Bright God Stone
		return 17;
	case 3953: // God Stone (400)
		return 18;
	case 3954: // God Stone (500)
		return 19;
	case 3955: // God Stone (600)
		return 20;
	case 3956: // God Stone (700)
		return 21;
	case 3957: // God Stone (800)
		return 22;
	case 3958: // God Stone (900)
		return 23;
	case 3959: // God Stone (1000)
		return 24;
	case 4051: // Great Magic Etherion
		return 24;
	default:
		return -1;
	}
	return -1;
}