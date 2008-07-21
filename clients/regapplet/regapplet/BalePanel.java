
//Title:        Athena Registration Applet
//Version:      
//Copyright:    Copyright (c) 1997
//Author:       Jeffrey I. Schiller
//Company:      MIT
//Description:  Applet to facilitate new user Athena account registration


package regapplet;

import java.awt.*;
import java.awt.event.*;

public class BalePanel extends Panel {
  TextBlock textArea1 = new TextBlock(600,150);
  Button BaleButton = new Button();
  Button ReturnButton = new Button();
  GridBagLayout gridBagLayout1 = new GridBagLayout();
  Regapplet applet;

  public BalePanel(Regapplet tapplet) {
    applet = tapplet;
    try {
      jbInit();
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  }

  public void jbInit() throws Exception{
    this.setLayout(gridBagLayout1);
    BaleButton.setLabel("Yes, I want to quit!");
    BaleButton.setBackground(Color.white);
    ReturnButton.setBackground(Color.white);
    ReturnButton.setLabel("No, go back to where I was!");
    textArea1.setText(applet.res.getString("bale"));
    textArea1.setBackground(Color.white);
    this.add(textArea1);
    gridBagLayout1.setConstraints(textArea1, new GridBagConstraints2(0, 0, 3, 3, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.BOTH, new Insets(10, 10, 10, 10), 0, 0));
    this.add(BaleButton);
    gridBagLayout1.setConstraints(BaleButton,new GridBagConstraints2(1, 4, 1, 1, 0.0, 1.0
		  ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 20, 0), 0, 0));
    this.add(ReturnButton);
    gridBagLayout1.setConstraints(ReturnButton,new GridBagConstraints2(2, 4, 1, 1, 0.0, 1.0
		  ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 20, 0), 0, 0));
    
    
  }
  public boolean action (Event evt, Object arg) {
         if (evt.target == BaleButton) {
	   if (applet.isStandalone) {
	     applet.worker.close();
	     System.exit(0);
	   } else {
	     applet.worker.close();
	     applet.workthread = null;
	     applet.worker = null;
	     applet.showInit();
	   }
         } else if (evt.target == ReturnButton) {
	   if (applet.worker != null) {
	     applet.worker.nextstate = applet.worker.oldstate;
	     applet.worker.state = Worker.DIALOG;
	     applet.worker.run();	// Don't need a separate thread, no net i/o
	   }
	 }
         return false;
  }
//    public void setText(String s, boolean contOK) {
//          textArea1.setText(s);
// 	 textArea1.repaint();
// 	 (this.getParent()).validate();
// 	 System.err.println("BalePanel: Called parent validate routine");
//   }
}

