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

#ifndef LM_POST_BLEACH_HPP_
#define LM_POST_BLEACH_HPP_

#include <xygine/PostProcess.hpp>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Clock.hpp>

#include <array>

namespace lm
{
    class PostBlur final : public xy::PostProcess
    {
    public:
        PostBlur();
        ~PostBlur() = default;

        void apply(const sf::RenderTexture&, sf::RenderTarget&) override;

    private:

        float m_amount;
        sf::Clock m_clock;

        bool m_enabled;
        sf::Shader m_blurShader;
        sf::Shader m_downsampleShader;
        sf::Shader m_outShader;

        using TexturePair = std::array<sf::RenderTexture, 2u>;
        TexturePair m_firstPassTextures;
        TexturePair m_secondPassTextures;

        void initTextures(sf::Vector2u);
        void blurMultipass(TexturePair&);
        void blur(const sf::RenderTexture&, sf::RenderTexture&, const sf::Vector2f&);
        void downSample(const sf::RenderTexture&, sf::RenderTexture&);
    };
}

#endif //LM_POST_BLEACH_HPP_