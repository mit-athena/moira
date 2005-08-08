//Title:      Athena Registration Applet
//Version:    
//Copyright:  Copyright (c) 1997
//Author:     Jeffrey I. Schiller
//Company:    MIT
//Description:Applet to facilitate new user Athena account registration
package regapplet;

import java.awt.*;
import java.io.OutputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.awt.event.*;
import java.applet.*;
import mit.cipher.MITCipherException;
import mit.cipher.Encap;

// For sockets...

import java.net.Socket;
import java.net.ServerSocket;	// For debugging we are a server
import java.util.ResourceBundle;

public class Regapplet extends Applet {
  //  ResourceBundle res = ResourceBundle.getBundle("regapplet.Res");
  static final boolean doRules = true;
  mybundle res = new mybundle();
  boolean isStandalone = false;
  CardLayout cardLayout1 = new CardLayout();
  Panel NamePanel = new Panel();
  Label titlelabel = new Label();
  Label FirstNameLabel = new Label();
  Label LastNameLabel = new Label();
  Label MiddleNameLabel = new Label();
  Label MITIDlabel = new Label();
  TextField FirstName = new TextField("", 30);
  TextField LastName = new TextField("", 30);
  TextField MiddleName = new TextField("", 1);
  TextField MITID = new TextField("", 20); // Should only be 9, but Netscape loses
  Panel WordPanel = new Panel();
  TextBlock WordInstructions = new TextBlock(600,40);
  Button NameContButton = new Button();
  Button NameCancelButton = new Button();
  Button WordCancelButton = new Button();
  Button LoginCancelButton = new Button();
  Button PasswordCancelButton = new Button();
  Label WordLabel = new Label();
  TextField Word1 = new TextField("", 6);
  TextField Word2 = new TextField("", 6);
  TextField Word3 = new TextField("", 6);
  TextField Word4 = new TextField("", 6);
  TextField Word5 = new TextField("", 6);
  TextField Word6 = new TextField("", 6);
  Button WordContButton = new Button();
  GridBagLayout gridBagLayout1 = new GridBagLayout();
  GridBagLayout gridBagLayout2 = new GridBagLayout();
  GridLayout gridLayout1 = new GridLayout(3, 1);
  Button ExitButton = new Button();

  // PIN Panel.
  Panel PinPanel = new Panel();
  TextBlock PinInstructions = new TextBlock(600,40);
  Button PinCancelButton = new Button();
  Button PinContButton = new Button();
  TextField Pin = new TextField("", 8);
  Label PinLabel = new Label();
  GridBagLayout gridBagLayout7 = new GridBagLayout();

  // login confirmation panel.
  Panel LoginConfirmPanel = new Panel();
  TextBlock LoginConfirmInstructions = new TextBlock(600,40);
  Button LoginConfirmCancelButton = new Button();
  Button LoginConfirmContButton = new Button();
  GridBagLayout gridBagLayout8 = new GridBagLayout();

  Panel LoginPanel = new Panel();
  TextBlock LoginNameBanner = new TextBlock(600,30);
  Label LoginNameLabel = new Label();
  TextField LoginName = new TextField("", 15);
  Button LoginContButton = new Button();
  GridBagLayout gridBagLayout3 = new GridBagLayout();
  Panel PasswordPanel = new Panel();
  TextBlock PasswordBanner = new TextBlock(600,30);
  GridBagLayout gridBagLayout4 = new GridBagLayout();
  Label PasswordLabel1 = new Label();
  TextField Password1 = new TextField("", 10);
  Label PasswordLabel2 = new Label();
  TextField Password2 = new TextField("", 10);
  Button PasswordContButton = new Button();
  Panel GreetingPanel = new Panel();
  Button GreetingButton = new Button();
  Label Greeting1 = new Label();
  GridBagLayout gridBagLayout5 = new GridBagLayout();
  MessagePanel mp = new MessagePanel(this);
  BalePanel bp = new BalePanel(this);
  GridBagLayout gridBagLayout6 = new GridBagLayout();
  Panel RulePanel = new Panel();
  Button RuleButton = new Button();
  TextArea Rules = new TextArea(res.getString("rules"), 20, 72);
  Label RuleBanner = new Label();
  public Worker worker = null;
  public Thread workthread = null;
  public String guesslogin = "";
  public String chosenlogin = "";
 
