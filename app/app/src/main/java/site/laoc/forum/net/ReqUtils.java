package site.laoc.forum.net;

import com.alibaba.fastjson.JSON;
import com.blankj.utilcode.util.LogUtils;

import org.jetbrains.annotations.NotNull;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.FormBody;
import okhttp3.Headers;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import okio.BufferedSink;
import okio.Okio;
import okio.Sink;


public class ReqUtils {

    private static OkHttpClient okHttpClient;

    public ReqUtils(){
        if(okHttpClient == null){
            okHttpClient = new OkHttpClient();
            okHttpClient.newBuilder()
                    .connectTimeout(120, TimeUnit.SECONDS)
                    .readTimeout(200, TimeUnit.SECONDS)
                    .writeTimeout(120, TimeUnit.SECONDS).build();
        }
    }

    public void getAsync(String url, final OnRes onRes){
        Request request = new Request.Builder().url(url).get().build();
        Call call = okHttpClient.newCall(request);
        call.enqueue(new Callback() {
            @Override
            public void onFailure(@NotNull Call call, @NotNull IOException e) {
                Result result = new Result();
                result.setCode(999);
                result.setData(null);
                result.setMessage(e.getMessage());
                onRes.onFailure(result);
            }

            @Override
            public void onResponse(@NotNull Call call, @NotNull Response response) throws IOException {
                String bc = response.body().string();

                if(response.isSuccessful()){
                    LogUtils.e(bc);
                    Result result = JSON.parseObject(bc, Result.class);

                    onRes.onSucess(result);
                }else{
                    LogUtils.e(bc);
                    Result result = new Result();
                    result.setCode(999);
                    result.setData(null);
                    result.setMessage(bc);
                    onRes.onFailure(result);
                }
            }
        });

    }

    public Result getSybnc(String url){
        LogUtils.e("1111111111......");
        Request request = new Request.Builder().url(url).build();
        LogUtils.e("222222222......");
        try{
            Response response = okHttpClient.newCall(request).execute();
            String bc = response.body().string();
            LogUtils.e("3333333......");
            if(response.isSuccessful()){
                LogUtils.e(bc);
                return JSON.parseObject(bc, Result.class);
            }else{
                response.close();
                LogUtils.e(bc);
                Result result = new Result();
                result.setCode(999);
                result.setData(null);
                result.setMessage(bc);
                return result;
            }
        }catch (Exception e){
            e.printStackTrace();
            LogUtils.e("1111111111......" + e.getMessage());
            Result result = new Result();
            result.setCode(999);
            result.setData(null);
            result.setMessage(e.getMessage());
            return result;
        }


    }


    public void postKvRequestAsync(String url, final List<Req> reqs, List<Req> hds, final OnRes onRes){
        Headers.Builder builder_header = new Headers.Builder();
        //添加头
        if(hds != null){
            for(int i = 0;i < hds.size(); i++){
                builder_header.add(hds.get(i).getKey(),hds.get(i).getValue());
            }
        }

        Headers headers = builder_header.build();

        FormBody.Builder builder_body = new FormBody.Builder();
        for(int i = 0;i < reqs.size();i++) {
            builder_body.add(reqs.get(i).getKey(), reqs.get(i).getValue());
        }

        FormBody formBody = builder_body.build();

        Request request = new Request.Builder().url(url).headers(headers).post(formBody).build();

        Call call = okHttpClient.newCall(request);

        call.enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                Result result = new Result();
                result.setCode(999);
                result.setData(null);
                result.setMessage(e.getMessage());
                onRes.onFailure(result);
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                String bc = response.body().string();

                if(response.isSuccessful()){
                    LogUtils.e(bc);
                    Result result = JSON.parseObject(bc, Result.class);

                    onRes.onSucess(result);
                }else{
                    LogUtils.e(bc);
                    Result result = new Result();
                    result.setCode(999);
                    result.setData(null);
                    result.setMessage(bc);
                    onRes.onFailure(result);
                }
            }
        });
    }

    public void jsonAsync(String url, String json, final OnRes onRes){
        jsonAsync(url,json,"",onRes);
    }

    public void jsonAsync(String url, String json, String sid, final OnRes onRes){
        String resp = "";

        RequestBody requestBody = FormBody.create(MediaType.parse("application/json; charset=utf-8")
                ,json);

        Request request = new Request.Builder()
                .url(url)//请求的url
                .addHeader("sid", sid)
                .addHeader("Content-Type", "application/json")
                .post(requestBody)
                .build();

        Call call = okHttpClient.newCall(request);
        call.enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                Result result = new Result();
                result.setCode(999);
                result.setData(null);
                result.setMessage(e.getMessage());
                onRes.onFailure(result);
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                String bc = response.body().string();

                if(response.isSuccessful()){
                    LogUtils.e(bc);
                    Result result = JSON.parseObject(bc, Result.class);

                    onRes.onSucess(result);
                }else{
                    LogUtils.e(bc);
                    Result result = new Result();
                    result.setCode(999);
                    result.setData(null);
                    result.setMessage(bc);
                    onRes.onFailure(result);
                }
            }
        });
    }

    public void jsonImageCheckAsync(String url, String json, final OnImageRes onImageRes){
        String resp = "";

        RequestBody requestBody = FormBody.create(MediaType.parse("application/json; charset=utf-8")
                ,json);

        Request request = new Request.Builder()
                .url(url)//请求的url
                .addHeader("Content-Type", "application/json")
                .post(requestBody)
                .build();

        Call call = okHttpClient.newCall(request);
        call.enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                ImgResult result = new ImgResult();
                result.setCode(999);
                result.setData(null);
                result.setMessage(e.getMessage());
                onImageRes.onFailure(result);
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                String bc = response.body().string();

                if(response.isSuccessful()){
                    LogUtils.e(bc);
                    ImgResult result = JSON.parseObject(bc, ImgResult.class);

                    onImageRes.onSucess(result);
                }else{
                    LogUtils.e(bc);
                    ImgResult result = new ImgResult();
                    result.setCode(999);
                    result.setData(null);
                    result.setMessage(bc);
                    onImageRes.onFailure(result);
                }
            }
        });
    }

    public File downLoad(String url, File file){
        Request request = new Request.Builder().url(url).build();

        Call call = okHttpClient.newCall(request);

        try{
            Response response = call.execute();

            if(response.isSuccessful()){
                Sink sink = null;
                BufferedSink bufferedSink = null;
                sink = Okio.sink(file);
                bufferedSink = Okio.buffer(sink);
                bufferedSink.writeAll(response.body().source());
                bufferedSink.close();

                if(bufferedSink != null){
                    bufferedSink.close();
                }
            }else{
                return null;
            }
        }catch(Exception e){
            e.printStackTrace();
            return null;
        }

        return file;
    }
}
