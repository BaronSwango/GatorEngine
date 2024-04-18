#include "GameEngine.h"


GameEngine::GameEngine() {}
GameEngine& GameEngine::GetInstance()
{
	// TODO: insert return statement here
	static GameEngine instance_;

	if (!instance_.initialized_)
	{
		instance_.init();
		instance_.initialized_ = true;
	}
	return instance_;
}

// void GameEngine::ChangeScene(const std::string &sceneName, std::shared_ptr<Scene> scene, bool endcurrent_scene_)
// {
// 	if (endcurrent_scene_)
// 	{
// 		current_scene_->onEnd();
// 	}

// 	// current_scene_ = sceneName;
// 	current_scene_ = scene;
// }

void GameEngine::quit()
{
	running_ = false;
}

void GameEngine::run()
{
	running_ = true;
}

sf::RenderWindow& GameEngine::window()
{
	return window_;
}

AssetManager& GameEngine::assets()
{
	return AssetManager::GetInstance();
}

void GameEngine::init()
{
	//Intializing all png files as textures in Start Assets folder
	AssetManager::GetInstance().IntializeTextureAssets("assets/StartAssets");
	AssetManager::GetInstance().AddTexture("Ground", "assets/Terrain/Terrain (16x16).png");
	AssetManager::GetInstance().AddTexture("Tree", "assets/Terrain/Terrain (16x16).png");
	Animation ani = Animation("DefaultAnimation", AssetManager::GetInstance().GetTexture("DefaultAnimation"), 11, 1);
	AssetManager::GetInstance().AddAnimation("DefaultAnimation", ani);
	Animation ani2 = Animation("RunningAnimation", AssetManager::GetInstance().GetTexture("RunningAnimation"), 12, 1);
	AssetManager::GetInstance().AddAnimation("RunningAnimation", ani2);
	readFromJSONFile("last-scene.json");

	//123
	window_.setFramerateLimit(60);
}

void GameEngine::changeScene(const std::string& path) 
{
	EntityManager::GetInstance().reset();
	Editor::kState = Editor::State::None;
	Editor::kActiveEntity = nullptr;
	Scene scene;
	scene.readFromJSONFile(path);	
	current_scene_path_ = path;
	writeToJSONFile("last-scene.json");
}

void GameEngine::saveScene(const std::string& path) 
{
	Scene scene;
	scene.writeToJSONFile(path);
	current_scene_path_ = path;
	writeToJSONFile("last-scene.json");
}

void GameEngine::addEntitiesForTest() 
{
	// add player
	auto player = EntityManager::GetInstance().addEntity("player");
	player->addComponent<CAnimation>();
	player->getComponent<CAnimation>()->animation = AssetManager::GetInstance().GetAnimation("DefaultAnimation");
	player->addComponent<CTransform>(Vec2(224, 200));
	player->addComponent<CUserInput>();
	player->addComponent<CName>("Player1");
	GatorPhysics::GetInstance().createBody(player.get(), false);


	// add ground tile 
	auto ground = EntityManager::GetInstance().addEntity("Ground");
	// The parameters to construct a transform are position and scale and angle of rotation
	ground->addComponent<CTransform>(Vec2(224, 300), Vec2(1, 1), 0);
	ground->addComponent<CSprite>("Ground");
	ground->addComponent<CName>("Ground");
	//ground->getComponent<CSprite>()->texture = GameEngine::GetInstance().assets().GetTexture("Ground");
	//Need to select ground portion of the texture
	ground->getComponent<CSprite>()->setTexturePortion(sf::IntRect(95, 0, 48, 48));
	GatorPhysics::GetInstance().createBody(ground.get(), true);
}

