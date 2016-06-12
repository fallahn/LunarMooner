/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

/*!
\mainpage XYGINE

A 2D Game Engine Framework built around SFML, xygine implements a scene graph along with an Entity Component System.
The source for xygine is released under the zlib license in the hope that it may be useful for other people - 
but offers no warranty that it is fit for any particular purpose, even those for which it was specifically written...

Explanations of systems and examples can be found on the <a href="https://github.com/fallahn/xygine/wiki">xygine wiki</a>

*/

#ifndef XY_APP_HPP_
#define XY_APP_HPP_

#include <xygine/StateStack.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Score.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Config.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <functional>

/*!
\brief Main xygine namespace
*/
namespace xy
{   
    /*!
    \brief Abstract base class for new games or applications

    When creating a new game or application this class provides
    management for basic proeprties such as video settings and
    event handling. All events and system messages are despatched
    from here so that they are available across the entire application.
    */
    class XY_EXPORT_API App
    {
    public:

        /*!
        \brief VideoSettings struct

        Contains the current video settings for the application
        which a read and written to configuration file
        */
        struct VideoSettings final
        {
            sf::Int32 WindowStyle;
            sf::VideoMode VideoMode;
            bool VSync;
            std::vector<sf::VideoMode> AvailableVideoModes;

            VideoSettings()
                : WindowStyle(sf::Style::Close /*sf::Style::Fullscreen*/),
                VideoMode(1024, 768),
                VSync(true){}

            bool operator == (const VideoSettings& vs)
            {
                if (&vs == this) return true;
                return
                    (vs.VideoMode == this->VideoMode
                    && vs.VSync == this->VSync
                    && vs.WindowStyle == this->WindowStyle);
            }
        };
        /*!
        \brief Audio settings struct

        Contains the audio settings which are read / written to the settings file
        */
        struct AudioSettings final
        {
            bool muted = false;
            float volume = 1.f;
        };

        /*!
        \brief Game settings struct

        Contains game settings which are read / written to the settings file
        */
        struct GameSettings final
        {
            Difficulty difficulty = Difficulty::Easy;
            bool controllerEnabled = true;
            std::array<char, 3u> playerInitials;
            GameSettings() : playerInitials({ { '-', '-', '-' } }){}
        };

        /*!
        \brief Constructor.
        \param sf::ContextSettings. 
        Default context settings are supplied when inherting the App class,
        but provide the option to request a specific context if needed.
        For example when creating a game which uses the Model component
        (or other 3D features) a context with OpenGL version 3.2 or 
        higher is needed.
        */
        App(sf::ContextSettings = sf::ContextSettings());
        virtual ~App() = default;
        App(const App&) = delete;
        const App& operator = (const App&) = delete;

        /*!
        \brief Starts the application

        This should be called once from within main()
        This will start the application and enter the game loop
        */
        void run();
        /*!
        \brief Pauses the app's logic update function
        */
        void pause();
        /*!
        \brief Resumes the app's update function, if it is paused
        */
        void resume();
        /*!
        \brief Returns a reference to the struct containing
        the current audio settings.

        Audio settings properties are updated via the messaging
        system
        \see MessageBus
        */
        const AudioSettings& getAudioSettings() const;
        /*!
        \brief Returns a reference to a struct containing the current
        Video settings.
        */
        const VideoSettings& getVideoSettings() const;
        /*!
        \brief Applies a given set of video settings

        Video settings can be updated va the messaging system
        or by providing this function with a struct containing
        the desired settings. If requested settings are invalid
        for any reason the settigns will not be applied and
        a message will be printed when in debug mode.
        */
        void applyVideoSettings(const VideoSettings&);
        /*!
        \brief Returns a reference to the struct containing
        the current game settings
        */
        const GameSettings& getGameSettings() const;
        /*!
        \brief Returns a reference to the message bus
        \see MessageBus
        */
        MessageBus& getMessageBus();
        /*!
        \brief Add a name / value pair to the score board

        The app contains a list of scores along with player
        names which are written to an external file on exit.
        */
        void addScore(const std::string& name, int value);
        /*!
        \brief Returns a vector containing the scores loaded
        from an external file

        The vector can be used to display name / value lists
        in menus or high score tables
        \see UI::ScoreList
        */
        const std::vector<Scores::Item>& getScores() const;
        /*!
        \brief Return the index in the score list of the last
        score to be added
        */
        int getLastScoreIndex() const;
        /*!
        \brief Sets the current player's initials

        The initials can be displayed throughout the game, and
        can be written to the score list
        */
        void setPlayerInitials(const std::string&);

        /*!
        \brief Returns the mouse position in world coordinates

        This is only valid while an instance of the app is running
        and will cause an assertion error otherwise
        */
        static sf::Vector2f getMouseWorldPosition();

        /*!
        \brief Sets the visibility of the mouse cursor.
        Prefer this to setting via the render window directly to allow
        imgui windows to correctly display/hide the mouse based on their
        visibility status
        */
        static void setMouseCursorVisible(bool);

        /*!
        \brief Quits the application
        */
        static void quit();

    protected:
        /*!
        \brief Returns a reference to the current render window
        */
        sf::RenderWindow& getRenderWindow();
        /*!
        \brief Function for despatching all window events

        This should be implemented in the derived game or application
        to allow passing down any received SFML events throughout the
        program. Usually this would be passing the event to a state stack
        */
        virtual void handleEvent(const sf::Event&) = 0;
        /*!
        \brief Function for despatching messages received by the message bus

        This should be implemented by any derived game or application
        to allow custom or system messages to be handled by xygine objects
        */
        virtual void handleMessage(const Message&) = 0;
        /*!
        \brief Registers a custom state with a state stack

        This should be implements by derived games or applications
        so that custom states can be registered with the xygine
        state stack instance.
        \see State
        */
        virtual void registerStates() = 0;
        /*!
        \brief Updates derived applications with the elapsed frame time
        during the game loop

        Logic updates should be performed here by any game objects such
        as the state stack. The frame time is fixed at 1/60 second
        */
        virtual void updateApp(float dt) = 0;
        /*!
        \brief Draws to the render window.
        All drawing operations should be started from here. clear() and
        display() are called automatically by xygine and need to be manually
        used in derived applications.
        */
        virtual void draw() = 0;

        /*!
        \brief Called when the application is first run.

        Optionally overridable this should be used when performing
        operations which may throw exceptions such as loading configuration
        data, which should not be performed in the constructor of a derived
        application.
        */
        virtual void initialise();

        /*!
        \brief Called when the application shuts down

        Optionally overridable this allows derived classes to tidy
        up any data when the program exits, such as clearing the state
        stack.
        */
        virtual void finalise();

    private:

        struct SettingsFile final
        {
            int ident;
            int version;

            sf::VideoMode videoMode;
            sf::Int32 windowStyle;
            AudioSettings audioSettings;
            GameSettings gameSettings;
        };

        GameSettings m_gameSettings;
        AudioSettings m_audioSettings;

        VideoSettings m_videoSettings;
        sf::RenderWindow m_renderWindow;

        MessageBus m_messageBus;

        Scores m_scores;
        //Difficulty m_pendingDifficulty;

        std::function<void(float)> update;
        std::function<void(const sf::Event&)> eventHandler;

        void loadSettings();
        void saveSettings();

        void saveScreenshot();

        void handleEvents();
        void handleMessages();
    };
}
#endif //XY_APP_HPP_