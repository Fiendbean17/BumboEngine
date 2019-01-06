#include "CharacterBase.h"
#include <string>

#ifndef CHR_ALLMIGHT_H
#define CHR_ALLMIGHT_H

class Chr_AllMight : public CharacterBase
{
public:
	Chr_AllMight(int center_position_x, int center_position_y, int unique_object_ID, WorldSprite world_sprite, char direction, PlayerDefinition &player, int screen_width, int screen_height, Matrix &world_matrix, std::vector<std::vector<std::pair<int, int>>> &element_has_object, Matrix &screen_matrix, BitmapDefinition &image_file_path,
		// START CONFIGURABLE VARIABLES HERE -------------------------------------------------


		// Character will attack player immediatly, upon seeing them.
		bool attack_on_sight = false,

		// TRUE -> Basic Popup dialog | FALSE -> Advanced Dialog with player choices
		bool use_basic_dialog = false,
		// Basic Popup Dialog (Only used if use_basic_dialog == TRUE)
		PopupDefinition popup_sprite = PopupDefinition("This is my only dialog! Hello", 'X', 23, 9),


		// Basic ASCII Example (The all Might face)
		/*BossFightDefinition boss_fight_definition = BossFightDefinition(
			22, // boss health
			40, // his smile/eyes (overlay) offset X position
			20, // his smile/eyes (overlay) offset Y position
			"ALL MIGHT", 
			// ASCII Art for All Might
			"            ,,#,@@@@@,@,*@@@@,*#@@@%             Z           ,,,,,,,@@(&,@@@@@@@@@@@@@%,           Z          @,#,,,,,,,,,,,@,%@@@@@&,,@@@           Z         ,,,@@,,,,,,,,,,,,%@@@@@@,,@@@@          Z         @@,#,,,,,,,,,,,,,%@@@@@@,,,@@@          Z         @@@,,,,,,,,,,,,,,@@@@@@@@,,,@@          Z        ,,@,,,,,,,,,,,,,,,@@@@@@@@,,,@@@         Z        ,@@,,&,&,@,,,.,,,@@@@@,@,@,,,,@@         Z       ,@#,,,,,@@&,@,#(%,,@,&@@@@@@,,,@@@        Z        ,@,,,,,%@@@@@,,/#*@@@@@@@@,,,,@@@        Z       ,@#,,,,,,,(@@@%,@@@@@@@@@@@@,,,*@,,       Z      ,,@,,,,,,,,,,,,,,,,@@@,,*@@@@@,,,,@,       Z      ,@,,,,,,,,,,,,,,,,,@@@,,,,@@@@,,,@,,       Z      @,*,#,,,,,,,,,,,,,,@@@@,,@@@@@@,@@,,       Z      ,@@,,@,@,,,,,,@@@@@@@@@&,@@@@@@,,@,,       Z      ,,@,,,@,,,,&,,,,,@@@@/,@@@@@@@@,@@,.       Z       ,,@,,,,,@,,,,,,,,,,,,,,(@@@@@,,@%,,       Z      ,&,,,,,,@,,,,,,,,,,,,,,,,,,@@@,,,,@@,&     Z     ,,,@@,,,,,@@%@**,,,,,,,,,,,@@@@,,@@@@  .    Z   @@,,@,@#,,/,,@&,,,,,,,,,,,,@,@@@@,,@@@@@      Z    ,(@@,,@,,@,,,&,,,,,,,,,,,,@@@@@@,,@@.@@@@,   Z   ,@@@@,,/,,@@,,,,,,,,,,,,@@@@@@@@@,.@@,@@@@@@  Z &@@@@@@,,,@,,,,,,,,,,,,,@@@@@@@@@@@,@@@&@@(     Z@@@@ @@(,,,,@@,,,,,,,,,,,@@@@@@@@@,@@@@@@*@@@@,  Z    @@@,,,,,,,@@,,,,,,,,,@@@@@@@,@@@@@@@@,       Z      @,,,,,,,,,,@@,,,,,*@@@@%,@@@@@@@@@@@%      Z       @@,,,,,,,,,,@@@@@@@@@@@@@@@@@@@@@@@       Z     @@@@@@@@/,,,,,,@@@@@@@@@@@@@@@@@@@@@@@@     ",
			// ASCI Art for his smile and his eyes (The overlay)
			"X*XXXXX________XXXXXXZ,X*  --        -- *X,Z,,X*   --------  *X,,Z,,,,X**        **X,,,Z,,,,,,,XXXXXXXX,,,,,,"
		),//*/

		// Advanced ASCII (Highly detailed) and read from a file as a screenshot/image
		BossFightDefinition boss_fight_definition = BossFightDefinition(
			1,
			"ALL MIGHT", 
			"bonny_neutral.bmp", 
			"bonny_neutral.bmp", 
			"bonny_neutral.bmp",
			"bonny_neutral.bmp"
		),

		/* Use Event at end of battle (Whether slay or spare is called) | Must match ID of an actual event in the events folder */
		int event_ID = 0) // 0 = no event

