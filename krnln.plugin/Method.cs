using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using wnxd.E_NET;

namespace krnln.plugin
{
    enum krnln_method : uint
    {
        载入 = 0xC8,
        取窗口句柄 = 0xD7
    }
    [LibGuid("d09f2340818511d396f6aaf844c7e325")]
    class Method : Plugin
    {
        [LibMethod((uint)krnln_method.载入)]
        static bool 载入(RuntimeTypeHandle 窗口类型, ref 窗口 欲载入的窗口, IWin32Window 父窗口 = null, bool 是否采用对话框方式 = true)
        {
            Application.EnableVisualStyles();
            if (欲载入的窗口 == null) 欲载入的窗口 = (窗口)Activator.CreateInstance(Type.GetTypeFromHandle(窗口类型));
            if (是否采用对话框方式) Application.Run(欲载入的窗口);
            else 欲载入的窗口.Show(父窗口);
            return true;
        }
    }
}
