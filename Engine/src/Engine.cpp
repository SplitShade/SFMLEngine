// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include <memory>
#include <fstream>

class Entity : public sf::Drawable
{
private:
	sf::Uint32 _ref;

	bool _isShape = false;
	std::unique_ptr<sf::Shape> _shape = nullptr;

	bool _hasText = false;
	std::unique_ptr<sf::Text> _text = nullptr;

	sf::Vector2f _velocity{ 0.f, 0.f };

public:
	static sf::Uint32 sCnt;

	[[nodiscard]] bool isShape() const { return _isShape; }
	[[nodiscard]] sf::Shape& getShape() const { return *_shape; }
	[[nodiscard]] bool hasText() const { return _hasText; }
	[[nodiscard]] sf::Text& getText() const { return *_text; }

	void update(const sf::RenderWindow& target)
	{
		sf::Vector2f currentMove(_velocity);
		sf::Vector2f diff = { 0.f, 0.f };
		bool outOfBounds = false;
		if (_isShape)
		{
			_shape->move(currentMove);

			sf::FloatRect globalBounds = _shape->getGlobalBounds();
			sf::Vector2u targetSize = target.getSize();
			if (globalBounds.left < 0.f)
			{
				_velocity.x *= -1;
				outOfBounds = true;
				diff.x = 2 * -globalBounds.left;
			}
			if (globalBounds.left + globalBounds.width > targetSize.x)
			{
				_velocity.x *= -1;
				outOfBounds = true;
				diff.x = 2 * (targetSize.x - globalBounds.left - globalBounds.width);
			}
			if (globalBounds.top < 0.f)
			{
				_velocity.y *= -1;
				outOfBounds = true;
				diff.y = 2 * -globalBounds.top;
			}
			if (globalBounds.top + globalBounds.height > targetSize.y)
			{
 				_velocity.y *= -1;
				outOfBounds = true;
				diff.y = 2 * (targetSize.y - globalBounds.top - globalBounds.height);
			}
			if (outOfBounds)
			{
				_shape->move(diff);
			}
		}
		if (_hasText)
		{
			_text->move(currentMove);
			if (outOfBounds)
			{
				_text->move(diff);
			}
		}
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// Draw other high-level objects
		if (_isShape)
		{
			target.draw(*_shape, states);
		}

		if (_hasText)
		{
			target.draw(*_text, states);
		}

		// Or use the low-level API
		//states.texture = &m_texture;
		//target.draw(m_vertices, states);

	}

	Entity() 
		: _ref { sCnt++ }
	{}

	Entity(std::unique_ptr<sf::Shape> Shape, std::unique_ptr<sf::Text> Text, float PosX, float PosY, sf::Uint8 R, sf::Uint8 G, sf::Uint8 B, float VelX, float VelY)
		: 
		_ref { sCnt++ },
		_isShape { true },
		_shape { std::move(Shape) },
		_hasText{ true },
		_text{ std::move(Text) },
		_velocity { VelX , VelY }
	{
		_shape->setPosition( PosX, PosY );
		_shape->setFillColor(std::move(sf::Color(R, G, B)));
		_text->setPosition( PosX + _shape->getLocalBounds().width / 2.f - _text->getLocalBounds().width / 2.f, PosY + _shape->getLocalBounds().height / 2.f - _text->getLocalBounds().height / 2.f - _text->getLocalBounds().top );
	}
	Entity(std::unique_ptr<sf::Shape> Shape, std::unique_ptr<sf::Text> Text, sf::Vector2f Position, sf::Uint8 R, sf::Uint8 G, sf::Uint8 B, sf::Vector2f Velocity)
		:
		_ref { sCnt++ },
		_isShape { true },
		_shape { std::move(Shape) },
		_hasText { true },
		_text { std::move(Text) },
		_velocity { std::move(Velocity) }
	{
		_shape->setPosition( std::move(Position) );
		_shape->setFillColor( std::move(sf::Color(R, G, B)) );
		_text->setPosition( Position + sf::Vector2f{ _shape->getLocalBounds().width / 2.f - _text->getLocalBounds().width / 2.f, _shape->getLocalBounds().height / 2.f - _text->getLocalBounds().height / 2.f - _text->getLocalBounds().top });
	}
};

sf::Uint32 Entity::sCnt = 0;


void loadConfigFromFile(std::string FileName, sf::RenderWindow& Window, std::vector<std::unique_ptr<Entity>>& EntityVec);
void gameRun(sf::RenderWindow& Window, std::vector<std::unique_ptr<Entity>>& EntityVec);

class GlobalSettings
{
public:
	static sf::Font sGlobalFont;
	static std::size_t sCirclePoints;
	static unsigned int sTextSize;
	static sf::Color sTextColor;
};
sf::Font GlobalSettings::sGlobalFont;
std::size_t GlobalSettings::sCirclePoints;
unsigned int GlobalSettings::sTextSize;
sf::Color GlobalSettings::sTextColor;

void initGlobalSettings()
{
	if (!GlobalSettings::sGlobalFont.loadFromFile("fonts/arial.ttf"))
	{
		std::cout << "Missing default font" << std::endl;
	}
	GlobalSettings::sCirclePoints = 60U;
	GlobalSettings::sTextSize = 30U;
	GlobalSettings::sTextColor = sf::Color{ 0xFFFFFFFF };

}

