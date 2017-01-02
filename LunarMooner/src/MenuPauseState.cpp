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

#include <MenuPauseState.hpp>

#include <xygine/App.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/Label.hpp>

#include <SFML/Window/Event.hpp>

MenuPauseState::MenuPauseState(xy::StateStack& ss, Context context, xy::TextureResource& tr, xy::FontResource& fr)
    : xy::State         (ss, context),
    m_textureResource   (tr),
    m_fontResource      (fr),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_uiContainer       (m_messageBus)
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));

    const auto& font = m_fontResource.get("pause_menu_45");
    buildMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateID = States::ID::Pause;
}

//public
bool MenuPauseState::update(float dt)
{
    m_uiContainer.update(dt);
    return false;
}

void MenuPauseState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(m_uiContainer);
    rw.draw(m_cursorSprite);
}

bool MenuPauseState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    switch (evt.type)
    {
    default: break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::P:
        case sf::Keyboard::Escape:
        case sf::Keyboard::Pause:
            close();
            break;
        }
        break;
    case sf::Event::JoystickButtonPressed:
        if (evt.joystickButton.joystickId != 0) break;
        switch (evt.joystickButton.button)
        {
        case 6: //back on xbox controller
        //case 7: //start on xbox controller
            close();
            break;
        default:break;
        }
        break;
    }
    
    return false;
}

void MenuPauseState::handleMessage(const xy::Message& msg)
{

}

//private
void MenuPauseState::buildMenu(const sf::Font& font)
{
    const float centreX = xy::DefaultSceneSize.x / 2.f;

    auto label = xy::UI::create<xy::UI::Label>(font);
    label->setAlignment(xy::UI::Alignment::Centre);
    label->setPosition(centreX, 300.f);
    label->setString("PAUSED");
    m_uiContainer.addControl(label);
    
    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setString("Continue");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(centreX, 425.f);
    button->addCallback([this]()
    {
        close();
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setString("Options");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(centreX, 525.f);
    button->addCallback([this]()
    {
        close();
        requestStackPush(States::ID::PausedOptions);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setString("Quit to Main");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(centreX, 625.f);
    button->addCallback([this]()
    {
        close();
        requestStackClear();
        requestStackPush(States::ID::MenuBackground);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setString("Exit to Desktop");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(centreX, 725.f);
    button->addCallback([this]()
    {
        xy::App::quit();
    });
    m_uiContainer.addControl(button);
}

void MenuPauseState::close()
{
    requestStackPop();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuClosed;
    msg->value = 0.f;
    msg->stateID = States::ID::Pause;
}