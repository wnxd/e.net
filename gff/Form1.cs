using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;

namespace gff
{
    public partial class Form1 : Form
    {
        private string framework;
        private string code;
        private string codeini;
        private IList<string> frameworkrefer = new List<string>();
        private IList<string> refer = new List<string>();
        public Form1(string[] args)
        {
            InitializeComponent();
            if (args.Length == 0)
            {
                this.openFileDialog1.Filter = "易源码 (*.e)|*.e";
                if (this.openFileDialog1.ShowDialog() == DialogResult.OK)
                {
                    this.code = this.openFileDialog1.FileName;
                }
                else Application.Exit();
            }
            else this.code = args[0];
            if (!File.Exists(this.code)) Application.Exit();
            this.codeini = code + ".wnxd";
            this.framework = Environment.GetFolderPath(Environment.SpecialFolder.Windows) + "\\Microsoft.NET\\Framework\\v4.0.30319";
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.Text = this.code;
            if (File.Exists(this.codeini))
            {
                string[] info = File.ReadAllLines(this.codeini);
                foreach (string path in info)
                {
                    if (File.Exists(path))
                    {
                        this.refer.Add(path);
                        this.listBox2.Items.Add(Path.GetFileNameWithoutExtension(path));
                    }
                }
            }
            if (Directory.Exists(this.framework))
            {
                string[] files = Directory.GetFiles(this.framework, "*.dll");
                foreach (string file in files)
                {
                    try
                    {
                        Assembly assembly = Assembly.LoadFrom(file);
                        if (assembly != null)
                        {
                            this.frameworkrefer.Add(file);
                            this.listBox1.Items.Add(Path.GetFileNameWithoutExtension(file));
                        }
                    }
                    catch
                    {

                    }
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (this.listBox1.CanSelect)
            {
                string path = this.frameworkrefer[this.listBox1.SelectedIndex];
                int index = this.refer.IndexOf(path);
                if (index == -1)
                {
                    index = this.refer.Count;
                    this.refer.Add(path);
                    this.listBox2.Items.Add(Path.GetFileNameWithoutExtension(path));
                }
                this.listBox1.ClearSelected();
                this.listBox2.SelectedIndex = index;
            }
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (this.listBox2.CanSelect)
            {
                int index = this.listBox2.SelectedIndex;
                this.refer.RemoveAt(index);
                this.listBox2.Items.RemoveAt(index);
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            File.WriteAllLines(this.codeini, this.refer);
            string path = Application.StartupPath + "\\donet2ec.exe";
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = path;
            psi.Arguments = "\"" + this.codeini + "\"";
            psi.WindowStyle = ProcessWindowStyle.Hidden;
            Process.Start(psi);
        }

        private void add_Click(object sender, EventArgs e)
        {
            this.openFileDialog1.Filter = ".net类库 (*.dll)|*.dll";
            if (this.openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                string path = this.openFileDialog1.FileName;
                try
                {
                    if (Assembly.LoadFrom(path) != null)
                    {
                        this.refer.Add(path);
                        this.listBox2.Items.Add(Path.GetFileNameWithoutExtension(path));
                    }
                }
                catch
                {

                }
            }
        }
    }
}
