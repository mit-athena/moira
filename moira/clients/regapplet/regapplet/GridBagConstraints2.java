package regapplet;

import java.awt.GridBagConstraints;
import java.awt.Insets;

public synchronized class GridBagConstraints2 extends GridBagConstraints
{
    public GridBagConstraints2(int i1, int j1, int k1, int i2, double d1, double d2, int j2, int k2, Insets insets, int i3, int j3)
    {
        gridx = i1;
        gridy = j1;
        gridwidth = k1;
        gridheight = i2;
        fill = k2;
        ipadx = i3;
        ipady = j3;
        this.insets = insets;
        anchor = j2;
        weightx = d1;
        weighty = d2;
    }

    public String toString()
    {
        return String.valueOf(String.valueOf(String.valueOf(String.valueOf(String.valueOf(String.valueOf(String.valueOf(": ").concat(String.valueOf(gridx))).concat(String.valueOf(","))).concat(String.valueOf(gridy))).concat(String.valueOf(","))).concat(String.valueOf(gridwidth))).concat(String.valueOf(","))).concat(String.valueOf(gridheight));
    }
}
