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

#include <MenuHighScores.hpp>

#include <xygine/Resource.hpp>
#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>

#include <xygine/ui/Button.hpp>
#include <xygine/ui/ScoreList.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

MenuHighScoreState::MenuHighScoreState(xy::StateStack& ss, Context context, xy::TextureResource& tr, xy::FontResource& fr)
    : xy::State         (ss, context),
    m_textureResource   (tr),
    m_fontResource      (fr),
    m_messageBus        (context.appInstance.getMessageBus())
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));

    const auto& font = fr.get("high_scores_49");
    buildMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateId = States::ID::HighScores;
}

//public
bool MenuHighScoreState::update(float dt)
{
    m_uiContainer.update(dt);
    return true;
}

void MenuHighScoreState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(m_uiContainer);
    rw.draw(m_cursorSprite);
}

bool MenuHighScoreState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    return false; //consume events
}

void MenuHighScoreState::handleMessage(const xy::Message& msg)
{

}

//private
void MenuHighScoreState::buildMenu(const sf::Font& font)
{
    const auto& scores = getContext().appInstance.getScores();
    auto list = xy::UI::create<xy::UI::ScoreList>(font);
    list->setAlignment(xy::UI::Alignment::Centre);
    list->setPosition(960.f, 590.f);
    list->setList(scores);
    list->setIndex(getContext().appInstance.getLastScoreIndex());
    m_uiContainer.addControl(list);

    auto upScroll = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/scroll_arrow_vertical.png"));
    upScroll->setAlignment(xy::UI::Alignment::Centre);
    upScroll->setPosition(1310, 470.f);
    upScroll->addCallback([list]()
    {
        list->scroll(list->getVerticalSpacing());
    });
    m_uiContainer.addControl(upScroll);

    auto downScroll = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/scroll_arrow_vertical.png"));
    downScroll->setAlignment(xy::UI::Alignment::Centre);
    downScroll->setRotation(180.f);
    downScroll->setPosition(1310.f, 720.f);
    downScroll->addCallback([list]()
    {
        list->scroll(-list->getVerticalSpacing());
    });
    m_uiContainer.addControl(downScroll);
    
    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("OK");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(960.f, 875.f);
    button->addCallback([this]()
    {
        requestStackClear();

        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::MenuClosed;
        msg->value = 0.f;
        msg->stateId = States::ID::HighScores;

        requestStackPush(States::ID::MenuMain);
    });
    m_uiContainer.addControl(button);
}