#pragma once
#ifndef CHR_QRBADGUY3_H
#define CHR_QRBADGUY3_H

#include "CharacterBase.h"

// Final Boss, very difficult boss fight
class Chr_QRBadGuy3 : public CharacterBase
{
public:
	Chr_QRBadGuy3(int center_position_x, int center_position_y, int unique_object_ID, WorldSpriteContainer world_sprite, char direction, PlayerDefinition& player, int screen_width, int screen_height, Matrix& world_matrix, std::vector<std::vector<std::pair<int, int>>>& element_has_object, Matrix& screen_matrix, BitmapDefinition& bitmap, AudioDefinition& audio,
		// START CONFIGURABLE VARIABLES HERE -------------------------------------------------


		// Character will attack player immediatly, upon seeing them.
		bool attack_on_sight = false,

		// TRUE -> Basic Popup dialog | FALSE -> Advanced Dialog with player choices
		bool use_basic_dialog = false,
		// Basic Popup Dialog (Only used if use_basic_dialog == TRUE)
		PopupDefinition popup_sprite = PopupDefinition("This is my only dialog! hello ", 'X', 23, 9),


		// Advanced ASCII (Highly detailed) and read from a file as a screenshot/image
		BossFightDefinition boss_fight_definition = BossFightDefinition(
			13,
			"QRCode Guy",
			"Peter_Schilling_-_Major_Tom.mp3",
			666,
			666,
			666,
			666
		),

		/* Use Event at end of battle (Whether slay or spare is called) | Must match ID of an actual event in the events folder */
		int event_ID = 10022) // 0 = no event

		: CharacterBase(center_position_x, center_position_y, popup_sprite, unique_object_ID, world_matrix, element_has_object, screen_matrix, screen_width, screen_height, event_ID, player, boss_fight_definition, attack_on_sight, use_basic_dialog, bitmap, audio, world_sprite)
	{
		faceDirection(direction);

		// (In-Battle) Dialog:		( player dialog choice; boss's response; should progress dialog? )
		std::vector<std::tuple<std::string, std::string, bool>> dialog_choice_1;
		dialog_choice_1.push_back(std::make_tuple("We can talk this out", "COMMAND NOT RECOGNIZED! TRY ASKING AGAIN!", false));

		dialog_choices_.push_back(dialog_choice_1);

		/* Just a little check to make sure you typed the above code correctly.
		* This will throw an exception if you added more than more dialog choices
		* Remember! Vector Size cannot be greater than 4! (always 4 dialog options at once) */
#ifdef _DEBUG
		for (auto dialog_choice : dialog_choices_)
			if (dialog_choice.size() > 4)
				throw "dialog_choice size must not be greater than 4! There can only be 4 dialog options at a time";
#endif
	}

	/* Creates all attacks */
	void initializeAttackPatterns(int screen_width, int screen_height, Matrix& screen_matrix, PlayerDefinition& player)
	{
		AttackPatternBase* attack_pattern_1 = new SafeSquares_Slowest(screen_width, screen_height, screen_matrix, player, 3); // [Easy-Hard] Slowest
		AttackPatternBase* attack_pattern_2 = new AttackPattern_SnakeHailStorm(screen_width, screen_height, screen_matrix, player, 150, 75, 175, 'l', 1, 'd', false, false, 0, 25000); // [INSANE]
		AttackPatternBase* attack_pattern_3 = new AttackPattern_Snake(screen_width, screen_height, screen_matrix, player, 5, 13000, 100); // [INSANE] 5
		AttackPatternBase* attack_pattern_4 = new AttackPattern_Wall(screen_width, screen_height, screen_matrix, player, 10, 7, 100, 800); // [Hard] CLUMP
		AttackPatternBase* attack_pattern_5 = new VerticleGap_VeryFast(screen_width, screen_height, screen_matrix, player); // [Medium] Fast
		AttackPatternBase* attack_pattern_6 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 200, 75, 50, 'l', 1, 'u', true, false, 16000); // [HARD]
		AttackPatternBase* attack_pattern_7 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 400, 40, 250, 'r', 1, 'd', false, false, 0); // [INSANE] --SINGLECOLUMN
		AttackPatternBase* attack_pattern_8 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 200, 75, 15, 'r', 0, ' ', false, false, 0); // [HARD]
		AttackPatternBase* attack_pattern_9 = new ShootExplode_Fast(screen_width, screen_height, screen_matrix, player, 10); // [Hard]
		AttackPatternBase* attack_pattern_10 = new VerticleGap_Wavy(screen_width, screen_height, screen_matrix, player); // [Hard] Wavy
		AttackPatternBase* attack_pattern_11 = new AttackPattern_CoordinatedStorm(screen_width, screen_height, screen_matrix, player, 400, 50, 100, 'l', 1, 'd', false, false, 1000); // [INSANE] Rows
		AttackPatternBase* attack_pattern_12 = new AttackPattern_Snake(screen_width, screen_height, screen_matrix, player, 1, 45000, 100); // [INSANE] Long

		attack_patterns_.push_back(attack_pattern_12);
		attack_patterns_.push_back(attack_pattern_11);
		attack_patterns_.push_back(attack_pattern_10);
		attack_patterns_.push_back(attack_pattern_9);
		attack_patterns_.push_back(attack_pattern_8);
		attack_patterns_.push_back(attack_pattern_7);
		attack_patterns_.push_back(attack_pattern_6);
		attack_patterns_.push_back(attack_pattern_5);
		attack_patterns_.push_back(attack_pattern_4);
		attack_patterns_.push_back(attack_pattern_3);
		attack_patterns_.push_back(attack_pattern_2);
		attack_patterns_.push_back(attack_pattern_1);
	}

	/* Advanced Dialog	(Shows multiple text screens with dialog options. Leave BLANK for minor characters) */
	void setDialogNodes()
	{
		/* ACTIONS (Mini-Tutorial)
		*	"FIGHT"		Will start a battle with the NPC
		*	item		Including an Item will have the NPC give the player the provided item
		*	"SAVE"		Will Save the current position in the dialog. So if the player exits the
		*				dialog and re-opens it, the conservsation will start at the "SAVE"d dialog choice
		*/

		// CREATE DIALOG NODES
		DialogNode* node_1 = new DialogNode("", "ALL NO4C ATTENDEES MUST REACH THE MINIMUM SCAN SCORE");
		DialogNode* node_1_1 = new DialogNode("...", "AND YOUR SCORE IS... INSUFFICIENT", "FIGHT");

		node_1->setChoice1(node_1_1);

		setHeadNode(node_1);
	}

	// Sets ANYTHING you want about a specific character (If more than 1 character uses this, try refactoring your code)
	void setUniqueAttributes()
	{
		setPersistent();
	}
};

#endif // !CHR_QRBADGUY3_H




