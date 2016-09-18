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

#include <TutorialTip.hpp>
#include <CommandIds.hpp>

#include <xygine/Log.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/MessageBus.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <SFML/Audio/Listener.hpp>

using namespace lm;

namespace
{
    const float maxScale = 60.f;
    const float scaleGrowth = 180.f;

    float restoreVol = 0.f;
    float elapsedTime = 0.f;

    const sf::Vector2f textPos(134.f, 100.f);
    const std::array<sf::Vector2f, 6u> vertPositions = 
    {
        sf::Vector2f(46.67f, 45.25f),
        {45.25f, 46.67f},
        {100.41f, 99.f},
        {99.59f, 101.f},
        {130.f, 99.f},
        {130.f, 101.f}
    };
}

TutorialTip::TutorialTip(sf::Font& font, xy::MessageBus& mb)
    : m_messageBus  (mb),
    m_state         (State::Reset),
    m_alpha         (0.f)
{
    //set up graphics colour properties etc    
    m_text.setFont(font);
    m_text.setFillColor(sf::Color::Yellow);

    m_circle.setRadius(1.f);
    m_circle.setOrigin(1.f, 1.f);
    m_circle.setScale(0.f, 0.f);
    m_circle.setFillColor(sf::Color::Transparent);
    m_circle.setOutlineColor(sf::Color::Yellow);

    std::size_t i = 0;
    for (auto& v : m_vertices)
    {
        v.color = sf::Color::Transparent;
        v.position = vertPositions[i++];
    }

    setShaderActive(false);
}

//public
void TutorialTip::setString(const std::string& str)
{
    m_text.setString(str);
}

bool TutorialTip::update(float dt)
{
    elapsedTime += dt;

    switch (m_state)
    {
    default: return false;
    case State::Reset: 
        //fade out
    {       
        m_alpha = std::max(0.f, m_alpha - dt);

        sf::Color colour(255, 255, 0, static_cast<sf::Uint8>(m_alpha * 255.f));
        m_circle.setOutlineColor(colour);
        for (auto& v : m_vertices) v.color = colour;
        m_text.setFillColor(colour);
    }
    return false;
    case State::Started:
        //update animation
    {
        auto scale = m_circle.getScale();
        scale.x += scaleGrowth * dt;
        scale.y += scaleGrowth * dt;
        if (scale.x < maxScale)
        {
            m_circle.setScale(scale);
            m_circle.setOutlineThickness(2.f * (1.f / scale.x)); //inverse scale

            getShader()->setUniform("u_time", elapsedTime);
            getShader()->setUniform("u_amount", std::max(1.f, scale.x / maxScale));

            sf::Uint8 alpha = xy::Util::Random::value(0, 1) * 255;
            sf::Color colour(255, 255, 0, alpha);
            for (auto& v : m_vertices) v.color = colour;
            m_text.setFillColor(colour);
        }
        else
        {
            m_state = State::Finished;
            setShaderActive(false);
            for (auto& v : m_vertices) v.color = sf::Color::Yellow;
            m_text.setFillColor(sf::Color::Yellow);
        }
    }
    return true;
    case State::Finished:
        //check for continue
        return true;
    }
}

void TutorialTip::reset()
{
    if (m_state == State::Finished)
    {
        m_alpha = 1.f;
        //sf::Listener::setGlobalVolume(restoreVol);

        m_state = State::Reset;

        auto msg = m_messageBus.post<LMTutorialEvent>(TutorialEvent);
        msg->action = LMTutorialEvent::Closed;
    }
}

void TutorialTip::start()
{
    if (m_state == State::Reset)
    {
        //reset graphics
        m_circle.setScale({});
        for (auto& v : m_vertices) v.color = sf::Color::Transparent;        
        m_circle.setOutlineColor(sf::Color::Yellow);
        m_text.setFillColor(sf::Color::Yellow);
               
        //set drawable positions
        auto pos = getPosition();
        sf::Vector2f origin(0.f, (m_text.getLocalBounds().height / 2.f) + m_text.getLocalBounds().top);
        if (pos.x < xy::DefaultSceneSize.x / 2.f)
        {
            origin.x = 0.f;
            pos.x = textPos.x;

            auto i = 0u;
            for (auto& v : m_vertices) v.position.x = vertPositions[i++].x;
        }
        else
        {
            origin.x = m_text.getLocalBounds().width;
            pos.x = -textPos.x;

            auto i = 0u;
            for (auto& v : m_vertices) v.position.x = -vertPositions[i++].x;
        }

        if (pos.y < xy::DefaultSceneSize.y / 2.f)
        {
            pos.y = textPos.y;

            auto i = 0u;
            for (auto& v : m_vertices) v.position.y = vertPositions[i++].y;
        }
        else
        {
            pos.y = -textPos.y;

            auto i = 0u;
            for (auto& v : m_vertices) v.position.y = -vertPositions[i++].y;
        }
        m_text.setOrigin(origin);
        m_text.setPosition(pos);


        //restoreVol = sf::Listener::getGlobalVolume();
        //sf::Listener::setGlobalVolume(0.f);

        setShaderActive(true);

        auto msg = m_messageBus.post<LMStateEvent>(StateEvent);
        msg->type = LMStateEvent::Tutorial;
        msg->stateID = States::ID::Tutorial;

        m_state = State::Started;
    }
}

//private
void TutorialTip::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();

    rt.draw(m_circle, states);
    rt.draw(m_vertices.data(), m_vertices.size(), sf::TriangleStrip, states);
    
    getShader()->setUniform("u_texture", m_text.getFont()->getTexture(m_text.getCharacterSize()));
    states.shader = getActiveShader();
    rt.draw(m_text, states);
}