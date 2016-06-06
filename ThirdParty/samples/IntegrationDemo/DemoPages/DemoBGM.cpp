// Copyright (C) 2010 Audiokinetic Inc 

#include "stdafx.h"
#include "Platform.h"
#if defined INTEGRATIONDEMO_BGM

#include "DemoBGM.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		// IDs generated by Wwise

#define GAME_OBJECT_RECORDABLE 10
#define GAME_OBJECT_NON_RECORDABLE 20

#if defined AK_PS4 || defined AK_XBOXONE
#define BGM_OUTPUT_TYPE (AkAudioOutputType)(AkOutput_BGM | AkOutput_OptionNotRecordable)
#else
#define BGM_OUTPUT_TYPE AkOutput_MergeToMain
#endif


/// DemoBGMusic class constructor
DemoBGMusic::DemoBGMusic( Menu& in_ParentMenu ) : Page( in_ParentMenu, "Background Music Demo" )
{
	m_szHelp  = "This demo shows how to setup the background music so the DVR doesn't record it. "
		"This is necessary on platforms that support recording features (DVR) and have a TCR to enforce the proper use of licensed music. "
		"Both streams will be muted when the OS-provided music player starts.";
}

/// Initializes the demo.
/// \return True if successful and False otherwise.
bool DemoBGMusic::Init()
{
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "BGM.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "BGM.bnk" );
		return false;
	}

	//Add a secondary output tied to the BGM endpoint of the console.
	//This output will be tied to listener #8 (any can be used, as long as no other output uses it)
	AK::SoundEngine::AddSecondaryOutput(0 /*Ignored for BGM*/, BGM_OUTPUT_TYPE, 0x80 /*Use the listener #8 (bit mask)*/);

	// In order to show the difference between a recordable sound and a non-recordable sound, let's set up 2 game objects.
	// Register the "Recordable music object" game object.  
	AK::SoundEngine::RegisterGameObj( GAME_OBJECT_RECORDABLE, "Recordable music" );
	// Register the "Non-recordable music object" game object
	AK::SoundEngine::RegisterGameObj( GAME_OBJECT_NON_RECORDABLE, "Non-recordable music" );
	//Make the non-recordable object emit sound only to listener #8.  Nothing to do on the other object as by default everything is output to the main output, and is recordable.
	AK::SoundEngine::SetActiveListeners(GAME_OBJECT_NON_RECORDABLE, 0x80);

	m_bPlayLicensed = false;
	m_bPlayCopyright = false;

	// Initialize the page
	return Page::Init();
}

/// Releases resources used by the demo.
void DemoBGMusic::Release()
{
	AK::SoundEngine::UnregisterGameObj(GAME_OBJECT_RECORDABLE);
	AK::SoundEngine::UnregisterGameObj(GAME_OBJECT_NON_RECORDABLE);
	AK::SoundEngine::UnloadBank("BGM.bnk", NULL);

	AK::SoundEngine::RemoveSecondaryOutput(0 /*Ignored for BGM*/, BGM_OUTPUT_TYPE);
}

void DemoBGMusic::InitControls()
{
	//UI stuff.
	ButtonControl* newBtn;

	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Play recordable music" );
	newBtn->SetDelegate( (PageMFP)&DemoBGMusic::Recordable_Pressed );
	m_Controls.push_back( newBtn );

	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Play non-recordable music" );
	newBtn->SetDelegate( (PageMFP)&DemoBGMusic::NonRecordable_Pressed );
	m_Controls.push_back( newBtn );
}

void DemoBGMusic::Recordable_Pressed( void*sender, ControlEvent* )
{
	if (m_bPlayLicensed)
	{
		AK::SoundEngine::StopAll(GAME_OBJECT_RECORDABLE);
		m_bPlayLicensed = false;
		((ButtonControl*)sender)->SetLabel( "Play recordable music" );
	}
	else
	{
		// Plays the music on the game object linked to the main output.
		AK::SoundEngine::PostEvent("Play_RecordableMusic", GAME_OBJECT_RECORDABLE);
		m_bPlayLicensed = true;
		((ButtonControl*)sender)->SetLabel( "Stop" );
	}
}

void DemoBGMusic::NonRecordable_Pressed( void*sender, ControlEvent*)
{
	if (m_bPlayCopyright)
	{
		AK::SoundEngine::StopAll(GAME_OBJECT_NON_RECORDABLE);
		m_bPlayCopyright = false;
		((ButtonControl*)sender)->SetLabel( "Play non-recordable music" );
	}
	else
	{
		// Plays the non-recordable music on the game object linked to the listener that outputs on the BGM end-point.
		AK::SoundEngine::PostEvent("Play_NonRecordableMusic", GAME_OBJECT_NON_RECORDABLE);
		m_bPlayCopyright = true;
		((ButtonControl*)sender)->SetLabel( "Stop" );
	}
}

void DemoBGMusic::Draw()
{
	Page::Draw();
	DrawTextOnScreen(m_szHelp.c_str(), 70, 300, DrawStyle_Text);
}

#endif
