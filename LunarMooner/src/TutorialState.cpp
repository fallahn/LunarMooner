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

#include <TutorialState.hpp>
#include <CommandIds.hpp>

#include <xygine/Resource.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
#include "KeyMappings.inl"

    const std::string fragShader =
        "#version 120\n"

        "uniform float amount = 1.0;\n"
        "uniform sampler2D u_texture;\n"
        "uniform float u_time;\n"

        "float noise(vec2 coord)\n"
        "{\n"
        "    return fract(sin(dot(coord.xy, vec2(12.9898, 78.233))) * 43758.5453);\n"
        "}\n"

        "void main()\n"
        "{\n"
        "    float alpha = clamp((noise(gl_TexCoord[0].xy + u_time) * amount), 0.0, 1.0);\n"
        "    vec4 colour = texture2D(u_texture, gl_TexCoord[0].xy);\n"
        "    gl_FragColor = vec4(colour.rgb * gl_Color.rgb, colour.a * alpha);\n"
        "}\n";
}

TutorialState::TutorialState(xy::StateStack& stack, Context context, xy::StateID parentID, xy::FontResource& fr)
    : xy::State (stack, context),
    m_testShape (fr.get("tut_tip")),
    m_active    (false)
{
    switch (parentID)
    {
    default:
        messageHandler = [](const xy::Message&) {};
        break;
    case States::ID::SinglePlayer:
        messageHandler = std::bind(&TutorialState::handleGameMessage, this, std::placeholders::_1);
        break;
    case States::ID::PlanetHopping:
        messageHandler = std::bind(&TutorialState::handleHopMesage, this, std::placeholders::_1);
        break;
    }

    m_shader.loadFromMemory(fragShader, sf::Shader::Fragment);

    m_testShape.setPosition(xy::DefaultSceneSize / 2.f);
    m_testShape.setShader(&m_shader);
    m_testShape.setShaderActive(false);
}

//public
bool TutorialState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case keyFire:
            m_testShape.reset();
            break;
        }
    }
    //TODO asert controller is enabled
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        switch (evt.joystickButton.button)
        {
        default: break;
        case buttonA:
            m_testShape.reset();
            break;
        }
    }
    
    return !m_active;
}

void TutorialState::handleMessage(const xy::Message& msg)
{
    messageHandler(msg);
}

bool TutorialState::update(float dt)
{
    m_active = m_testShape.update(dt);
    
    //for(auto& t : things) t.update(dt);
    return !m_active;
}

void TutorialState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);
    
    rw.draw(m_testShape);


    //for(auto& t : things) rw.draw(t);
}

//private
void TutorialState::handleGameMessage(const xy::Message& msg)
{
    if (msg.id == GameEvent)
    {
        const auto& data = msg.getData<LMGameEvent>();
        switch (data.type)
        {
        default: break;
        case LMGameEvent::PlayerLanded:
            m_testShape.setPosition({ data.posX, data.posY });
            m_testShape.setString("Buns, flaps and dicketry");
            m_testShape.start();
            break;
        }
    }
}

void TutorialState::handleHopMesage(const xy::Message& msg)
{

}