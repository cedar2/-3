package site.laoc.forum.net;

public class ImgResult {
    private String Name;
    private String Session;
    private Integer code;
    private String Message;
    private ImgResData Data;



    public String getName() {
        return Name;
    }

    public void setName(String name) {
        Name = name;
    }

    public String getSession() {
        return Session;
    }

    public void setSession(String session) {
        Session = session;
    }

    public Integer getCode() {
        return code;
    }

    public void setCode(Integer code) {
        this.code = code;
    }

    public String getMessage() {
        return Message;
    }

    public void setMessage(String message) {
        Message = message;
    }

    public ImgResData getData() {
        return Data;
    }

    public void setData(ImgResData data) {
        Data = data;
    }
}
