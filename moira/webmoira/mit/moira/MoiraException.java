package mit.moira;

public class MoiraException extends Exception {
    int code = 0;

    MoiraException(String mess) {
	super(mess);
    }

    MoiraException(String mess, int code) {
	super(mess);
	this.code = code;
    }

    public int getCode() {
	return (code);
    }

}
