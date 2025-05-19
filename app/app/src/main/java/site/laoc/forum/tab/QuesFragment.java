package site.laoc.forum.tab;

import static com.blankj.utilcode.util.ViewUtils.runOnUiThread;

import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresOptIn;

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

import java.util.ArrayList;
import java.util.List;

import site.laoc.forum.R;
import site.laoc.forum.adapter.QuesAdapter;
import site.laoc.forum.entity.Ques;
import site.laoc.forum.net.OnRes;
import site.laoc.forum.net.ReqModelUtils;
import site.laoc.forum.net.ReqUtils;
import site.laoc.forum.net.Result;
import site.laoc.forum.utils.Constant;
import site.laoc.forum.utils.DataUtils;
import site.laoc.forum.view.BaseFragment;

public class QuesFragment extends BaseFragment {

    private View root;

    private Context context;

    private RefreshLayout refreshLayout;

    private ListView lv_ques;

    private List<Ques> quesList = new ArrayList<>();
    private QuesAdapter quesAdapter;

    private int page = 1;
    private int limit = 10;
    private int pages = 1;

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
            root = inflater.inflate(R.layout.fragment_ques,container,false);
        }

        context = getActivity();

        refreshLayout = (RefreshLayout)root.findViewById(R.id.refreshLayout);
        refreshLayout.setRefreshHeader(new ClassicsHeader(context));
        refreshLayout.setRefreshFooter(new ClassicsFooter(context));
        refreshLayout.setOnRefreshListener(new OnRefreshListener() {
            @Override
            public void onRefresh(RefreshLayout refreshlayout) {
                //refreshlayout.finishRefresh(2000/*,false*/);//传入false表示刷新失败
                page = 1;
                quesList.clear();
                getQuesList();
            }
        });
        refreshLayout.setOnLoadMoreListener(new OnLoadMoreListener() {
            @Override
            public void onLoadMore(RefreshLayout refreshlayout) {
                //refreshlayout.finishLoadMore(2000/*,false*/);//传入false表示加载失败
                if(page == pages){
                    TipDialog.show("再无最新数据", WaitDialog.TYPE.ERROR);
                    return;
                }

                page = page + 1;
                getQuesList();
            }
        });

        lv_ques = (ListView) root.findViewById(R.id.lv_ques);
        quesAdapter = new QuesAdapter(context,quesList);
        lv_ques.setAdapter(quesAdapter);

        getQuesList();
        return root;
    }

    private void getQuesList(){
        showLoading("正在获取信息...");

        String reqData = ReqModelUtils.getPageReq(page,limit);

        reqUtils.jsonAsync(Constant.BASE_URL + "quesList",reqData,new OnRes() {
            @Override
            public void onSucess(Result result) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        dealWithQuestList(result);
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
                        refreshLayout.finishRefresh();
                        refreshLayout.finishLoadMore();
                        TipDialog.show(result.getMessage(), WaitDialog.TYPE.ERROR);

                    }
                });
            }
        });
    }

    private void dealWithQuestList(Result result){
        hideLoading();
        refreshLayout.finishRefresh();
        refreshLayout.finishLoadMore();

        if(result.getCode() != 20000){
            TipDialog.show(result.getMessage(), WaitDialog.TYPE.ERROR);
            return;
        }

        JSONObject jo = JSON.parseObject(result.getData());
        JSONObject pageParam = jo.getJSONObject("pageParam");

        pages = pageParam.getInteger("pages");

        JSONArray ja = pageParam.getJSONArray("records");

        for(int i = 0;i < ja.size();i++){
            String str = ja.getString(i);

            Ques ques = JSON.parseObject(str,Ques.class);
            quesList.add(ques);
        }

        quesAdapter.notifyDataSetChanged();
    }
}
