package site.laoc.forum;

import android.Manifest;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.blankj.utilcode.util.LogUtils;
import com.kongzue.dialogx.dialogs.TipDialog;
import com.kongzue.dialogx.dialogs.WaitDialog;

import org.xutils.view.annotation.ViewInject;
import org.xutils.x;

import site.laoc.forum.net.OnRes;
import site.laoc.forum.net.ReqModelUtils;
import site.laoc.forum.net.ReqUtils;
import site.laoc.forum.net.Result;
import site.laoc.forum.utils.Constant;

public class RegActivity extends BaseActivity{

    private Context context;

    @ViewInject(R.id.iv_header_back)
    private ImageView iv_header_back;

    @ViewInject(R.id.et_uname)
    private EditText et_uname;

    @ViewInject(R.id.et_passwd)
    private EditText et_passwd;

    @ViewInject(R.id.et_repasswd)
    private EditText et_repasswd;

    @ViewInject(R.id.et_invite_code)
    private EditText et_invite_code;

    @ViewInject(R.id.tv_reg_btn)
    private TextView tv_reg_btn;

    private ReqUtils reqUtils = new ReqUtils();

    @Override
    public void initWidget(Bundle savedInstanceState) {
        setContentView(R.layout.activity_reg);

        this.context = RegActivity.this;

        x.view().inject(RegActivity.this);

        initView();
    }

    private void initView(){
        tv_reg_btn.setOnClickListener(this);
        iv_header_back.setOnClickListener(this);
    }

    @Override
    public void widgetClick(View v) {
        switch(v.getId()){
            case R.id.iv_header_back:
                RegActivity.this.finish();
                break;
            case R.id.tv_reg_btn:
                reg();
                break;
        }
    }

    private void reg(){
        String uname = et_uname.getText().toString();
        String passwd = et_passwd.getText().toString();
        String repass = et_repasswd.getText().toString();
        String code = et_invite_code.getText().toString();

        if(uname.equals("")){
            TipDialog.show("请输入用户名", WaitDialog.TYPE.ERROR);
            return;
        }

        if(passwd.equals("")){
            TipDialog.show("请输入密码", WaitDialog.TYPE.ERROR);
            return;
        }

        if(repass.equals("")){
            TipDialog.show("请再次输入密码", WaitDialog.TYPE.ERROR);
            return;
        }

        if(code.equals("")){
            TipDialog.show("请输入邀请码", WaitDialog.TYPE.ERROR);
            return;
        }

        showLoading("正在获取信息...");

        String reqData = ReqModelUtils.getRegReq(uname,passwd,repass,code);

        reqUtils.jsonAsync(Constant.BASE_URL + "appRegister",reqData,new OnRes() {
            @Override
            public void onSucess(Result result) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        dealWithReg(result);
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

    private void dealWithReg(Result result){
        hideLoading();

        if(result.getCode() != 20000){
            TipDialog.show(result.getMessage(), WaitDialog.TYPE.ERROR);
            return;
        }

        RegActivity.this.finish();
    }

    @Override
    void afterRequestPermission(int requestCode, boolean isAllGranted) {

    }
}
