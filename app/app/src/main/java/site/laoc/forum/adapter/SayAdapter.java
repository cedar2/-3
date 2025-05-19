package site.laoc.forum.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

import site.laoc.forum.R;
import site.laoc.forum.entity.Blog;
import site.laoc.forum.entity.Say;
import site.laoc.forum.utils.DateUtils;

public class SayAdapter extends BaseAdapter {

    private Context context;
    private LayoutInflater inflater;
    private List<Say> sayList = new ArrayList<>();

    public SayAdapter(Context context, List<Say> sayList){
        this.context = context;
        this.sayList = sayList;
        this.inflater = LayoutInflater.from(context);
    }

    public int getCount() {
        return sayList == null ? 0 : sayList.size();
    }

    @Override
    public Object getItem(int i) {
        return sayList.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        Holder holder = null;

        if(view == null){
            view = inflater.inflate(R.layout.item_say,null);
            holder = new Holder();
            holder.tv_say_name = (TextView) view.findViewById(R.id.tv_say_name);
            holder.tv_say_time = (TextView) view.findViewById(R.id.tv_say_time);
            holder.tv_say_con = (TextView) view.findViewById(R.id.tv_say_con);

            view.setTag(holder);
        }else{
            holder = (Holder) view.getTag();
        }

        holder.tv_say_name.setText(sayList.get(i).getTitle());
        holder.tv_say_time.setText(DateUtils.parseDateToStr(DateUtils.YYYY_MM_DD,sayList.get(i).getGmtCreate()));

        holder.tv_say_con.setText(sayList.get(i).getContent());

        return view;
    }

    class Holder{
        TextView tv_say_name;
        TextView tv_say_time;
        TextView tv_say_con;
    }
}
