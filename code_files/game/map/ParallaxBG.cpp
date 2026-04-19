#include "ParallaxBG.h"

ParallaxBG::Layer::Layer(
    const sf::Texture& inTexture,
    float inScrollPercentX,
    float inScrollPercentY,
    const sf::Color& inTint
)
    : texture(&inTexture)
    , scrollPercentX(inScrollPercentX)
    , scrollPercentY(inScrollPercentY)
    , tint(inTint)
    , vertices{}
{}

ParallaxBG::ParallaxBG(std::size_t expectedLayerCount)
    : m_layers()
    , m_anchorWorldPosition(0.0f, 0.0f)
    , m_cameraWorldTopLeft(0.0f, 0.0f)
    , m_viewportSize(1600.0f, 900.0f)
{
    m_layers.reserve(expectedLayerCount);
}

void ParallaxBG::reserveLayers(std::size_t expectedLayerCount)
{
    m_layers.reserve(expectedLayerCount);
}

void ParallaxBG::clearLayers()
{
    m_layers.clear();
}

void ParallaxBG::addLayer(
    const sf::Texture& texture,
    float scrollPercentOfFrontLayerX,
    float scrollPercentOfFrontLayerY,
    const sf::Color& tint
)
{
    m_layers.emplace_back(texture, scrollPercentOfFrontLayerX, scrollPercentOfFrontLayerY, tint);
    rebuildLayerVertices(m_layers.back());
}

void ParallaxBG::addLayer(
    const sf::Texture& texture,
    float scrollPercentOfFrontLayer,
    const sf::Color& tint
)
{
    addLayer(texture, scrollPercentOfFrontLayer, scrollPercentOfFrontLayer, tint);
}

void ParallaxBG::setAnchorWorldPosition(float x, float y)
{
    m_anchorWorldPosition.x = x;
    m_anchorWorldPosition.y = y;
}

void ParallaxBG::setAnchorWorldPosition(const sf::Vector2f& worldPosition)
{
    m_anchorWorldPosition = worldPosition;
}

sf::Vector2f ParallaxBG::getAnchorWorldPosition() const
{
    return m_anchorWorldPosition;
}

void ParallaxBG::update(const sf::Vector2f& cameraWorldTopLeft, const sf::Vector2f& viewportSize)
{
    m_cameraWorldTopLeft = cameraWorldTopLeft;
    m_viewportSize = viewportSize;

    for (Layer& layer : m_layers)
    {
        rebuildLayerVertices(layer);
    }
}

void ParallaxBG::update(float cameraLeft, float cameraTop, float viewWidth, float viewHeight)
{
    update(
        sf::Vector2f(cameraLeft, cameraTop),
        sf::Vector2f(viewWidth, viewHeight)
    );
}

void ParallaxBG::update(const sf::View& view)
{
    const sf::Vector2f viewSize = view.getSize();
    const sf::Vector2f viewCenter = view.getCenter();

    const sf::Vector2f cameraTopLeft(
        viewCenter.x - (viewSize.x * 0.5f),
        viewCenter.y - (viewSize.y * 0.5f)
    );

    update(cameraTopLeft, viewSize);
}

std::size_t ParallaxBG::getLayerCount() const
{
    return m_layers.size();
}

void ParallaxBG::setLayerTint(std::size_t layerIndex, const sf::Color& tint)
{
    Layer& layer = requireLayer(layerIndex);
    layer.tint = tint;
    rebuildLayerVertices(layer);
}

void ParallaxBG::setLayerAlpha(std::size_t layerIndex, std::uint8_t alpha)
{
    Layer& layer = requireLayer(layerIndex);
    layer.tint.a = alpha;
    rebuildLayerVertices(layer);
}

void ParallaxBG::setLayerScrollPercent(std::size_t layerIndex, float scrollPercentX, float scrollPercentY)
{
    Layer& layer = requireLayer(layerIndex);
    layer.scrollPercentX = scrollPercentX;
    layer.scrollPercentY = scrollPercentY;
    rebuildLayerVertices(layer);
}

const ParallaxBG::Layer& ParallaxBG::getLayer(std::size_t layerIndex) const
{
    return requireLayer(layerIndex);
}

ParallaxBG::Layer& ParallaxBG::getLayer(std::size_t layerIndex)
{
    return requireLayer(layerIndex);
}

