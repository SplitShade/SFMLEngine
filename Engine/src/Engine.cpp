// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include <memory>
#include <fstream>

class Window
{
private:
	sf::Uint32 _width = 800, _height = 600;
	std::string _name = "My window";
	std::shared_ptr<sf::RenderWindow> _window;

public:

	[[nodiscard]] sf::Uint32 getWidth() const { return _width; }
	[[nodiscard]] sf::Uint32 getHeight() const { return _height; }
	[[nodiscard]] std::shared_ptr<sf::RenderWindow> getWindow() const { return _window; }

	void setWidth(sf::Uint32 Width) { _width = Width; }
	void setHeight(sf::Uint32 Height) { _height = Height; }
	[[nodiscard]] bool isOpen() const { return _window->isOpen(); }

	Window() 
		:_window{ std::make_unique<sf::RenderWindow>(sf::VideoMode(_width, _height), _name) }
	{}
	Window(sf::Uint32 Width, sf::Uint32 Height, std::string Name = "My window")
		:_width{ Width }, _height{ Height }, _name{ Name }, _window{ std::make_unique<sf::RenderWindow>(sf::VideoMode(Width, Height), Name) }
	{}
};

class Entity
{
private:
	sf::Uint32 _ref;

	bool _isShape = false;
	std::unique_ptr<sf::Shape> _shape = nullptr;

	bool _hasText = false;
	std::unique_ptr<sf::Text> _text = nullptr;

	float _posX = 0, _posY = 0, _posZ = 0;

	float _velX = 0, _velY = 0;

public:
	static sf::Uint32 sCnt;

	[[nodiscard]] bool isShape() const { return _isShape; }
	[[nodiscard]] sf::Shape& getShape() const { return *_shape; }
	[[nodiscard]] bool hasText() const { return _hasText; }
	[[nodiscard]] sf::Text& getText() const { return *_text; }


	Entity() 
		: _ref { sCnt++ }
	{}

	Entity(std::unique_ptr<sf::Shape> Shape, std::unique_ptr<sf::Text> Text, float PosX, float PosY, float PosZ, sf::Uint8 R, sf::Uint8 G, sf::Uint8 B, float VelX, float VelY)
		: 
		_ref { sCnt++ },
		_isShape { true },
		_shape { std::move(Shape) },
		_hasText{ true },
		_text{ std::move(Text) },
		_posX{ PosX }, _posY{ PosY }, _posZ{ PosZ },
		_velX { VelX }, _velY { VelY }
	{
		_shape->setPosition(sf::Vector2f{ _posX, _posY });
		_shape->setFillColor(std::move(sf::Color(R, G, B)));
		_text->setPosition(sf::Vector2f{ _posX + _shape->getLocalBounds().width/2.f - _text->getLocalBounds().width / 2.f, _posY + _shape->getLocalBounds().height / 2.f - _text->getLocalBounds().height / 2.f - _text->getLocalBounds().top });
	}

};

sf::Uint32 Entity::sCnt = 0;


void loadConfigFromFile(std::string FileName, std::shared_ptr<Window> Window, std::vector<std::unique_ptr<Entity>>& EntityVec);
void gameRun(std::shared_ptr<Window> Window, std::vector<std::unique_ptr<Entity>>& EntityVec);

class GlobalFontSettings
{
public:
	static sf::Font sGlobalFont;
};
sf::Font GlobalFontSettings::sGlobalFont;

void initGlobalSettings()
{
	if (!GlobalFontSettings::sGlobalFont.loadFromFile("fonts/arial.ttf"))
	{
		std::cout << "Missing default font" << std::endl;
	}
}

sf::Uint32 main()
{
	initGlobalSettings();
	std::shared_ptr<Window> window = std::make_shared<Window>(800, 600);
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
std::unique_ptr<Entity> ConstructEntity(float PosX, float PosY, float PosZ, float VelX, float VelY, sf::Uint8 ColorR8, sf::Uint8 ColorG8, sf::Uint8 ColorB8, std::unique_ptr<sf::Text> Text, Args... ArgsPack)
{
	return std::make_unique<Entity>(std::make_unique<T>(ArgsPack...), std::move(Text), PosX, PosY, PosZ, ColorR8, ColorG8, ColorB8, VelX, VelY);
}

void loadConfigFromFile(std::string FileName, std::shared_ptr<Window> Window, std::vector<std::unique_ptr<Entity>>& EntityVec) 
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
			float speedX = 0.0f, speedY = 0.0f;
			float velX = 0.0f, velY = 0.0f;
			sf::Int32 colorR32 = 0U, colorG32 = 0U, colorB32 = 0U;
			configFileStream >> textStr >> posX >> posY >> speedX >> speedY >> colorR32 >> colorG32 >> colorB32;
			sf::Uint8 colorR8 = colorR32, colorG8 = colorG32, colorB8 = colorB32;
			std::unique_ptr<sf::Text> text = std::make_unique<sf::Text>(textStr, GlobalFontSettings::sGlobalFont);

			Shapes shapeNum = shapeIterator->second;
			switch (shapeNum)
			{
			case Shapes::Circle:
			{
				std::size_t points = 300U;
				float radius = 0.0f;
				configFileStream >> radius;
				EntityVec.push_back(std::move(ConstructEntity<sf::CircleShape>(posX, posY, posZ, velX, velY, colorR8, colorG8, colorB8, std::move(text), radius, points)));
				break;
			}
			case Shapes::Rectangle:
			{
				std::size_t points = 300U;
				float width = 0.0f, height = 0.0f;
				configFileStream >> width >> height;
				EntityVec.push_back(std::move(ConstructEntity<sf::RectangleShape>(posX, posY, posZ, velX, velY, colorR8, colorG8, colorB8, std::move(text), std::move(sf::Vector2f{ width, height }))));
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
			Window->getWindow()->setSize(sf::Vector2u{ width, height });
		}
		else if (setting == "Font")
		{
			std::string fontPath;
			configFileStream >> fontPath;
			if (!GlobalFontSettings::sGlobalFont.loadFromFile(fontPath))
			{
				std::cout << "Font " << fontPath << " not found" << std::endl;
			}
			
		}
		else
		{
			std::cout << "Setting from line " << cnt << " not supported" << std::endl;
			std::getline(configFileStream, _);
		}
		cnt++;
	}
}

void gameRun(std::shared_ptr<Window> Window, std::vector<std::unique_ptr<Entity>>& EntityVec)
{
	const auto& window = Window->getWindow();
	window->setFramerateLimit(60U);
	while (window->isOpen())
	{
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window->pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window->close();
		}

		// clear the window with black color
		window->clear(sf::Color::Black);

		// draw everything here...
		for (std::unique_ptr<Entity>& entityPtr : EntityVec)
		{
			const Entity& entity = *entityPtr;
			if (entity.isShape())
			{
				window->draw(entity.getShape());
				auto a = entity.getShape().getLocalBounds();
				std::cout << a.top << " " << a.left << " " << a.height << " " << a.width << " " << std::endl;
				auto b = entity.getShape().getGlobalBounds();
				std::cout << b.top << " " << b.left << " " << b.height << " " << b.width << " " << std::endl;
			}
			if (entity.hasText())
			{
				window->draw(entity.getText());
			}
		}

		// end the current frame
		window->display();
	}
}