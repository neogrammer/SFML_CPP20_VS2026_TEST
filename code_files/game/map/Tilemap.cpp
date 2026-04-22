#include "Tilemap.h"
#include "../res/Cfg.h"
#include <SFML/Graphics.hpp>
#include <fstream>

Tile::Tile()
	: Tile{ Cfg::Textures::Default, { {0,0},{64,64} }, true, { 0.f,0.f }, { 64.f,64.f } }
{}

Tile::Tile(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_, sf::Vector2f position_, sf::Vector2f size_)
	: GObj::GObj{ texID_, texRect_, uniDirectional_, position_, size_ }
{}

Tile::~Tile()
{}

void Tile::lazyInit(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_, sf::Vector2f position_, sf::Vector2f size_)
{
	
	this->texID = texID_;
	this->texRect = texRect_;
	this->uniDirectional = uniDirectional_;
	this->position = position_;
	this->size = size_;
}

void Tileset::loadTiles(const std::string& tilesetFilename_, Cfg::Textures texID)
{
	std::ifstream tFile;
	tFile.open(tilesetFilename_);

	int tw, th;

	if (tFile.is_open() && tFile.good() && !tFile.eof())
	{
		tFile >> mPitch >> mTotalTiles >> tw >> th;
		int rows = (int)(std::ceil((float)mTotalTiles / (float)mPitch));
		
		for (int y = 0; y < rows; y++)
			for (int x = 0; x < mPitch; x++)
			{
				int num = y * mPitch + x;
				tiles.emplace(std::pair{ num, Tile{texID, {{x * tw, y * th},{tw, th}}, true, {0.f,0.f},{(float)tw, (float)th} } });
				tiles[num].solid = true;
			}

		tFile.close();
	}
	else
	{
		throw std::runtime_error{"Definitely could not open your tileset file " + tilesetFilename_ + " you jackass!"};
	}

	if (tFile.is_open())
	{
		tFile.close();
	}

	mTilesLoaded = true;
}

sf::Vector2f Tileset::getTileSize()
{
	if (!mTilesLoaded)
		return sf::Vector2f();
	else
	{
		return tiles.at(0).getSize();
	}
}

Tile Tilemap::BlankTile = Tile{ Cfg::Textures::BlankTile, {{0,0},{64,64}}, true, {0.f,0.f}, {64.f,64.f} };

void Tilemap::loadMap(const std::string& tilemapFilename_, const std::string& tilesetFilename_)
{
	mTset.loadTiles(tilesetFilename_, Cfg::Textures::TilesetJungle1);
	std::ifstream tFile;
	tFile.open(tilemapFilename_);


	if (tFile.is_open() && tFile.good() && !tFile.eof())
	{



		tFile >> mCols >> mRows;
		

		solids.reserve(mCols * mRows);
		mTmap.reserve(mCols * mRows);
		for (int y = 0; y < mRows; y++)
			for (int x = 0; x < mCols; x++)
			{
				int num = y * mCols + x;
				int tNum;
				tFile >> tNum;

				if (tNum >= 0)
				{

					auto found = mTset.tiles.find(tNum);
					if (found == mTset.tiles.end())
					{
						// not in tileset, error
						throw std::runtime_error("Shit, tileset does not have the tile you asked for");
					}
					auto& t = mTset.tiles[tNum];
					mTmap.emplace_back(Tile{ t.getTexID(), t.getRect(), true, { float(x * getTileSize().x), float(y * getTileSize().y) }, { getTileSize() } });
					mTmap[num].solid = true;
					mTmap[num].blank = false;
					solids.emplace_back(&mTmap[num]);
				}
				else
				{
					if (tNum == -1)
					{
						mTmap.emplace_back(Tile{});
						mTmap[num].solid = false;
						auto& t = Tilemap::BlankTile;
						mTmap[num].lazyInit(t.getTexID(), t.getRect(), true, { float(x * getTileSize().x), float(y * getTileSize().y) }, { getTileSize() });
						mTmap[num].blank = true;
					}
					else
					{
						throw std::runtime_error("Crazy tile nums");
					}
				}

			}

		solids.shrink_to_fit();
		tFile.close();
	}
	else
	{
		throw std::runtime_error{ "Definitely could not open your tileset file " + tilesetFilename_ + " you jackass!" };
	}

	if (tFile.is_open())
	{
		tFile.close();
	}

}

sf::Vector2f Tilemap::getTileSize()
{
	return mTset.getTileSize();
}

void Tilemap::renderMap(sf::RenderWindow& wnd_)
{
	for (int i = 0; i < mTmap.size(); i++)
	{
		if (!mTmap[i].blank)
			wnd_.draw(*mTmap[i].sprite());
	}
}

void Tilemap::renderScreen(sf::RenderWindow& wnd_, const sf::View& view)
{
	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size = view.getSize();
	const float left = center.x - (size.x * 0.5f);
	const float top = center.y - (size.y * 0.5f);
	const float right = center.x + (size.x * 0.5f);
	const float bottom = center.y + (size.y * 0.5f);

	for (Tile& tile : mTmap)
	{
		if (tile.blank)
		{
			continue;
		}

		const sf::Vector2f tilePos = tile.getPosSafe();
		const sf::Vector2f tileSize = tile.getSizeSafe();
		const bool visible =
			tilePos.x < right &&
			tilePos.x + tileSize.x > left &&
			tilePos.y < bottom &&
			tilePos.y + tileSize.y > top;

		if (visible)
		{
			wnd_.draw(*tile.sprite());
		}
	}
}


std::vector<GObj*>& Tilemap::getSolids()
{
	return solids;
}

const std::vector<GObj*>& Tilemap::getSolids() const
{
	return solids;
}
