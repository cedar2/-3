package site.laoc.forum;

import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;

import com.hjm.bottomtabbar.BottomTabBar;

import org.xutils.view.annotation.ViewInject;
import org.xutils.x;

import site.laoc.forum.tab.BlogFragment;
import site.laoc.forum.tab.MeFragment;
import site.laoc.forum.tab.NoticeFragment;
import site.laoc.forum.tab.QuesFragment;
import site.laoc.forum.tab.ResFragment;
import site.laoc.forum.utils.DipPx;

public class HomeActivity extends BaseActivity{

    private Context context;

    @ViewInject(R.id.bottomTabBar)
    private BottomTabBar bottomTabBar;

    @Override
    public void initWidget(Bundle savedInstanceState) {
        setContentView(R.layout.activity_home);
        x.view().inject(HomeActivity.this);

        this.context = HomeActivity.this;

        initView();
    }

    private void initView(){
        int sesColor = Color.parseColor("#FF484A");
        int unSesColor = Color.parseColor("#CCCCCC");

        int szpx = DipPx.dip2px(context,10);

        bottomTabBar.init(getSupportFragmentManager())
                .setImgSize(szpx, szpx)
                .setFontSize(12)
                .setTabPadding(DipPx.dip2px(context,4), 4, DipPx.dip2px(context,4))
                .setChangeColor(sesColor, unSesColor)
                .addTabItem("问答", R.drawable.tabques, QuesFragment.class)
                .addTabItem("博客", R.drawable.tabblog, BlogFragment.class)
                .addTabItem("资源", R.drawable.tabziyuan, ResFragment.class)
                .addTabItem("公告", R.drawable.tabgonggao, NoticeFragment.class)
                .addTabItem("我的", R.drawable.tabme, MeFragment.class)
                .isShowDivider(true)
                .setOnTabChangeListener(new BottomTabBar.OnTabChangeListener() {
                    @Override
                    public void onTabChange(int i, String s, View view) {

                    }
                });

    }

    @Override
    public void widgetClick(View v) {

    }

    @Override
    void afterRequestPermission(int requestCode, boolean isAllGranted) {

    }
}
