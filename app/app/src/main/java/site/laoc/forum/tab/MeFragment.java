package site.laoc.forum.tab;

import static com.blankj.utilcode.util.ViewUtils.runOnUiThread;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.blankj.utilcode.util.LogUtils;
import com.kongzue.dialogx.dialogs.TipDialog;
import com.kongzue.dialogx.dialogs.WaitDialog;
import com.scwang.smart.refresh.footer.ClassicsFooter;
import com.scwang.smart.refresh.header.ClassicsHeader;
import com.scwang.smart.refresh.layout.api.RefreshLayout;
import com.scwang.smart.refresh.layout.listener.OnLoadMoreListener;
import com.scwang.smart.refresh.layout.listener.OnRefreshListener;

import org.w3c.dom.Text;

import site.laoc.forum.HomeActivity;
import site.laoc.forum.LoginActivity;
import site.laoc.forum.R;
import site.laoc.forum.entity.Blog;
import site.laoc.forum.net.OnRes;
import site.laoc.forum.net.ReqModelUtils;
import site.laoc.forum.net.ReqUtils;
import site.laoc.forum.net.Result;
import site.laoc.forum.utils.Constant;
import site.laoc.forum.utils.DataUtils;
import site.laoc.forum.utils.DateUtils;
import site.laoc.forum.view.BaseFragment;

public class MeFragment extends BaseFragment {

    private View root;

    private Context context;

    private TextView tv_uname;

    private TextView tv_blog_size;

    private TextView tv_ques_size;

    private TextView tv_back_size;

    private RelativeLayout rl_infor_mod;

    private TextView tv_quit;

    private ReqUtils reqUtils = new ReqUtils();

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {

        if(root == null){
            //解析fragment的xml
            root = inflater.inflate(R.layout.fragment_me,container,false);
        }

        context = getActivity();

        tv_uname = (TextView) root.findViewById(R.id.tv_uname);
        tv_blog_size = (TextView) root.findViewById(R.id.tv_blog_size);
        tv_ques_size = (TextView) root.findViewById(R.id.tv_ques_size);
        tv_back_size = (TextView) root.findViewById(R.id.tv_back_size);

        rl_infor_mod = (RelativeLayout) root.findViewById(R.id.rl_infor_mod);

        tv_quit = (TextView) root.findViewById(R.id.tv_quit);

        tv_quit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(getActivity(), LoginActivity.class);
                startActivity(intent);
                getActivity().finish();
            }
        });

        getUInfor();
        return root;
    }

    private void getUInfor(){
        showLoading("正在获取信息...");

        String reqData = ReqModelUtils.getUidReq(DataUtils.getUid(context));

        reqUtils.jsonAsync(Constant.BASE_URL + "uinfor",reqData,new OnRes() {
            @Override
            public void onSucess(Result result) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        dealWithUinfor(result);
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

    private void dealWithUinfor(Result result){
        hideLoading();

        if(result.getCode() != 20000){
            TipDialog.show(result.getMessage(), WaitDialog.TYPE.ERROR);
            return;
        }

        JSONObject jo = JSON.parseObject(result.getData());
        JSONObject userInfo = jo.getJSONObject("userInfo");

        int blogCount = jo.getInteger("blogCount");
        int questionCount = jo.getInteger("questionCount");
        int commentCount = jo.getInteger("commentCount");

        tv_uname.setText(DataUtils.getUname(context));
        tv_blog_size.setText(""+blogCount);
        tv_ques_size.setText("" + questionCount);
        tv_back_size.setText("" + commentCount);

    }
}
