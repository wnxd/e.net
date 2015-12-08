using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using wnxd.E_NET;

namespace krnln.plugin
{
    [LibGuid("d09f2340818511d396f6aaf844c7e325")]
    [LibType]
    public class 窗口 : Form, Base, Plugin
    {
        public int 左边 { get; set; }
        public int 顶边 { get; set; }
        public int 宽度 { get; set; }
        public int 高度 { get; set; }
        public string 标记 { get; set; }
        public bool 可视 { get; set; }
        public bool 禁止 { get; set; }
        public byte[] 鼠标指针 { get; set; }
        public string 标题 { get; set; }
    }
}
