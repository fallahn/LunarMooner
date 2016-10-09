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

#ifndef LM_CONST_PARAMS_INL_
#define LM_CONST_PARAMS_INL_

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>

const sf::FloatRect alienArea(280.f, 200.f, 1360.f, 480.f);
const sf::Color SceneAmbientColour({ 76, 70, 72 });
const float SceneLightIntensity = 0.6f;
const sf::Color SceneDiffuseLight(255, 255, 200);
const sf::Color SceneSpecularLight(250, 255, 248);
const sf::Vector3f SceneLightDirection(0.25f, 0.5f, -1.f);
const sf::Color SceneWaterColour(200, 200, 255);
const float playerOffsetDepth = 0.f;// -60.f;
const float groundOffset = 60.f;
const std::string propsDirectory = "assets/models/props/";

#endif //LM_CONST_PARAMS_INL_