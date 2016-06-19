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

//main menu state

#ifndef MENU_MAIN_STATE_HPP_
#define MENU_MAIN_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>

#include <iostream>

namespace xy
{
    class MessageBus;
    class TextureResource;
    class FontResource;
}
class MenuMainState final : public xy::State
{
public:
    MenuMainState(xy::StateStack&, Context, xy::TextureResource&, xy::FontResource&);
    ~MenuMainState() = default;

    bool update(float) override;
    void draw() override;
    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    xy::StateID stateID() const override
    {
        return States::ID::MenuMain;
    }
private:
    xy::TextureResource& m_textureResource;
    xy::FontResource& m_fontResource;

    xy::MessageBus& m_messageBus;
    xy::UI::Container m_uiContainer;
    sf::Sprite m_cursorSprite;

    void buildMenu();
    void close(bool = true);
};
#endif //MENU_MAIN_STATE_HPP_