#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <stdexcept>

class ParallaxBG : public sf::Drawable
{
public:
    class Layer
    {
    public:
        Layer(
            const sf::Texture& inTexture,
            float inScrollPercentX,
            float inScrollPercentY,
            const sf::Color& inTint
        );

    public:
        const sf::Texture* texture;
        float scrollPercentX;
        float scrollPercentY;
        sf::Color tint;

        // 4 quads * 6 verts each = 24 verts
        std::array<sf::Vertex, 24> vertices;
    };

public:
    explicit ParallaxBG(std::size_t expectedLayerCount = 0);

public:
    void reserveLayers(std::size_t expectedLayerCount);
    void clearLayers();

    // Add farthest -> closest.
    void addLayer(
        const sf::Texture& texture,
        float scrollPercentOfFrontLayerX,
        float scrollPercentOfFrontLayerY,
        const sf::Color& tint = sf::Color::White
    );

    void addLayer(
        const sf::Texture& texture,
        float scrollPercentOfFrontLayer,
        const sf::Color& tint = sf::Color::White
    );

    void setAnchorWorldPosition(float x, float y);
    void setAnchorWorldPosition(const sf::Vector2f& worldPosition);
    sf::Vector2f getAnchorWorldPosition() const;

    // Explicit camera top-left update
    void update(const sf::Vector2f& cameraWorldTopLeft, const sf::Vector2f& viewportSize);
    void update(float cameraLeft, float cameraTop, float viewWidth, float viewHeight);

    // Readability overload for SFML view
    void update(const sf::View& view);

    std::size_t getLayerCount() const;

    // Layer controls
    void setLayerTint(std::size_t layerIndex, const sf::Color& tint);
    void setLayerAlpha(std::size_t layerIndex, std::uint8_t alpha);
    void setLayerScrollPercent(std::size_t layerIndex, float scrollPercentX, float scrollPercentY);

    const Layer& getLayer(std::size_t layerIndex) const;
    Layer& getLayer(std::size_t layerIndex);

private:
    void rebuildLayerVertices(Layer& layer);

    void writeQuad(
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
    );

    Layer& requireLayer(std::size_t layerIndex);
    const Layer& requireLayer(std::size_t layerIndex) const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    std::vector<Layer> m_layers;

    sf::Vector2f m_anchorWorldPosition;
    sf::Vector2f m_cameraWorldTopLeft;
    sf::Vector2f m_viewportSize;
};