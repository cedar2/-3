package com.kuang.appcontroller;

import com.alibaba.fastjson.JSON;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.kuang.pojo.*;
import com.kuang.result.R;
import com.kuang.service.*;
import com.kuang.service.impl.UserServiceImpl;
import com.kuang.utils.KuangUtils;
import com.kuang.vo.LoginReq;
import com.kuang.vo.PageReq;
import com.kuang.vo.RegisterForm;
import com.kuang.vo.UidReq;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
public class AppController {

    @Autowired
    InviteService inviteService;
    @Autowired
    UserService userService;
    @Autowired
    UserInfoService userInfoService;

    @Autowired
    UserServiceImpl userServiceImpl;

    @Autowired
    QuestionCategoryService questionCategoryService;
    @Autowired
    QuestionService questionService;
    @Autowired
    CommentService commentService;

    @Autowired
    BlogCategoryService blogCategoryService;
    @Autowired
    BlogService blogService;

    @Autowired
    SayService sayService;

    @GetMapping("/app/test")
    public R test(){

        // 注册成功，重定向到登录页面
        return R.ok();

    }

    // 注册业务
    @PostMapping("/app/appRegister")
    public R register(@RequestBody  RegisterForm registerForm){
        KuangUtils.print("注册表单信息："+registerForm.toString());

        // 用户名已存在
        User hasUser = userService.getOne(new QueryWrapper<User>().eq("username", registerForm.getUsername()));
        if (hasUser!=null){
            return R.error("用户名已存在");
        }

        User user = new User();
        user.setUid(KuangUtils.getUuid()); // 用户唯一id
        user.setRoleId(2);
        user.setUsername(registerForm.getUsername());
        // 密码加密
        String bCryptPassword = new BCryptPasswordEncoder().encode(registerForm.getPassword());
        user.setPassword(bCryptPassword);
        user.setGmtCreate(KuangUtils.getTime());
        user.setLoginDate(KuangUtils.getTime());
        // 保存对象！
        userService.save(user);
        KuangUtils.print("新用户注册成功："+user);

        // todo: 用户信息
        userInfoService.save(new UserInfo().setUid(user.getUid()));

        // 注册成功，重定向到登录页面
        return R.ok();

    }

    @PostMapping("/app/appLogin")
    public R appLogin(@RequestBody LoginReq loginReq){
        KuangUtils.print("登录信息："+ JSON.toJSONString(loginReq));

        UserDetails ud = userServiceImpl.loadUserByUsername(loginReq.getUsername());

        KuangUtils.print("用户信息："+JSON.toJSONString(ud));

        if(ud == null){
            return R.error("用户名不存在");
        }

        if(KuangUtils.passwdMatch(loginReq.getPassword(),ud.getPassword()) == true){
            User user = userServiceImpl.getUserByUsername(loginReq.getUsername());

            Map<String,Object> map = new HashMap<>();
            map.put("uid",user.getUid());
            map.put("uname",user.getUsername());

            R r = R.ok();
            r.setData(map);
            return r;
        }else{
            return R.error("密码错误");
        }

    }

    @PostMapping("/app/quesList")
    public R questionListPage(@RequestBody PageReq pageReq){

        int page = pageReq.getPage();
        int limit = pageReq.getLimit();

        if (page < 1){
            page = 1;
        }
        Page<Question> pageParam = new Page<>(page, limit);
        questionService.page(pageParam,new QueryWrapper<Question>().orderByDesc("gmt_create"));

        // 结果
        List<Question> questionList = pageParam.getRecords();

        Map<String,Object> map = new HashMap<>();
        map.put("pageParam",pageParam);

        R r = R.ok();
        r.setData(map);

        return r;
    }


    @PostMapping("/app/blogList")
    public R blogListPage(@RequestBody PageReq pageReq){

        int page = pageReq.getPage();
        int limit = pageReq.getLimit();

        if (page < 1){
            page = 1;
        }
        Page<Blog> pageParam = new Page<>(page, limit);
        blogService.page(pageParam,new QueryWrapper<Blog>().orderByDesc("gmt_create"));

        Map<String,Object> map = new HashMap<>();
        map.put("pageParam",pageParam);

        R r = R.ok();
        r.setData(map);

        return r;
    }

    @PostMapping("/app/sayList")
    public R sayList(){

        Page<Say> pageParam = new Page<>(1, 50);
        sayService.page(pageParam,new QueryWrapper<Say>().orderByDesc("gmt_create"));

        Map<String,Object> map = new HashMap<>();
        map.put("pageParam",pageParam);

        R r = R.ok();
        r.setData(map);

        return r;
    }

    @PostMapping("/app/uinfor")
    public R uinfor(@RequestBody UidReq uidReq){
        Map<String,Object> map = new HashMap<>();

        UserInfo userInfo = userInfoService.getById(uidReq.getUid());
        map.put("userInfo",userInfo);
        if (userInfo.getHobby()!=null && !userInfo.getHobby().equals("")){
            String[] hobbys = userInfo.getHobby().split(",");
            map.put("infoHobbys",hobbys);
        }
        // 获取用户的问题，博客，回复数
        int blogCount = blogService.count(new QueryWrapper<Blog>().eq("author_id", uidReq.getUid()));
        int questionCount = questionService.count(new QueryWrapper<Question>().eq("author_id", uidReq.getUid()));
        int commentCount = commentService.count(new QueryWrapper<Comment>().eq("user_id", uidReq.getUid()));

        map.put("blogCount",blogCount);
        map.put("questionCount",questionCount);
        map.put("commentCount",commentCount);

        R r = R.ok();
        r.setData(map);

        return r;
    }
}
