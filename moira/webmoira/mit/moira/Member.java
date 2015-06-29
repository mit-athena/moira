package mit.moira;

public class Member {
    String member_type;
    String member_id;

    public Member(String mt, String mi) {
	member_type = mt;
	member_id = mi;
    }

    public String getMemberType() {
	return (member_type);
    }

    public String getMemberId() {
	return (member_id);
    }

    public String toString() {
	return("[Member type=" + member_type + ", id=" + member_id + "]");
    }
}