		: CharacterBase(center_position_x, center_position_y, popup_sprite, unique_object_ID, world_matrix, element_has_object, screen_matrix, screen_width, screen_height, event_ID, player, boss_fight_definition, attack_on_sight, use_basic_dialog, image_file_path, world_sprite)
	{
		faceDirection(direction);

		// (In-Battle) Dialog:		( player dialog choice; boss's response; should progress dialog? )
		std::vector<std::tuple<std::string, std::string, bool>> dialog_choice_1;
		dialog_choice_1.push_back(std::make_tuple("HELLO WORLD 1.1", "1.1 NO", false));
		dialog_choice_1.push_back(std::make_tuple("HELLO WORLD 1.2", "1.2 NO", false));
		dialog_choice_1.push_back(std::make_tuple("HELLO WORLD 1.C", "1.C YES", true));
		dialog_choice_1.push_back(std::make_tuple("HELLO WORLD 1.4", "1.4 NO", false));

		std::vector<std::tuple<std::string, std::string, bool>> dialog_choice_2;
		dialog_choice_2.push_back(std::make_tuple("HELLO WORLD 2.1", "2.1 NO", false));
		dialog_choice_2.push_back(std::make_tuple("HELLO WORLD 2.2", "2.2 NO", false));
		dialog_choice_2.push_back(std::make_tuple("HELLO WORLD 2.3", "2.3 NO", false));
		dialog_choice_2.push_back(std::make_tuple("HELLO WORLD 2.C", "2.C YES", true));

		std::vector<std::tuple<std::string, std::string, bool>> dialog_choice_3;
		dialog_choice_3.push_back(std::make_tuple("HELLO WORLD 3.C", "3.C YES", true));
		dialog_choice_3.push_back(std::make_tuple("HELLO WORLD 3.2", "3.2 NO", false));
		dialog_choice_3.push_back(std::make_tuple("HELLO WORLD 3.3", "3.3 NO", false));
		dialog_choice_3.push_back(std::make_tuple("HELLO WORLD 3.4", "3.4 NO", false));

		dialog_choices_.push_back(dialog_choice_1);
		dialog_choices_.push_back(dialog_choice_2);
		dialog_choices_.push_back(dialog_choice_3);

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
	void initializeAttackPatterns(int screen_width, int screen_height, Matrix &screen_matrix, PlayerDefinition &player)
	{
		AttackPatternBase *attack_pattern_1;
		attack_pattern_1 = new AttackPattern_Wall(screen_width, screen_height, screen_matrix, player, 10, 5, 1);
		AttackPatternBase *attack_pattern_2;
		attack_pattern_2 = new Explode_Slow(screen_width, screen_height, screen_matrix, player, 200);
		AttackPatternBase *attack_pattern_3;
		attack_pattern_3 = new Explode_Slowest(screen_width, screen_height, screen_matrix, player, 50);
		AttackPatternBase *attack_pattern_4;
		attack_pattern_4 = new AttackPattern_ShootHorizontal(screen_width, screen_height, screen_matrix, player, 10);
		AttackPatternBase *attack_pattern_5;
		attack_pattern_5 = new AttackPattern_ShootAtPlayer(screen_width, screen_height, screen_matrix, player, 10); //50
		AttackPatternBase *attack_pattern_6;
		attack_pattern_6 = new VerticleGap_VerySlow(screen_width, screen_height, screen_matrix, player);
		AttackPatternBase *attack_pattern_7;
		attack_pattern_7 = new AttackPattern_Snake(screen_width, screen_height, screen_matrix, player, 1);
		AttackPatternBase *attack_pattern_8;
		attack_pattern_8 = new ShootandExplode_Fast(screen_width, screen_height, screen_matrix, player, 10);
		AttackPatternBase *attack_pattern_9;
		attack_pattern_9 = new ShootandExplode_Slow(screen_width, screen_height, screen_matrix, player, 10);
		AttackPatternBase *attack_pattern_10;
		attack_pattern_10 = new AttackPattern_ShootandSnake(screen_width, screen_height, screen_matrix, player, 10);
		attack_patterns_.push_back(attack_pattern_1);
		attack_patterns_.push_back(attack_pattern_8);
		attack_patterns_.push_back(attack_pattern_9);
		attack_patterns_.push_back(attack_pattern_10);
		//attack_patterns_.push_back(attack_pattern_3);
		/*attack_patterns_.push_back(attack_pattern_3);
		attack_patterns_.push_back(attack_pattern_2);
		attack_patterns_.push_back(attack_pattern_1);
		attack_patterns_.push_back(attack_pattern_5);
		attack_patterns_.push_back(attack_pattern_3);
		attack_patterns_.push_back(attack_pattern_4);
		attack_patterns_.push_back(attack_pattern_5);
		attack_patterns_.push_back(attack_pattern_6);
		attack_patterns_.push_back(attack_pattern_7);//*/
	}

	/* Advanced Dialog	(Shows multiple text screens with dialog options. Leave BLANK for minor characters) */
	void setDialogNodes()
	{
		Item health_potion("Bottle o' syrup", 1);

		/* ACTIONS (Mini-Tutorial)
		*	"FIGHT"		Will start a battle with the NPC
		*	item		Including an Item will have the NPC give the player the provided item
		*	"SAVE"		Will Save the current position in the dialog. So if the player exits the
		*				dialog and re-opens it, the conservsation will start at the "SAVE"d dialog choice
		*/

		// CREATE DIALOG NODES
		DialogNode *node_1 = new DialogNode("", "Oh hello, heh he heh fancy seeing you here");
		DialogNode *node_1_1 = new DialogNode("Whats your name?", "All MIGHT! Can you guess why?");
		DialogNode *node_1_2 = new DialogNode("Want to hear a joke?", "Sure! I love me a good joke!");
		DialogNode *node_1_3 = new DialogNode("Goodbye...", "SEE YOU LATER!");
		DialogNode *node_1_1_1 = new DialogNode("No", "You irritated me! Let's fight!!!!", "FIGHT");
		DialogNode *node_1_1_2 = new DialogNode("Because you're ALL MIGHT", "You guess it! Here, have my most valuble possession!", health_potion);
		DialogNode *node_1_1_3 = new DialogNode("*sigh* and walk away", "Hey! where are you going!!!?");
		DialogNode *node_1_2_1 = new DialogNode("Whats smiling and red?", "I dont know... What?");
		DialogNode *node_1_2_2 = new DialogNode("Too bad! ha ha ha!", "YOU DARE!!! DIE!! DIE!!! DIEE!!!", "FIGHT");
		DialogNode *node_1_2_3 = new DialogNode("YOUR EXPRESSION! get it?", "I will never speak with you again");
		DialogNode *node_1_2_1_1 = new DialogNode("YOUR FACE WHEN I BREAK IT", "...", "FIGHT");
		DialogNode *node_1_1_2_1 = new DialogNode("Goodbye", "Back again I see. Sorry, I only had the one item...", "SAVE");
		DialogNode *node_1_1_1_1 = new DialogNode("Goodbye", "I lost", "SAVE");

		// Link Dialog Nodes
		node_1->setChoice1(node_1_1);
		node_1->setChoice2(node_1_2);
		node_1->setChoice3(node_1_3);

		node_1_1->setChoice1(node_1_1_1);
		node_1_1->setChoice2(node_1_1_2);
		node_1_1->setChoice3(node_1_1_3);

		node_1_2->setChoice1(node_1_2_1);
		node_1_2->setChoice2(node_1_2_2);
		node_1_2->setChoice3(node_1_2_3);

		node_1_2_1->setChoice1(node_1_2_1_1);
		node_1_1_2->setChoice1(node_1_1_2_1);
		node_1_1_1->setChoice1(node_1_1_1_1);

		setHeadNode(node_1);
	}
};

#endif // !CHR_ALLMIGHT_H
