#ifndef _CONTROL_UPGRADE_H_
#define _CONTROL_UPGRADE_H_

#include "Config.h"

#include "BasicStatic.h"
#include "DefenseValMgr.h"

#include <dsound.h>

enum INTERFACE_TYPE
{
	INTERFACE_TYPE_UPARMOR ,
	INTERFACE_TYPE_UPWEAPON ,
	INTERFACE_TYPE_CONVERSION ,
	INTERFACE_TYPE_CONFUSION ,
	INTERFACE_TYPE_PROCESS	,
	INTERFACE_TYPE_320LV_WEAPON_CONVERSION ,
	INTERFACE_TYPE_320LV_WEAPON_UPGRADE ,
	INTERFACE_TYPE_320LV_WEAPON_GODWPOWER ,
	INTERFACE_TYPE_LEVELDOWN ,
	INTERFACE_TYPE_ITEMMAKE ,
	INTERFACE_TYPE_ITEMSEPARATE ,
	INTERFACE_TYPE_GMAGICSTONE ,
	INTERFACE_TYPE_GOD_WEAPON_UPGRADE,
	INTERFACE_TYPE_UPARMOR_400,
	INTERFACE_TYPE_BUFFMAKE_MAKE,
	INTERFACE_TYPE_BUFFMAKE_GATCHA,
	INTERFACE_TYPE_BUFFMAKE_MEDAL,

	INTERFACE_TYPE_END_OF_ENUM
};

//#define NPC_UPGRADE_CAN_ARMOR_LEVEL 275 // Currently, only armor level 275 or higher can be strengthened. // Changed the current condition check to just an index check.
#define NPC_UPGRADE_NEED_REGENT_LEVEL 8 // +Level at which the required gem changes from diamond to regent



//#define MIN_ING_ANI_TIME 2000 // Tick type that makes the reinforcement animation run at a minimum. (If the packet is late, it can run more.)
//#define MAX_ING_ANI_TIME 5000 // Tick limit that allows the enhancement animation to run as much as possible.

enum { UPGRADE_ANI_NON = -1, UPGRADE_ANI_ING, UPGRADE_ANI_SUCCESS, UPGRADE_ANI_FAIL, UPGRADE_ANI_BROKEN, MAX_UPGRADE_ANI };
enum { UPGRADE_NEED_DIA = 0, UPGRADE_NEED_REGENT_DIA, UPGRADE_NEED_LEVELDOWN_DIA, MAX_NEED_ITEM_IN_NPC_UPGRADE };

class CSurface;
class CTcpIpConnection;
class CItemRender;
class CTextOutBox;
class CItem;
class CBasicButton;
class CSprite; // 2D Sprite animation class.


class LHItemSystem_Manager;




// NPC Equipment reinforcement and processing interface through
class CControlUpgrade
{
public:
	CControlUpgrade();
	virtual ~CControlUpgrade();

	void	Init();

	void	DeleteRes();
	void	LoadRes( int Type, CItemRender *pItemRender);
	HRESULT RestoreSurfaces();

