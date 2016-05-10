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

#ifndef LM_DEMO_RECORDER_HPP_
#define LM_DEMO_RECORDER_HPP_

#include <LMPlayerState.hpp>

#include <xygine/Difficulty.hpp>

#include <vector>

class DemoRecorder
{
public:
    DemoRecorder();
    ~DemoRecorder() = default;
    DemoRecorder(const DemoRecorder&) = delete;
    DemoRecorder operator = (const DemoRecorder&) = delete;

    sf::Uint32 getSeed();
    void start(const lm::PlayerState&, xy::Difficulty);
    void recordInput(sf::Uint8);
    void stop(bool = true);

private:
    sf::Uint32 m_seed;
    std::vector<char> m_buffer;
    char* m_ptr;
    bool m_started;
};

#endif //LM_DEMO_RECORDER_HPP_