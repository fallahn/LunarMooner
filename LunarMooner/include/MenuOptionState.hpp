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

#ifndef MENU_OPTION_STATE_HPP_
#define MENU_OPTION_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace xy
{
    class MessageBus;
    class TextureResource;
    class FontResource;
}
class MenuOptionState final : public xy::State
{
public:
    MenuOptionState(xy::StateStack& stateStack, Context context, xy::TextureResource&, xy::FontResource&, bool = false);
    ~MenuOptionState() = default;

    bool update(float dt) override;
    void draw() override;
    bool handleEvent(const sf::Event& evt) override;
    void handleMessage(const xy::Message&) override;
    xy::StateID stateID() const override
    {
        return States::ID::MenuOptions;
    }
private:
    xy::TextureResource& m_textureResource;
    xy::FontResource& m_fontResource;

    xy::MessageBus& m_messageBus;
    sf::Sprite m_cursorSprite;

    xy::UI::Container m_optionContainer;
    xy::UI::Container m_inputContainer;
    xy::UI::Container* m_activeContainer;

    bool m_pausedGame;

    void buildOptionsMenu(const sf::Font&);
    void buildControlMenu(const sf::Font&);

    //void buildMenu(const sf::Font&);
    void close();
};
#endif //MENU_OPTION_STATE_HPP_