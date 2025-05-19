package site.laoc.forum.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import org.w3c.dom.Text;

import java.util.ArrayList;
import java.util.List;

import site.laoc.forum.R;
import site.laoc.forum.entity.Ques;
import site.laoc.forum.utils.DateUtils;

public class QuesAdapter extends BaseAdapter {

    private Context context;
    private LayoutInflater inflater;
    private List<Ques> quesList = new ArrayList<>();

    public QuesAdapter(Context context,List<Ques> quesList){
        this.context = context;
        this.quesList = quesList;
        this.inflater = LayoutInflater.from(context);
    }

    @Override
    public int getCount() {
        return quesList == null ? 0 : quesList.size();
    }

    @Override
    public Object getItem(int i) {
        return quesList.get(i);
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

        holder.tv_ques_name.setText(quesList.get(i).getTitle());
        holder.tv_ques_time.setText(DateUtils.parseDateToStr(DateUtils.YYYY_MM_DD,quesList.get(i).getGmtCreate()));

        if(quesList.get(i).getStatus() == 0){
            holder.tv_dealed.setText("未解决");
        }else{
            holder.tv_dealed.setText("已解决");
        }

        holder.tv_see_nums.setText("" + quesList.get(i).getViews());

        return view;
    }

    class Holder{
        TextView tv_ques_name;
        TextView tv_dealed;
        TextView tv_see_nums;
        TextView tv_ques_time;
    }
}