void GameEngine::update()
{
	EntityManager::GetInstance().update();
	sUserInput();

	if (Editor::kState == Editor::State::Testing) {
		sTouchTrigger();
		sMovement();
		sPhysics();
		sCollision();
	}

	sBackground();
	sRender();
	//sRenderColliders();
	//GatorPhysics &physics = GatorPhysics::GetInstance();


	// EntityManager::GetInstance().update();
	// sUserInput();

	// if (Editor::kState == Editor::State::Testing) {
	// 	sMovement();
	// 	sPhysics();
	// 	sCollision();
	// }

	// sAnimation();
	// sRender();
	// GatorPhysics &physics = GatorPhysics::GetInstance();
	// current_scene_->update();
}

void GameEngine::sUserInput()
{
	// Clear the bus from previous frame's input
	ActionBus::GetInstance().Clear();
	//std::cout << "sUserInput" << std::endl;
	sf::Event event;
	while (window_.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed) {
			window_.close();
		}

		// Update the SFML's render window dimensions to prevent weird sprite scaling
		if (event.type == sf::Event::Resized)
		{
			sf::FloatRect view(0, 0, event.size.width, event.size.height);
			window_.setView(sf::View(view));
		}

		// Editor-specific hotkeys
		if (Editor::kActiveEntity && Editor::kState != Editor::State::Testing) {
			// Ctrl+D to copy active entity
			if (event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::D) {
				EntityManager::GetInstance().cloneEntity(Editor::kActiveEntity);
			}

			// Ctrl+X to delete active entity
			if (event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::X) {
				EntityManager::GetInstance().removeEntity(Editor::kActiveEntity);
			}

			// Ctrl+Z hotkey does not exist. Good luck o7
		}

		// Lambda to process key or mouse events for the player
		auto processInputEvent = [](const sf::Event &event, const sf::Event::EventType &eventType)
		{
			auto &entities = EntityManager::GetInstance().getEntities();
			for (auto &entity : entities)
			{
				std::cout << "Event button: before " << std::endl;
				// Skip entities without a cUserInput component
				if (!entity->getComponent<CUserInput>())
					continue;

				auto findAndDispatch = [entity](auto &inputMap, const auto &eventButton)
				{
					// For each key in the inputmap
					std::cout << "Event button: " << eventButton << std::endl;
					for (auto &actionKeys : inputMap)
					{
						if (actionKeys.first == eventButton)
						{
							ActionBus::GetInstance().Dispatch(entity, actionKeys.second);
						}
					}
				};

				// Find potential actions and dispatch them based on the event type
				if (eventType == sf::Event::KeyPressed)
				{
					auto &inputMap = entity->getComponent<CUserInput>()->key_map;
					findAndDispatch(inputMap, event.key.code);
				}
				else
				{
					auto &inputMap = entity->getComponent<CUserInput>()->mouse_map;
					findAndDispatch(inputMap, event.mouseButton.button);
				}
			}
		};

		// Process key or mouse input events with the lambda
		if (event.type == sf::Event::KeyPressed)
		{
			processInputEvent(event, sf::Event::KeyPressed);
		}
		else if (event.type == sf::Event::MouseButtonPressed)
		{
			processInputEvent(event, sf::Event::MouseButtonPressed);
		}
	}

	if (Editor::kState == Editor::State::Testing)
	{
		EntityManager::GetInstance().update();
		// other systems here
		current_frame_++;
	}
	else
	{
		current_frame_ = 0;
		/*
			player_->cTransform->position = Vec2(400, 400);
			- resetting player position back to a mock position if not testing, could be an idea to reset everything here?
		*/
	}

	// sRender outside of testing check here?
}

void GameEngine::sTouchTrigger()
{
	auto& entities = EntityManager::GetInstance().getEntities();
	for (auto& entity : entities) {
		// Skip entities without a touch trigger component
		if (!entity->hasComponent<CTouchTrigger>()) continue;
		auto touchTrigger = entity->getComponent<CTouchTrigger>();
		auto triggerRect = entity->GetRect();

		// If has touch trigger, check if it is touching any other entity
		for (auto& actionTags : touchTrigger->tag_map) {
			for (auto& entityTouched : entities) {
				// Skip entities without the tag we're caring about
				if (actionTags.first != entityTouched->getComponent<CInformation>()->tag) continue;

				// Check if the entity is touching the entity with the touch trigger
				auto entityTouchedRect = entityTouched->GetRect(5); // Add leeway to the entity touched rect
				if (triggerRect.intersects(entityTouchedRect)) {
					ActionBus::GetInstance().Dispatch(entityTouched, actionTags.second);
				}
			}
		}
	}
}

