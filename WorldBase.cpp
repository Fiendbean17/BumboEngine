#include "WorldBase.h"

WorldBase::WorldBase(int screen_width, int screen_height, int world_width, int world_height, int starting_position_x, int starting_position_y, PlayerDefinition &player, Matrix &screen_matrix, Inventory &inventory, BitmapDefinition &bitmap, AudioDefinition &audio, std::string directory)
	: screen_width_{ screen_width }, screen_height_{ screen_height }, world_width_{ world_width }, world_height_{ world_height }, start_time_player_speed_(0), element_has_object_(world_height, std::vector<std::pair<int, int>>(world_width, std::make_pair<int, int>(0, 0))),
	world_matrix_(world_width, world_height), screen_matrix_{ screen_matrix }, player_{ player }, player_sprite_{ 12, 10, screen_matrix }, player_speed_modifier_(30), inventory_{ inventory }, DEBUG_has_initialized_{ false }, audio_{ audio },
	DEBUG_showing_collisions_{ false }, opposite_player_direction_('d'), should_enter_battle_{ false }, is_event_active_{ false }, bitmap_{ bitmap }, enter_key_pressed_{ false }, should_roll_credits_{ false }
{
	screen_position_.x = starting_position_x - screen_width / 2;
	screen_position_.y = starting_position_y - screen_height / 2;

	audio_.setDirectory(directory);
	generateWorld();
	player_sprite_.initializeSprites(SpriteSheet::player);
}

// Runs once whenever the player enters the world (Like when exiting a battle or the inventory)
void WorldBase::onEnterWorld()
{
	start_time_player_speed_ = GetTickCount64();
	should_enter_battle_ = false;
	shouldDespawnCharacter();
}

// Calls every frame
void WorldBase::refreshScreen()
{
	if (is_viewing_popup_ && getFacingEntity() != std::make_pair<int, int>(0, 0)) // Called onInteract (Press E)
	{
		switch (getFacingEntity().first)
		{
		case 1: // Signpost
			for (Signpost *signpost : signposts_)
				if (signpost->getUniqueObjectID() == getFacingEntity().second)
					signpost->refreshPopup();
			break;
		case 2: // Pickup
			for (Pickup *pickup : pickups_)
				if (pickup->getUniqueObjectID() == getFacingEntity().second)
				{
					pickup->refreshPopup();
					pickup->pickupItem();
				}
			break;
		case 3: // Character
			for (CharacterBase *character : characters_)
				if (character->getUniqueObjectID() == getFacingEntity().second)
				{
					if (character->useBasicDialog()) // Character uses basic dialog popup
					{
						displayScreen();
						character->refreshPopup();
					}
					else // Character uses advanced dialog system
					{
						selected_character_ = character;
						selected_character_->showDialog();
					}
					character->faceDirection(opposite_player_direction_);
					player_sprite_.setMoving("not verticle");
					player_sprite_.setMoving("not horizontal");
				}
			is_viewing_popup_ = false;
			break;
		default:
			is_viewing_popup_ = false;
			break;
		}
	}
	else
	{
		checkForItem();
		checkForBattle();
		checkForEvent();
		displayScreen();

		if (is_event_active_) // EVENTS
		{
			selected_event_->refreshEvent();
			if (selected_event_->shouldEnterBattle())
			{
				selected_character_ = selected_event_->getAttachedCharacter();
				should_enter_battle_ = true;
				selected_character_->stopBattle();
			}
			shouldDespawnCharacter();
			shouldRemoveEvent();
		}
	}
	evaluatePlayerInput();
}

// Returns address to selected character (Either the one fighting player, talking, etc...)
CharacterBase * WorldBase::getSelectedCharacter()
{
	return selected_character_;
}

// Returns true if player collides with something
bool WorldBase::hasCollided(char direction, int offset)
{
#ifdef _DEBUG
	if (DEBUG_mode_enabled_)
		return false;
#endif

	switch (direction)
	{
	case 'u':
		for (int j = 0; j < player_sprite_.getWidth() / 2 + 2; j++)
			if (world_matrix_[screen_position_.y + screen_height_ / 2 - player_sprite_.getHeight() / 2 + offset][screen_position_.x + screen_width_ / 2 - player_sprite_.getWidth() / 2 + 2 + j] != ' ')
				return true;
		return false;
		break;
	case'd':
		for (int j = 0; j < player_sprite_.getWidth() / 2 + 2; j++)
			if (world_matrix_[screen_position_.y + screen_height_ / 2 + player_sprite_.getHeight() / 2 - offset][screen_position_.x + screen_width_ / 2 - player_sprite_.getWidth() / 2 + 2 + j] != ' ')
				return true;
		return false;
		break;
	case 'l':
		for (int i = 3; i < player_sprite_.getHeight() / 2 + 1; i++) // 3: only bottom half of player should collide
			if (world_matrix_[screen_position_.y + screen_height_ / 2 - player_sprite_.getHeight() / 2 + 3 + i][screen_position_.x + screen_width_ / 2 - player_sprite_.getWidth() / 2 + offset] != ' ')
				return true;
		return false;
		break;
	case'r':
		for (int i = 3; i < player_sprite_.getHeight() / 2 + 1; i++) // 3: only bottom half of player should collide
			if (world_matrix_[screen_position_.y + screen_height_ / 2 - player_sprite_.getHeight() / 2 + 3 + i][screen_position_.x + screen_width_ / 2 + player_sprite_.getWidth() / 2 - offset] != ' ')
				return true;
		return false;
		break;
	default:
		return false;
	}
}

// Returns entity data for the object in front of player, whether it be a signpost, item, enemy, etc...
std::pair<int, int> WorldBase::getFacingEntity()
{
	switch (player_sprite_.getDirection())
	{
	case 'u':
		for (int j = -2; j < 2; j++)
			for (int i = 0; i < 2; i++) // Collision on Two Lines
				if (element_has_object_[screen_position_.y + screen_height_ / 2 - 3 + i][screen_position_.x + screen_width_ / 2 + j].first != 0)
					return element_has_object_[screen_position_.y + screen_height_ / 2 - 3 + i][screen_position_.x + screen_width_ / 2 + j];
		break;
	case 'd':
		for (int j = -2; j < 2; j++)
			if (element_has_object_[screen_position_.y + screen_height_ / 2 + 4][screen_position_.x + screen_width_ / 2 + j].first != 0)
				return element_has_object_[screen_position_.y + screen_height_ / 2 + 4][screen_position_.x + screen_width_ / 2 + j];
		break;
	case 'r':
		for (int i = -1; i < 3; i++)
			if (element_has_object_[screen_position_.y + screen_height_ / 2 + i][screen_position_.x + screen_width_ / 2 + 5].first != 0)
				return element_has_object_[screen_position_.y + screen_height_ / 2 + i][screen_position_.x + screen_width_ / 2 + 5];
		break;
	case 'l':
		for (int i = -1; i < 3; i++)
			if (element_has_object_[screen_position_.y + screen_height_ / 2 + i][screen_position_.x + screen_width_ / 2 - 6].first != 0)
				return element_has_object_[screen_position_.y + screen_height_ / 2 + i][screen_position_.x + screen_width_ / 2 - 6];
		break;
	default:
		break;
	}
	return std::pair<int, int>();
}

