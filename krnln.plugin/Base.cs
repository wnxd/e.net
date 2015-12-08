using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace krnln.plugin
{
    public interface Base
    {
        int 左边 { get; set; }
        int 顶边 { get; set; }
        int 宽度 { get; set; }
        int 高度 { get; set; }
        string 标记 { get; set; }
        bool 可视 { get; set; }
        bool 禁止 { get; set; }
        byte[] 鼠标指针 { get; set; }
    }
}
