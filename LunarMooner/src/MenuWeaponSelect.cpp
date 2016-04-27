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

#include <MenuWeaponSelect.hpp>
#include <UIWeaponSelect.hpp>
#include <LMPlayerState.hpp>
#include <PlayerProfile.hpp>
#include <CommandIds.hpp>

#include <xygine/Resource.hpp>
#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>

#include <xygine/ui/Button.hpp>
#include <xygine/ui/Label.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

MenuWeaponState::MenuWeaponState(xy::StateStack& ss, Context context, xy::TextureResource& tr, xy::FontResource& fr, PlayerProfile& profile)
    : xy::State(ss, context),
    m_textureResource(tr),
    m_fontResource(fr),
    m_profile(profile),
    m_messageBus(context.appInstance.getMessageBus()),
    m_uiContainer(m_messageBus)
{
    m_cursorSprite.setTexture(m_textureResource.get("assets/images/ui/cursor.png"));
    m_cursorSprite.setPosition(context.renderWindow.mapPixelToCoords(sf::Mouse::getPosition(context.renderWindow)));

    const auto& font = fr.get("weapons_52");
    buildMenu(font);

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::MenuOpened;
    msg->stateId = States::ID::MenuWeapon;
}

//public
bool MenuWeaponState::update(float dt)
{
    m_uiContainer.update(dt);
    return true;
}

void MenuWeaponState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);

    rw.draw(m_uiContainer);
    rw.draw(m_cursorSprite);
}

bool MenuWeaponState::handleEvent(const sf::Event& evt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    m_uiContainer.handleEvent(evt, mousePos);
    m_cursorSprite.setPosition(mousePos);

    return false; //consume events
}

void MenuWeaponState::handleMessage(const xy::Message& msg)
{

}

namespace
{

}

//private
void MenuWeaponState::buildMenu(const sf::Font& font)
{
    auto label = xy::UI::create<xy::UI::Label>(font);
    label->setString("Select Secondary Weapon");
    label->setAlignment(xy::UI::Alignment::Centre);
    label->setPosition(960.f, 86.f);
    label->setCharacterSize(40u);
    m_uiContainer.addControl(label);
   
    label = xy::UI::create<xy::UI::Label>(font);
    label->setString("Rank " + std::to_string(m_profile.getRank()) + ": " +std::to_string(m_profile.getXP()) + "XP");
    label->setAlignment(xy::UI::Alignment::Centre);
    label->setPosition(960.f, 160.f);
    m_uiContainer.addControl(label);

    //weapon selector
    auto weaponSelect = xy::UI::create<lm::ui::WeaponSelect>(m_textureResource.get("assets/images/ui/weapon_select.png"));
    weaponSelect->setAlignment(xy::UI::Alignment::Centre);
    weaponSelect->setPosition(960.f, 420.f);
    
    sf::Int32 count = AchievementID::Rank50 - AchievementID::Rank10;
    sf::Uint8 flags = 0u;
    for (auto i = 0; i < count; ++i)
    {
        if (!m_profile.hasAchievement(static_cast<AchievementID>(AchievementID::Rank10 + i)))
        {
            flags |= (1 << i);
        }
    }
    weaponSelect->setLockedFlags(flags);

    m_uiContainer.addControl(weaponSelect);

    auto button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Back");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(600.f, 975.f);
    button->addCallback([this]()
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::MenuClosed;
        msg->value = 0.f;
        msg->stateId = States::ID::MenuWeapon;

        requestStackPop();
        requestStackPush(States::ID::MenuMain);
    });
    m_uiContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setText("Begin!");
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(1320.f, 975.f);
    button->addCallback([this, weaponSelect, flags]()
    {
        //set the currently selected weapon
        lm::SpecialWeapon weapon = lm::SpecialWeapon::None;
        auto idx = weaponSelect->getSelectedIndex();
        if ((flags & (1 << idx)) == 0)
        {
            weapon = static_cast<lm::SpecialWeapon>(idx + 1);
        }

        m_profile.setSpecialWeapon(weapon);
        
        //and raise message to say we're done
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::MenuClosed;
        msg->value = 0.f;
        msg->stateId = States::ID::MenuWeapon;

        requestStackClear();
        requestStackPush(States::ID::SinglePlayer);
    });
    m_uiContainer.addControl(button);
}