// Displays World and Player to screen
void WorldBase::displayScreen()
{
	for (int i = 0; i < screen_height_; i++)
	{
		for (int j = 0; j < screen_width_; j++)
		{
			char temp = world_matrix_[screen_position_.y + i][screen_position_.x + j];
			screen_matrix_[i][j] = temp;
			screen_matrix_[i][j].setColor(world_matrix_[screen_position_.y + i][screen_position_.x + j].getRGBA());
		}
	}

	player_sprite_.displaySprite(screen_width_, screen_height_);
	if (selected_character_ != nullptr && selected_character_->shouldShowDialog())
		selected_character_->displayDialogMenu();

#ifdef _DEBUG
	if (DEBUG_mode_enabled_)
		DEBUG_refresh();
#endif // !_DEBUG
}

// Takes input and decides whether to move player
void WorldBase::evaluatePlayerInput()
{
	double current_time_move_player = GetTickCount64() - start_time_player_speed_;

	if (selected_character_ != nullptr && selected_character_->shouldShowDialog()) // Navigating dialog
	{
		if (current_time_move_player >= 300) // Movement UP, DOWN, LEFT, RIGHT
		{
			if (GetAsyncKeyState(VK_RETURN) & 0x8000)
			{
				selected_character_->moveDialogCursor("RETURN");
				start_time_player_speed_ = GetTickCount64();
			}
			else if (GetAsyncKeyState(VK_UP) & 0x8000)
			{
				selected_character_->moveDialogCursor("UP");
				start_time_player_speed_ = GetTickCount64();
			}
			else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			{
				selected_character_->moveDialogCursor("DOWN");
				start_time_player_speed_ = GetTickCount64();
			}
			else if (GetAsyncKeyState(VK_BACK) & 0x8000)
			{
				selected_character_->closeDialog();
			}
		}
	}
	else if (selected_event_ != nullptr && is_event_active_) // Traversing Event Dialog
	{
		if (GetAsyncKeyState(VK_RETURN) & 0x8000)
		{
			if (!enter_key_pressed_)
			{
				selected_event_->progressPopup();
				enter_key_pressed_ = true;
			}
		}
		else
		{
			enter_key_pressed_ = false;
		}
	}
	else // Walking in world
	{
		if (GetAsyncKeyState(0x45) & 0x8000) // Press E
		{
			is_viewing_popup_ = true;
		}
		else
		{
			checkRemovePickup();
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) // Running
			{
				player_speed_modifier_ = 1;
				player_sprite_.setPlayerAnimationSpeed(180);
			}
			else
			{
				player_speed_modifier_ = 30;
				player_sprite_.setPlayerAnimationSpeed(240);
			}
			if (current_time_move_player >= player_speed_modifier_) // Movement UP, DOWN, LEFT, RIGHT
			{
				if (GetAsyncKeyState(VK_UP) & 0x8000)
				{
					if (screen_position_.y > 0 && !hasCollided('u', 5)) // 2
					{
						--screen_position_.y;
						player_sprite_.setMoving("verticle");
					}
					player_sprite_.setDirection('u');
					opposite_player_direction_ = 'd';
					shouldStartEventByLocation();
				}
				else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
				{
					if (screen_position_.y + screen_height_ < world_height_ - 1 && !hasCollided('d', 1))
					{
						++screen_position_.y;
						player_sprite_.setMoving("verticle");
					}
					player_sprite_.setDirection('d');
					opposite_player_direction_ = 'u';
					shouldStartEventByLocation();
				}
				else
					player_sprite_.setMoving("not verticle");
				if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
				{
					if (screen_position_.x + screen_width_ < world_width_ - 1 && !hasCollided('r', 1))
					{
						++screen_position_.x;
						player_sprite_.setMoving("horizontal");
					}
					player_sprite_.setDirection('r');
					opposite_player_direction_ = 'l';
					shouldStartEventByLocation();
				}
				else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
				{
					if (screen_position_.x > 0 && !hasCollided('l', 0))
					{
						--screen_position_.x;
						player_sprite_.setMoving("horizontal");
					}
					player_sprite_.setDirection('l');
					opposite_player_direction_ = 'r';
					shouldStartEventByLocation();
				}
				else
					player_sprite_.setMoving("not horizontal");
				start_time_player_speed_ = GetTickCount64();
			}
			is_viewing_popup_ = false;
		}
	}

#ifdef _DEBUG
	if (GetAsyncKeyState(0x47) & 0x8000) // Open DEBUG Mode!		Press G
		DEBUG_mode_enabled_ = true;
	if (GetAsyncKeyState(0x48) & 0x8000) // Close DEBUG Mode!		Press H
	{
		if (DEBUG_showing_collisions_)
			DEBUG_stopDisplayingCollisions();
		DEBUG_showing_collisions_ = false;
		DEBUG_mode_enabled_ = false;
	}
	if (DEBUG_mode_enabled_)
	{
		if (GetAsyncKeyState(0x43) & 0x8000) // Collisions!				HOLD C
			DEBUG_showing_collisions_ = true;
		else
		{
			if (DEBUG_showing_collisions_)
				DEBUG_stopDisplayingCollisions();
			DEBUG_showing_collisions_ = false;
		}
		if (GetAsyncKeyState(0x54) & 0x8000) // Teleport Player			Press T
			teleportPlayer(1128, 59); // 1048, 210
		else if (GetAsyncKeyState(0x4D) & 0x8000) // Teleport Player    Press M (Teleport to Maze)
			teleportPlayer(353, 130); // Maze: 296, 231
		else if (GetAsyncKeyState(0x46) & 0x8000) // Teleport Player    Press F (Teleport to Main Characters)
			teleportPlayer(1033, 248);
		else if (GetAsyncKeyState(0x50) & 0x8000) // Add Placeholder to Map    Press P (Place Placeholder)
			DEBUG_createPlaceholder(screen_position_.x + screen_width_ / 2,
				screen_position_.y + screen_height_ / 2);
	}
