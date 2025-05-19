package site.laoc.forum.view;

import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.Toast;

import androidx.fragment.app.Fragment;

import com.kongzue.dialogx.dialogs.MessageDialog;
import com.kongzue.dialogx.dialogs.WaitDialog;
import com.kongzue.dialogx.interfaces.OnDialogButtonClickListener;
import com.shashank.sony.fancytoastlib.FancyToast;


public class BaseFragment extends Fragment {

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
        FancyToast.makeText(getActivity(),msg,FancyToast.LENGTH_LONG,FancyToast.SUCCESS,true);
    }

    protected void showToastFailure(String msg) {
        FancyToast.makeText(getActivity(),msg,FancyToast.LENGTH_LONG,FancyToast.ERROR,true);
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


}
