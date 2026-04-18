#ifndef TILE_H__
#define TILE_H__
#include "../GObj.h"
#include "../misc/util.h"

class Tile : public GObj
{
	friend class Tilemap;
	friend class Tileset;
	bool solid{ false };
	bool blank{ false };
public:
	Tile();
	Tile(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_ = false, sf::Vector2f position_ = { 0.f,0.f }, sf::Vector2f size_ = { 0.f,0.f });
	~Tile();

	void lazyInit(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_ = false, sf::Vector2f position_ = { 0.f,0.f }, sf::Vector2f size_ = { 0.f,0.f });
};
#endif

#ifndef TILESET_H__
#define TILESET_H__


#include <string>
#include <map>
class Tileset
{
	friend class Tilemap;
	int mPitch{ 0 };
	int mTotalTiles{ 0 };
	bool mTilesLoaded{ false };

public:
	
	std::map<int, Tile> tiles{};

	void loadTiles(const std::string& tilesetFilename_, Cfg::Textures texID);
	sf::Vector2f getTileSize();

};

#endif

#ifndef TILEMAP_H__
#define TILEMAP_H__

#include <vector>
class Tilemap
{
	int mCols{}, mRows{};
	Tileset mTset{};
	std::vector<Tile> mTmap{};

	static Tile BlankTile;

public:

	void loadMap(const std::string& tilemapFilename_, const std::string& tilesetFilename_);

	sf::Vector2f getTileSize();

	void renderMap(sf::RenderWindow& wnd_);

};

#endif