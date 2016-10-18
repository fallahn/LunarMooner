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

#include <PostShieldDistortion.hpp>

#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/RenderTexture.hpp>

#include <string>
#include <vector>

using namespace lm;

namespace
{
#include "ConstParams.inl" 
    
    const std::string fragmentShader =
        "#version 120\n"

        "uniform sampler2D u_inputMap;\n"
        "uniform sampler2D u_normalMap;\n"

        "void main()\n"
        "{\n"
        "    vec2 texCoord = texture2D(u_normalMap, vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y)).rg * 2.0 - 1.0;\n"
        "    gl_FragColor =  texture2D(u_inputMap, gl_TexCoord[0].xy + (texCoord * 0.05));\n"
        "    //gl_FragColor =  texture2D(u_normalMap, vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y));\n"
        "}";

    const float divisor = 2.f;    
    const float falloff = 2800.f / divisor;
    const float falloffSqr = falloff * falloff;
    const float radSqr = std::pow(shieldRadius / divisor, 2.f);
    const float falloffDist = radSqr - falloffSqr;
}

PostShield::PostShield(float radius)
{
    if (m_shader.loadFromMemory(fragmentShader, sf::Shader::Fragment))
    {
        //create normal map texture
        sf::Vector2u size(xy::DefaultSceneSize / divisor);
        sf::Vector2f relShieldPos = shieldPosition / divisor;
        std::vector<sf::Vector2f> heightData(size.x * size.y);

        for (auto y = 0u; y < size.y; ++y)
        {
            for (auto x = 0u; x < size.x; ++x)
            {
                sf::Vector2f direction = sf::Vector2f(x, y) - relShieldPos;
                float distSqr = xy::Util::Vector::lengthSquared(direction);

                heightData[y * size.x + x] = sf::Vector2f(127.f, 127.f);

                if (distSqr < radSqr)
                {
                    if (distSqr > falloffSqr)
                    {
                        float amount = (radSqr - distSqr) / (radSqr - falloffSqr);                        
                        sf::Vector2f offset(((1.f - amount) * 127.f) * xy::Util::Vector::normalise(direction));
                        offset.y = 255.f - offset.y;
                        heightData[y * size.x + x] += offset;
                    }
                }
            }
        }

        std::vector<sf::Uint8> mapData(size.x * size.y * 4);
        auto i = 0u;
        for (auto y = 1u; y < size.y - 1; ++y)
        {
            for (auto x = 1u; x < size.x - 1; ++x)
            {
                auto i = ((y * size.x) + x);
                auto j = i * 4;

                mapData[j] = static_cast<sf::Uint8>(heightData[i].x);
                mapData[j+1] = static_cast<sf::Uint8>(heightData[i].y);
                mapData[j+2] = static_cast<sf::Uint8>(0.f);
                mapData[j+3] = 255;
            }
        }

        sf::Image img;
        img.create(static_cast<sf::Uint32>(xy::DefaultSceneSize.x / divisor), static_cast<sf::Uint32>(xy::DefaultSceneSize.y / divisor), mapData.data());
        m_normalMap.loadFromImage(img);
    }
    else
    {
        xy::Logger::log("Failed creating Post Shield shader", xy::Logger::Type::Error);
    }
}

void PostShield::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    m_shader.setUniform("u_inputMap", src.getTexture());
    m_shader.setUniform("u_normalMap", m_normalMap);
    applyShader(m_shader, dst);
}