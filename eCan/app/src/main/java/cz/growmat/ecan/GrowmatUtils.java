package cz.growmat.ecan;

import android.content.SharedPreferences;
import android.util.Log;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import android.content.SharedPreferences;

import static java.security.AccessController.getContext;

/**
 * Created by pravoj01 on 28.9.2017.
 */

public class GrowmatUtils {

    final public static String TAG = GrowmatUtils.class.getName();
    public static final String PREFS_NAME = "GrowmatWeb";
    public static final String PREFS_NAME_CHANNEL_NO = "CHANNEL_NO";
    public static final String PREFS_NAME_TALKBACK_ID = "TALKBACK_ID";
    public static final String PREFS_NAME_TALKBACK_API_KEY = "TALKBACK_API_KEY";

    static public StringBuffer request(String urlString) {
        // TODO Auto-generated method stub


        StringBuffer buffer = new StringBuffer("");
        try{
            URL url = new URL(urlString);
            HttpURLConnection connection = (HttpURLConnection)url.openConnection();
            connection.setRequestProperty("User-Agent", "");
            //connection.setRequestMethod("POST");
            connection.setRequestMethod("GET");
            connection.setDoInput(true);
            connection.connect();

            InputStream inputStream = connection.getInputStream();

            BufferedReader rd = new BufferedReader(new InputStreamReader(inputStream));
            String line = "";
            while ((line = rd.readLine()) != null) {
                buffer.append(line);
            }
        }
        catch (IOException e) {
            // Writing exception to log
            e.printStackTrace();
        }
        return buffer;
    }

    //static String post ="http://api.thingspeak.com/talkbacks/" + 18221 + "/commands?api_key=" + ED57BAC7V0GLPWL2 + "&command_string=";//?api_key=ED57BAC7V0GLPWL2&command_string=" + s;//TURN_ON";
    static String post2;
    static void sendCommand(String s, String talkbackId, String talkbackApiKey) {
        //Log.d(TAG, "command=" + s);

        post2 = "http://api.thingspeak.com/talkbacks/" + talkbackId + "/commands?api_key=" + talkbackApiKey + "&command_string=";//?api_key=ED57BAC7V0GLPWL2&command_string=" + s;//TURN_ON";
        post2 = post2 + s;


        Log.d(TAG, post2);
        //Toast.makeText("NOT ALLOWED IN DEMO", Toast.LENGTH_SHORT).show();
        //Log.d(TAG, request2(get + s).toString());

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                StringBuffer buffer = new StringBuffer("");
                try {
                    URL url = new URL(post2);
                    HttpURLConnection connection = (HttpURLConnection)url.openConnection();
                    connection.setRequestProperty("User-Agent", "");
                    connection.setRequestMethod("POST");
                    //connection.setRequestMethod("GET");
                    //connection.setRequestProperty("api_key","ED57BAC7V0GLPWL2");
                    //connection.setRequestProperty("command[command_string]",command_string);
                    connection.setDoOutput(true);
                    connection.setDoInput(true);
                    connection.connect();


                    InputStream inputStream = connection.getInputStream();
                    BufferedReader rd = new BufferedReader(new InputStreamReader(inputStream));
                    String line = "";
                    while ((line = rd.readLine()) != null) {
                        buffer.append(line);
                    }
                    Log.d(TAG, buffer.toString());


                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        thread.start();
        try {
            //thread.join();
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    static StringBuffer buffer2 = new StringBuffer("");
    static String urlString2;
    static public StringBuffer request2(String urlString) {
        urlString2 = urlString;
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    //response = request("https://api.thingspeak.com/channels/222602/fields/1/last").toString();
                    buffer2 = request(urlString2);


                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        thread.start();
        try {
            thread.join();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return buffer2;
    }

    static String getOutputsState(String s) {
        int i = 0;
        try {
            i = Integer.parseInt(s);
        }
        catch  (Exception e) {
            e.printStackTrace();
        }

        String result = "";

        if((i & 2) > 0)
            result += "MANUAL ";
        else
            result += "AUTO ";
        if((i & 1) > 0)
            result += "OPEN ";
        else
            result += "CLOSE ";

        return  result;
    }

    static String getAlarmName(String s) {
        int i = 0;
        try {
            i = Integer.parseInt(s);
        }
        catch  (Exception e) {
            e.printStackTrace();
        }

        if(i < 0)
            return "OFFLINE";

        if(i == 0)
            return "OK";

        String result = "";
        if(i > 0) {
            result += "LEVEL ";
            if ((i & 1) >= 1)
                result += "MIN ";
            if ((i & 2) >= 1)
                result += "MAX ";
            if ((i & 4) >= 1)
                result += "EX ";
        }
        return result;
    }

    static boolean isOnline(String sCreated_at)  {
        if(sCreated_at == null)
            return false;

        //2017-09-28T07:19:43Z
        sCreated_at = sCreated_at.replace('T', ' ');
        sCreated_at = sCreated_at.replace('Z', ' ');
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss ");
        format.setTimeZone(TimeZone.getTimeZone("UTC"));



        try {
            Date d = format.parse(sCreated_at);

            //format.setTimeZone(TimeZone.getTimeZone("Europe/Prague"));

            String gmtTime = format.format(new Date());
            Date d2 = format.parse(gmtTime);

            Log.d(TAG, String.valueOf(d.getTime()));
            Log.d(TAG, String.valueOf(d2.getTime()));

            Log.d(TAG, d.toString());
            Log.d(TAG, d2.toString());
            //Log.d(TAG, String.valueOf(System.currentTimeMillis()));
            //Log.d(TAG, String.valueOf(Calendar.getInstance().getTime().getTime()));

            if(d2.getTime() - d.getTime() < 120000) {
                Log.d(TAG, "ONLINE");
                return true;
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;

    }


}
