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

#ifndef LM_GAME_MESSAGE_HPP_
#define LM_GAME_MESSAGE_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Text.hpp>

class GameMessage final : public xy::Component, public sf::Drawable
{
public:
    GameMessage(xy::MessageBus&, const sf::Font&, const std::string&, sf::Uint32 = 80u);
    ~GameMessage() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
    void entityUpdate(xy::Entity&, float) override;
    sf::FloatRect globalBounds() const override { return m_text.getGlobalBounds(); }

    void clear() { m_clear = true; }

private:
    bool m_clear;
    sf::Text m_text;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //LM_GAME_MESSAGE_HPP_