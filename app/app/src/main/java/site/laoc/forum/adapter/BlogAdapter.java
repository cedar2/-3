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
import site.laoc.forum.entity.Ques;
import site.laoc.forum.utils.DateUtils;

public class BlogAdapter extends BaseAdapter {

    private Context context;
    private LayoutInflater inflater;
    private List<Blog> blogList = new ArrayList<>();

    public BlogAdapter(Context context, List<Blog> blogList){
        this.context = context;
        this.blogList = blogList;
        this.inflater = LayoutInflater.from(context);
    }

    @Override
    public int getCount() {
        return blogList == null ? 0 : blogList.size();
    }

    @Override
    public Object getItem(int i) {
        return blogList.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        Holder holder = null;

        if(view == null){
            view = inflater.inflate(R.layout.item_ques,null);
            holder = new Holder();
            holder.tv_ques_name = (TextView) view.findViewById(R.id.tv_ques_name);
            holder.tv_dealed = (TextView) view.findViewById(R.id.tv_dealed);
            holder.tv_see_nums = (TextView) view.findViewById(R.id.tv_see_nums);
            holder.tv_ques_time = (TextView) view.findViewById(R.id.tv_ques_time);

            view.setTag(holder);
        }else{
            holder = (Holder) view.getTag();
        }

        holder.tv_ques_name.setText(blogList.get(i).getTitle());
        holder.tv_ques_time.setText(DateUtils.parseDateToStr(DateUtils.YYYY_MM_DD,blogList.get(i).getGmtCreate()));

        holder.tv_see_nums.setText("" + blogList.get(i).getViews());

        holder.tv_dealed.setVisibility(View.GONE);

        return view;
    }

    class Holder{
        TextView tv_ques_name;
        TextView tv_dealed;
        TextView tv_see_nums;
        TextView tv_ques_time;
    }
}