sf::Uint32 main()
{
	initGlobalSettings();

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(800, 600), "Main Window", sf::Style::Default, settings);
	std::vector<std::unique_ptr<Entity>> entityVec;
	std::string fileName = "config.txt";
	loadConfigFromFile(fileName, window, entityVec);

	gameRun(window, entityVec);

}

enum class Shapes : sf::Uint32 {
	None = 0,
	Circle,
	Rectangle
};

std::map<std::string, Shapes> correspondingShape
{
	{"Circle", Shapes::Circle},
	{"Rectangle", Shapes::Rectangle}
};


template<typename T, typename... Args>
std::unique_ptr<Entity> ConstructEntity(float PosX, float PosY, float VelX, float VelY, sf::Uint8 ColorR8, sf::Uint8 ColorG8, sf::Uint8 ColorB8, std::unique_ptr<sf::Text> Text, Args... ArgsPack)
{
	return std::make_unique<Entity>(std::make_unique<T>(ArgsPack...), std::move(Text), PosX, PosY, ColorR8, ColorG8, ColorB8, VelX, VelY);
}

void loadConfigFromFile(std::string FileName, sf::RenderWindow& Window, std::vector<std::unique_ptr<Entity>>& EntityVec)
{
	std::ifstream configFileStream(FileName);
	std::string setting = "";
	std::string _ = "";
	sf::Uint32 cnt = 1;
	while (configFileStream.is_open() && configFileStream >> setting)
	{
		auto shapeIterator = correspondingShape.find(setting);
		if (shapeIterator != correspondingShape.end())
		{
			std::string textStr = "";
			float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
			float velX = 0.0f, velY = 0.0f;
			sf::Uint32 colorR = 0U, colorG = 0U, colorB = 0U;
			configFileStream >> textStr >> posX >> posY >> velX >> velY >> colorR >> colorG >> colorB;
			std::unique_ptr<sf::Text> text = std::make_unique<sf::Text>(textStr, GlobalSettings::sGlobalFont, GlobalSettings::sTextSize);
			text->setFillColor(GlobalSettings::sTextColor);
			Shapes shapeNum = shapeIterator->second;
			switch (shapeNum)
			{
			case Shapes::Circle:
			{
				std::size_t points = GlobalSettings::sCirclePoints;
				float radius = 0.0f;
				configFileStream >> radius;
				EntityVec.push_back(std::move(ConstructEntity<sf::CircleShape>(posX, posY, velX, velY, colorR, colorG, colorB, std::move(text), radius, points)));
				break;
			}
			case Shapes::Rectangle:
			{
				float width = 0.0f, height = 0.0f;
				configFileStream >> width >> height;
				EntityVec.push_back(std::move(ConstructEntity<sf::RectangleShape>(posX, posY, velX, velY, colorR, colorG, colorB, std::move(text), std::move(sf::Vector2f{ width, height }))));
				break;
			}
			default:
				std::cout << "Shape from line " << cnt << " not supported" << std::endl;
				std::getline(configFileStream, _);
				break;
			}
		}
		else if (setting == "Window")
		{
			sf::Uint32 width = 800U, height = 600U;
			configFileStream >> width >> height;
			Window.create(sf::VideoMode(width, height), "My window");
		}
		else if (setting == "Font")
		{
			std::string fontPath = "";
			unsigned int textSize = 0U;
			sf::Uint32 r = 0U, g = 0U, b = 0U;
			configFileStream >> fontPath >> textSize >> r >> g >> b;
			if (!GlobalSettings::sGlobalFont.loadFromFile(fontPath))
			{
				std::cout << "Font " << fontPath << " not found" << std::endl;
			}
			GlobalSettings::sTextSize = textSize;
			GlobalSettings::sTextColor = sf::Color( r, g, b );
			
		}
		else
		{
			std::cout << "Setting from line " << cnt << " not supported" << std::endl;
			std::getline(configFileStream, _);
		}
		cnt++;
	}
}

void gameRun(sf::RenderWindow& Window, std::vector<std::unique_ptr<Entity>>& EntityVec)
{
	Window.setFramerateLimit(60U);
	bool paused = true;
	while (Window.isOpen())
	{
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (Window.pollEvent(event))
		{
			switch (event.type)
			{
				// "close requested" event: we close the window
			case sf::Event::Closed:
			{
				Window.close();
				break;
			}
			// "key pressed" event
			case sf::Event::KeyPressed:
			{
				switch (event.key.scancode)
				{
					case sf::Keyboard::Scan::Space:
					{
						paused = !paused;
						break;
					}
				}
				break;
			}
			case sf::Event::Resized:
			{
				if (event.type == sf::Event::Resized)
				{
					Window.create(sf::VideoMode(Window.getSize().x, Window.getSize().y), "My Window");
					Window.setFramerateLimit(60U);
					std::cout << "new width: " << event.size.width << std::endl;
					std::cout << "new height: " << event.size.height << std::endl;
				}
				break;
			}

			}
		}

		// clear the window with black color
		Window.clear(sf::Color::Black);
		//Window.set

		// draw everything here...
		for (std::unique_ptr<Entity>& entity : EntityVec)
		{
			Window.draw(*entity);

			if (!paused)
			{
				entity->update(Window);
			}
		}

		// end the current frame
		Window.display();
	}
}