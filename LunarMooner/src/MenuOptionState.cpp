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

#include <MenuOptionState.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/Resource.hpp>
#include <xygine/KeyBinds.hpp>

#include <xygine/ui/Slider.hpp>
#include <xygine/ui/CheckBox.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/TextBox.hpp>
#include <xygine/ui/KeyBinds.hpp>

#include <SFML/Window/Event.hpp>

MenuOptionState::MenuOptionState(xy::StateStack& stateStack, Context context, xy::TextureResource& tr, xy::FontResource& fr, bool paused)
    : State             (stateStack, context),
    m_textureResource   (tr),
    m_fontResource      (fr),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_optionContainer   (m_messageBus),
    m_inputContainer    (m_messageBus),
    m_activeContainer   (&m_optionContainer),
    m_pausedGame        (paused)
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));
    
    const auto& font = m_fontResource.get("option_menu_57");
    buildOptionsMenu(font);
    buildControlMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateID = (paused) ? States::ID::PausedOptions : States::ID::MenuOptions;
}

//public
bool MenuOptionState::update(float dt)
{
    m_activeContainer->update(dt);
    return !m_pausedGame;
}

void MenuOptionState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(*m_activeContainer);
    rw.draw(m_cursorSprite);
}

bool MenuOptionState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        case sf::Keyboard::Tab:

            return false;
        case sf::Keyboard::Space:

            return false;
        default: break;
        }
    }
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        switch (evt.joystickButton.button)
        {
        case 7: //start on xbox

            return false;
        default: break;
        }
    }
    
    //pass remaining events to menu
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_activeContainer->handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    return false; //consume events
}

void MenuOptionState::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    case xy::Message::Type::UIMessage:
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        case xy::Message::UIEvent::MenuClosed:

            break;
        default: break;
        }
        break;
    }
    default: break;
    }
}

