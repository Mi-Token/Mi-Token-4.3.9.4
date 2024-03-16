using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BLEWatcherStateMachine
{
    public partial class DoubleBufferedListbox : ListBox
    {
        public DoubleBufferedListbox()
        {
            InitializeComponent();
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.ResizeRedraw | ControlStyles.UserPaint, true);
            this.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
            this.DoubleBuffered = true;
        }

        private const int SRCCOPY = 0xCC0020;
        [System.Runtime.InteropServices.DllImportAttribute("gdi32.dll")]
        private static extern int BitBlt(
          IntPtr hdcDest,     // handle to destination DC (device context)
          int nXDest,         // x-coord of destination upper-left corner
          int nYDest,         // y-coord of destination upper-left corner
          int nWidth,         // width of destination rectangle
          int nHeight,        // height of destination rectangle
          IntPtr hdcSrc,      // handle to source DC
          int nXSrc,          // x-coordinate of source upper-left corner
          int nYSrc,          // y-coordinate of source upper-left corner
          int dwRop  // raster operation code
          );

        [System.Runtime.InteropServices.DllImportAttribute("gdi32.dll")]
        public static extern IntPtr CreateCompatibleDC(IntPtr hdc);

        [System.Runtime.InteropServices.DllImportAttribute("gdi32.dll")]
        public static extern IntPtr SelectObject(IntPtr hdc, IntPtr obj);

        [System.Runtime.InteropServices.DllImportAttribute("gdi32.dll")]
        public static extern void DeleteObject(IntPtr obj);


        private void GDI_Copy(Graphics input, Rectangle bounds, Graphics output, Point topLeft)
        {
            IntPtr destDC = output.GetHdc();
            
            IntPtr srcDC = input.GetHdc();

            BitBlt(destDC, topLeft.X, topLeft.Y, bounds.Width, bounds.Height, srcDC, bounds.X, bounds.Y, SRCCOPY);

            output.ReleaseHdc(destDC);
            input.ReleaseHdc(srcDC);
        }

        
        protected override void OnDrawItem(DrawItemEventArgs e)
        {


            DrawItemEventArgs newArgs = e; // new DrawItemEventArgs(bufferedGraphics.Graphics, e.Font, newBounds, e.Index, e.State, e.ForeColor, e.BackColor);
            if (this.Items.Count > 0)
            {
                newArgs.DrawBackground();
                newArgs.Graphics.DrawString(this.Items[newArgs.Index].ToString().Replace("\r\n",""), newArgs.Font, Brushes.Black, newArgs.Bounds, StringFormat.GenericDefault);
            }
                //newArgs.DrawFocusRectangle();
                //base.OnDrawItem(newArgs);
                //GDI_Copy(bufferedGraphics.Graphics, newBounds, e.Graphics, new Point(0, 0));
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            BufferedGraphicsContext currentContext = BufferedGraphicsManager.Current;
            Region iRegion = new Region(e.ClipRectangle);

            Rectangle newBounds = new Rectangle(0, 0, e.ClipRectangle.Width, e.ClipRectangle.Height);
            using (BufferedGraphics bufferedGraphics = currentContext.Allocate(e.Graphics, newBounds))
            {
                bufferedGraphics.Graphics.FillRegion(new SolidBrush(this.BackColor), iRegion);
                if (this.Items.Count > 0)
                {
                    for (int i = 0; i < this.Items.Count; ++i)
                    {
                        System.Drawing.Rectangle irect = this.GetItemRectangle(i);
                        if (e.ClipRectangle.IntersectsWith(irect))
                        {
                            if ((this.SelectionMode == SelectionMode.One && this.SelectedIndex == i) ||
                                (this.SelectionMode == SelectionMode.MultiSimple && this.SelectedIndices.Contains(i)) ||
                                (this.SelectionMode == SelectionMode.MultiExtended && this.SelectedIndices.Contains(i)))
                            {
                                OnDrawItem(new DrawItemEventArgs(bufferedGraphics.Graphics, this.Font, irect, i, DrawItemState.Selected, this.ForeColor, this.BackColor));
                            }
                            else
                            {
                                OnDrawItem(new DrawItemEventArgs(bufferedGraphics.Graphics, this.Font, irect, i, DrawItemState.Default, this.ForeColor, this.BackColor));
                            }

                            iRegion.Complement(irect);

                        }
                    }

                }
                GDI_Copy(bufferedGraphics.Graphics, newBounds, e.Graphics, new Point(0, 0));
            }
            //base.OnPaint(e);
        }
    }
}
