#include "WorldBase.h"

WorldBase::WorldBase(int screen_width, int screen_height, int world_width, int world_height, int starting_position_x, int starting_position_y, PlayerDefinition &player, Matrix &screen_matrix, Inventory &inventory, BitmapDefinition &bitmap, AudioDefinition &audio, std::string directory)
	: screen_width_{ screen_width }, screen_height_{ screen_height }, world_width_{ world_width }, world_height_{ world_height }, start_time_player_speed_(0), element_has_object_(world_height, std::vector<std::pair<int, int>>(world_width, std::make_pair<int, int>(0, 0))),
	world_matrix_(world_width, world_height), screen_matrix_{ screen_matrix }, player_{ player }, player_sprite_{ 12, 10, screen_matrix }, player_speed_modifier_(30), inventory_{ inventory }, score_display(16, screen_matrix), DEBUG_has_initialized_{ false }, audio_{ audio },
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
		case 4: // Collectible
			for (Collectible *collectible : collectibles)
				if (collectible->getUniqueObjectID() == getFacingEntity().second)
				{
					collectible->collect();
				}
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
	score_display.displaySprite(0, 0);
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
			checkRemoveCollectible();
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
		if (GetAsyncKeyState(0x54) & 0x8000) // Teleport Player			Press T Area 2
			teleportPlayer(235, 332);
		else if (GetAsyncKeyState(0x4D) & 0x8000) // Teleport Player    Press M (Teleport to Area 3)
			teleportPlayer(276, 239);
		else if (GetAsyncKeyState(0x46) & 0x8000) // Teleport Player    Press F (Teleport to start)
			teleportPlayer(103, 387);
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
	GENERATE_Collectibles();
	GENERATE_Signposts();
	GENERATE_Events();

	setNPCAttributes();
}

// Checks for a pickup to remove from map (That was already picked up)
void WorldBase::checkRemovePickup()
{
	if (is_viewing_popup_ && getFacingEntity().first == 2)
		for (auto itr = pickups_.begin(); itr != pickups_.end(); ++itr) {
			auto item = *itr;
			if (item->getUniqueObjectID() == getFacingEntity().second) {
				pickups_.erase(itr);
				delete item;
				break;
			}
		}
}

void WorldBase::checkRemoveCollectible()
{
	if(is_viewing_popup_ && getFacingEntity().first == 4)
		for (auto itr = collectibles.begin(); itr != collectibles.end(); ++itr) {
			auto item = *itr;
			if ((*itr)->getUniqueObjectID() == getFacingEntity().second) {
				collectibles.erase(itr);
				delete item;
				break;
			}
		}
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
	// Outside area / map
	Texture mountain(320, 300, SpriteSheet::mountain, world_matrix_); // Area 1
	Texture maze_3(320, 203, SpriteSheet::maze_3, world_matrix_);     // Area 2
	Texture maze_2(320, 100, SpriteSheet::maze_2, world_matrix_);     // Area 3

	// Fence
	//Texture fence(1161, 181, SpriteSheet::fence, world_matrix_);

	// Trees
	Texture tree_1(140, 371, SpriteSheet::tree, world_matrix_);
	Texture tree_2(170, 374, SpriteSheet::tree, world_matrix_);
	Texture tree_3(190, 368, SpriteSheet::tree, world_matrix_);
	Texture tree_4(208, 368, SpriteSheet::tree, world_matrix_);
	Texture tree_5(255, 368, SpriteSheet::tree, world_matrix_);
	Texture tree_6(319, 376, SpriteSheet::tree, world_matrix_);
	Texture tree_7(306, 229, SpriteSheet::tree, world_matrix_);

	// Rocks
	Texture rock_1(213, 328, SpriteSheet::rock, world_matrix_);
	Texture rock_2(265, 340, SpriteSheet::rock, world_matrix_);
	Texture rock_3(84, 290, SpriteSheet::rock, world_matrix_);
	Texture rock_4(151, 276, SpriteSheet::rock, world_matrix_);
	Texture rock_5(56, 402, SpriteSheet::rock, world_matrix_);


	Texture brokenqr1(131, 345, SpriteSheet::broken_qr1, world_matrix_);
	Texture brokenqr2(147, 344, SpriteSheet::broken_qr2, world_matrix_);
	Texture brokenqr3(81, 283, SpriteSheet::broken_qr1, world_matrix_);
}

