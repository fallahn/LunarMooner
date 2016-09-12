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

#ifndef LM_TUTORIAL_TIP_HPP_
#define LM_TUTORIAL_TIP_HPP_

#include <xygine/ShaderProperty.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>

namespace sf
{
    class Font;
}

namespace xy
{
    class MessageBus;
}

namespace lm
{
    class TutorialTip final : public sf::Drawable, public sf::Transformable, public xy::ShaderProperty
    {
    public:
        TutorialTip(sf::Font&, xy::MessageBus&);
        ~TutorialTip() = default;

        void setString(const std::string&);

        bool update(float);
        void reset();
        void start();

    private:

        xy::MessageBus& m_messageBus;

        enum class State
        {
            Started,
            Reset,
            Finished
        } m_state;

        float m_alpha;
        sf::CircleShape m_circle;
        sf::Text m_text;
        std::array<sf::Vertex, 6u> m_vertices;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_TUTORIAL_TIP_HPP_