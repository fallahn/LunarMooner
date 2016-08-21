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

#include <PlanetHoppingState.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

PlanetHoppingState::PlanetHoppingState(xy::StateStack& stack, Context context)
    : xy::State(stack, context)
{
    m_loadingSprite.setTexture(m_textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();

    quitLoadingScreen();
}

//public
bool PlanetHoppingState::update(float dt)
{
    return false;
}

bool PlanetHoppingState::handleEvent(const sf::Event& evt)
{
    return false;
}

void PlanetHoppingState::handleMessage(const xy::Message& msg)
{

}

void PlanetHoppingState::draw()
{

}

//private
void PlanetHoppingState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}