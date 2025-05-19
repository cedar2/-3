package site.laoc.forum.manager;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;
import android.os.Environment;

import com.blankj.utilcode.util.CrashUtils;
import com.blankj.utilcode.util.LogUtils;
import com.blankj.utilcode.util.Utils;
import com.kongzue.dialogx.DialogX;

import org.xutils.x;

import java.io.File;


public class MApplication extends Application {
    public static Context context;

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();

        context = getApplicationContext();

        x.Ext.init(this);

        //x.Ext.setDebug(BuildConfig.DEBUG); // 是否输出debug日志, 开启debug会影响性能.
        x.Ext.setDebug(true); // 是否输出debug日志, 开启debug会影响性能.
        Utils.init(this);
        initLog();
        initCrash();
        DialogX.init(this);

    }

    public void initLog() {
        final LogUtils.Config config = LogUtils.getConfig()
                .setLogSwitch(org.xutils.BuildConfig.DEBUG)
                .setConsoleSwitch(org.xutils.BuildConfig.DEBUG)
                .setGlobalTag(null)
                .setLogHeadSwitch(true)
                .setLog2FileSwitch(false)
                .setDir("")
                .setFilePrefix("")
                .setBorderSwitch(true)
                .setSingleTagSwitch(true)
                .setConsoleFilter(LogUtils.V)
                .setFileFilter(LogUtils.V)
                .setStackOffset(0);
        LogUtils.d(config.toString());


    }

    @SuppressLint("MissingPermission")
    private void initCrash() {
        CrashUtils.init(Environment.getExternalStorageDirectory().getAbsolutePath()
                        + File.separator + "IdCardVeriDemo" + File.separator + "crash",
                new CrashUtils.OnCrashListener() {
                    @Override
                    public void onCrash(CrashUtils.CrashInfo crashInfo) {

                    }

                });
    }



    @Override
    public void onTerminate() {
        // TODO Auto-generated method stub
        super.onTerminate();

    }

    @Override
    public void onLowMemory() {
        // TODO Auto-generated method stub
        super.onLowMemory();
    }
}
