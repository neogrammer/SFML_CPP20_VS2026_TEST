#include "GObj.h"
#include <iostream>
sf::Texture GObj::defaultTex = sf::Texture{"assets/textures/invariant.png"};


void GObj::addComponent(std::shared_ptr<Component> component)
{

}



void GObj::setRect(sf::IntRect rect_)
{
    texRect = rect_;
}

void GObj::setID(Cfg::Textures texID_)
{
    texID = texID_;
}

void GObj::setUniDirectional(bool cond_)
{
    uniDirectional = cond_;
}

void GObj::setFacingRight(bool cond_)
{
    facingRight = cond_;
}

void GObj::setSize(sf::Vector2f size_)
{
    size = size_;
}

void GObj::setOffset(sf::Vector2f offset_)
{
    offset = offset_;
}

sf::IntRect GObj::getRect()
{
    return texRect;
}

void GObj::setAccleration(sf::Vector2f acceleration_)
{
    acceleration = acceleration_;
}



GObj::GObj()
    : texID{ Cfg::Textures::None }
    , texRect{ sf::IntRect{{0,0},{32,32}} }
    , uniDirectional{ false }
    , position{ 0.f,0.f }
    , size{ 32.f, 32.f }
    , offset{ 0.f,0.f }
    , facingRight{ true }
{
    auto check = defaultTex.loadFromFile("Assets/textures/invariant.png");
    if (!check)
    {
        std::cout << "Unable to open invariant into the default texture variable GObj" << std::endl;
    }
}

GObj::GObj(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_, sf::Vector2f position_, sf::Vector2f size_, sf::Vector2f offset_)
    : texID{ texID_ }
    , texRect{ texRect_ }
    , uniDirectional{ uniDirectional_ }
    , position{ position_ }
    , size{ size_ }
    , offset{ offset_ }
    , facingRight{ true }
{

}

GObj::~GObj()
{
    if (copy)
        delete copy;
}

GObj::GObj(const GObj& o)
    : GObj{ o.texID, o.texRect, o.uniDirectional, o.position, o.size, o.offset }
{
}

GObj& GObj::operator=(const GObj& o)
{
    GObj& me = *this;
    me.texID = o.texID;
    me.texRect = o.texRect;
    me.uniDirectional = o.uniDirectional;
    me.position = o.position;
    me.size = o.size;
    me.offset = o.offset;
    me.facingRight = o.facingRight;

    return me;
}

sf::Vector2f GObj::getOffset()
{
    return offset;
}

sf::Vector2f GObj::getAcceleration()
{
    return acceleration;
}

Cfg::Textures GObj::getTexID()
{
    return texID;
}

sf::Vector2f GObj::getPos()
{
    return position;
}

sf::Vector2f GObj::getSize()
{
    return size;
}

sf::Vector2f GObj::getVelocity()
{
    return velocity;
}

void GObj::setPos(sf::Vector2f pos_)
{
    position = pos_;
}

void GObj::setVel(sf::Vector2f vel_)
{
    velocity = vel_;
}

void GObj::setUniDirectionalCpy(bool cond_)
{
    copy->setUniDirectional(cond_);
}

void GObj::setFacingRightCpy(bool cond_)
{
    copy->setFacingRight(cond_);
}

void GObj::setSizeCpy(sf::Vector2f size)
{
    copy->setSize(size);
}

void GObj::setOffsetCpy(sf::Vector2f off)
{
    copy->setOffset(off);
}

void GObj::setRectCpy(sf::IntRect rect_)
{
    copy->setRect(rect_);
}

void GObj::setIDCpy(Cfg::Textures texID_)
{
    copy->setID(texID_);
}

void GObj::setAcclerationCpy(sf::Vector2f offset_)
{
    copy->setAccleration(offset_);
}

void GObj::setPosCpy(sf::Vector2f pos_)
{
    copy->setPos(pos_);
}

void GObj::setVelCpy(sf::Vector2f vel_)
{
    copy->setVel(vel_);
}

sf::Vector2f GObj::getVel()
{
    return velocity;
}

void GObj::move(sf::Vector2f amt_)
{
    position += amt_;
}

bool GObj::isFacingRight()
{
    if (uniDirectional) return true;

    if (facingRight)
        return true;
    else
        return false;
}

bool GObj::isUniDirectional()
{
    return uniDirectional;
}
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

std::unique_ptr<sf::Sprite> GObj::sprite()
{

    std::unique_ptr<sf::Sprite> out;

    if (texID == Cfg::Textures::Default)
    {
        out = std::make_unique<sf::Sprite>(defaultTex);  //  Get from Cfg::Textures
    }
    else
    {
        out = std::make_unique<sf::Sprite>(Cfg::textures.get((int)texID));  //  Get from Cfg::Textures
    }

    out->setPosition(position - offset);
    out->setTextureRect(texRect);

    return std::move(out);

}

void GObj::update(float dt_)
{
    if (copy == nullptr)
    {
        std::cout << "gobj copy not good!" << std::endl;
        auto err = std::runtime_error(R"(gobj copy not good!)");
        throw err;
    }
    if (copy)
    {
    // friction
    if (this->velocity.x > 0.f) 
        copy->velocity.x = this->velocity.x - 0.009f;
    if (this->velocity.x < 0.f) 
        copy->velocity.x = this->velocity.x + 0.009f;

    
        copy->velocity = this->velocity + this->acceleration;
        copy->position = this->position + copy->velocity * dt_;
        copy->acceleration = { 0.f,0.f };
    }
}

void GObj::swapdate()
{
    this->acceleration = copy->acceleration;
    this->velocity = copy->velocity;
    this->position = copy->position;
}