  //Get a parameter value
  public String getParameter(String key, String def) {
    return isStandalone ? System.getProperty(key, def) :
      (getParameter(key) != null ? getParameter(key) : def);
  }

  //Construct the applet
  public Regapplet() {
  }

  //Initialize the applet
  public void init() {
    try { jbInit(); } catch (Exception e) { e.printStackTrace(); }
    try {
      this.add("MessagePanel", mp);
      this.add("BalePanel", bp);
    } catch (Exception e) {
      e.printStackTrace();
    }
    this.setBackground(Color.white);
  }

  //Component initialization
  public void jbInit() throws Exception{
      //    this.resize(new Dimension(600, 600));
    titlelabel.setForeground(Color.red);
    titlelabel.setFont(new Font("Helvetica", 1, 18));
    titlelabel.setText(res.getString("welcome"));
    titlelabel.setBackground(Color.white);
    FirstNameLabel.setText("Please Enter your First Name");
    FirstNameLabel.setBackground(Color.white);
    LastNameLabel.setText("Please Enter your Last Name");
    LastNameLabel.setBackground(Color.white);
    MiddleNameLabel.setText("Please Enter your Middle Initial");
    MiddleNameLabel.setBackground(Color.white);
    MITIDlabel.setText("Please enter your MIT ID Number");
    MITIDlabel.setBackground(Color.white);
    WordInstructions.setText(res.getString("getw"));
    NameContButton.setLabel("Continue");
    NameContButton.setBackground(Color.white);
    NameCancelButton.setLabel("Cancel");
    NameCancelButton.setBackground(Color.white);
    WordLabel.setText("Key Words:");
    WordLabel.setBackground(Color.white);
    WordContButton.setLabel("Continue");
    WordContButton.setBackground(Color.white);
    WordCancelButton.setLabel("Cancel");
    WordCancelButton.setBackground(Color.white);

    // PIN initialization.
    PinInstructions.setText(res.getString("geti"));
    PinLabel.setText("PIN:");
    PinLabel.setBackground(Color.white);
    PinContButton.setLabel("Continue");
    PinContButton.setBackground(Color.white);
    PinCancelButton.setLabel("Cancel");
    PinCancelButton.setBackground(Color.white);
    Pin.setEchoCharacter('*');

    // login confirmation initialization.
    LoginConfirmContButton.setLabel("Continue");
    LoginConfirmContButton.setBackground(Color.white);
    LoginConfirmCancelButton.setLabel("Cancel");
    LoginConfirmCancelButton.setBackground(Color.white);

    ExitButton.setLabel("Exit");
    ExitButton.setBackground(Color.white);
    LoginNameLabel.setText("Chosen Login Name: ");
    LoginNameLabel.setBackground(Color.white);
    LoginContButton.setLabel("Continue");
    LoginContButton.setBackground(Color.white);
    LoginCancelButton.setLabel("Cancel");
    LoginCancelButton.setBackground(Color.white);
    PasswordBanner.setText(res.getString("getp"));
    PasswordLabel1.setText("Please choose and enter a password");
    PasswordLabel1.setBackground(Color.white);
    Password1.setEchoCharacter('*');
    PasswordLabel2.setText("Please enter your password again");
    PasswordLabel2.setBackground(Color.white);
    Password2.setEchoCharacter('*');
    PasswordContButton.setLabel("Continue");
    PasswordContButton.setBackground(Color.white);
    PasswordCancelButton.setLabel("Cancel");
    PasswordCancelButton.setBackground(Color.white);
    PasswordPanel.setLayout(gridBagLayout4);
    LoginPanel.setLayout(gridBagLayout3);
    WordPanel.setLayout(gridBagLayout1);
    NamePanel.setLayout(gridBagLayout2);
    this.setLayout(cardLayout1);
    this.add("NamePanel", NamePanel);
    NamePanel.add(titlelabel);
    gridBagLayout2.setConstraints(titlelabel, new GridBagConstraints2(0, 0, 5, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(31, 24, 10, 24), 0, 0));
    NamePanel.add(FirstNameLabel);
    gridBagLayout2.setConstraints(FirstNameLabel, new GridBagConstraints2(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 10, 0, 0), 0, 0));
    NamePanel.add(MiddleNameLabel);
    gridBagLayout2.setConstraints(MiddleNameLabel, new GridBagConstraints2(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 10, 0, 0), 0, 0));
    NamePanel.add(LastNameLabel);
    gridBagLayout2.setConstraints(LastNameLabel, new GridBagConstraints2(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 10, 0, 0), 0, 0));
    NamePanel.add(MITIDlabel);
    gridBagLayout2.setConstraints(MITIDlabel, new GridBagConstraints2(0, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 10, 0, 0), 0, 0));
    NamePanel.add(FirstName);
    gridBagLayout2.setConstraints(FirstName, new GridBagConstraints2(2, 1, 3, 1, 0.0, 0.0
            ,GridBagConstraints.EAST, GridBagConstraints.HORIZONTAL, new Insets(10, 0, 0, 10), 0, 0));
    NamePanel.add(MiddleName);
    gridBagLayout2.setConstraints(MiddleName, new GridBagConstraints2(2, 2, 3, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 0, 0, 10), 0, 0));
    NamePanel.add(LastName);
    gridBagLayout2.setConstraints(LastName, new GridBagConstraints2(2, 3, 3, 1, 0.0, 0.0
            ,GridBagConstraints.EAST, GridBagConstraints.HORIZONTAL, new Insets(10, 0, 0, 10), 0, 0));
    NamePanel.add(MITID);
    gridBagLayout2.setConstraints(MITID, new GridBagConstraints2(2, 4, 3, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 0, 0, 10), 0, 0));
    NamePanel.add(NameContButton);
    gridBagLayout2.setConstraints(NameContButton, new GridBagConstraints2(2, 5, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(20, 10, 10, 10), 0, 0));
    NamePanel.add(NameCancelButton);
    gridBagLayout2.setConstraints(NameCancelButton, new GridBagConstraints2(4, 5, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(20, 10, 10, 10), 0, 0));

    if (isStandalone) {
      GreetingPanel.add(ExitButton);
      gridBagLayout5.setConstraints(ExitButton, new GridBagConstraints2(5, 5, 1, 1, 0.0, 1.0
            ,GridBagConstraints.EAST, GridBagConstraints.NONE, new Insets(20, 10, 10, 10), 0, 0));
    }
    this.add("WordPanel", WordPanel);
    WordPanel.add(WordInstructions);
    gridBagLayout1.setConstraints(WordInstructions, new GridBagConstraints2(0, 0, 7, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(19, 31, 10, 31), 0, 0));
    WordPanel.add(WordLabel);
    gridBagLayout1.setConstraints(WordLabel, new GridBagConstraints2(0, 1, 1, 2, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 10, 0, 0), 0, 0));
    WordPanel.add(Word1);
    gridBagLayout1.setConstraints(Word1, new GridBagConstraints2(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 10), 0, 0));
    WordPanel.add(Word2);
    gridBagLayout1.setConstraints(Word2, new GridBagConstraints2(2, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 10), 0, 0));
    WordPanel.add(Word3);
    gridBagLayout1.setConstraints(Word3, new GridBagConstraints2(3, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 10), 0, 0));
    WordPanel.add(Word4);
    gridBagLayout1.setConstraints(Word4, new GridBagConstraints2(1, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 10), 0, 0));
    WordPanel.add(Word5);
    gridBagLayout1.setConstraints(Word5, new GridBagConstraints2(2, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 10), 0, 0));
    WordPanel.add(Word6);
    gridBagLayout1.setConstraints(Word6, new GridBagConstraints2(3, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 10), 0, 0));
    WordPanel.add(WordContButton);
    gridBagLayout1.setConstraints(WordContButton, new GridBagConstraints2(2, 3, 2, 2, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 10, 0), 0, 0));
    WordPanel.add(WordCancelButton);
    gridBagLayout1.setConstraints(WordCancelButton, new GridBagConstraints2(4, 3, 2, 2, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 10, 0), 0, 0));
    this.add("LoginConfirmPanel", LoginConfirmPanel);
    LoginConfirmPanel.add(LoginConfirmInstructions);
    gridBagLayout8.setConstraints(LoginConfirmInstructions, new GridBagConstraints2(0, 0, 3, 3, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.BOTH, new Insets(10, 10, 10, 10), 0, 0));
    LoginConfirmPanel.add(LoginConfirmContButton);
    gridBagLayout8.setConstraints(LoginConfirmContButton, new GridBagConstraints2(1, 4, 1, 1, 0.0, 1.0
                  ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 20, 0), 0, 0));
    LoginConfirmPanel.add(LoginConfirmCancelButton);
    gridBagLayout8.setConstraints(LoginConfirmCancelButton, new GridBagConstraints2(2, 4, 1, 1, 0.0, 1.0
                  ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 20, 0), 0, 0));
    this.add("PinPanel", PinPanel);
    PinPanel.add(PinInstructions);
    gridBagLayout7.setConstraints(PinInstructions, new GridBagConstraints2(0, 0, 4, 1, 0.0, 0.0
	    ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(20, 10, 10, 20), 0, 0));
    PinPanel.add(PinLabel);
    gridBagLayout7.setConstraints(PinLabel, new GridBagConstraints2(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 10, 0, 0), 0, 0));
    PinPanel.add(Pin);
    gridBagLayout7.setConstraints(Pin, new GridBagConstraints2(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 0, 0, 10), 2, 0));
    PinPanel.add(PinContButton);
    gridBagLayout7.setConstraints(PinContButton, new GridBagConstraints2(1, 2, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 10, 10, 10), 0, 0));
    PinPanel.add(PinCancelButton);
    gridBagLayout7.setConstraints(PinCancelButton, new GridBagConstraints2(3, 2, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 10, 10, 10), 0, 0));
    this.add("LoginPanel", LoginPanel);
    LoginPanel.add(LoginNameBanner);
    gridBagLayout3.setConstraints(LoginNameBanner, new GridBagConstraints2(0, 0, 4, 1, 0.0, 0.0
	    ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(20, 10, 10, 20), 0, 0));
    LoginPanel.add(LoginNameLabel);
    gridBagLayout3.setConstraints(LoginNameLabel, new GridBagConstraints2(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 10, 0, 0), 0, 0));
    LoginPanel.add(LoginName);
    gridBagLayout3.setConstraints(LoginName, new GridBagConstraints2(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 0, 0, 10), 2, 0));
    LoginPanel.add(LoginContButton);
    gridBagLayout3.setConstraints(LoginContButton, new GridBagConstraints2(1, 2, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 10, 10, 10), 0, 0));
    LoginPanel.add(LoginCancelButton);
    gridBagLayout3.setConstraints(LoginCancelButton, new GridBagConstraints2(3, 2, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 10, 10, 10), 0, 0));
    this.add("PasswordPanel", PasswordPanel);
    PasswordPanel.add(PasswordBanner);
    gridBagLayout4.setConstraints(PasswordBanner, new GridBagConstraints2(0, 0, 6, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(20, 10, 10, 20), 0, 0));
    PasswordPanel.add(PasswordLabel1);
    gridBagLayout4.setConstraints(PasswordLabel1, new GridBagConstraints2(0, 1, 3, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 10, 0, 0), 0, 0));
    PasswordPanel.add(Password1);
    gridBagLayout4.setConstraints(Password1, new GridBagConstraints2(3, 1, 3, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 10, 0, 10), 0, 0));
    PasswordPanel.add(PasswordLabel2);
    gridBagLayout4.setConstraints(PasswordLabel2, new GridBagConstraints2(0, 2, 3, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 10, 0, 0), 0, 0));
    PasswordPanel.add(Password2);
    gridBagLayout4.setConstraints(Password2, new GridBagConstraints2(3, 2, 3, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(10, 10, 0, 10), 0, 0));
    PasswordPanel.add(PasswordContButton);
    gridBagLayout4.setConstraints(PasswordContButton, new GridBagConstraints2(3, 4, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 0, 0), 0, 0));
    PasswordPanel.add(PasswordCancelButton);
    gridBagLayout4.setConstraints(PasswordCancelButton, new GridBagConstraints2(5, 4, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 0, 0, 0), 0, 0));
    GreetingPanel.setLayout(gridBagLayout5);
    GreetingButton.setLabel("Start!");
    GreetingButton.setBackground(Color.white);
    Greeting1.setForeground(Color.red);
    Greeting1.setFont(new Font("Dialog", 1, 18));
    Greeting1.setText(res.getString("welcome"));
    Greeting1.setBackground(Color.white);
    this.add("GreetingPanel", GreetingPanel);
    GreetingPanel.add(GreetingButton);
    gridBagLayout5.setConstraints(GreetingButton, new GridBagConstraints2(1, 1, 1, 1, 0.0, 1.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10, 10, 20, 10), 0, 0));
    GreetingPanel.add(Greeting1);
    gridBagLayout5.setConstraints(Greeting1, new GridBagConstraints2(0, 0, 2, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(20, 10, 10, 10), 0, 0));
    RulePanel.setLayout(gridBagLayout6);
    RuleButton.setLabel("I have read and understand the Rules of Use");
    Rules.setBackground(Color.white);
    RuleButton.setBackground(Color.white);
    RuleBanner.setFont(new Font("Helvetica", 1, 14));
    RuleBanner.setText("Please Read the Rules of Use");
    RuleBanner.setBackground(Color.white);
    RulePanel.add(RuleBanner);
    gridBagLayout6.setConstraints(RuleBanner, new GridBagConstraints2(0, 0, 3, 1, 0.0, 0.0
           ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(10,10, 10, 10), 0, 0));
    RulePanel.add(Rules);
    Rules.setEditable(false);
    gridBagLayout6.setConstraints(Rules, new GridBagConstraints2(0, 1, 3, 1, 0.0, 0.0
           ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(10,10, 10, 10), 0, 0));
    RulePanel.add(RuleButton);
    gridBagLayout6.setConstraints(RuleButton, new GridBagConstraints2(2, 2, 1, 1, 0.0, 1.0
           ,GridBagConstraints.NORTHWEST, GridBagConstraints.NONE, new Insets(10,10, 10, 10), 0, 0));
    this.add("RulePanel", RulePanel);
    showInit();
  }

  public void clear() {
    LastName.setText("");
    LoginName.setText("");
    MiddleName.setText("");
    FirstName.setText("");
    MITID.setText("");
    Password1.setText("");
    Password2.setText("");
    Word1.setText("");
    Word2.setText("");
    Word3.setText("");
    Word4.setText("");
    Word5.setText("");
    Word6.setText("");
    Pin.setText("");
    guesslogin = "";
    chosenlogin = "";
  }

  //Start the applet
  public void start() {
  }

  //Stop the applet
  public void stop() {
  }

  //Destroy the applet
  public void destroy() {
  }

  //Get Applet information
  public String getAppletInfo() {
    return "Athena Registration Applet by Jeffrey I. Schiller 3/28/98";
  }

  //Get parameter info
  public String[][] getParameterInfo() {
    return null;
  }

  //Main method
  static public void main(String[] args) {
    Regapplet applet = new Regapplet();
    applet.isStandalone = true;
    Frame frame = new Frame();
    frame.setTitle("Athena Registration Applet");
    frame.add("Center", applet);
    //    frame.add(BorderLayout.CENTER, applet);
    applet.init();
    applet.start();
    //frame.pack();
    //    frame.setSize(700,550);
    frame.resize(910,550);
    Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
    //    frame.setLocation((d.width - frame.getSize().width) / 2, (d.height - frame.getSize().height) / 2);
    frame.show();
    //    frame.setVisible(true);
  }

  public void showNameDiag() {
    cardLayout1.show(this, "NamePanel");
    FirstName.requestFocus();
  }

  public void showWordsDiag() {
    cardLayout1.show(this, "WordPanel");
    Word1.requestFocus();
  }

  public void showPinDiag() {
    cardLayout1.show(this, "PinPanel");
    Pin.requestFocus();
  }

  public void showLoginDiag() {
    if (!guesslogin.equals("")) {
      LoginNameBanner.setText(res.getString("getl1") + "\n" + res.getString("getl2") + " " + guesslogin + "\n" + res.getString("getl3"));
    } else {
      LoginNameBanner.setText(res.getString("getl1") + "\n" + res.getString("getl3"));
    }
    cardLayout1.show(this, "LoginPanel");
    LoginName.requestFocus();
  }

  public void showLoginConfirmDiag() {
    if (!chosenlogin.equals("")) {
    LoginConfirmInstructions.setText("We have confirmed that the Athena username " + chosenlogin + " is available for your use." + "\n" + "If you choose to continue, your email address will be " + chosenlogin + "@mit.edu, and any personal web content you choose to publish will be available at the URL http://web.mit.edu/" + chosenlogin + "/www/ . After choosing to continue, this username will be registered for you without further confirmation, and CANNOT BE CHANGED." + "\n" + "Are you sure you would like to proceed?");
    }
    cardLayout1.show(this, "LoginConfirmPanel");
    LoginConfirmContButton.requestFocus();
  }

  public void showPassDiag() {
    cardLayout1.show(this, "PasswordPanel");
    Password1.requestFocus();
  }

  public void showRules() {
    cardLayout1.show(this, "RulePanel");
    RuleButton.requestFocus();
  }

  public void showInit() {
    if (worker != null) {
       worker.close();
       worker = null;
    }
    this.clear();
    cardLayout1.show(this, "GreetingPanel");
    GreetingButton.requestFocus();
  }

  public void showError() {
    showMessage(res.getString("err"));
  }

  public void showError(boolean val) {
    showMessage(res.getString("err"), val);
  }

  public void showError1(boolean val) {
    if (isStandalone) {
       showMessage(res.getString("err1"), val);
    } else {
       showMessage(res.getString("err1a"), val);
    }
  }

  public void showMessage(String mess) {
    mp.setText(mess, false);
    cardLayout1.show(this, "MessagePanel");
  }
  public void showMessage(String mess, boolean contOK) {
    mp.setText(mess, true);
    cardLayout1.show(this, "MessagePanel");
    mp.requestFocus();
  }

  private void doBale() {
    if (worker != null) {
      worker.oldstate = worker.state;
      cardLayout1.show(this, "BalePanel");
    }
  }

  public boolean keyDown(Event evt, int c) {
    //    System.out.println("Applet: KeyDown: Event: " + evt.toString() + " c: " + c);
    if (evt.target instanceof Button) {
      if (c == 10) {
	this.action(evt, ((Button) evt.target).getLabel());
	return true;
      }
    }
    return false;
  }

  public boolean action(Event evt, Object arg) {
    System.err.println("action: " + evt.toString() + arg.toString());
    if (evt.target == GreetingButton) {
      if (worker != null) {
	System.err.println("Worker not null when Greeting Button pressed!");
      } else {
	worker = new Worker(this);
	workthread = new Thread(worker);
	workthread.start();
      }
      return true;
    } else if ((evt.target == WordContButton) ||
	       (evt.target == PinContButton) || 
               (evt.target == LoginConfirmContButton) ||
	       (evt.target == LoginContButton) ||
               (evt.target == PasswordContButton) ||
               (evt.target == NameContButton) ||
	       (evt.target == MITID) ||
	       (evt.target == LoginName) ||
	       (evt.target == Word6) ||
	       (evt.target == Password2) ||
	       (evt.target == RuleButton)) {
      workthread = new Thread(worker);
      workthread.start();
      return true;
    } else if (evt.target == LoginConfirmCancelButton) {
	  chosenlogin = "";
	  showLoginDiag();
    } else if (evt.target == ExitButton) {
      if (isStandalone) System.exit (0);
    } else if (evt.target == NameCancelButton ||
		evt.target == WordCancelButton ||
	        evt.target == PinCancelButton ||
		evt.target == LoginCancelButton ||
		evt.target == PasswordCancelButton) {
      doBale();
      return true;
    }
    return false;
  }
}
