#pragma once
#ifndef CHR_QRBADGUY2_H
#define CHR_QRBADGUY2_H

#include "CharacterBase.h"

// Somewhat difficult boss. Someone who player thinks may have stolen package. Is innocent
class Chr_QRBadGuy2 : public CharacterBase
{
public:
	Chr_QRBadGuy2(int center_position_x, int center_position_y, int unique_object_ID, WorldSpriteContainer world_sprite, char direction, PlayerDefinition& player, int screen_width, int screen_height, Matrix& world_matrix, std::vector<std::vector<std::pair<int, int>>>& element_has_object, Matrix& screen_matrix, BitmapDefinition& bitmap, AudioDefinition& audio,
		// START CONFIGURABLE VARIABLES HERE -------------------------------------------------


		// Character will attack player immediatly, upon seeing them.
		bool attack_on_sight = false,

		// TRUE -> Basic Popup dialog | FALSE -> Advanced Dialog with player choices
		bool use_basic_dialog = false,
		// Basic Popup Dialog (Only used if use_basic_dialog == TRUE)
		PopupDefinition popup_sprite = PopupDefinition("This is my onlyZdialog! helloZ", 'X', 23, 9),


		// Advanced ASCII (Highly detailed) and read from a file as a screenshot/image
		BossFightDefinition boss_fight_definition = BossFightDefinition(
			11,
			"QRCode Guy",
			"Led_Zeppelin_-_Achilles_Last_Stand.mp3",
			666,
			666,
			666,
			666
		),//*/

		/* Use Event at end of battle (Whether slay or spare is called) | Must match ID of an actual event in the events folder */
		int event_ID = 3) // 0 = no event

		: CharacterBase(center_position_x, center_position_y, popup_sprite, unique_object_ID, world_matrix, element_has_object, screen_matrix, screen_width, screen_height, event_ID, player, boss_fight_definition, attack_on_sight, use_basic_dialog, bitmap, audio, world_sprite)
	{
		faceDirection(direction);

		// (In-Battle) Dialog:		( player dialog choice; boss's response; should progress dialog? )
		std::vector<std::tuple<std::string, std::string, bool>> dialog_choice_1;
		dialog_choice_1.push_back(std::make_tuple("WHAT ARE YOU?", "COMMAND NOT RECOGNIZED", false));
		dialog_choice_1.push_back(std::make_tuple("Can I Scan your QR Code?", "NO4C ATTENDEEEEEEEESSS MAY SCAN QR CODES", false));

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
		AttackPatternBase* attack_pattern_1 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 75, 250, 50, 'r', 1, 'd', false, false, 0); // [EASY]
		AttackPatternBase* attack_pattern_2 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 75, 250, 50, 'l', 0, ' ', false, false, 0); // [EASY]
		AttackPatternBase* attack_pattern_3 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 50, 50, 100, 'r', 1, 'u', false, false, 0); // [EASY] --SINGLECOLUMN
		AttackPatternBase* attack_pattern_4 = new AttackPattern_Wall(screen_width, screen_height, screen_matrix, player, 20, 9, 1, 500); // [Medium] FAST
		AttackPatternBase* attack_pattern_5 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 50, 350, 20, 'u', 0, ' ', false, false, 0); // [EASY] --FAST
		AttackPatternBase* attack_pattern_6 = new VerticleGap_VeryFast(screen_width, screen_height, screen_matrix, player); // [Medium] Fast
		AttackPatternBase* attack_pattern_7 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 75, 150, 50, 'l', 1, 'u', false, false, 0); // [MEDIUM]
		AttackPatternBase* attack_pattern_8 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 75, 150, 50, 'r', 0, ' ', false, false, 0); // [MEDIUM]
		AttackPatternBase* attack_pattern_9 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 50, 250, 20, 'd', 1, 'l', false, false, 0); // [MEDIUM] --FAST
		AttackPatternBase* attack_pattern_10 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 75, 50, 100, 'l', 0, ' ', false, false, 0); // [MEDIUM] --SINGLECOLUMN
		AttackPatternBase* attack_pattern_11 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 200, 75, 50, 'd', 0, ' ', false, false, 0); // [HARD]
		AttackPatternBase* attack_pattern_12 = new VerticleGap_Wavy(screen_width, screen_height, screen_matrix, player); // [Hard] Wavy
		AttackPatternBase* attack_pattern_13 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 150, 120, 25, 'l', 1, 'u', false, false, 0); // [HARD] --FAST
		AttackPatternBase* attack_pattern_14 = new AttackPattern_CoordinatedStorm(screen_width, screen_height, screen_matrix, player, 200, 50, 50, 'u', 1, ' ', false, false, 0); // [Medium] Symmetrical
		AttackPatternBase* attack_pattern_15 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 150, 120, 25, 'r', 1, 'd', false, false, 0); // [HARD] --FAST
		AttackPatternBase* attack_pattern_16 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 100, 30, 100, 'l', 1, 'd', false, false, 0); // [HARD] --SINGLECOLUMN
		AttackPatternBase* attack_pattern_17 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 200, 75, 50, 'd', 1, 'r', false, false, 0); // [HARD]
		AttackPatternBase* attack_pattern_18 = new AttackPattern_HailStorm(screen_width, screen_height, screen_matrix, player, 400, 40, 250, 'r', 1, 'd', false, false, 0); // [INSANE] --SINGLECOLUMN

		attack_patterns_.push_back(attack_pattern_18);
		attack_patterns_.push_back(attack_pattern_17);
		attack_patterns_.push_back(attack_pattern_16);
		attack_patterns_.push_back(attack_pattern_15);
		attack_patterns_.push_back(attack_pattern_14);
		attack_patterns_.push_back(attack_pattern_13);
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
		DialogNode* node_1 = new DialogNode("", "Hello NO%C Attendee. I see you haven't scanned my QR Code yet.");
		DialogNode* node_1_1 = new DialogNode("*SCAN HIS QR CODE*", "You've scanned *TWO* QR Codes. Congratulations!");
		DialogNode* node_1_1_1 = new DialogNode("Thanks...", "NOW IT IS MY TURN TO SCAN YOUR QR CODE!", "FIGHT");
		DialogNode* node_1_2 = new DialogNode("*Refuse*", "XXXXXXXXXXX XXXXXXXXXXXX XXXXXXXXX XXXXXXXX", "FIGHT");

		// Link Dialog Nodes
		node_1->setChoice1(node_1_1);
		node_1_1->setChoice1(node_1_1_1);
		node_1->setChoice2(node_1_2);

		setHeadNode(node_1);
	}
};

#endif // !CHR_QRBADGUY2_H



