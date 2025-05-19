package site.laoc.forum;

import android.Manifest;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.blankj.utilcode.util.LogUtils;
import com.kongzue.dialogx.dialogs.TipDialog;
import com.kongzue.dialogx.dialogs.WaitDialog;

import org.xutils.view.annotation.ViewInject;
import org.xutils.x;

import pub.devrel.easypermissions.EasyPermissions;
import site.laoc.forum.net.OnRes;
import site.laoc.forum.net.ReqModelUtils;
import site.laoc.forum.net.ReqUtils;
import site.laoc.forum.net.Result;
import site.laoc.forum.utils.Constant;
import site.laoc.forum.utils.DataUtils;

public class LoginActivity extends BaseActivity{

    private Context context;

    @ViewInject(R.id.et_uname)
    private EditText et_uname;

    @ViewInject(R.id.et_passwd)
    private EditText et_passwd;

    @ViewInject(R.id.tv_login_btn)
    private TextView tv_login_btn;

    @ViewInject(R.id.tv_toregister)
    private TextView tv_toregister;

    @ViewInject(R.id.iv_header_back)
    private ImageView iv_header_back;

    private ReqUtils reqUtils = new ReqUtils();

    private static final String[] NEEDED_PERMISSIONS = new String[]{
            android.Manifest.permission.CAMERA,
            android.Manifest.permission.READ_PHONE_STATE,
            android.Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    @Override
    public void initWidget(Bundle savedInstanceState) {
        setContentView(R.layout.activity_login);

        this.context = LoginActivity.this;

        x.view().inject(LoginActivity.this);

        if (EasyPermissions.hasPermissions(LoginActivity.this, NEEDED_PERMISSIONS)) {
        } else {
            EasyPermissions.requestPermissions(LoginActivity.this, "需要读取文件的权限和摄像头权限",
                    1000, NEEDED_PERMISSIONS);
        }


        initView();
    }

    private void initView(){
        tv_login_btn.setOnClickListener(this);
        tv_toregister.setOnClickListener(this);
        iv_header_back.setOnClickListener(this);
    }

    @Override
    public void widgetClick(View v) {
        switch(v.getId()){
            case R.id.tv_login_btn:
                login();
                break;
            case R.id.tv_toregister:
                toReg();
                break;
            case R.id.iv_header_back:
                LoginActivity.this.finish();
                break;
        }
    }

    private void login(){
        String uname = et_uname.getText().toString();
        String passwd = et_passwd.getText().toString();

        if(uname.equals("")){
            TipDialog.show("请输入用户名", WaitDialog.TYPE.ERROR);
            return;
        }

        if(passwd.equals("")){
            TipDialog.show("请输入密码", WaitDialog.TYPE.ERROR);
            return;
        }

        showLoading("正在获取信息...");

        String reqData = ReqModelUtils.getLoginReq(uname,passwd);

        reqUtils.jsonAsync(Constant.BASE_URL + "appLogin",reqData,new OnRes() {
            @Override
            public void onSucess(Result result) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        dealWithLogin(result);
                    }
                });
            }

            @Override
            public void onFailure(Result result) {
                runOnUiThread(new Runnable(){
                    @Override
                    public void run() {
                        LogUtils.e(result.getMessage());
                        hideLoading();
                        TipDialog.show(result.getMessage(), WaitDialog.TYPE.ERROR);
                    }
                });
            }
        });
    }

    private void dealWithLogin(Result result){
        hideLoading();

        if(result.getCode() != 20000){
            TipDialog.show(result.getMessage(), WaitDialog.TYPE.ERROR);
            return;
        }

        JSONObject jo = JSON.parseObject(result.getData());
        String uname = jo.getString("uname");
        String uid = jo.getString("uid");

        DataUtils.setUname(context,uname);
        DataUtils.setUid(context,uid);

        toHome();

    }

    private void toHome(){
        Intent intent = new Intent(LoginActivity.this,HomeActivity.class);
        startActivity(intent);
        LoginActivity.this.finish();
    }

    private void toReg(){
        Intent intent = new Intent(LoginActivity.this,RegActivity.class);
        startActivity(intent);
    }

    @Override
    void afterRequestPermission(int requestCode, boolean isAllGranted) {

    }
}