void GameEngine::sMovement()
{
	for (auto entity : EntityManager::GetInstance().getEntities()) {
		if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRigidBody>()) continue;
		float speed = 5.0;
		//Vec2 finalVelocity = entity->getComponent<CTransform>()->velocity;
		//Vec2 finalAcceleration = Vec2(0, 0);
		b2Body* body = GatorPhysics::GetInstance().GetEntityToBodies()[entity.get()];
		
		b2Vec2 resultMovement = b2Vec2(0, 0);
		if (ActionBus::GetInstance().Received(entity, MoveRight))
			resultMovement += b2Vec2(speed, 0);
		//finalVelocity = finalVelocity + Vec2(speed, 0);

		if (ActionBus::GetInstance().Received(entity, MoveLeft))
			resultMovement += b2Vec2(-speed, 0);
		//finalVelocity = finalVelocity + Vec2(-speed, 0);

		if (ActionBus::GetInstance().Received(entity, Jump))
			body->ApplyForceToCenter(b2Vec2(0, 30), true);

		b2Vec2 velocity = body->GetLinearVelocity();
		resultMovement = b2Vec2(resultMovement.x, velocity.y);

		body->SetLinearVelocity(resultMovement);
	}
}

void GameEngine::sPhysics()
{

	//First check if any new entities have a new rigid body component and
	// have not been added to the physics world
	
	for (auto entity : EntityManager::GetInstance().getEntities())
	{
		if (entity->hasComponent<CRigidBody>())
		{
			auto rigidBodyComponent = entity->getComponent<CRigidBody>();
			std::map<Entity*, b2Body*>& entity_to_bodies_ = GatorPhysics::GetInstance().GetEntityToBodies();
			//If the entity is not in the physics world, add it
			if (entity_to_bodies_.find(entity.get()) == entity_to_bodies_.end())
			{
				GatorPhysics::GetInstance().createBody(entity.get(), rigidBodyComponent->static_body);
			}
		}
	}

	// For each entity move them based on their velocity and physics components
	for (auto entity : EntityManager::GetInstance().getEntities())
	{
		if (entity->hasComponent<CTransform>())
		{
			auto transform = entity->getComponent<CTransform>();
			// Update the position based on the velocity
			transform->position = transform->position + transform->velocity;
		}
	}

	// Any other movement that should be done based on other physics components
	// should be done below here
}

void GameEngine::sCollision()
{
	GatorPhysics::GetInstance().update();
}

// void GameEngine::sAnimation()
// {
// 	// Need to add GetComponent, AddComponent templates to entity.
// 	auto &entityManager = EntityManager::GetInstance();
// 	std::vector<std::shared_ptr<Entity>> &entityList = entityManager.getEntities();

// 	for (auto &entity : entityList)
// 	{
// 		if (entity->hasComponent<CAnimation>())
// 		{
// 			auto transformComponent = entity->getComponent<CTransform>();
// 			Vec2 scale = transformComponent->scale;
// 			Vec2 position = transformComponent->position; // getting the scale and positioning from the transform component in order to render sprite at proper spot
// 			auto animationComponent = entity->getComponent<CAnimation>();
// 			animationComponent->changeSpeed();
// 			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;
// 			sf::Sprite sprite(animationComponent->animation.sprite);
// 			sprite.setPosition(position.x, position.y + yOffset); //Removed the +150 from the y position
// 			sprite.setScale(scale.x, scale.y);
// 			window_.draw(sprite);

// 			if (Editor::kState == Editor::State::Testing) {
// 				animationComponent->update();
// 			}
// 		}
// 	}
// }

