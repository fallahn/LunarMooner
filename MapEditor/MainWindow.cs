/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

Lunar Mooner Map Editor - Zlib license.

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

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;


namespace MapEditor
{
    public partial class MainWindow : Form
    {
        private SfmlControl m_sfmlControl = new SfmlControl();
        private SFML.Window.Vector2f m_maxTextureSize = new SFML.Window.Vector2f(1362, 320);
        private SFML.Graphics.RectangleShape m_backgroundShape = new SFML.Graphics.RectangleShape();

        public MainWindow()
        {
            InitializeComponent();
            splitContainer1.Panel1.Controls.Add(m_sfmlControl);

            m_sfmlControl.DrawDelegates.Add(this.Draw);
            m_sfmlControl.DrawDelegates.Add(this.DrawSprites);
            m_sfmlControl.DrawDelegates.Add(this.DrawBoxes);
            m_sfmlControl.DrawDelegates.Add(this.DrawLines);

            m_sfmlControl.UpdateDelegates.Add(this.UpdateLines);

            m_sfmlControl.Dock = DockStyle.Fill;
            m_sfmlControl.BackgroundColour = SFML.Graphics.Color.White;
            m_sfmlControl.ViewCentre = m_maxTextureSize / 2;

            m_sfmlControl.MouseUp += sfmlControl_MouseUp;
            m_sfmlControl.MouseDown += sfmlControl_MouseDown;
            m_sfmlControl.MouseMove += sfmlControl_MouseMove;

            m_backgroundShape.FillColor = SFML.Graphics.Color.Black;
            m_backgroundShape.Size = m_maxTextureSize;

            listBoxTextures.DisplayMember = "Name";

            this.StartPosition = FormStartPosition.CenterScreen;
            this.WindowState = FormWindowState.Maximized;
        }

        public void DispatchDrawingEvents()
        {
            m_sfmlControl.HandleEvents();
            m_sfmlControl.Draw();
        }

        private void Draw(SFML.Graphics.RenderWindow window)
        {
            window.Draw(m_backgroundShape);
        }

    }
}
