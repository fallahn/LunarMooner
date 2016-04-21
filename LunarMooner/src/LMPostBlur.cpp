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

#include <LMPostBlur.hpp>
#include <CommandIds.hpp>
#include <StateIds.hpp>

#include <xygine/shaders/PostGaussianBlur.hpp>
#include <xygine/shaders/PostDownSample.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace lm;

namespace
{
    const std::string fragShader =
        "#version 120\n"
        "\n"
        /*"uniform float u_amount;\n"
        "uniform vec2 u_offset;\n"*/
        "uniform sampler2D u_srcTexture;\n"

        "void main()\n"
        "{\n"
        /*"    vec3 sat = vec3(1.0) * u_amount;\n"
        "    vec4 colour = texture2D(u_srcTexture, gl_TexCoord[0].xy + (u_offset * u_amount));\n"
        "    colour.rgb += sat;\n"
        "    gl_FragColor = colour;\n"*/
        "    gl_FragColor = texture2D(u_srcTexture, gl_TexCoord[0].xy);\n"
        "}\n";

    const float fadeSpeed = 5.f;
}

PostBlur::PostBlur()
    : m_amount(0.f),
    m_enabled (false)
{
    m_blurShader.loadFromMemory(xy::Shader::Default::vertex, xy::Shader::PostGaussianBlur::fragment);
    m_downsampleShader.loadFromMemory(xy::Shader::Default::vertex, xy::Shader::PostDownSample::fragment);
    m_outShader.loadFromMemory(xy::Shader::Default::vertex, fragShader);

    //messsage callbacks to enable / disable
    xy::PostProcess::MessageHandler mh;
    mh.id = xy::Message::UIMessage;
    mh.action = [this](const xy::Message& msg)
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        /*switch (msgData.stateId)
        {
        default: break;
        case States::ID::Pause:*/
            if (msgData.type == xy::Message::UIEvent::MenuOpened)
            {
                m_enabled = true;
            }
            else if(msgData.type == xy::Message::UIEvent::MenuClosed)
            {
                m_enabled = false;
            }
          /*  break;
        }*/
    };
    addMessageHandler(mh);

    initTextures({ 1920, 1080 });
}

//public
void PostBlur::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    //fade in / out for 1 sec
    if (m_enabled)
    {
        m_amount = std::min(1.f, m_amount + (m_clock.restart().asSeconds() * fadeSpeed));
    }
    else
    {
        m_amount = std::max(0.f, m_amount - (m_clock.restart().asSeconds() * fadeSpeed));
    }
    

    //and draw....
    if (m_amount == 0)
    {
        m_outShader.setUniform("u_srcTexture", src.getTexture());
        applyShader(m_outShader, dst);
    }
    else
    {        
        downSample(src, m_firstPassTextures[0]);
        blurMultipass(m_firstPassTextures);
        downSample(m_firstPassTextures[0], m_secondPassTextures[0]);
        blurMultipass(m_secondPassTextures);
        m_outShader.setUniform("u_srcTexture", m_secondPassTextures[0].getTexture());
        
    }
    applyShader(m_outShader, dst);
}

//private
void PostBlur::initTextures(sf::Vector2u size)
{
    size /= 2u;
    if (m_firstPassTextures[0].getSize() != size)
    {
        m_firstPassTextures[0].create(size.x, size.y);
        m_firstPassTextures[0].setSmooth(true);

        m_firstPassTextures[1].create(size.x, size.y);
        m_firstPassTextures[1].setSmooth(true);

        m_secondPassTextures[0].create(size.x / 2, size.y / 2);
        m_secondPassTextures[0].setSmooth(true);

        m_secondPassTextures[1].create(size.x / 2, size.y / 2);
        m_secondPassTextures[1].setSmooth(true);
    }
}

void PostBlur::blurMultipass(TexturePair& textures)
{
    auto textureSize = textures[0].getSize();
    for (auto i = 0u; i < 2; ++i)
    {
        blur(textures[0], textures[1], { 0.f, 1.f / static_cast<float>(textureSize.y) });
        blur(textures[1], textures[0], { 1.f / static_cast<float>(textureSize.x), 0.f });
    }
}

void PostBlur::blur(const sf::RenderTexture& src, sf::RenderTexture& dst, const sf::Vector2f& offset)
{
    m_blurShader.setUniform("u_sourceTexture", src.getTexture());
    m_blurShader.setUniform("u_offset", offset * m_amount);

    applyShader(m_blurShader, dst);
    dst.display();
}

void PostBlur::downSample(const sf::RenderTexture& src, sf::RenderTexture& dst)
{
    m_downsampleShader.setUniform("u_sourceTexture", src.getTexture());
    m_downsampleShader.setUniform("u_sourceSize", sf::Vector2f(src.getSize()));

    applyShader(m_downsampleShader, dst);
    dst.display();
}