void GameEngine::sBackground() {
	// Find first component of type CBackground and draw it
	auto entityList = EntityManager::GetInstance().getEntities();
	for (auto& entity : entityList) {
		if (entity->hasComponent<CBackgroundColor>()) {
			auto background = entity->getComponent<CBackgroundColor>();
			window_.clear(background->color);
			return;
		}
	}

	// Otherwise, default to a black background
	window_.clear(sf::Color(0, 0, 0));
}

void GameEngine::sRender()
{
	auto &entityManager = EntityManager::GetInstance();

	std::vector<std::shared_ptr<Entity>> &entityList = entityManager.getEntitiesRenderingList();

	for (auto &entity : entityList)
	{ // Looping through entity list and drawing the sprites to the render window.
		if (entity->hasComponent<CSprite>())
		{
			auto transformComponent = entity->getComponent<CTransform>();
			Vec2 scale = transformComponent->scale;
			Vec2 position = Editor::kState == Editor::State::Testing ? transformComponent->position : transformComponent->origin; // getting the scale and positioning from the transform component in order to render sprite at proper spot
			auto spriteComponent = entity->getComponent<CSprite>();
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;

			// Set the origin of the sprite to its center
			sf::FloatRect bounds = spriteComponent->sprite.getLocalBounds();
			spriteComponent->sprite.setOrigin(bounds.width / 2, bounds.height / 2);
			spriteComponent->sprite.setPosition(position.x, position.y + yOffset);
			spriteComponent->sprite.setScale(scale.x, scale.y);
      
      //Rotation
			float angle = transformComponent->angle * -1;
			spriteComponent->sprite.setRotation(angle);
			

			if (spriteComponent->draw_sprite)
				window_.draw(spriteComponent->sprite);
		}

		if (entity->hasComponent<CAnimation>())
		{
			auto transformComponent = entity->getComponent<CTransform>();
			Vec2 scale = transformComponent->scale;
			Vec2 position = transformComponent->position; // getting the scale and positioning from the transform component in order to render sprite at proper spot
			auto animationComponent = entity->getComponent<CAnimation>();
			animationComponent->changeSpeed();
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;
			sf::Sprite sprite(animationComponent->animation.sprite);
			// Set the origin of the sprite to its center
			sf::FloatRect bounds = sprite.getLocalBounds();
			sprite.setOrigin(bounds.width / 2, bounds.height / 2);

			// Set the position of the sprite to the center position
			sprite.setPosition(position.x, position.y + yOffset);
			sprite.setScale(scale.x, scale.y);
			float angle = transformComponent->angle * -1;
			sprite.setRotation(angle);
			window_.draw(sprite);
			if (Editor::kState == Editor::State::Testing) {
				animationComponent->update();
			}
		}
	}
}

bool GameEngine::isRunning()
{
	return running_;
}

bool GameEngine::isKeyPressed(sf::Keyboard::Key key)
{
	return sf::Keyboard::isKeyPressed(key);
}

// void GameEngine::sUserInput()
// {
	//sf::Event event;
	//while (window_.pollEvent(event))
	//{
	//	ImGui::SFML::ProcessEvent(event);

	//	if (event.type == sf::Event::Closed) {
	//		//window_.close();
	//	}

	//	// Update the SFML's render window_ dimensions to prevent weird sprite scaling
	//	if (event.type == sf::Event::Resized) {
	//		sf::FloatRect view(0, 0, event.size.width, event.size.height);
	//		window_.setView(sf::View(view));
	//	}

	//	// The game engine handles the following key presses:
	//	// - Enter: Toggles the game's running state
	//	// - Escape: Closes the game
	//	if (event.type == sf::Event::KeyPressed) {
	//		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) 
	//		{
	//			if (running)
	//			{
	//				quit();
	//			}
	//			else
	//			{
	//				run();
	//			}
	//		}
	//		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
	//		{
	//			window_.close();
	//		}
	//	}
	//}
// }