void ParallaxBG::rebuildLayerVertices(Layer& layer)
{
    if (layer.texture == nullptr)
    {
        return;
    }

    const sf::Vector2u textureSizeU = layer.texture->getSize();
    if (textureSizeU.x == 0 || textureSizeU.y == 0)
    {
        return;
    }

    const float textureWidth = static_cast<float>(textureSizeU.x);
    const float textureHeight = static_cast<float>(textureSizeU.y);

    const float parallaxSampleLeft =
        (m_cameraWorldTopLeft.x - m_anchorWorldPosition.x) * layer.scrollPercentX;

    const float parallaxSampleTop =
        (m_cameraWorldTopLeft.y - m_anchorWorldPosition.y) * layer.scrollPercentY;

    const float tileIndexX = std::floor(parallaxSampleLeft / textureWidth);
    const float tileIndexY = std::floor(parallaxSampleTop / textureHeight);

    const float tileSampleOriginX = tileIndexX * textureWidth;
    const float tileSampleOriginY = tileIndexY * textureHeight;

    const float firstTileWorldLeft =
        m_cameraWorldTopLeft.x - (parallaxSampleLeft - tileSampleOriginX);

    const float firstTileWorldTop =
        m_cameraWorldTopLeft.y - (parallaxSampleTop - tileSampleOriginY);

    writeQuad(
        layer.vertices, 0,
        firstTileWorldLeft,
        firstTileWorldTop,
        textureWidth,
        textureHeight,
        0.0f, 0.0f,
        textureWidth,
        textureHeight,
        layer.tint
    );

    writeQuad(
        layer.vertices, 6,
        firstTileWorldLeft + textureWidth,
        firstTileWorldTop,
        textureWidth,
        textureHeight,
        0.0f, 0.0f,
        textureWidth,
        textureHeight,
        layer.tint
    );

    writeQuad(
        layer.vertices, 12,
        firstTileWorldLeft,
        firstTileWorldTop + textureHeight,
        textureWidth,
        textureHeight,
        0.0f, 0.0f,
        textureWidth,
        textureHeight,
        layer.tint
    );

    writeQuad(
        layer.vertices, 18,
        firstTileWorldLeft + textureWidth,
        firstTileWorldTop + textureHeight,
        textureWidth,
        textureHeight,
        0.0f, 0.0f,
        textureWidth,
        textureHeight,
        layer.tint
    );
}

void ParallaxBG::writeQuad(
    std::array<sf::Vertex, 24>& outVertices,
    std::size_t startVertex,
    float left,
    float top,
    float width,
    float height,
    float texLeft,
    float texTop,
    float texWidth,
    float texHeight,
    const sf::Color& color
)
{
    const float right = left + width;
    const float bottom = top + height;

    const float texRight = texLeft + texWidth;
    const float texBottom = texTop + texHeight;

    sf::Vertex& v0 = outVertices[startVertex + 0];
    sf::Vertex& v1 = outVertices[startVertex + 1];
    sf::Vertex& v2 = outVertices[startVertex + 2];
    sf::Vertex& v3 = outVertices[startVertex + 3];
    sf::Vertex& v4 = outVertices[startVertex + 4];
    sf::Vertex& v5 = outVertices[startVertex + 5];

    v0.position = sf::Vector2f(left, top);
    v1.position = sf::Vector2f(right, top);
    v2.position = sf::Vector2f(right, bottom);

    v3.position = sf::Vector2f(left, top);
    v4.position = sf::Vector2f(right, bottom);
    v5.position = sf::Vector2f(left, bottom);

    v0.texCoords = sf::Vector2f(texLeft, texTop);
    v1.texCoords = sf::Vector2f(texRight, texTop);
    v2.texCoords = sf::Vector2f(texRight, texBottom);

    v3.texCoords = sf::Vector2f(texLeft, texTop);
    v4.texCoords = sf::Vector2f(texRight, texBottom);
    v5.texCoords = sf::Vector2f(texLeft, texBottom);

    v0.color = color;
    v1.color = color;
    v2.color = color;
    v3.color = color;
    v4.color = color;
    v5.color = color;
}

ParallaxBG::Layer& ParallaxBG::requireLayer(std::size_t layerIndex)
{
    if (layerIndex >= m_layers.size())
    {
        throw std::out_of_range("ParallaxBG layer index out of range");
    }

    return m_layers[layerIndex];
}

const ParallaxBG::Layer& ParallaxBG::requireLayer(std::size_t layerIndex) const
{
    if (layerIndex >= m_layers.size())
    {
        throw std::out_of_range("ParallaxBG layer index out of range");
    }

    return m_layers[layerIndex];
}

void ParallaxBG::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (const Layer& layer : m_layers)
    {
        if (layer.texture == nullptr)
        {
            continue;
        }

        states.texture = layer.texture;

        target.draw(
            layer.vertices.data(),
            layer.vertices.size(),
            sf::PrimitiveType::Triangles,
            states
        );
    }
}