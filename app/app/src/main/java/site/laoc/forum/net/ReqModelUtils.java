package site.laoc.forum.net;

import com.alibaba.fastjson.JSON;

import site.laoc.forum.net.reqmodel.LoginReq;
import site.laoc.forum.net.reqmodel.PageReq;
import site.laoc.forum.net.reqmodel.RegReq;
import site.laoc.forum.net.reqmodel.UidReq;

public class ReqModelUtils {

    public static String getLoginReq(String uname,String passwd){
        LoginReq loginReq = new LoginReq();
        loginReq.setUsername(uname);
        loginReq.setPassword(passwd);

        return JSON.toJSONString(loginReq);
    }

    public static String getRegReq(String uname,String passwd,String repass,String code){
        RegReq regReq = new RegReq();
        regReq.setUsername(uname);
        regReq.setPassword(passwd);
        regReq.setRepassword(repass);
        regReq.setCode(code);

        return JSON.toJSONString(regReq);
    }

    public static String getPageReq(int page,int limit){
        PageReq pageReq = new PageReq();
        pageReq.setPage(page);
        pageReq.setLimit(limit);

        return JSON.toJSONString(pageReq);
    }

    public static String getUidReq(String uid){
        UidReq uidReq = new UidReq();
        uidReq.setUid(uid);

        return JSON.toJSONString(uidReq);
    }

}
