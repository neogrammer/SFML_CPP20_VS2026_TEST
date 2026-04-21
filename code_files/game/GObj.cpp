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

void GObj::setFacingRightCpy(bool cond_)
{
    copy->facingRight = cond_;
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

const sf::Vector2f GObj::getOffSafe() const
{
    return offset;
}

void GObj::setAccleration(sf::Vector2f acceleration_)
{
    acceleration = acceleration_;
}



GObj::GObj()
    : GObj(Cfg::Textures::None, sf::IntRect{ {0,0},{32,32} }, false, { 0.f,0.f }, { 32.f,32.f }, {0.f,0.f}, false)
{   
}

GObj::GObj(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_, sf::Vector2f position_, sf::Vector2f size_, sf::Vector2f offset_, bool isCopy)
    : texID{ texID_ }
    , texRect{ texRect_ }
    , uniDirectional{ uniDirectional_ }
    , position{ position_ }
    , size{ size_ }
    , offset{ offset_ }
    , facingRight{ true }
{

    if (!isCopy)
        copy = new GObj{ texID_, texRect_, uniDirectional_, position_, size_, offset_, true };
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

const sf::Vector2f GObj::getPosSafe() const
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

const sf::Vector2f GObj::getVelSafe() const
{
    return velocity;
}

const sf::Vector2f GObj::getSizeSafe() const
{
    return size;
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

    out->setPosition(position - offset);// - offset);
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
    if (copy->velocity.x > 0.f) 
        copy->velocity.x = copy->velocity.x - 0.009f;
    if (copy->velocity.x < 0.f) 
        copy->velocity.x = copy->velocity.x + 0.009f;

    
        copy->velocity = copy->velocity + copy->acceleration;
        
       
    }

}

void GObj::setCopyPos(float dt_)
{
    copy->position = this->position + copy->velocity * dt_;
}
void GObj::swapdate()
{
    this->acceleration = copy->acceleration;
    copy->acceleration = { 0.f,0.f };
    this->velocity = copy->velocity;
    this->position = copy->position;
    this->grounded = copy->grounded;

    this->setFacingRight(copy->isFacingRight());
}

