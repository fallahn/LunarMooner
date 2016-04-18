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

#include <MenuAchievementState.hpp>
#include <PlayerProfile.hpp>

#include <xygine/Resource.hpp>
#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>

#include <xygine/ui/Button.hpp>
#include <xygine/ui/Label.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

MenuAchievementState::MenuAchievementState(xy::StateStack& ss, Context context, xy::TextureResource& tr, xy::FontResource& fr, const PlayerProfile& profile)
    : xy::State(ss, context),
    m_textureResource(tr),
    m_fontResource(fr),
    m_profile(profile),
    m_messageBus(context.appInstance.getMessageBus()),
    m_uiContainer(m_messageBus)
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));

    const auto& font = fr.get("achievements_52");
    buildMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateId = States::ID::MenuAchievement;
}

//public
bool MenuAchievementState::update(float dt)
{
    m_uiContainer.update(dt);
    return true;
}

void MenuAchievementState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    for (const auto& g : m_graphics)
    {
        rw.draw(g.c);
        rw.draw(g.t);
    }

    rw.draw(m_uiContainer);
    rw.draw(m_cursorSprite);
}

bool MenuAchievementState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    return false; //consume events
}

void MenuAchievementState::handleMessage(const xy::Message& msg)
{

}

namespace
{
    const sf::Vector2f circlePos(460.f, 300.f);
    const sf::Vector2f textPos(510.f, 300.f);
    const float rowSpace = 40.f;
}

//private
void MenuAchievementState::buildMenu(const sf::Font& font)
{
    auto label = xy::UI::create<xy::UI::Label>(font);
    label->setString("Achievements");
    label->setAlignment(xy::UI::Alignment::Centre);
    label->setPosition(960.f, 160.f);
    label->setCharacterSize(40u);
    m_uiContainer.addControl(label);

    for (auto i = 0; i < AchievementID::Count; ++i)
    {
        auto& c = m_graphics[i].c;
        c.setRadius(10.f);
        c.setPosition(circlePos.x, circlePos.y + (i * rowSpace));
        c.setFillColor((m_profile.hasAchievement(static_cast<AchievementID>(i))) ? sf::Color::Green : sf::Color::Red);

        auto& t = m_graphics[i].t;
        t.setFont(font);
        t.setPosition(textPos.x, textPos.y + (i * rowSpace));
        t.setString(achievementNames[i]);
    }    
    
    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("OK");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 875.f);
    button->addCallback([this]()
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::MenuClosed;
        msg->value = 0.f;
        msg->stateId = States::ID::MenuAchievement;

        requestStackPop();
        requestStackPush(States::ID::MenuMain);
    });
    m_uiContainer.addControl(button);
}