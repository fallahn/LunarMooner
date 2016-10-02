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

#include <LMHumanController.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Wavetable.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/AudioSource.hpp>

#include <xygine/Reports.hpp>

using namespace lm;

namespace
{
    //const float walkSpeed = 120.f;

    const float gravity = 12.f;
    const float initialVelocity = 9.f;
    const float maxXVelocity = 200.f;

    const auto waveTable = xy::Util::Wavetable::sine(5.f, 3.f);
    const float modelOffset = 20.f;
}

HumanController::HumanController(xy::MessageBus& mb, xy::Model& ad)
    : xy::Component     (mb, this),
    m_drawable          (ad),
    m_gotoDestination   (false),
    m_waveTableIndex    (xy::Util::Random::value(0, waveTable.size() - 1)),
    m_velocity          (0.f, -initialVelocity)
{

}

//public
void HumanController::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_gotoDestination)
    {
        auto position = entity.getPosition();

        //add gravity 
        m_velocity.y += gravity;
        //boost pack (do particle anim) if too low
        if(position.y > m_destination.y)
        {
            m_velocity.y -= gravity * 1.2f;
        }
        //add direction to target
        m_velocity += (m_destination - position) * 0.05f;

        //clamp the speeds within some bounds
        m_velocity *= 0.95f;

        float xRatio = maxXVelocity / std::abs(m_velocity.x);
        if (std::abs(m_velocity.x) > maxXVelocity)
        {
            m_velocity.x *= (xRatio);
        }
        entity.getComponent<xy::AudioSource>()->setPitch(xRatio);

        //move by velocity
        entity.move(m_velocity * dt);

        //if dist to target < X we're there
        if (xy::Util::Vector::lengthSquared(m_destination - entity.getPosition()) < 100.f)
        {
            entity.destroy();
            auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
            msg->type = LMGameEvent::HumanPickedUp;
            msg->posX = position.x;
            msg->posY = position.y;
        }

        entity.getComponent<xy::ParticleSystem>()->setInertia(m_velocity);
    }
    else if (destroyed())
    {
        entity.destroy();
    }

    //do a little bobbing animation.
    if (!m_gotoDestination)
    {
        m_waveTableIndex = (m_waveTableIndex + 1) % waveTable.size();
        m_drawable.setPosition({ 0.f, waveTable[m_waveTableIndex] + modelOffset, 0.f });
    }
    else
    {
        m_drawable.setPosition({ 0.f, modelOffset, 0.f });
    }
    //store position for when switching player states
    m_position = entity.getPosition();
}

void HumanController::setDestination(const sf::Vector2f& dest)
{
    m_destination = dest;
    m_gotoDestination = true;

    //m_drawable.playAnimation((dest.x - m_position.x > 0) ? AnimationID::RunRight : AnimationID::RunLeft);
    m_drawable.setRotation((dest.x - m_position.x > 0) ? sf::Vector3f(0.f, 0.f, 90.f) : sf::Vector3f(0.f, 0.f, -90.f));
}