#endif
}

// Teleports player to specified coordinates
void WorldBase::teleportPlayer(int position_x, int position_y)
{
	screen_position_.x = position_x - screen_width_ + 40; // An offset
	screen_position_.y = position_y - screen_height_ + 18;
}

// creates the world
void WorldBase::generateWorld()
{
	GENERATE_Maze();
	GENERATE_OutsideArea();
	GENERATE_WorldBorder();
	GENERATE_AdditionalObjects();
	GENERATE_Enemies();
	GENERATE_NonHostileNPCs();
	GENERATE_Pickups();
	GENERATE_Signposts();
	GENERATE_Events();

	setNPCAttributes();
}

// Checks for a pickup to remove from map (That was already picked up)
void WorldBase::checkRemovePickup()
{
	if (is_viewing_popup_ && getFacingEntity().first == 2)
		for (Pickup *pickup : pickups_)
			if (pickup->getUniqueObjectID() == getFacingEntity().second)
				delete(pickup);
}

// Checks whether or not to give the player an item from dialog
void WorldBase::checkForItem()
{
	if (selected_character_ != nullptr && selected_character_->shouldGiveItem())
	{
		inventory_.addItem(selected_character_->givenItem());
		selected_character_->stopGivingItem();
	}
}

// Checks whether or not to enter battle from dialog or an event
void WorldBase::checkForBattle()
{
	if (selected_character_ != nullptr && selected_character_->shouldEnterBattle())
	{
		should_enter_battle_ = true;
		selected_character_->stopBattle();
	}
}

// Checks whether or not to start an event (Based on if character starts event)
void WorldBase::checkForEvent() 
{
	if (selected_character_ != nullptr && selected_character_->shouldStartEvent())
	{
		shouldStartEventByID(selected_character_->eventID());
	}
}

// Decides whether or not to removes character from array (They are dead). Called after battle has ended in player's favor
void WorldBase::shouldDespawnCharacter()
{
	if (selected_character_ != nullptr)
	{
		if (selected_character_->isDestroyed())
		{
			if (selected_character_->shouldRestart()) // Player died (so they can restart)
			{
				should_enter_battle_ = true;
				inventory_.reset();
				selected_character_->reset();
			}
			else
			{
				shouldStartEventByID(selected_character_->eventID());
				selected_character_->onDespawn();
				{
					auto it = std::find(characters_.begin(), characters_.end(), selected_character_);
					if (it != characters_.end()) { characters_.erase(it); }
				}
				selected_character_ = nullptr;
			}
		}
		else {
			shouldStartEventByID(selected_character_->eventID());
		}
	}
}

// Decides whether or not to remove event from array (Event has ended).
void WorldBase::shouldRemoveEvent()
{
	if (selected_event_ != nullptr)
	{
		if (selected_event_->isComplete())
		{
			if (selected_event_->isRepeatable())
				selected_event_->reset();
			else
			{
				auto it = std::find(events_.begin(), events_.end(), selected_event_);
				if (it != events_.end()) { events_.erase(it); }
			}
			is_event_active_ = false;
			selected_event_ = nullptr;
		}
	}
}

// Decides whether or not to start an event (based on the player's location)
void WorldBase::shouldStartEventByLocation()
{
	if (getFacingEntity().first == 4)
		for (EventBase *event : events_)
			if (event->getUniqueObjectID() == getFacingEntity().second && event->isAvailable())
			{
				player_sprite_.setMoving("not verticle");
				player_sprite_.setMoving("not horizontal");

				event->onStartEvent();
				selected_event_ = event;
				is_event_active_ = true;
			}
}

// Decides whether or not to start an event (based on a given ID number)
void WorldBase::shouldStartEventByID(int event_ID)
{
	if (event_ID == 0)
		return;
	else
		for (EventBase *event : events_)
			if (event_ID == event->getUniqueObjectID())
			{
				player_sprite_.setMoving("not verticle");
				player_sprite_.setMoving("not horizontal");

				event->onStartEvent();
				selected_event_ = event;
				is_event_active_ = true;
			}
}

