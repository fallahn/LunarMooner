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

#include <MenuMainState.hpp>

#include <xygine/App.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/Resource.hpp>

#include <SFML/Window/Mouse.hpp>

MenuMainState::MenuMainState(xy::StateStack& stack, Context context, xy::TextureResource& tr, xy::FontResource& fr)
    : State             (stack, context),
    m_textureResource   (tr),
    m_fontResource      (fr),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_uiContainer       (m_messageBus)
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));

    buildMenu();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->value = 0.f;
    msg->stateId = States::ID::MenuMain;

    context.renderWindow.setMouseCursorVisible(false);
}

//public
bool MenuMainState::update(float dt)
{
    m_uiContainer.update(dt);
    return true;
}

void MenuMainState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(m_uiContainer);
    rw.draw(m_cursorSprite);
}

bool MenuMainState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    return false;
}

void MenuMainState::handleMessage(const xy::Message& msg)
{

}

//private
void MenuMainState::buildMenu()
{
    const auto& font = m_fontResource.get("main_menu_90");
    
    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("One Player");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 325.f);
    button->addCallback([this]()
    {
        close();
        requestStackPush(States::ID::SinglePlayer);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Two Player");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 425.f);
    button->addCallback([this]()
    {
        close();
        requestStackPush(States::ID::MultiPlayer);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Options");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 525.f);
    button->addCallback([this]()
    {
        close(false);
        requestStackPush(States::ID::MenuOptions);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("High Scores");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 625.f);
    button->addCallback([this]()
    {
        close(false);
        requestStackPush(States::ID::HighScoresMenu);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Achievements");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 725.f);
    button->addCallback([this]()
    {
        close(false);
        requestStackPush(States::ID::MenuAchievement);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Quit");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 825.f);
    button->addCallback([this]()
    {
        getContext().renderWindow.close();
    });
    m_uiContainer.addControl(button);
}

void MenuMainState::close(bool clear)
{
    if (clear)
    {
        requestStackClear();
    }
    else
    {
        requestStackPop();
    }

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuClosed;
    msg->value = 0.f;
    msg->stateId = States::ID::MenuMain;
}