package site.laoc.forum.utils;

import android.content.Context;

public class DipPx {
    public static int dip2px(Context context, float dpValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dpValue * scale + 0.5f);
    }


    public static int px2dip(Context context, float pxValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (pxValue / scale + 0.5f);
    }

    /**
     * px转换为sp
     * @param context  上下文环境
     * @param pxValue  需要转换的值
     * @return int
     * @createtime 2022/9/16 14:04
     **/
    public static int pxToSp(Context context, int pxValue){
        final float density = context.getResources().getDisplayMetrics().scaledDensity;
        return (int) (pxValue / density + 0.5f);
    }

    /**
     * sp转换为px
     * @param context  上下文环境
     * @param spValue  需要转换的值
     * @return int
     * @createtime 2022/9/16 14:05
     **/
    public static int spToPx(Context context, int spValue){
        final float density = context.getResources().getDisplayMetrics().scaledDensity;
        return (int) (spValue * density + 0.5f);
    }
}
