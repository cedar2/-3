package site.laoc.forum.utils;

import android.content.Context;
import android.content.SharedPreferences;

public class DataUtils {

    private static final String PREF_FORUM_MACH = "PREF_FORUM";

    private static final String FORUM_NAME = "FORUM_NAME";

    private static final String FORUM_UID = "FORUM_UID";


    public static void setUname(Context context, String uname){
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREF_FORUM_MACH,Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(FORUM_NAME, uname);
        editor.commit();
    }

    public static String getUname(Context context){
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREF_FORUM_MACH,Context.MODE_PRIVATE);

        String name = sharedPreferences.getString(FORUM_NAME,"");

        return name;
    }

    public static void setUid(Context context, String uid){
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREF_FORUM_MACH,Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(FORUM_UID, uid);
        editor.commit();
    }

    public static String getUid(Context context){
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREF_FORUM_MACH,Context.MODE_PRIVATE);

        String name = sharedPreferences.getString(FORUM_UID,"");

        return name;
    }



}