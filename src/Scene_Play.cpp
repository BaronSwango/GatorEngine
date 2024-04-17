#include "Scene_Play.h"
#include "GatorPhysics.h"
#include "AssetManager.h"
#include "GameEngine.h"
#include "components/Component.h"
#include <fstream>


Scene_Play::Scene_Play()
{
	spawnPlayer();
	// Create a platform and a tree:
	std::shared_ptr<Entity> ground = m_entityManager.addEntity("Ground");
	// Create a platform and a tree:
	std::shared_ptr<Entity> ground2 = m_entityManager.addEntity("Ground");
	// Create a platform and a tree:
	std::shared_ptr<Entity> ground3 = m_entityManager.addEntity("Ground");
	// The parameters to construct a transform are position and scale and angle of rotation
	ground->addComponent<CTransform>(Vec2(224, 300), Vec2(1, 1), 0);
	ground->addComponent<CSprite>("Grass Tile");
	ground->addComponent<CName>("Ground");
	ground->addComponent<CInformation>();
	GatorPhysics::GetInstance().createBody(ground.get(), true);


	// The parameters to construct a transform are position and scale and angle of rotation
	ground2->addComponent<CTransform>(Vec2(272, 300), Vec2(1, 1), 0);
	ground2->addComponent<CSprite>("Grass Tile");
	ground2->addComponent<CName>("Ground2");
	ground2->addComponent<CInformation>();
	GatorPhysics::GetInstance().createBody(ground2.get(), true);


	// The parameters to construct a transform are position and scale and angle of rotation
	ground3->addComponent<CTransform>(Vec2(320, 300), Vec2(1, 1), 0);
	ground3->addComponent<CSprite>("Grass Tile");
	ground3->addComponent<CName>("Ground3");
	ground3->addComponent<CInformation>();
	ground3->setDisabled(true);

	GatorPhysics::GetInstance().createBody(ground3.get(), true);
	/*std::shared_ptr<Entity> tree = EntityManager::addEntity("Tree");
	tree->addComponent<CTransform>(Vec2(200, 400), Vec2(20, 50));
	tree->addComponent<CSprite>(m_game->assets().getTexture("Tree"));*/



}

Scene_Play::Scene_Play(const std::string& levelPath) : m_levelPath(levelPath)
{
	LoadScene(levelPath);
}

void Scene_Play::LoadScene(const std::string& filename)
{
	// reset the entity manager every time we load a level
	EntityManager::GetInstance().reset();

	// TODO: Read the level file and spawn entities based on the data

	spawnPlayer();

	// some sample entities
	auto brick = m_entityManager.addEntity("tile");
	brick->addComponent<CSprite>();
	brick->getComponent<CSprite>()->texture_ = GameEngine::GetInstance().assets().GetTexture("DefaultSprite");
	brick->addComponent<CTransform>(Vec2(96, 200), Vec2(5, 5), 0);
}

void Scene_Play::spawnPlayer()
{
	// here is a sample player entity which you can use to construct other entities
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CAnimation>();
	m_player->getComponent<CAnimation>()->animation_ = GameEngine::GetInstance().assets().GetAnimation("DefaultAnimation");
	m_player->addComponent<CTransform>(Vec2(224, 200));
	m_player->addComponent<CUserInput>();
	m_player->addComponent<CName>("Player1");
	m_player->addComponent<CInformation>();
	m_player->addComponent<CHealth>();
	GatorPhysics::GetInstance().createBody(m_player.get(), false);

	//Attach a script to the player instance
	//std::shared_ptr<LuaState> new_lua_state = std::make_shared<LuaState>("script.lua", m_player);
	//GameEngine::GetInstance().lua_states[m_player] = new_lua_state;
	// TODO: be sure to add the remaining components to the player
}

void Scene_Play::update()
{
	m_entityManager.update();
	sUserInput();
	sTouchTrigger();
	sScripts();
	sMovement();
	sPhysics();
	sCollision();
	sBackground();
	sRender();
	sUI();
	//sRenderColliders();
	//GatorPhysics &physics = GatorPhysics::GetInstance();
}

void Scene_Play::sCollision()
{
	GatorPhysics::GetInstance().update();
	//Determine if the player touched the ground
}

