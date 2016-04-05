/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

LunarMooner - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#ifndef LM_PLANET_HPP_
#define LM_PLANET_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/MultiRenderTexture.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>

namespace lm
{
    class PlanetDrawable final : public sf::Drawable, public xy::Component
    {
    public:
        PlanetDrawable(xy::MessageBus&, float);
        ~PlanetDrawable() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;

        sf::FloatRect globalBounds() const override
        {
            return m_globalBounds;
        }

        void setBaseNormal(sf::Texture&);
        void setDetailNormal(sf::Texture&);
        void setDiffuseTexture(sf::Texture&);
        void setMaskTexture(sf::Texture&);

        void setPrepassShader(sf::Shader&);
        void setNormalShader(sf::Shader&);

    private:

        sf::Texture* m_baseNormal;
        sf::Texture* m_detailNormal;
        sf::Texture* m_diffuseTexture;
        sf::Texture* m_maskTexture;

        xy::MultiRenderTexture m_renderTexture;

        sf::Shader* m_prepassShader;
        sf::Shader* m_normalShader;

        sf::Vector2f m_textureOffset;
        sf::Vector2f m_textureVelocity;

        std::vector<sf::Vertex> m_vertices;
        float m_radius;
        sf::FloatRect m_localBounds;
        sf::FloatRect m_globalBounds;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_PLANET_HPP_