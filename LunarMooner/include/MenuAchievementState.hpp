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

#ifndef LM_ACHIEVEMENT_STATE_HPP_
#define LM_ACHIEVEMENT_STATE_HPP_

#include <StateIds.hpp>
#include <Achievements.hpp>

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>

namespace xy
{
    class TextureResource;
    class FontResource;
    class MessageBus;
}

namespace sf
{
    class Font;
}

class PlayerProfile;
class MenuAchievementState final : public xy::State
{
public:
    MenuAchievementState(xy::StateStack&, Context, xy::TextureResource&, xy::FontResource&, const PlayerProfile&);
    ~MenuAchievementState() = default;

    bool update(float) override;
    void draw() override;
    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;

    xy::StateID stateID() const override
    {
        return States::ID::MenuAchievement;
    }

private:

    xy::TextureResource& m_textureResource;
    xy::FontResource& m_fontResource;
    const PlayerProfile& m_profile;

    xy::MessageBus& m_messageBus;
    xy::UI::Container m_uiContainer;
    sf::Sprite m_cursorSprite;

    struct Graphic
    {
        sf::CircleShape c;
        sf::Text t;
    };
    std::array<Graphic, AchievementID::Count> m_graphics;

    void buildMenu(const sf::Font&);
};

#endif //LM_ACHIEVEMENT_STATE_HPP_