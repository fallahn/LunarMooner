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

#include <GameOverState.hpp>
#include <CommandIds.hpp>

#include <xygine/Resource.hpp>
#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>

#include <xygine/ui/Button.hpp>
#include <xygine/ui/TextBox.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

GameOverState::GameOverState(xy::StateStack& ss, Context context, xy::TextureResource& tr, xy::FontResource& fr)
    : xy::State         (ss, context),
    m_textureResource   (tr),
    m_fontResource      (fr),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_uiContainer       (m_messageBus),
    m_scores            (4), //I doubt we'll want more than 4 players, but if we do, change this!!
    m_playerCount       (0u),
    m_currentPlayer     (0u)
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));

    const auto& font = m_fontResource.get("game_over_state_49");
    buildMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateId = States::ID::GameOver;
}

//public
bool GameOverState::update(float dt)
{
    m_uiContainer.update(dt);
    return true;
}

void GameOverState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(m_uiContainer);
    rw.draw(m_cursorSprite);
}

bool GameOverState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    return false; //consume events
}

void GameOverState::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    default:break;
    case LMMessageId::MenuEvent:
    {
        auto& msgData = msg.getData<LMMenuEvent>();
        m_scores[msgData.playerId] = msgData.score;
        if (msgData.playerId >= m_playerCount)
        {
            m_playerCount = msgData.playerId + 1;
        }

        m_scoreLabel->setString("Your Score: " + std::to_string(m_scores[0]));
    }
        break;
    }
}

//private
void GameOverState::buildMenu(const sf::Font& font)
{
    const float centreX = xy::DefaultSceneSize.x / 2.f;

    auto label = xy::UI::create<xy::UI::Label>(font);
    label->setAlignment(xy::UI::Alignment::Centre);
    label->setPosition(centreX, 400.f);
    label->setString("GAME OVER");
    m_uiContainer.addControl(label);
    
    m_scoreLabel = xy::UI::create<xy::UI::Label>(font);
    m_scoreLabel->setAlignment(xy::UI::Alignment::Centre);
    m_scoreLabel->setPosition(centreX, 460.f);
    m_scoreLabel->setString("Your Score:");
    m_uiContainer.addControl(m_scoreLabel);

    auto textbox = xy::UI::create<xy::UI::TextBox>(font);
    textbox->setAlignment(xy::UI::Alignment::Centre);
    textbox->setPosition(centreX, 540.f);
    textbox->setLabelText("Enter your name:");
    textbox->setText("Player 1");
    textbox->setMaxLength(18u);
    m_uiContainer.addControl(textbox);

    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("OK");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(centreX, 675.f);
    button->addCallback([this, textbox]()
    {
        if (m_playerCount > 0 && m_currentPlayer < m_playerCount)
        {
            getContext().appInstance.addScore(textbox->getText(), m_scores[m_currentPlayer++]);
            if (m_currentPlayer < m_playerCount)
            {
                textbox->setText("Player " + std::to_string(m_currentPlayer + 1));
                m_scoreLabel->setString("Your Score: " + std::to_string(m_scores[m_currentPlayer]));
            }
        }
        else
        {
            close();
            requestStackPush(States::ID::HighScoresEnd);
        }
    });
    m_uiContainer.addControl(button);
}

void GameOverState::close()
{
    requestStackPop();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuClosed;
    msg->value = 0.f;
    msg->stateId = States::ID::GameOver;
}