// Creates the walls of the maze as well as objects that should be placed INSIDE the maze
void WorldBase::GENERATE_Maze()
{
	// Maze
	//Texture maze_1(300, 137, SpriteSheet::maze_1, world_matrix_);

	// Rocks
	//Texture rock_1(457, 86, SpriteSheet::rock, world_matrix_);
	//Texture rock_2(485, 96, SpriteSheet::rock, world_matrix_);
	//Texture rock_3(495, 90, SpriteSheet::rock, world_matrix_);
	//Texture rock_4(697, 61, SpriteSheet::rock_2, world_matrix_); // Rock Blocking Door
	//Texture rock_5(152, 33, SpriteSheet::rock, world_matrix_);
	//Texture rock_6(126, 52, SpriteSheet::rock, world_matrix_);
	//Texture rock_7(178, 52, SpriteSheet::rock, world_matrix_);
	//Texture rock_8(205, 49, SpriteSheet::rock, world_matrix_);
	//Texture rock_9(277, 32, SpriteSheet::rock, world_matrix_);
	//Texture rock_10(263, 36, SpriteSheet::rock, world_matrix_);
}

// creates NPCs that SHOULD attack (They don't have to at first, but if they attack at any time, put them here)
void WorldBase::GENERATE_Enemies()
{
	// Main Characters
	CharacterBase *qrbadguy1 = new Chr_QRBadGuy1(129, 386, 11, SpriteSheet::qrcode1, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *qrbadguy2 = new Chr_QRBadGuy2(235, 324, 16, SpriteSheet::qrcode2, 'd', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);
	CharacterBase *qrbadguy3 = new Chr_QRBadGuy3(276, 239, 13, SpriteSheet::qrcode3, 'l', player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_);

	qrbadguy1->initializeCharacter();
	qrbadguy2->initializeCharacter();
	qrbadguy3->initializeCharacter();

	characters_.push_back(qrbadguy1);
	characters_.push_back(qrbadguy2);
	characters_.push_back(qrbadguy3);
}

// creates NPCs that SHOULD NOT attack (They are capable of it, but this section is for NPCs that shouldn't)
void WorldBase::GENERATE_NonHostileNPCs()
{
	CharacterBase *qrguy1 = new Chr_BackgroundNPC(259, 378, 2, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("SCAN MY QR CODE TO INCREASE YOUR POINT COUNT!", 'X', 23, 9), SpriteSheet::qrcode1, 'd');
	CharacterBase* qrguy2 = new Chr_BackgroundNPC(259, 378, 2, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("SCA- SCA- SCAN MU- MY Q-QR CODE TO INCREASE YOUR POINT COUNT! COUNT C%%%! NNTKLLA%LHMNS", 'X', 24, 9), SpriteSheet::qrcode1, 'r');
	CharacterBase* qrguy3 = new Chr_BackgroundNPC(247, 270, 2, player_, screen_width_, screen_height_, world_matrix_, element_has_object_, screen_matrix_, bitmap_, audio_,
		PopupDefinition("YOU CAN USE THE NOOO%%%%CCCC APP TO SCAN QR CODES. SCAN ME! INCREASE YOUR POINT COUNT NOW!", 'X', 25, 9), SpriteSheet::qrcode1, 'r');

	qrguy1->initializeCharacter();
	qrguy2->initializeCharacter();
	qrguy3->initializeCharacter();

	characters_.push_back(qrguy1);
	characters_.push_back(qrguy2);
	characters_.push_back(qrguy3);
}

// creates all the sign posts (These show popups)
void WorldBase::GENERATE_Signposts()
{
	Signpost *welcome_sign = new Signpost(97, 372, 23, 9, 1, "NO4C 2 7_  Tomorrow B gins Tod*y", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	//Signpost *checkpoint_sign_2 = new Signpost(1000, 210, 23, 9, 2, "Nakinom Border Checkpoint", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	//Signpost *shift_run = new Signpost(923, 215, 23, 9, 6, "Hold SHIFT to run", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	//Signpost *deep_cave = new Signpost(318, 191, 23, 9, 3, "Welcome ye pirates to the DEEP cave", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	//Signpost *clear_cave = new Signpost(85, 88, 23, 9, 4, "Welcome to the CLEAR cave", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);
	//Signpost *no_entry = new Signpost(377, 47, 23, 9, 5, "Stop! No Entry beyond this point!", world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_);

	signposts_.push_back(welcome_sign);
	//signposts_.push_back(checkpoint_sign_2);
	//signposts_.push_back(shift_run);
	//signposts_.push_back(deep_cave);
	//signposts_.push_back(clear_cave);
	//signposts_.push_back(no_entry);

	// Displays all sign posts
	for (auto signpost : signposts_)
		signpost->createWorldSprite();
}

// creates all the items
void WorldBase::GENERATE_Pickups()
{
	Item tang("Tang", 9, "Pouch contains .%9%% O%%NG J%%C%. D% NOT %%% %%%%% %%% ADD W%TER %%% LE%% %% SE%IOU% INJ%RY");
	Pickup *pickup_tang = new Pickup(276, 277, 23, 9, 1, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, tang, inventory_);

	//Item item_clue_1("Cigar Box", 3, "Assembled in Cuba : Made in China. Restores 3 Health");
	//Pickup *pickup_clue_1 = new Pickup(1130, 212, 23, 9, 10, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_1, inventory_);
	//Item item_clue_2("Long Sword", "ATTACKUP", 3, "There's a tag on the side: If lost return to Ryuuko. Attack x3 for one turn");
	//Pickup *pickup_clue_2 = new Pickup(1127, 204, 23, 9, 11, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_2, inventory_);
	//Item item_clue_3("Feather", 5, "It's a turkey feather cut to mimic a hawk feather: someone was short-changed. Wearing restores 5 health");
	//Pickup *pickup_clue_3 = new Pickup(1144, 213, 23, 9, 12, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_3, inventory_);
	//Item item_clue_4("Glove", 2, "A single black glove, missing from a pair. Attack x2 for one turn");
	//Pickup *pickup_clue_4 = new Pickup(1146, 205, 23, 9, 13, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, item_clue_4, inventory_);
	//Pickup *unobtainable = new Pickup(462, 222, 23, 9, 14, world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, Item(), inventory_);

	pickups_.push_back(pickup_tang);
	//pickups_.push_back(pickup_clue_1);
	//pickups_.push_back(pickup_clue_2);
	//pickups_.push_back(pickup_clue_3);
	//pickups_.push_back(pickup_clue_4);
	//pickups_.push_back(unobtainable);

	// Displays all pickups
	for (auto pickup : pickups_)
		pickup->createWorldSprite();
}

void WorldBase::GENERATE_Collectibles()
{
	int collectibleUIDs = 40;
#define IMG_COLLECTIBLE (new Image("/=\\Z|$|Z\\=/Z", "RRRZRYRZRRRZ"))
#define COLLECTIBLE(x,y,uid,value) collectibles.push_back(new Collectible(x,y,23,9,uid,world_matrix_, element_has_object_, screen_matrix_, screen_width_, screen_height_, score_display, IMG_COLLECTIBLE, value))
#define COL(x,y,value) COLLECTIBLE(x,y,(++collectibleUIDs),value)
	COL(50, 403, 50);
	COL(230, 380, 50);
	COL(240, 280, 100);
#undef COL

#undef COLLECTIBLE
#undef IMG_COLLECTIBLE
	for (auto c : collectibles) {
		c->createWorldSprite();
	}
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
	//Event_Tutorial *tutorial = new Event_Tutorial(10000, 790, 232, 10, 11, 1, audio_.getDirectory(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_BorderIncident *border_incident = new Event_BorderIncident(10002, 1070, 206, 4, 4, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_RollCredits *roll_credits = new Event_RollCredits(10021, should_roll_credits_, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);

	Event_TeleportPlayer *teleport_to_area2 = new Event_TeleportPlayer(10001, 294, 378, 13, 3, 240, 355, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_to_area3 = new Event_TeleportPlayer(10004, 125, 265, 13, 3, 128, 227, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_to_area3_hallway = new Event_TeleportPlayer(10005, 341, 225, 13, 3, 68, 179, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	Event_TeleportPlayer *teleport_to_final = new Event_TeleportPlayer(10006, 236, 110, 13, 7, 341, 175, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_TeleportPlayer *teleport_to_aki = new Event_TeleportPlayer(10007, 393, 21, 10, 8, 679, 87, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_TeleportPlayer *teleport_from_aki = new Event_TeleportPlayer(10008, 679, 103, 10, 8, 393, 33, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_SetupEnding *teleport_to_mini_bosses = new Event_SetupEnding(10009, 731, 59, 10, 8, 857, 65, 1, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_MoveNPCIfDefeated *move_doorguard_sharktooth = new Event_MoveNPCIfDefeated(10017, 538, 167, 2, 24, 198, 165, 38, 13, true, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_MoveNPCIfDefeated *move_doorguard_ryuuko = new Event_MoveNPCIfDefeated(10018, 104, 234, 14, 2, 198, 165, 16, 14, true, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_MoveNPCIfDefeated *move_aki = new Event_MoveNPCIfDefeated(10019, 105, 228, 14, 2, 296, 195, 22, 14, true, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_MoveNPCIfDefeated *move_bad_ending_guy = new Event_MoveNPCIfDefeated(10025, 872, 67, 3, 3, 924, 72, 50, 12, false, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_MoveNPC *mov_mini_boss_1 = new Event_MoveNPC(10023, 198, 165, 10, 10, 1007, 'x', 20, 'd', 20, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_, 198, 165);
	//Event_MoveNPC *mov_mini_boss_2 = new Event_MoveNPC(10024, 198, 165, 10, 10, 54, 'y', 30, 'd', 21, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_TeleportNPC *teleport_doorguard_sharktooth = new Event_TeleportNPC(10026, 198, 165, 10, 10, 198, 165, 38, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_StopAudio *stopTutorialFall = new Event_StopAudio(10027, 886, 224, 6, 6, "falling", 0, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_StopAudio *stopThrowFall = new Event_StopAudio(10028, 497, 199, 6, 6, "falling", 0, true, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);


	//// Inside Cave
	//Event_LostDevice *lost_device = new Event_LostDevice(10003, 296, 224, 56, 1, player_.getPlayerName(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_AkiClearCave *aki_clear_cave = new Event_AkiClearCave(10010, 297, 179, 10, 11, player_.getPlayerName(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_BridgeRally *bridge_rally = new Event_BridgeRally(10011, 353, 113, 20, 1, 1, false, world_matrix_, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_BridgeRally2 *bridge_rally2 = new Event_BridgeRally2(10012, 285, 90, 5, 5, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_BridgeRally3 *bridge_rally3 = new Event_BridgeRally3(10013, 215, 85, 5, 5, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_Cubans *cubans = new Event_Cubans(10014, 198, 165, 10, 10, 1, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_ThotPatrol *thot_patrol = new Event_ThotPatrol(10022, 198, 165, 10, 11, player_.getPlayerName(), selected_character_, false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_ThrowOffCliff *throw_off_cliff = new Event_ThrowOffCliff(10016, 496, 225, 20, 1, 1, audio_.getDirectory(), false, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);
	//Event_RemoveObject *remove_object = new Event_RemoveObject(10020, 198, 165, 10, 10, 697, 61, 7, 3, 1, false, world_matrix_, element_has_object_, screen_matrix_, characters_, screen_position_, screen_width_, screen_height_);

	//// events_.push_back(test);
	//events_.push_back(tutorial);
	events_.push_back(teleport_to_area2);
	//events_.push_back(border_incident);
	//events_.push_back(lost_device);
	events_.push_back(teleport_to_area3);
	events_.push_back(teleport_to_area3_hallway);
	events_.push_back(teleport_to_final);
	//events_.push_back(teleport_to_aki);
	//events_.push_back(teleport_from_aki);
	//events_.push_back(move_doorguard_sharktooth);
	//events_.push_back(move_doorguard_ryuuko);
	//events_.push_back(move_aki);
	//events_.push_back(teleport_to_mini_bosses);
	//events_.push_back(aki_clear_cave);
	//events_.push_back(bridge_rally);
	//events_.push_back(bridge_rally2);
	//events_.push_back(bridge_rally3);
	//events_.push_back(cubans);
	//events_.push_back(thot_patrol);
	//events_.push_back(throw_off_cliff);
	//events_.push_back(remove_object);
	//events_.push_back(roll_credits);
	//events_.push_back(mov_mini_boss_1);
	//events_.push_back(mov_mini_boss_2);
	//events_.push_back(move_bad_ending_guy);
	//events_.push_back(teleport_doorguard_sharktooth);
	//events_.push_back(stopTutorialFall);
	//events_.push_back(stopThrowFall);

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

	Image teleport("Teleport to area:ZPress T, M, F          Z");
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
