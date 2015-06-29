
//Title:        Athena Registration Applet
//Version:      
//Copyright:    Copyright (c) 1997
//Author:       Jeffrey I. Schiller
//Company:      MIT
//Description:  Applet to facilitate new user Athena account registration


package regapplet;

import java.awt.*;
import java.awt.event.*;

public class MessagePanel extends Panel {
  TextBlock textArea1 = new TextBlock(600,150);
  Button nokButton = new Button();
  Button okButton = new Button();
  Button sbutton = null;
  GridBagLayout gridBagLayout1 = new GridBagLayout();
  Regapplet applet;

  public MessagePanel(Regapplet tapplet) {
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
    textArea1.setBackground(Color.white);
    nokButton.setBackground(Color.white);
    okButton.setBackground(Color.white);
    this.add(textArea1);
    gridBagLayout1.setConstraints(textArea1, new GridBagConstraints2(0, 0, 3, 3, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.BOTH, new Insets(20, 20, 10, 20), 0, 0));
  }
  public boolean action (Event evt, Object arg) {
         if (evt.target == sbutton) {
            if (applet.worker != null) {
               applet.workthread = new Thread(applet.worker);
               applet.workthread.start();
            }
         }
         return false;
  }

  public boolean keyDown(Event evt, int c) {
    System.out.println("KeyDown: Event: " + evt.toString() + " Char: " + c);
    if (evt.target == sbutton && c == 10) { // Return typed on button
      this.action(evt, "Go");
      return (true);
    }
    return (false);
  }

  public void requestFocus() {
    sbutton.requestFocus();
  }

  public void setText(String s, boolean contOK) {
         textArea1.setText(s);
	 Button button;
	 if (contOK) {
	   button = okButton;
	   button.setLabel("Click Here to Continue");
	   button.requestFocus();
	 } else {
	   button = nokButton;
	   button.setLabel("Please Wait");
	 }
	 if (sbutton != null && sbutton != button) {
	   this.remove(sbutton);
	   sbutton = null;
	 }
	 if (sbutton == null) {
	   this.add(button);
	   gridBagLayout1.setConstraints(button,new GridBagConstraints2(2, 4, 1, 1, 0.0, 1.0
						     ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 20, 0), 0, 0));
	   sbutton = button;
	   if (contOK) { 
	     button.enable();
	     button.requestFocus();
	   }
	   else button.disable();
	 }
	 textArea1.repaint();
	 (this.getParent()).validate();
	 System.err.println("MessagePanel: Called parent validate routine");
  }
}