	void ProcessState(); // For state transitions. Fired when m_ChangeState is different from m_NowState.
    CItem* Draw(); // The return of this function is used to update the item information displayed in the DrawItemInfo() function.
    void DrawNeedInfo(); // Print the gem and amount needed to enhance the item.
    void DrawMySlot(); // A function that draws its own exchange window inventory.
	LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	HRESULT BrokenPopupMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, int nMsg);

	BOOL	IsInside( int x, int y );
	BOOL	IsPlayArea( int x, int y ); // Check to see if you clicked on the ground outside the interface.

	void	UpdateTextInfo(int nType, int TextInfoIndex ); // TextInfo The guy who changes the surface and loads it.

	int CheckUpgradeNeed(); // Get the gems (type, number) and limes required for enhancement. Returns a pointer to the measured item. (If multiple items are mounted at the same time, NULL is returned. Only returns properly when only one piece of armor is mounted. If NULL, the user uploaded it incorrectly.)
    int CheckSlotPure(); // Check if the upgrade attempt contains inappropriate items.
    BOOL CheckBrokenItem(); // Display a warning message when there is a risk that the item will be broken. Breakable check function.

	void	SetNkCha( CNkCharacter *pNkCha )
	{
		m_pNkCha = pNkCha;
	}

	int		GetUpgradeType()
	{
		return m_iType;
	}

	HWND			m_hWnd;
	RECT			m_ClientRc;
	CItemRender *m_pItemRender; // We need this to render the item.
    CItem *m_pOverItem;
    CNkCharacter *m_pNkCha; // Reference pointer to the main character.

    CSurface *m_pBackSur; // interface background
    CSurface *m_pBack_Title;
    CSurface *m_pTextInfo; // An explanatory sheet.

    CSurface *m_pEnableSlot; // For the active slot background image.
    CSurface *m_pDisableSlot; // For disabled slot background image.
    CSurface *m_pExistSlot; // For the slot background image where the item is placed.

    CBasicButton m_UpgradeBtn; // Enhance button
    CBasicButton m_BtnCancel; // cancel button

    CSurface *m_pDiaGem; // Diamond picture. (Used to display required gems)
    CSurface *m_pRegentDiaGem; // Regent diamond picture. (Used to display required gems)
    CSurface *m_pRegOrGrtDiaGem;

    CTextOutBox m_NeedGemText; // for the text "necessary gem"
    CTextOutBox m_NeedGemNumText; // to print the number of gems needed
    CTextOutBox m_NeedMoneyText1; // The required amount. (info part)
    CTextOutBox m_NeedMoneyText2; // The required amount. (the upper part of the holding amount)
    CTextOutBox m_NeedMoneyText3; // The required amount. (the upper part of the holding amount)
    CTextOutBox m_HaveMoneyText; // The holding amount field.

	int m_iNeedGemVnum[NEEDGEM_MAX];
	int m_iNeedGemCount[NEEDGEM_MAX];

	CSprite m_SpriteAni[MAX_UPGRADE_ANI]; // Reinforcement related sprite animation.
    DWORD m_dwLastTickTime; // Tick storage variable for animation progress. (For frame update.)
    DWORD m_dwStartTickTime; // Variable to remember when the animation started. to make you wait for a certain amount of time.
    DWORD m_dwMinAniTime; // A tick type that causes the reinforcement animation to run at least (more if the packet is late).
    DWORD m_dwMaxAniTime; // Tick limit that allows the enhancement animation to run as much as possible.
    int m_iAniChangeFrame; // Frame at which animation transitions from 'ing' to another. During the 'ing' animation, this frame moves to another animation.


	LPDIRECTSOUNDBUFFER m_pAniSound[MAX_UPGRADE_ANI]; // Sound effects per animation.

	int m_iType; // Flag whether it is reinforcement or processing.
    int m_my_slotstart_x, m_my_slotstart_y;

    BOOL m_bSlotLock; // A flag to prevent ejection from the slot when replacing.
    int m_NowState; // Current state. ( 0 - Waiting, 1 - Reinforcing animation, 2 - Success animation, 3 - Failure animation )
    // BOOL m_bUpgrading; // Flag indicating whether reinforcement animation is in progress. (During reinforcement animation, it turns off the interface or prevents other actions.) (except for success and failure animations.)

	__int64 NeedItem[MAX_NEED_ITEM_IN_NPC_UPGRADE]; // The number required for each item type when strengthening or processing.
    __int64 NeedLaim; // Required lime.

    double m_iOldMoney; // Previous holding amount ///Increase the maximum amount


    int m_Result; // Reinforcement result, (0-success,1-failure,2-broken)

    int m_iTextInfoIndex, m_iOldTextInfoIndex; // During conversion, the text info is loaded differently depending on the situation and used. A variable that remembers which image is loaded and the number index of the previous image.

	DWORD	m_dwMouseClickTickTime;


	LHItemSystem_Manager * m_pItemSystemMgr;

	int				m_need_gem[3];
	CBasicStaticEx	m_gem_image;

	int GetGemToIndex(int item_index);

	void ResetText();
};

#endif // _CONTROL_UPGRADE_H_