// Displays all Characters & Gives them an attack_on_sight event (if necessary)
void WorldBase::setNPCAttributes()
{
	for (auto character : characters_)
	{
		character->createWorldSprite();
		if (character->shouldAttackOnSight())
		{
			Event_AttackOnSight *attack_on_sight = new Event_AttackOnSight(character->getUniqueObjectID(), character->getCenterPositionX(), character->getCenterPositionY(), 10, 10, character->getUniqueObjectID(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
			attack_on_sight->createEvent();

			events_.push_back(attack_on_sight);
		}
	}
}

// creates a nice border around the world. Useful for debugging, helps tell if you are building near edge of world
void WorldBase::GENERATE_WorldBorder()
{
	for (int i = 0; i < world_height_; i++)
		world_matrix_[i][0] = 'X';
	for (int i = 0; i < world_height_ - 15; i++)
		world_matrix_[i][world_width_ - 2] = 'X';
	for (int j = 0; j < world_width_; j++)
		world_matrix_[0][j] = 'X';
	for (int j = 0; j < world_width_; j++)
		world_matrix_[world_height_ - 2][j] = 'X';
}

// creates objects for the cliff area outside the maze. This is the start/end of the game
void WorldBase::GENERATE_OutsideArea()
{
	/* Start of Game ---------- */

	// Outside area / map
	Texture mountain(900, 137, SpriteSheet::mountain, world_matrix_);

	// Fence
	Texture fence(1161, 181, SpriteSheet::fence, world_matrix_);

	// Trees
	Texture tree_1(665, 205, SpriteSheet::tree, world_matrix_);
	Texture tree_2(700, 210, SpriteSheet::tree, world_matrix_);
	Texture tree_3(740, 210, SpriteSheet::tree, world_matrix_);
	Texture tree_4(790, 217, SpriteSheet::tree, world_matrix_);
	Texture tree_5(830, 192, SpriteSheet::tree, world_matrix_);
	Texture tree_6(870, 192, SpriteSheet::tree, world_matrix_);
	Texture tree_7(912, 206, SpriteSheet::tree, world_matrix_);
	Texture tree_8(950, 203, SpriteSheet::tree, world_matrix_);
	Texture tree_9(1096, 173, SpriteSheet::tree, world_matrix_);

	// Rocks
	Texture rock_1(685, 240, SpriteSheet::rock, world_matrix_);
	Texture rock_2(715, 222, SpriteSheet::rock, world_matrix_);
	Texture rock_3(780, 239, SpriteSheet::rock, world_matrix_);

	/* End of Game ---------- */

	// Trees
	Texture tree_10(872, 54, SpriteSheet::tree, world_matrix_);
	Texture tree_11(906, 60, SpriteSheet::tree, world_matrix_);
	Texture tree_12(953, 51, SpriteSheet::tree, world_matrix_);
	Texture tree_13(1019, 46, SpriteSheet::tree, world_matrix_);
	Texture tree_14(1044, 43, SpriteSheet::tree, world_matrix_);
	Texture tree_15(1069, 41, SpriteSheet::tree, world_matrix_);
	Texture tree_16(1105, 47, SpriteSheet::tree, world_matrix_);
}

// Creates the walls of the maze as well as objects that should be placed INSIDE the maze
void WorldBase::GENERATE_Maze()
{
	// Maze
	Texture maze_1(300, 137, SpriteSheet::maze_1, world_matrix_);
	Texture maze_2(300, 1, SpriteSheet::maze_2, world_matrix_);
	Texture maze_3(900, 1, SpriteSheet::maze_3, world_matrix_);

	// Rocks
	Texture rock_1(457, 86, SpriteSheet::rock, world_matrix_);
	Texture rock_2(485, 96, SpriteSheet::rock, world_matrix_);
	Texture rock_3(495, 90, SpriteSheet::rock, world_matrix_);
	Texture rock_4(697, 61, SpriteSheet::rock_2, world_matrix_); // Rock Blocking Door
	Texture rock_5(152, 33, SpriteSheet::rock, world_matrix_);
	Texture rock_6(126, 52, SpriteSheet::rock, world_matrix_);
	Texture rock_7(178, 52, SpriteSheet::rock, world_matrix_);
	Texture rock_8(205, 49, SpriteSheet::rock, world_matrix_);
	Texture rock_9(277, 32, SpriteSheet::rock, world_matrix_);
	Texture rock_10(263, 36, SpriteSheet::rock, world_matrix_);
}

// creates NPCs that SHOULD attack (They don't have to at first, but if they attack at any time, but them here)
void WorldBase::GENERATE_Enemies()
{
	// Main Characters
	CharacterBase *tutorial_npc = new Chr_TutorialNPC(838, 225, 1, SpriteSheet::tutorial_NPC, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *aki_final = new Chr_AkiFinal(1146, 59, 10, SpriteSheet::aki, 'r', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	CharacterBase *ryuuko = new Chr_Ryuuko(1023, 259, 11, SpriteSheet::ryuuko, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *aki = new Chr_Aki(1033, 259, 12, SpriteSheet::aki, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *bonny = new Chr_Bonny(1043, 259, 13, SpriteSheet::bonny, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sharktooth = new Chr_Sharktooth(1053, 259, 14, SpriteSheet::sharktooth, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	// Cave (Before Bridge)
	CharacterBase *door_guard_sharktooth = new Chr_DoorGuardSharktooth(297, 179, 38, SpriteSheet::pirate_10, 'd', SpriteSheet::face_pirate_1, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *door_guard_ryuuko = new Chr_DoorGuardRyuuko(354, 164, 16, SpriteSheet::pirate_9, 'd', SpriteSheet::face_pirate_2, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sleeping = new Chr_SleepingPirate(336, 170, 17, SpriteSheet::pirate_15, 'l', SpriteSheet::face_pirate_3, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	// Checkpoint Guard Interrogation
	CharacterBase *guard = new Chr_CheckpointGuard(492, 229, 19, SpriteSheet::checkpoint_guard, 'r', SpriteSheet::face_guard, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	// Final Outside Area
	CharacterBase *mini_boss_1 = new Chr_MiniBoss1(965, 64, 20, SpriteSheet::mini_boss_1, 'l', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *mini_boss_2 = new Chr_MiniBoss2(1055, 56, 21, SpriteSheet::mini_boss_2, 'l', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *thot_patrol_1 = new Chr_ThotPatrol(188, 165, 36, SpriteSheet::thot_patrol, 'r', SpriteSheet::face_thot_patrol, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	tutorial_npc->initializeCharacter();
	aki_final->initializeCharacter();
	aki->initializeCharacter();
	ryuuko->initializeCharacter();
	bonny->initializeCharacter();
	sharktooth->initializeCharacter();

	door_guard_sharktooth->initializeCharacter();
	door_guard_ryuuko->initializeCharacter();
	sleeping->initializeCharacter();
	guard->initializeCharacter();
	mini_boss_1->initializeCharacter();
	mini_boss_2->initializeCharacter();
	thot_patrol_1->initializeCharacter();


	characters_.push_back(tutorial_npc);
	characters_.push_back(aki_final);
	characters_.push_back(aki);
	characters_.push_back(ryuuko);
	characters_.push_back(bonny);
	characters_.push_back(sharktooth);
	characters_.push_back(door_guard_sharktooth);
	characters_.push_back(door_guard_ryuuko);
	characters_.push_back(sleeping);
	characters_.push_back(guard);
	characters_.push_back(mini_boss_1);
	characters_.push_back(mini_boss_2);
	characters_.push_back(thot_patrol_1);

#ifdef _DEBUG
	CharacterBase *test = new Chr_MajorNPC(732, 223, 0, SpriteSheet::player, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	test->initializeCharacter();
	characters_.push_back(test);

	/*CharacterBase *sprite_1 = new Chr_MajorNPC(662, 213, -1, SpriteSheet::pirate_1, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_2 = new Chr_MajorNPC(672, 213, -2, SpriteSheet::pirate_2, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_3 = new Chr_MajorNPC(682, 213, -3, SpriteSheet::pirate_3, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_4 = new Chr_MajorNPC(692, 213, -4, SpriteSheet::pirate_5_monkey, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_5 = new Chr_MajorNPC(702, 213, -5, SpriteSheet::pirate_5_monkey, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_6 = new Chr_MajorNPC(712, 213, -6, SpriteSheet::pirate_6, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_7 = new Chr_MajorNPC(722, 213, -7, SpriteSheet::pirate_7, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_8 = new Chr_MajorNPC(732, 213, -8, SpriteSheet::pirate_8, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_9 = new Chr_MajorNPC(742, 213, -9, SpriteSheet::pirate_9, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_10 = new Chr_MajorNPC(752, 213, -10, SpriteSheet::pirate_10, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_11 = new Chr_MajorNPC(762, 213, -11, SpriteSheet::pirate_11, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_12 = new Chr_MajorNPC(772, 213, -12, SpriteSheet::pirate_12, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_13 = new Chr_MajorNPC(782, 213, -13, SpriteSheet::pirate_13, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_14 = new Chr_MajorNPC(792, 213, -14, SpriteSheet::pirate_14, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_15 = new Chr_MajorNPC(802, 213, -15, SpriteSheet::pirate_15, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_16 = new Chr_MajorNPC(812, 213, -16, SpriteSheet::pirate_16, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_17 = new Chr_MajorNPC(822, 213, -17, SpriteSheet::pirate_17, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_18 = new Chr_MajorNPC(832, 213, -18, SpriteSheet::sharktooth, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_19 = new Chr_MajorNPC(842, 213, -19, SpriteSheet::bonny, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *sprite_20 = new Chr_MajorNPC(852, 213, -20, SpriteSheet::ryuuko, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	
	sprite_1->initializeCharacter();
	sprite_2->initializeCharacter();
	sprite_3->initializeCharacter();
	sprite_4->initializeCharacter();
	sprite_5->initializeCharacter();
	sprite_6->initializeCharacter();
	sprite_7->initializeCharacter();
	sprite_8->initializeCharacter();
	sprite_9->initializeCharacter();
	sprite_10->initializeCharacter();
	sprite_11->initializeCharacter();
	sprite_12->initializeCharacter();
	sprite_13->initializeCharacter();
	sprite_14->initializeCharacter();
	sprite_15->initializeCharacter();
	sprite_16->initializeCharacter();
	sprite_17->initializeCharacter();
	sprite_18->initializeCharacter();
	sprite_19->initializeCharacter();
	sprite_20->initializeCharacter();

	characters_.push_back(sprite_1);
	characters_.push_back(sprite_2);
	characters_.push_back(sprite_3);
	characters_.push_back(sprite_4);
	characters_.push_back(sprite_5);
	characters_.push_back(sprite_6);
	characters_.push_back(sprite_7);
	characters_.push_back(sprite_8);
	characters_.push_back(sprite_9);
	characters_.push_back(sprite_10);
	characters_.push_back(sprite_11);
	characters_.push_back(sprite_12);
	characters_.push_back(sprite_13);
	characters_.push_back(sprite_14);
	characters_.push_back(sprite_15);
	characters_.push_back(sprite_16);
	characters_.push_back(sprite_17);
	characters_.push_back(sprite_18);
	characters_.push_back(sprite_19);
	characters_.push_back(sprite_20);//*/
#endif
}

// creates NPCs that SHOULD NOT attack (They are capable of it, but this section is for NPCs that shouldn't)
void WorldBase::GENERATE_NonHostileNPCs()
{
	// Lowest Empty ID: 52

	// Main Characters
	CharacterBase *aki_passive = new Chr_AkiPassive(296, 212, 22, SpriteSheet::aki, 'u', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	// Border NPCs
	CharacterBase *standing_in_line_1 = new Chr_BackgroundNPC(1027, 205, 2, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Been waitin' in line for five days...", 'X', 23, 9), SpriteSheet::pirate_1, 'u');
	CharacterBase *standing_in_line_2 = new Chr_BackgroundNPC(1034, 202, 3, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Yar har har 'n a bottle o' rum!", 'X', 23, 9), SpriteSheet::pirate_16, ' u');
	CharacterBase *standing_in_line_3 = new Chr_BackgroundNPC(1041, 200, 4, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("I be starvin'... But I don't wants t' leave th' line", 'X', 23, 9), SpriteSheet::pirate_14, 'r');
	CharacterBase *standing_in_line_4 = new Chr_BackgroundNPC(1048, 202, 5, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("STOP SHOVING! Oh... um, ahoy matey!", 'X', 23, 9), SpriteSheet::pirate_12, 'r');
	CharacterBase *standing_in_line_5 = new Chr_BackgroundNPC(1056, 201, 6, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Back o' th' line! Can't ye tell I was here first?", 'X', 23, 9), SpriteSheet::pirate_10, 'r');
	CharacterBase *standing_in_line_6 = new Chr_BackgroundNPC(1016, 210, 7, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Ye're a pirate too? Better get in line!", 'X', 23, 9), SpriteSheet::pirate_8, 'r');

	CharacterBase *boarder_guard_1 = new Chr_BackgroundNPC(1079, 203, 8, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Another pirate... Get in line with the rest!", '#', 23, 9), SpriteSheet::checkpoint_guard, 'l');
	CharacterBase *boarder_guard_2 = new Chr_BackgroundNPC(1071, 214, 9, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Another pirate... Get in line with the rest!", '#', 23, 9), SpriteSheet::checkpoint_guard, 'l');

	CharacterBase *boarder_incident_random = new Chr_BackgroundNPC(1013, 259, 15, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("", 'X', 23, 9), SpriteSheet::pirate_6, 'r');

	// Cave (Before Bridge)
	CharacterBase *bridge_rally_leader = new Chr_BackgroundNPC(354, 104, 24, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Aye, wha' 'tis?", 'X', 23, 9), SpriteSheet::pirate_2, 'd');
	CharacterBase *bridge_rally_1 = new Chr_BackgroundNPC(341, 127, 25, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Wha' are ye lookin' at?", 'X', 23, 9), SpriteSheet::pirate_1, 'u');
	CharacterBase *bridge_rally_2 = new Chr_BackgroundNPC(332, 122, 26, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("I'd be a whole lot happier wit' some rum!", 'X', 23, 9), SpriteSheet::pirate_17, 'u');
	CharacterBase *bridge_rally_3 = new Chr_BackgroundNPC(343, 120, 27, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Grog! I love me some grog!", 'X', 23, 9), SpriteSheet::pirate_16, 'u');
	CharacterBase *bridge_rally_4 = new Chr_BackgroundNPC(362, 125, 28, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("I be sick o' livin' in th' DEEP cave!", 'X', 23, 9), SpriteSheet::pirate_15, 'u');
	CharacterBase *bridge_rally_5 = new Chr_BackgroundNPC(372, 123, 29, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Those Weebs won't know wha' hit 'em! Let's get 'em!", 'X', 23, 9), SpriteSheet::pirate_14, 'u');
	CharacterBase *bridge_rally_6 = new Chr_BackgroundNPC(338, 112, 39, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Do ye reckon we'll win this war?", 'X', 23, 9), SpriteSheet::pirate_13, 'r');
	CharacterBase *bridge_rally_7 = new Chr_BackgroundNPC(329, 115, 40, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Why pirates always gotta wear red, huh?", 'X', 23, 9), SpriteSheet::pirate_12, 'r');
	CharacterBase *bridge_rally_8 = new Chr_BackgroundNPC(318, 120, 41, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Lost me fav'rit piece o' eight yesterday... a real shame.", 'X', 23, 9), SpriteSheet::pirate_11, 'r');
	CharacterBase *bridge_rally_9 = new Chr_BackgroundNPC(320, 112, 42, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Do ye reckon thar's a city gold on th' other side o' th' bridge?", 'X', 23, 9), SpriteSheet::pirate_10, 'r');
	CharacterBase *bridge_rally_10 = new Chr_BackgroundNPC(325, 102, 43, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("I wonder wha' th' CLEAR cave be like...", 'X', 23, 9), SpriteSheet::pirate_9, 'r');
	CharacterBase *bridge_rally_11 = new Chr_BackgroundNPC(335, 105, 44, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("It be time fer th' pirates t' rise up!", 'X', 23, 9), SpriteSheet::pirate_8, 'r');
	CharacterBase *bridge_rally_12 = new Chr_BackgroundNPC(369, 112, 45, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Could really go fer some spiced rum...", 'X', 23, 9), SpriteSheet::pirate_7, 'l');
	CharacterBase *bridge_rally_13 = new Chr_BackgroundNPC(380, 116, 46, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("I used t' know someone who looked like ye, killed that one too.", 'X', 23, 9), SpriteSheet::pirate_6, 'l');
	CharacterBase *bridge_rally_14 = new Chr_BackgroundNPC(390, 119, 47, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Do ye 'ave a cousin named Sven?", 'X', 23, 9), SpriteSheet::pirate_3, 'l');
	CharacterBase *bridge_rally_15 = new Chr_BackgroundNPC(388, 110, 48, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("I'll skewer thar gizzards!", 'X', 23, 9), SpriteSheet::pirate_2, 'l');
	CharacterBase *bridge_rally_16 = new Chr_BackgroundNPC(378, 107, 49, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Yo ho, yo ho 'n a bottle o' rum!", 'X', 23, 9), SpriteSheet::pirate_1, 'l');

	CharacterBase *hold_shift = new Chr_BackgroundNPC(351, 189, 51, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("Heh. Heh. Fighting too hard? Try holding [SHIFT] in combat to slow down.", 'X', 23, 9), SpriteSheet::pirate_3, 'd');

	CharacterBase *apple_salesman = new Chr_AppleSalesman(513, 158, 30, SpriteSheet::pirate_7, 'r', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *everything_salesman = new Chr_EverythingSalesman(525, 158, 31, SpriteSheet::pirate_6, 'l', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *feather_salesman = new Chr_FeatherSalesman(551, 172, 32, SpriteSheet::pirate_12, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *cuban_1 = new Chr_BackgroundNPC(519, 94, 33, SpriteSheet::pirate_11, 'r', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *cuban_2 = new Chr_BackgroundNPC(528, 94, 34, SpriteSheet::pirate_3, 'r', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *pacing = new Chr_PacingPirate(481, 102, 18, SpriteSheet::pirate_8, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	// Checkpoint Guard Interrogation
	CharacterBase *child = new Chr_BackgroundNPC(523, 229, 35, SpriteSheet::pirate_1, 'l', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	// Cave (After Bridge)
	/* ~Left Empty~ */

	// Final Outside Area
	CharacterBase *thot_patrol_2 = new Chr_BackgroundNPC(188, 165, 37, SpriteSheet::thot_patrol, 'r', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *bad_ending_guy = new Chr_BadEndingGuy(198, 165, 50, SpriteSheet::pirate_14, 'r', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	standing_in_line_1->initializeCharacter();
	standing_in_line_2->initializeCharacter();
	standing_in_line_3->initializeCharacter();
	standing_in_line_4->initializeCharacter();
	standing_in_line_5->initializeCharacter();
	standing_in_line_6->initializeCharacter();
	boarder_guard_1->initializeCharacter();
	boarder_guard_1->initializeCharacter();
	boarder_incident_random->initializeCharacter();

	aki_passive->initializeCharacter();
	bridge_rally_leader->initializeCharacter();
	bridge_rally_1->initializeCharacter();
	bridge_rally_2->initializeCharacter();
	bridge_rally_3->initializeCharacter();
	bridge_rally_4->initializeCharacter();
	bridge_rally_5->initializeCharacter();
	bridge_rally_6->initializeCharacter();
	bridge_rally_7->initializeCharacter();
	bridge_rally_8->initializeCharacter();
	bridge_rally_9->initializeCharacter();
	bridge_rally_10->initializeCharacter();
	bridge_rally_11->initializeCharacter();
	bridge_rally_12->initializeCharacter();
	bridge_rally_13->initializeCharacter();
	bridge_rally_14->initializeCharacter();
	bridge_rally_15->initializeCharacter();
	bridge_rally_16->initializeCharacter();
	hold_shift->initializeCharacter();
	apple_salesman->initializeCharacter();
	everything_salesman->initializeCharacter();
	feather_salesman->initializeCharacter();
	cuban_1->initializeCharacter();
	cuban_2->initializeCharacter();
	pacing->initializeCharacter();
	child->initializeCharacter();
	thot_patrol_2->initializeCharacter();
	bad_ending_guy->initializeCharacter();

	characters_.push_back(standing_in_line_1);
	characters_.push_back(standing_in_line_2);
	characters_.push_back(standing_in_line_3);
	characters_.push_back(standing_in_line_4);
	characters_.push_back(standing_in_line_5);
	characters_.push_back(standing_in_line_6);
	characters_.push_back(boarder_guard_1);
	characters_.push_back(boarder_guard_2);
	characters_.push_back(boarder_incident_random);
	characters_.push_back(aki_passive);
	characters_.push_back(bridge_rally_leader);
	characters_.push_back(bridge_rally_1);
	characters_.push_back(bridge_rally_2);
	characters_.push_back(bridge_rally_3);
	characters_.push_back(bridge_rally_4);
	characters_.push_back(bridge_rally_5);
	characters_.push_back(bridge_rally_6);
	characters_.push_back(bridge_rally_7);
	characters_.push_back(bridge_rally_8);
	characters_.push_back(bridge_rally_9);
	characters_.push_back(bridge_rally_10);
	characters_.push_back(bridge_rally_11);
	characters_.push_back(bridge_rally_12);
	characters_.push_back(bridge_rally_13);
	characters_.push_back(bridge_rally_14);
	characters_.push_back(bridge_rally_15);
	characters_.push_back(bridge_rally_16);
	characters_.push_back(hold_shift);
	characters_.push_back(apple_salesman);
	characters_.push_back(everything_salesman);
	characters_.push_back(feather_salesman);
	characters_.push_back(cuban_1);
	characters_.push_back(cuban_2);
	characters_.push_back(pacing);
	characters_.push_back(child);
	characters_.push_back(thot_patrol_2);
	characters_.push_back(bad_ending_guy);
}

// creates all the sign posts (These show popups)
void WorldBase::GENERATE_Signposts()
{
	Signpost *checkpoint_sign_1 = new Signpost(750, 224, 23, 9, 1, "Nakinom Border Checkpoint --------> 0.2 km", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	Signpost *checkpoint_sign_2 = new Signpost(1000, 210, 23, 9, 2, "Nakinom Border Checkpoint", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	Signpost *shift_run = new Signpost(923, 215, 23, 9, 6, "Hold SHIFT to run", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	Signpost *deep_cave = new Signpost(318, 191, 23, 9, 3, "Welcome ye pirates to the DEEP cave", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	Signpost *clear_cave = new Signpost(85, 88, 23, 9, 4, "Welcome to the CLEAR cave", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	Signpost *no_entry = new Signpost(377, 47, 23, 9, 5, "Stop! No Entry beyond this point!", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);

	signposts_.push_back(checkpoint_sign_1);
	signposts_.push_back(checkpoint_sign_2);
	signposts_.push_back(shift_run);
	signposts_.push_back(deep_cave);
	signposts_.push_back(clear_cave);
	signposts_.push_back(no_entry);

	// Displays all sign posts
	for (auto signpost : signposts_)
		signpost->createWorldSprite();
}

// creates all the items
void WorldBase::GENERATE_Pickups()
{
	Item cliff_item("Health Potion", 9, "Restores FULL Health");
	Pickup *cliff_pickup = new Pickup(665, 227, 23, 9, 1, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, cliff_item, inventory_);

	Item item_clue_1("Cigar Box", 3, "Assembled in Cuba : Made in China. Restores 3 Health");
	Pickup *pickup_clue_1 = new Pickup(1130, 212, 23, 9, 10, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_1, inventory_);
	Item item_clue_2("Long Sword", "ATTACKUP", 3, "There's a tag on the side: If lost return to Ryuuko. Attack x3 for one turn");
	Pickup *pickup_clue_2 = new Pickup(1127, 204, 23, 9, 11, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_2, inventory_);
	Item item_clue_3("Feather", 5, "It's a turkey feather cut to mimic a hawk feather: someone was short-changed. Wearing restores 5 health");
	Pickup *pickup_clue_3 = new Pickup(1144, 213, 23, 9, 12, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_3, inventory_);
	Item item_clue_4("Glove", 2, "A single black glove, missing from a pair. Attack x2 for one turn");
	Pickup *pickup_clue_4 = new Pickup(1146, 205, 23, 9, 13, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_4, inventory_);
	Pickup *unobtainable = new Pickup(462, 222, 23, 9, 14, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, Item(), inventory_);

	pickups_.push_back(cliff_pickup);
	pickups_.push_back(pickup_clue_1);
	pickups_.push_back(pickup_clue_2);
	pickups_.push_back(pickup_clue_3);
	pickups_.push_back(pickup_clue_4);
	pickups_.push_back(unobtainable);

	// Displays all pickups
	for (auto pickup : pickups_)
		pickup->createWorldSprite();
}

// creates things that don't fit into any other category
void WorldBase::GENERATE_AdditionalObjects()
{

}

// creates events that trigger cutscenes, battles, enemy_movement, etc...
void WorldBase::GENERATE_Events()
{
	/* Event_Test *test = new Event_Test(9999, 150, 649, 10, 10, element_has_object_, screen_matrix_, characters_, nullptr);
	 * Excluding the test event, Event Unique Object ID's should BEGIN at 10029
	 * Events with ID's 1 - 9998 are reserved for characters that start battles */
	Event_Tutorial *tutorial = new Event_Tutorial(10000, 790, 232, 10, 11, 1, audio_.getDirectory(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_BorderIncident *border_incident = new Event_BorderIncident(10002, 1070, 206, 4, 4, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_RollCredits *roll_credits = new Event_RollCredits(10021, should_roll_credits_, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);

	Event_TeleportPlayer *teleport_to_maze = new Event_TeleportPlayer(10001, 1107, 195, 18, 8, 296, 231, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_to_mountain = new Event_TeleportPlayer(10004, 296, 250, 18, 8, 1107, 202, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_to_sharktooth = new Event_TeleportPlayer(10005, 297, 174, 12, 4, 104, 241, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_from_sharktooth = new Event_TeleportPlayer(10006, 104, 253, 12, 8, 297, 179, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_to_aki = new Event_TeleportPlayer(10007, 393, 21, 10, 8, 679, 87, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_from_aki = new Event_TeleportPlayer(10008, 679, 103, 10, 8, 393, 33, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_SetupEnding *teleport_to_mini_bosses = new Event_SetupEnding(10009, 731, 59, 10, 8, 857, 65, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_MoveNPCIfDefeated *move_doorguard_sharktooth = new Event_MoveNPCIfDefeated(10017, 538, 167, 2, 24, 198, 165, 38, 13, true, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_MoveNPCIfDefeated *move_doorguard_ryuuko = new Event_MoveNPCIfDefeated(10018, 104, 234, 14, 2, 198, 165, 16, 14, true, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_MoveNPCIfDefeated *move_aki = new Event_MoveNPCIfDefeated(10019, 105, 228, 14, 2, 296, 195, 22, 14, true, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_MoveNPCIfDefeated *move_bad_ending_guy = new Event_MoveNPCIfDefeated(10025, 872, 67, 3, 3, 924, 72, 50, 12, false, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_MoveNPC *mov_mini_boss_1 = new Event_MoveNPC(10023, 198, 165, 10, 10, 1007, 'x', 20, 'd', 20, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_, 198, 165);
	Event_MoveNPC *mov_mini_boss_2 = new Event_MoveNPC(10024, 198, 165, 10, 10, 54, 'y', 30, 'd', 21, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportNPC *teleport_doorguard_sharktooth = new Event_TeleportNPC(10026, 198, 165, 10, 10, 198, 165, 38, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_StopAudio *stopTutorialFall = new Event_StopAudio(10027, 886, 224, 6, 6, "falling", 0, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_StopAudio *stopThrowFall = new Event_StopAudio(10028, 497, 199, 6, 6, "falling", 0, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);


	// Inside Cave
	Event_LostDevice *lost_device = new Event_LostDevice(10003, 296, 224, 56, 1, player_.getPlayerName(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_AkiClearCave *aki_clear_cave = new Event_AkiClearCave(10010, 297, 179, 10, 11, player_.getPlayerName(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_BridgeRally *bridge_rally = new Event_BridgeRally(10011, 353, 113, 20, 1, 1, false, world_matrix_, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_BridgeRally2 *bridge_rally2 = new Event_BridgeRally2(10012, 285, 90, 5, 5, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_BridgeRally3 *bridge_rally3 = new Event_BridgeRally3(10013, 215, 85, 5, 5, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_Cubans *cubans = new Event_Cubans(10014, 198, 165, 10, 10, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_ThotPatrol *thot_patrol = new Event_ThotPatrol(10022, 198, 165, 10, 11, player_.getPlayerName(), selected_character_, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_ThrowOffCliff *throw_off_cliff = new Event_ThrowOffCliff(10016, 496, 225, 20, 1, 1, audio_.getDirectory(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_RemoveObject *remove_object = new Event_RemoveObject(10020, 198, 165, 10, 10, 697, 61, 7, 3, 1, false, world_matrix_, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);

	// events_.push_back(test);
	events_.push_back(tutorial);
	events_.push_back(teleport_to_maze);
	events_.push_back(border_incident);
	events_.push_back(lost_device);
	events_.push_back(teleport_to_mountain);
	events_.push_back(teleport_to_sharktooth);
	events_.push_back(teleport_from_sharktooth);
	events_.push_back(teleport_to_aki);
	events_.push_back(teleport_from_aki);
	events_.push_back(move_doorguard_sharktooth);
	events_.push_back(move_doorguard_ryuuko);
	events_.push_back(move_aki);
	events_.push_back(teleport_to_mini_bosses);
	events_.push_back(aki_clear_cave);
	events_.push_back(bridge_rally);
	events_.push_back(bridge_rally2);
	events_.push_back(bridge_rally3);
	events_.push_back(cubans);
	events_.push_back(thot_patrol);
	events_.push_back(throw_off_cliff);
	events_.push_back(remove_object);
	events_.push_back(roll_credits);
	events_.push_back(mov_mini_boss_1);
	events_.push_back(mov_mini_boss_2);
	events_.push_back(move_bad_ending_guy);
	events_.push_back(teleport_doorguard_sharktooth);
	events_.push_back(stopTutorialFall);
	events_.push_back(stopThrowFall);

	// Set all event colliders / tiggers
	for (auto event : events_)
		event->createEvent();
}

// Refreshes DEBUG tools
void WorldBase::DEBUG_refresh()
{
	// Initialize Debug Mode
	if (!DEBUG_has_initialized_)
	{
		DEBUG_screen_matrix_ = Matrix(screen_width_, screen_height_, '/');
		DEBUG_has_initialized_ = true;
	}
	DEBUG_drawUI();
	if (DEBUG_showing_collisions_)
	{
		DEBUG_displayCollisions();
	}
	DEBUG_displayScreen();
}

// Creates DEBUG_UI
void WorldBase::DEBUG_drawUI()
{
	drawSolidRectangle(51, 2, 27, 14, ' ', DEBUG_screen_matrix_);
	drawRectangle(51, 2, 27, 14, 'X', DEBUG_screen_matrix_);
	Image player_position("Player Pos: (" + std::to_string(screen_position_.x + screen_width_ / 2) + std::string(",") + std::to_string(screen_position_.y + screen_height_ / 2) + std::string(")Z"));
	addImageToMatrix(64, 4, player_position, DEBUG_screen_matrix_);

	Image show_collision_info("Show Collisions: ZPress c          Z");
	addImageToMatrix(64, 7, show_collision_info, DEBUG_screen_matrix_);

	Image close_debugger("Close Debugger:  ZPress h          Z");
	addImageToMatrix(64, 10, close_debugger, DEBUG_screen_matrix_);

	Image teleport("Teleport to maze:ZPress t          Z");
	addImageToMatrix(64, 13, teleport, DEBUG_screen_matrix_);
}

// Display Collisions
void WorldBase::DEBUG_displayCollisions()
{
	for (auto pickup : pickups_)
		pickup->DEBUG_viewCollider();
	for (auto character : characters_)
		character->DEBUG_viewCollider();
	for (auto signpost : signposts_)
		signpost->DEBUG_viewCollider();
	for (auto event : events_)
		event->DEBUG_viewCollider(world_matrix_);
}

// Displays what was there before the collision markers replaced them
void WorldBase::DEBUG_stopDisplayingCollisions()
{
	for (auto pickup : pickups_)
		pickup->createWorldSprite();
	for (auto event : events_)
		event->DEBUG_hideCollider(world_matrix_);
	for (auto character : characters_)
		character->DEBUG_eraseSpriteColliders();
	for (auto signpost : signposts_)
		signpost->createWorldSprite();
}

// Display DEBUG UI
void WorldBase::DEBUG_displayScreen()
{
	for (int i = 0; i < screen_height_; i++)
	{
		for (int j = 0; j < screen_width_; j++)
		{
			if (DEBUG_screen_matrix_[i][j] != '/')
			{
				char temp = DEBUG_screen_matrix_[i][j];
				screen_matrix_[i][j] = temp;
				screen_matrix_[i][j].setColor(DEBUG_screen_matrix_[i][j].getRGBA());
			}
		}
	}
}

// Creates a placeholder the size of an NPC on the map (And displays Coordinates)
void WorldBase::DEBUG_createPlaceholder(int center_position_x, int center_position_y)
{
	drawRectangle(center_position_x - 4, center_position_y - 2, 8, 6, 'X', world_matrix_);
	Image stats(std::to_string(center_position_x) + std::string("Z") + std::to_string(center_position_y) + std::string(" Z "));
	addImageToMatrix(center_position_x, center_position_y + 1, stats, world_matrix_);
}
