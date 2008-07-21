package regapplet;
import java.awt.*;

public class TextBlock extends Canvas {
  private static int dyinc = 17;
  private static int dxinc = 97;
  String text;
  int width;
  int height;
  public TextBlock (int x, int y) {
    super();
    width = x;
    height = y;
  }
  public Dimension minimumSize() {
    System.err.println("TextBlock: minimumSize called " + width + " " + height);
    return (new Dimension(width, height));
  }
  public Dimension preferredSize() {
    System.err.println("TextBlock: preferredSize called " + width + " " + height);
    return (new Dimension(width, height));
  }
  public void setText(String itext) {
    text = itext;
    tsetsize();
    this.invalidate();
  }

  protected void tsetsize() {
    int len = text.length();
    int yinc = dyinc;		// Good guesses as we are called before the Font
    int xinc = dxinc;		// Stuff is setup. We update these values in paint()
    int i = len < xinc ? len : xinc;
    int z = 0;
    int cr = -1;
    int y = yinc + 2;
    int savei;
    while (z < len) {
      savei = i;
      while (i < len && text.charAt(i) != 0x20) { // Move backward looking for space
 	i--;
 	if (i == 0) {
 	  i = savei;
 	  break;
 	}
      }
      if (text.charAt(z) == 0x20 && z < i-1) z++;
      cr = (text.substring(z, i)).indexOf('\n');
      if (cr != -1) { // End line at newline character
 	i = cr + z;
      }
      //      g.drawString(text.substring(z, i), 2, y); // NO DRAW
      if (cr != -1) { // If we had newline, skip over it and add extra vert whsp
	i++;
 	y += yinc;
      }
      z = i;
      y += yinc;
      i = i+xinc > len?  len : i+xinc;
    }
    int setheight = y + 2 > 130 ? y + 2 : 130;
    if (height != setheight) {
      height = setheight;
      System.err.println("tsetsize: " + height);
    }
  }
  
  public void paint(Graphics g) {
    Rectangle r = g.getClipRect();
    //    System.err.println("TextBlock(paint) - Shape " + r);
    g.clearRect(0, 0, r.width, r.height);
    int len = text.length();
    int yinc = g.getFontMetrics().getHeight();
    int xinc = width/g.getFontMetrics().charWidth('a') - 3;
    dxinc = xinc;
    dyinc = yinc;
    int i = len < xinc ? len : xinc;
    int z = 0;
    int cr = -1;
    int y = yinc + 2;
    int savei;
    while (z < len) {
      savei = i;
      while (i < len && text.charAt(i) != 0x20) { // Move backward looking for space
	i--;
	if (i == 0) {
	  i = savei;
	  break;
	}
      }
      if (text.charAt(z) == 0x20 && z < i-1) z++;
      cr = (text.substring(z, i)).indexOf('\n');
      if (cr != -1) { // End line at newline character
	i = cr + z;
      }
      g.drawString(text.substring(z, i), 2, y);
      if (cr != -1) { // If we had newline, skip over it and add extra vert whsp
	i++;
	y += yinc;
      }
      z = i;
      y += yinc;
      i = i+xinc > len?  len : i+xinc;
    }
    int setheight = y + 2 > 130 ? y + 2 : 130;
    if (height != setheight) {
      height = setheight;
      System.err.println("TextBlock Parent: " + this.getParent());
      System.err.println("About to call parent validate routine");
      this.invalidate();
      if (this.getParent() != null) (this.getParent()).validate();
    }
  }
}