void Scene_Play::sUserInput()
{
	// Clear the bus from previous frame's input
	ActionBus::GetInstance().Clear();
	//std::cout << "sUserInput" << std::endl;
	sf::Event event;
	while (GameEngine::GetInstance().window().pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed) {
			GameEngine::GetInstance().window().close();
		}

		// Update the SFML's render window dimensions to prevent weird sprite scaling
		if (event.type == sf::Event::Resized)
		{
			sf::FloatRect view(0, 0, event.size.width, event.size.height);
			GameEngine::GetInstance().window().setView(sf::View(view));
		}

		// Editor-specific hotkeys
		if (Editor::active_entity_ && Editor::state != Editor::State::Testing) {
			// Ctrl+D to copy active entity
			if (event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::D) {
				EntityManager::GetInstance().cloneEntity(Editor::active_entity_);
			}

			// Ctrl+X to delete active entity
			if (event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::X) {
				EntityManager::GetInstance().removeEntity(Editor::active_entity_);
			}

			// Ctrl+Z hotkey does not exist. Good luck o7
		}

		// Lambda to process key or mouse events for the player
		auto processInputEvent = [](const sf::Event& event, const sf::Event::EventType& eventType)
		{
			auto& entities = EntityManager::GetInstance().getEntities();
			for (auto& entity : entities)
			{
				// Skip entities without a cUserInput component or that are disabled
				if (!entity->getComponent<CUserInput>() || entity->isDisabled())
					continue;

				auto findAndDispatch = [entity](auto& inputMap, const auto& eventButton)
				{
					// For each key in the inputmap
					for (auto& actionKeys : inputMap)
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
					auto& inputMap = entity->getComponent<CUserInput>()->keyMap;
					findAndDispatch(inputMap, event.key.code);
					//GameEngine::GetInstance().lua_states[0]->KeyPressed(event.key.code);
					GameEngine::GetInstance().last_key_pressed = event.key.code;
				}
				else
				{
					auto& inputMap = entity->getComponent<CUserInput>()->mouseMap;
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

	// sRender outside of testing check here?
}

void Scene_Play::sPhysics()
{

	//First check if any new entities have a new rigid body component and
	// have not been added to the physics world

	for (auto entity : EntityManager::GetInstance().getEntities())
	{
		if (entity->hasComponent<CRigidBody>() && !entity->isDisabled())
		{
			auto rigidBodyComponent = entity->getComponent<CRigidBody>();
			std::map<Entity*, b2Body*>& entity_to_bodies_ = GatorPhysics::GetInstance().GetEntityToBodies();
			//If the entity is not in the physics world, add it
			if (entity_to_bodies_.find(entity.get()) == entity_to_bodies_.end())
			{
				GatorPhysics::GetInstance().createBody(entity.get(), rigidBodyComponent->staticBody);
			}
		}
	}

	// For each entity move them based on their velocity and physics components
	for (auto entity : EntityManager::GetInstance().getEntities())
	{
		if (entity->hasComponent<CTransform>() && !entity->isDisabled())
		{
			auto transform = entity->getComponent<CTransform>();
			// Update the position based on the velocity
			transform->position = transform->position + transform->velocity;
		}
	}

	// Any other movement that should be done based on other physics components
	// should be done below here
}

void Scene_Play::sTouchTrigger()
{
	auto& entities = EntityManager::GetInstance().getEntities();
	for (auto& entity : entities) {
		// Skip entities without a touch trigger component
		if (!entity->hasComponent<CTouchTrigger>()) continue;
		auto touchTrigger = entity->getComponent<CTouchTrigger>();
		auto triggerRect = entity->GetRect();

		// If has touch trigger, check if it is touching any other entity
		for (auto& actionTags : touchTrigger->tagMap) {
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

void Scene_Play::sScripts()
{
	auto lua_states = GameEngine::GetInstance().lua_states;
	//First, check if there are any entities that have been given a script component. If so,
	//add them to map of entities to lua states
	for (auto entity : EntityManager::GetInstance().getEntities())
	{
		if (entity->hasComponent<CScript>() && !GameEngine::GetInstance().lua_states[entity])
		{
			//Verify that the script name that the user typed in the editor is valid
			std::string script_name = entity->getComponent<CScript>()->script_name;
			std::ifstream file(script_name);
			if (!file.good())
			{
				std::cout << "Invalid script name: " << script_name << std::endl;
				continue;
			}
			//Attach a script to the player instance
			
			std::shared_ptr<LuaState> new_lua_state = std::make_shared<LuaState>(entity->getComponent<CScript>()->script_name, entity);
			GameEngine::GetInstance().lua_states[entity] = new_lua_state;
			entity->getComponent<CScript>()->lua_state = new_lua_state.get();
		}
	}
	lua_states = GameEngine::GetInstance().lua_states;
	//For each lua state, update the script
	for (auto node : GameEngine::GetInstance().lua_states)
	{
		//For each lua state, update the script
		if (node.second)
		{
			node.second->Update();
		}
		
	}
}

void Scene_Play::onEnd()
{
	// TODO: When the scene ends, change back to the MENU scene
	//		 use m_game->changeScene(correct params);
}


void Scene_Play::sRender()
{
	auto& entityManager = EntityManager::GetInstance();

	std::vector<std::shared_ptr<Entity>>& entityList = entityManager.getEntitiesRenderingList();

	for (auto& entity : entityList)
	{ // Looping through entity list and drawing the sprites to the render window.
		if (entity->hasComponent<CSprite>() && !entity->isDisabled())
		{
			auto transformComponent = entity->getComponent<CTransform>();
			Vec2 scale = transformComponent->scale;
			Vec2 position = transformComponent->position; // getting the scale and positioning from the transform component in order to render sprite at proper spot
			auto spriteComponent = entity->getComponent<CSprite>();
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;

			// Set the origin of the sprite to its center
			sf::FloatRect bounds = spriteComponent->sprite_.getLocalBounds();
			spriteComponent->sprite_.setOrigin(bounds.width / 2, bounds.height / 2);
			spriteComponent->sprite_.setPosition(position.x, position.y + yOffset);
			spriteComponent->sprite_.setScale(scale.x, scale.y);
      
			//Rotation
			float angle = transformComponent->angle * -1;
			spriteComponent->sprite_.setRotation(angle);

			if (spriteComponent->drawSprite_)
				GameEngine::GetInstance().window().draw(spriteComponent->sprite_);
		}

		if (entity->hasComponent<CAnimation>() && !entity->isDisabled())
		{
			auto transformComponent = entity->getComponent<CTransform>();
			Vec2 scale = transformComponent->scale;
			Vec2 position = transformComponent->position; // getting the scale and positioning from the transform component in order to render sprite at proper spot
			auto animationComponent = entity->getComponent<CAnimation>();
			animationComponent->changeSpeed();
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;
			sf::Sprite sprite(animationComponent->animation_.sprite_);

			// Set the origin of the sprite to its center
			sf::FloatRect bounds = sprite.getLocalBounds();
			sprite.setOrigin(bounds.width / 2, bounds.height / 2);

			// Set the position of the sprite to the center position
			sprite.setPosition(position.x, position.y + yOffset);
			sprite.setScale(scale.x, scale.y);
			float angle = transformComponent->angle * -1;
			sprite.setRotation(angle);
			GameEngine::GetInstance().window().draw(sprite);
			

			if (animationComponent->playAnimation || Editor::state == 3)
				animationComponent->update();
		}
	}
}

void Scene_Play::sUI() {
	auto& entityManager = EntityManager::GetInstance();

	std::vector<std::shared_ptr<Entity>>& entityList = entityManager.getUIRenderingList(); // We only iterate through the UI rendering list 

	for (auto& entity : entityList) {

		if (entity->hasComponent<CHealth>() && entity->getComponent<CHealth>()->drawHealth_ && !entity->isDisabled()) 
		{ // Health Bar 
			auto healthComponent = entity->getComponent<CHealth>();
			
			sf::Sprite backHealth(healthComponent->backHealthBar_);
			sf::Sprite frontHealth(healthComponent->frontHealthBar_);

			//Making the sprite for the front Health bar
			healthComponent->Update();

			// Set the origin of the sprite to its center
			sf::FloatRect bounds = backHealth.getLocalBounds();
			backHealth.setOrigin(bounds.width / 2, bounds.height / 2);
			sf::FloatRect bounds2 = frontHealth.getLocalBounds();
			//Hard coded for now, the best way to allow the user to customize health bars would be a different approach
			frontHealth.setOrigin(bounds.width / 2 - 7, bounds2.height / 2);

			// Set the position of the sprite to the center position
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;

			Vec2 scale = healthComponent->healthBarScale_;

			if (healthComponent->followEntity) {
				// The position is above
				Vec2 position = entity->getComponent<CTransform>()->position + healthComponent->healthBarOffset_;
				backHealth.setPosition(position.x, position.y + yOffset);
				backHealth.setScale(scale.x, scale.y);

				frontHealth.setPosition(position.x, position.y + yOffset);
				frontHealth.setScale(scale.x, scale.y);
			}
			else {
				Vec2 position = healthComponent->healthBarPosition_;
				backHealth.setPosition(position.x, position.y + yOffset);
				backHealth.setScale(scale.x, scale.y);

				// Set the position of the sprite to the center position
				frontHealth.setPosition(position.x, position.y + yOffset);
				frontHealth.setScale(scale.x, scale.y);
			}
			
			GameEngine::GetInstance().window().draw(backHealth);
			GameEngine::GetInstance().window().draw(frontHealth);
			
		}

		if (entity->hasComponent<CText>() && !entity->isDisabled()) 
		{
			auto transformComponent = entity->getComponent<CTransform>(); // Transform related 
			Vec2 scale = transformComponent->scale;
			Vec2 position = transformComponent->position;
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;

			auto textComponent = entity->getComponent<CText>(); // Setting the properties of the text

			textComponent->text_.setFont(textComponent->font_);
			textComponent->text_.setString(textComponent->message_);
			textComponent->text_.setCharacterSize(textComponent->characterSize_);
			textComponent->text_.setFillColor(textComponent->textColor_);
			textComponent->text_.setStyle(textComponent->style_);
			
			sf::FloatRect bounds = textComponent->text_.getLocalBounds();
			textComponent->text_.setScale(scale.x, scale.y);
			textComponent->text_.setPosition(position.x, position.y + yOffset);
			textComponent->text_.setOrigin(bounds.width / 2, bounds.height / 2);

			GameEngine::GetInstance().window().draw(textComponent->text_);
		}
	} 
}

void Scene_Play::sRenderColliders() {
	auto& entityManager = EntityManager::GetInstance();

	std::vector<std::shared_ptr<Entity>>& entityList = entityManager.getEntities();

	for (auto& entity : entityList)
	{ // Looping through entity list and drawing the sprites to the render window.
		if (entity->hasComponent<CRigidBody>() && !entity->isDisabled())
		{
			auto rigidBodyComponent = entity->getComponent<CRigidBody>();
			b2Vec2 position = rigidBodyComponent->body->GetPosition();
			//These sizes are half widths and half heights
			b2Vec2 size = rigidBodyComponent->body->GetFixtureList()->GetAABB(0).GetExtents();
			float xScale = entity->getComponent<CTransform>()->scale.x;
			float yScale = entity->getComponent<CTransform>()->scale.y;
			float entityWidth = size.x * 2 / xScale / GatorPhysics::GetInstance().getScale();
			float entityHeight = size.y * 2 / yScale / GatorPhysics::GetInstance().getScale();
			auto spriteComponent = sf::RectangleShape();
			spriteComponent.setOrigin(entityWidth / 2, entityHeight / 2);
			spriteComponent.setFillColor(sf::Color::White);
			float yOffset = ImGui::GetMainViewport()->Size.y * .2 + 20;
			float worldY = GameEngine::GetInstance().window().getSize().y;
			float entityY = ((position.y / GatorPhysics::GetInstance().getScale()) - worldY) * -1;
			float entityX = position.x / GatorPhysics::GetInstance().getScale();
			spriteComponent.setPosition(entityX, entityY + yOffset); // Removed the +150 from the y position

			spriteComponent.setSize(sf::Vector2f(entityWidth, entityHeight));
			GameEngine::GetInstance().window().draw(spriteComponent);
		}
	}
}


void Scene_Play::sMovement()
{
	for (auto entity : EntityManager::GetInstance().getEntities()) {
		if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRigidBody>() || entity->isDisabled()) continue;
		float speed = 5.0;
		Vec2 jump_force = Vec2(0, 30);
		if (entity->hasComponent<CCharacter>())
		{
			speed = entity->getComponent<CCharacter>()->speed;
			jump_force = entity->getComponent<CCharacter>()->jump_force;
		}



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
		{
			if (entity->getComponent<CCharacter>()->is_grounded)
			{
				entity->getComponent<CCharacter>()->is_grounded = false;
				body->ApplyForceToCenter(b2Vec2(jump_force.x, jump_force.y), true);
			}
		}

		b2Vec2 velocity = body->GetLinearVelocity();
		resultMovement = b2Vec2(resultMovement.x, velocity.y);

		body->SetLinearVelocity(resultMovement);
	}
}

// Handles clearing of window and drawing scene background
void Scene_Play::sBackground() {
	// Find first component of type CBackground and draw it
	auto entityList = EntityManager::GetInstance().getEntities();
	for (auto& entity : entityList) {
		if (entity->hasComponent<CBackgroundColor>() && !entity->isDisabled()) {
			auto background = entity->getComponent<CBackgroundColor>();
			GameEngine::GetInstance().window().clear(background->color);
			return;
		}
	}

	// Otherwise, default to a black background
	GameEngine::GetInstance().window().clear(sf::Color(0, 0, 0));
}

