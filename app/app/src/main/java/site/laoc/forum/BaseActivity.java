package site.laoc.forum;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import com.blankj.utilcode.util.LogUtils;
import com.kongzue.dialogx.dialogs.MessageDialog;
import com.kongzue.dialogx.dialogs.WaitDialog;
import com.kongzue.dialogx.interfaces.OnDialogButtonClickListener;
import com.shashank.sony.fancytoastlib.FancyToast;

import org.xutils.common.util.LogUtil;

import site.laoc.forum.manager.AppManager;


public abstract class BaseActivity extends AppCompatActivity implements View.OnClickListener {

    private static final int ACTIVITY_RESUME = 0;
    private static final int ACTIVITY_STOP = 1;
    private static final int ACTIVITY_PAUSE = 2;
    private static final int ACTIVITY_DESTROY = 3;

    public int activityState;

    // 是否允许全屏
    private boolean mAllowFullScreen = true;

    public abstract void initWidget(Bundle savedInstanceState);

    public abstract void widgetClick(View v);

    public void setAllowFullScreen(boolean allowFullScreen) {
        this.mAllowFullScreen = allowFullScreen;
    }

    protected void showLoading() {
        WaitDialog.show("加载中...");
    }

    protected void showLoading(String msg) {
        WaitDialog.show(msg);
    }

    protected void hideLoading() {
        WaitDialog.dismiss();
    }

    protected void showToastSuccess(String msg) {
        FancyToast.makeText(this,msg,FancyToast.LENGTH_LONG,FancyToast.SUCCESS,true);
    }

    protected void showToastFailure(String msg) {
        FancyToast.makeText(this,msg,FancyToast.LENGTH_LONG,FancyToast.ERROR,true);
    }

    protected void showConfirmDia(String msg, View.OnClickListener click) {
        MessageDialog.show("提示", msg, "确定").setOkButton(new OnDialogButtonClickListener<MessageDialog>() {
            @Override
            public boolean onClick(MessageDialog baseDialog, View v) {
                click.onClick(v);
                return false;
            }
        });
    }


    protected void showProgressDia(String msg) {
        WaitDialog.show("正在加载...", 0f);
    }

    public void setProgress(float pro){
        WaitDialog.show("正在加载...", pro);
    }


    protected void dismissConfirmDia(){
        WaitDialog.dismiss();
    }

    /**
     * 权限检查
     *
     * @param neededPermissions 需要的权限
     * @return 是否全部被允许
     */
    protected boolean checkPermissions(String[] neededPermissions) {
        if (neededPermissions == null || neededPermissions.length == 0) {
            return true;
        }
        boolean allGranted = true;
        for (String neededPermission : neededPermissions) {
            allGranted &= ContextCompat.checkSelfPermission(this, neededPermission) == PackageManager.PERMISSION_GRANTED;
        }
        return allGranted;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        boolean isAllGranted = true;
        for (int grantResult : grantResults) {
            isAllGranted &= (grantResult == PackageManager.PERMISSION_GRANTED);
        }
        afterRequestPermission(requestCode, isAllGranted);
    }

    public void showToast(String message) {
        Toast.makeText(this.getApplication(), message, Toast.LENGTH_SHORT).show();
    }

    /**
     * 请求权限的回调
     *
     * @param requestCode  请求码
     * @param isAllGranted 是否全部被同意
     */
    abstract void afterRequestPermission(int requestCode, boolean isAllGranted);

    /* 组件单击 */
    @Override
    public void onClick(View v) {
        widgetClick(v);
    }

    private SoundPool soundPool = null;
    private int mSoundId = -1;

    public void playSound(Context context,int soundRes){
        if(soundPool == null){
            return;
        }

        soundPool.autoPause();
        soundPool.unload(mSoundId);
        mSoundId = -1;

        //加载声音资源文件（R.raw.message：音频文件）
        mSoundId = soundPool.load(context, soundRes, 1);
        //音频加载完成监听
        soundPool.setOnLoadCompleteListener(new   SoundPool.OnLoadCompleteListener() {
            @Override
            public void onLoadComplete(SoundPool soundPool, int i, int i2) {
                LogUtils.e("test---加载完成");
                //开始播放
                soundPool.play(mSoundId,  //声音id
                        1, //左声道
                        1, //右声道
                        1, //优先级
                        0, // 0表示不循环，-1表示循环播放
                        1);//播放比率，0.5~2，一般为1
                soundPool.unload(mSoundId);
            }
        });
    }


    /***************************************************************************
     *
     * 打印Activity生命周期
     *
     ***************************************************************************/

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LogUtil.d(this.getClass().getName() + "---------onCreate");

        // 竖屏锁定
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        if (mAllowFullScreen) {
            requestWindowFeature(Window.FEATURE_NO_TITLE);

            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }

        AppManager.getAppManager().addActivity(this);

        SoundPool.Builder builder = new SoundPool.Builder();
        //传入单次最多播放音频流数量,
        builder.setMaxStreams(1);
        //AudioAttributes封装音频属性的类
        AudioAttributes.Builder attrBuilder = new AudioAttributes.Builder();
        //设置音频流的媒体类型，
        //因为我后台有广告使用media播放，所以这类不用STREAM_MUSIC避免冲突
        attrBuilder.setLegacyStreamType(AudioManager.STREAM_ALARM);
        //设置AudioAttributes到构造器上
        builder.setAudioAttributes(attrBuilder.build());
        //构造出相应的对象
        soundPool = builder.build();

        initWidget(savedInstanceState);
    }

    @Override
    protected void onStart() {
        super.onStart();
        LogUtil.d(this.getClass().getName()+ "---------onStart ");
    }

    @Override
    protected void onResume() {
        super.onResume();
        activityState = ACTIVITY_RESUME;
        LogUtil.e(this.getClass().getName()+ "---------onResume ");
    }

    @Override
    protected void onStop() {
        super.onStop();
        activityState = ACTIVITY_STOP;
        LogUtil.d(this.getClass().getName()+ "---------onStop ");
    }

    @Override
    protected void onPause() {
        super.onPause();
        activityState = ACTIVITY_PAUSE;
        LogUtil.d(this.getClass().getName()+ "---------onPause ");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        LogUtil.d(this.getClass().getName()+ "---------onRestart ");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        activityState = ACTIVITY_DESTROY;

        if (soundPool != null) {
            soundPool.autoPause();
            soundPool.unload(mSoundId);
            mSoundId = -1;
            soundPool.release();
            soundPool = null;
        }

        LogUtil.d(this.getClass().getName()+ "---------onDestroy ");
        AppManager.getAppManager().finishActivity(this);
    }
}