//private
void MenuOptionState::buildOptionsMenu(const sf::Font& font)
{
    auto muteCheckbox = xy::UI::create<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    auto soundSlider = xy::UI::create<xy::UI::Slider>(font, m_textureResource.get("assets/images/ui/slider_handle.png"), 375.f);
    soundSlider->setPosition(640.f, 314.f);
    soundSlider->setString("Volume");
    soundSlider->setMaxValue(1.f);
    soundSlider->addCallback([this, muteCheckbox](const xy::UI::Slider* slider)
    {
        //send volume setting command
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::RequestVolumeChange;
        msg->value = slider->getValue();

        muteCheckbox->check(false);

    }, xy::UI::Slider::Event::ValueChanged);
    soundSlider->setValue(getContext().appInstance.getAudioSettings().volume); //set this *after* callback is set
    m_optionContainer.addControl(soundSlider);
   
    muteCheckbox->setPosition(1110.f, 274.f);
    muteCheckbox->setString("Mute");
    muteCheckbox->addCallback([soundSlider, this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestAudioMute : xy::Message::UIEvent::RequestAudioUnmute;
        msg->value = soundSlider->getValue(); //so we know what to restore unmute levels to
    }, xy::UI::CheckBox::Event::CheckChanged);
    muteCheckbox->check(getContext().appInstance.getAudioSettings().muted);
    m_optionContainer.addControl(muteCheckbox);


    auto resolutionBox = xy::UI::create<xy::UI::Selection>(font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    resolutionBox->setPosition(640.f, 354.f);

    const auto& modes = getContext().appInstance.getVideoSettings().AvailableVideoModes;
    auto i = 0u;
    auto j = 0u;
    for (const auto& m : modes)
    {
        std::string name = std::to_string(m.width) + " x " + std::to_string(m.height);
        sf::Int32 val = (m.width << 16) | m.height;
        resolutionBox->addItem(name, val);
        //select currently active mode
        if (getContext().appInstance.getVideoSettings().VideoMode != m)
            i++;
        else
            j = i;
    }
    if (i < modes.size()) resolutionBox->setSelectedIndex(j);

    m_optionContainer.addControl(resolutionBox);

    auto fullscreenCheckbox = xy::UI::create<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    fullscreenCheckbox->setPosition(1110.f, 354.f);
    fullscreenCheckbox->setString("Full Screen");
    fullscreenCheckbox->addCallback([this](const xy::UI::CheckBox*)
    {

    }, xy::UI::CheckBox::Event::CheckChanged);
    fullscreenCheckbox->check((getContext().appInstance.getVideoSettings().WindowStyle & sf::Style::Fullscreen) != 0);
    m_optionContainer.addControl(fullscreenCheckbox);

    auto difficultySelection = xy::UI::create<xy::UI::Selection>(font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    difficultySelection->setPosition(640.f, 434.f);
    difficultySelection->addItem("Easy", static_cast<int>(xy::Difficulty::Easy));
    difficultySelection->addItem("Normal", static_cast<int>(xy::Difficulty::Normal));
    difficultySelection->addItem("Hard", static_cast<int>(xy::Difficulty::Hard));
    difficultySelection->selectItem(0);
    difficultySelection->setCallback([this](const xy::UI::Selection* s)
    {
        //send message with new difficulty
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::RequestDifficultyChange;
        msg->difficulty = static_cast<xy::Difficulty>(s->getSelectedValue());
    });
    difficultySelection->selectItem(static_cast<int>(getContext().appInstance.getGameSettings().difficulty));
    m_optionContainer.addControl(difficultySelection);

    auto controllerCheckbox = xy::UI::create<xy::UI::CheckBox>(font, m_textureResource.get("assets/images/ui/checkbox.png"));
    controllerCheckbox->setPosition(1110.f, 434.f);
    controllerCheckbox->setString("Enable Controller");
    controllerCheckbox->addCallback([this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestControllerEnable : xy::Message::UIEvent::RequestControllerDisable;

    }, xy::UI::CheckBox::Event::CheckChanged);
    controllerCheckbox->check(getContext().appInstance.getGameSettings().controllerEnabled);
    m_optionContainer.addControl(controllerCheckbox);

    auto applyButton = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    applyButton->setString("Apply");
    applyButton->setAlignment(xy::UI::Alignment::Centre);
    applyButton->setPosition(xy::DefaultSceneSize.x / 2.f, 580.f);
    applyButton->addCallback([fullscreenCheckbox, resolutionBox, this]()
    {
        auto res = resolutionBox->getSelectedValue();

        xy::App::VideoSettings settings;
        settings.VideoMode.width = res >> 16;
        settings.VideoMode.height = res & 0xFFFF;
        settings.WindowStyle = (fullscreenCheckbox->checked()) ? sf::Style::Fullscreen : sf::Style::Close;
        getContext().appInstance.applyVideoSettings(settings);
        xy::App::setMouseCursorVisible(false);

        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::ResizedWindow;
    });
    m_optionContainer.addControl(applyButton);

    auto controlButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    controlButton->setString("Controls");
    controlButton->setAlignment(xy::UI::Alignment::Centre);
    controlButton->setPosition(840.f, 770.f);
    controlButton->addCallback([this]()
    {
        m_activeContainer = &m_inputContainer;
    });
    m_optionContainer.addControl(controlButton);

    auto backButton = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    backButton->setString("Back");
    backButton->setAlignment(xy::UI::Alignment::Centre);
    backButton->setPosition(1080.f, 770.f);
    backButton->addCallback([this]()
    {
        close();
        requestStackPush((m_pausedGame)? States::ID::Pause : States::ID::MenuMain);
    });
    m_optionContainer.addControl(backButton);
}

void MenuOptionState::buildControlMenu(const sf::Font& font)
{
    auto inputs = xy::UI::create<xy::UI::KeyBinds>(font);
    inputs->setPosition(400.f, 120.f);
    m_inputContainer.addControl(inputs);

    auto optionButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    optionButton->setString("Options");
    optionButton->setAlignment(xy::UI::Alignment::Centre);
    optionButton->setPosition(840.f, 770.f);
    optionButton->addCallback([this]()
    {
        m_activeContainer = &m_optionContainer;
    });
    m_inputContainer.addControl(optionButton);

    auto backButton = std::make_shared<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/button.png"));
    backButton->setString("Back");
    backButton->setAlignment(xy::UI::Alignment::Centre);
    backButton->setPosition(1080, 770.f);
    backButton->addCallback([this]()
    {
        close();
        requestStackPush((m_pausedGame) ? States::ID::Pause : States::ID::MenuMain);
    });
    m_inputContainer.addControl(backButton);
}

void MenuOptionState::close()
{
    xy::Input::save();
    
    requestStackPop();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuClosed;
    msg->stateID = (m_pausedGame) ? States::ID::PausedOptions : States::ID::MenuOptions;
}