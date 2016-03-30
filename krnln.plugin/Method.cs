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
        取窗口句柄 = 0xD7,
        销毁 = 0xD8,
        获取焦点 = 0xD9,
        可有焦点 = 0xDA,
        取用户区宽度 = 0xDB,
        取用户区高度 = 0xDC,
        禁止重画 = 0xDD,
        允许重画 = 0xDE,
        重画 = 0xDF,
        部分重画 = 0xE0,
        刷新显示 = 0xE2,
        移动 = 0xE3,
        弹出菜单 = 0xE5,
        加入文本 = 0x014C,
        发送信息 = 0x014D,
        投递信息 = 0x014E,
        激活 = 0x01B0,
        置托盘图标 = 0x01C2,
        弹出托盘菜单 = 0x01C3,
        调用反馈事件 = 0x026F,
        置父窗口 = 0x0272
    }
    [LibGuid("d09f2340818511d396f6aaf844c7e325")]
    class Method : Plugin
    {
        [LibMethod((uint)krnln_method.载入)]
        static bool 载入(RuntimeTypeHandle 窗口类型, ref 窗口 欲载入的窗口, IWin32Window 父窗口 = null, bool 是否采用对话框方式 = true)
        {
            欲载入的窗口 = (窗口)Activator.CreateInstance(Type.GetTypeFromHandle(窗口类型));
            if (是否采用对话框方式) Application.Run(欲载入的窗口);
            else 欲载入的窗口.Show(父窗口);
            return true;
        }
    }
}
