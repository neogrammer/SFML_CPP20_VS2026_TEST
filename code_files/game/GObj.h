#ifndef GOBJ_H__
#define GOBJ_H__
#include <memory>
#include <vector>
#include <utility>
#include "Component.h"
#include <SFML/Graphics.hpp>
#include "res/ResourceManager.h"
#include <res/Cfg.h>

class GObj
{
protected:
	sf::Vector2f position;

	Cfg::Textures texID;
	sf::IntRect texRect;
	sf::Vector2f velocity;
	sf::Vector2f acceleration;
	sf::Vector2f offset;
	sf::Vector2f size;
	bool facingRight{ true };
	bool uniDirectional;
	static sf::Texture defaultTex;
public:

	void setUniDirectional(bool cond_);
	void setFacingRight(bool cond_);
	void setSize(sf::Vector2f);
	void setOffset(sf::Vector2f);
	void setRect(sf::IntRect rect_);
	void setID(Cfg::Textures texID_);
	void setAccleration(sf::Vector2f offset_);
	void setPos(sf::Vector2f pos_);
	void setVel(sf::Vector2f vel_);
	sf::Vector2f getVel();
	void move(sf::Vector2f amt_);
	std::vector<std::shared_ptr<Component>> mComponents;

public:
	void addComponent(std::shared_ptr<Component> component);
	template <typename T>
	std::shared_ptr<T> getComponent();

		
public:
	GObj();
	GObj(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_ = false, sf::Vector2f position_ = { 0.f,0.f }, sf::Vector2f size_ = { 0.f,0.f }, sf::Vector2f offset_ = { 0.f,0.f });
	~GObj();
	GObj(const GObj&);
	GObj& operator=(const GObj&);

	sf::Vector2f getOffset();
	sf::Vector2f getAcceleration();
	bool isFacingRight();
	bool isUniDirectional();
	sf::Vector2f getPos();
	sf::Vector2f getSize();
	sf::Vector2f getVelocity();
	Cfg::Textures getTexID();

	std::unique_ptr<sf::Sprite> sprite();
	virtual void update(float dt_, sf::RenderWindow& wnd_);
};


#endif

template<typename T>
inline std::shared_ptr<T> GObj::getComponent()
{
	return std::get<0>(mComponents);
}
