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

        public MainWindow()
        {
            InitializeComponent();
            this.Controls.Add(m_sfmlControl);

            //m_sfmlControl.DrawDelegates.Add();
            m_sfmlControl.Dock = DockStyle.Fill;

            this.StartPosition = FormStartPosition.CenterScreen;
        }

        public void DispatchDrawingEvents()
        {
            m_sfmlControl.HandleEvents();
            m_sfmlControl.Draw();
        }
    }
}
