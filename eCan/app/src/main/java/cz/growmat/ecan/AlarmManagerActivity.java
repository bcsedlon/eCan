package cz.growmat.ecan;

import cz.growmat.ecan.R;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class AlarmManagerActivity extends Activity {

	private AlarmManagerBroadcastReceiver alarm;
	private final Handler handler = new Handler();
	final public static String TAG = AlarmManagerBroadcastReceiver.class.getName();

	public static final String PREFS_NAME = "GrowmatWeb";
	public static final String PREFS_NAME_CHANNEL_NO = "CHANNEL_NO";
	public static final String PREFS_NAME_TALKBACK_ID = "TALKBACK_ID";
	public static final String PREFS_NAME_TALKBACK_API_KEY = "TALKBACK_API_KEY";
	public static final String PREFS_NAME_ALARM_UP = "ALARM_UP";

	ToggleButton tButton;
	TextView twAlarm, twStatus, twTemperature;

	String sAlarm, sCreated_at, sLight, sFan, sTemperature = "";

    String sLcommand = "";
	String sFcommand = "";


	private void autoRefresh() {
		handler.postDelayed(new Runnable() {
			@Override
			public void run() {
				refresh(); // this is where you put your refresh code
				autoRefresh();
			}
		}, 60000);
	}

	@Override
	public void onResume() {
		super.onResume();

		SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
		String channelNo = settings.getString(PREFS_NAME_CHANNEL_NO, "");
		String talkbackId = settings.getString(PREFS_NAME_TALKBACK_ID, "");
		String talkbackApiKey = settings.getString(PREFS_NAME_TALKBACK_API_KEY, "");
		boolean alarmUp = settings.getBoolean(PREFS_NAME_ALARM_UP, false);
		((EditText)findViewById(R.id.editText2)).setText(channelNo);
		((EditText)findViewById(R.id.editText3)).setText(talkbackId);
		((EditText)findViewById(R.id.editText4)).setText(talkbackApiKey);


		Context context = this.getApplicationContext();
		NotificationManager mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.cancel(9011);


		//boolean alarmUp = (PendingIntent.getBroadcast(context, 0, new Intent(context, AlarmManagerBroadcastReceiver.class), PendingIntent.FLAG_NO_CREATE) != null);
		tButton.setChecked(alarmUp);

		refresh();
	}

	void refresh() {
		SharedPreferences settings = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
		String channelNo = settings.getString(PREFS_NAME_CHANNEL_NO, "");
		String talkbackId = settings.getString(PREFS_NAME_TALKBACK_ID, "");
		String talkbackApiKey = settings.getString(PREFS_NAME_TALKBACK_API_KEY, "");


		String response = GrowmatUtils.request2("https://api.thingspeak.com/channels/" + channelNo +"/feeds.json?api_key=" + talkbackApiKey + "&results=1").toString();
		Log.d(TAG, "response: " + response);
		try {
			JSONObject jObject = new JSONObject(response);
			JSONArray jFeedsArray = jObject.getJSONArray("feeds");

			JSONObject oneObject = jFeedsArray.getJSONObject(0);

			sAlarm = oneObject.getString("field1");
			sCreated_at = oneObject.getString("created_at");

			sTemperature = oneObject.getString("field2");
			sLight = oneObject.getString("field3");
			sFan = oneObject.getString("field4");

		} catch (JSONException e) {
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}

		if(!GrowmatUtils.isOnline(sCreated_at)) {
			sAlarm = "-1";
		}

		twAlarm.setText(GrowmatUtils.getAlarmName(sAlarm));
		twTemperature.setText(sTemperature);
		twStatus.setText("VALVE A " + GrowmatUtils.getOutputsState(sLight) + "\nVALVE B " + GrowmatUtils.getOutputsState(sFan));
	}


	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_alarm_manager);
        alarm = new AlarmManagerBroadcastReceiver();

		tButton = (ToggleButton) findViewById(R.id.toggleButton);
		twAlarm = (TextView) findViewById(R.id.textView6);
		twTemperature = (TextView) findViewById(R.id.textView7);
		twStatus = (TextView) findViewById(R.id.textView8);
/*
		SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
		String talkbackId = settings.getString(PREFS_NAME_CHANNEL_NO, "");
		String talkbackId = settings.getString(PREFS_NAME_TALKBACK_ID, "");
		String talkbackApiKey = settings.getString(PREFS_NAME_TALKBACK_API_KEY, "");
		((EditText)findViewById(R.id.editText2)).setText(channelNo);
		((EditText)findViewById(R.id.editText3)).setText(talkbackId);
		((EditText)findViewById(R.id.editText4)).setText(talkbackApiKey);*/
		autoRefresh();
    }

	public void OnClick(View view) {
		String channelNo = ((EditText)findViewById(R.id.editText2)).getText().toString();
		String talkbackId = ((EditText)findViewById(R.id.editText3)).getText().toString();
		String talkbackApiKey = ((EditText)findViewById(R.id.editText4)).getText().toString();
		SharedPreferences.Editor editor = getSharedPreferences(PREFS_NAME, MODE_PRIVATE).edit();
        switch (view.getId()) {



            case R.id.toggleButton: {
                if (tButton.isChecked())
                    startRepeatingTimer(view);
                else
                    cancelRepeatingTimer(view);
            	}

				editor.putBoolean(PREFS_NAME_ALARM_UP, tButton.isChecked());
				editor.apply();
            	break;
            case R.id.buttonLoff:
                GrowmatUtils.sendCommand("A0", talkbackId, talkbackApiKey);
				break;
            case R.id.buttonLon:
                GrowmatUtils.sendCommand("A1", talkbackId, talkbackApiKey);
				break;
            case R.id.buttonLauto:
                GrowmatUtils.sendCommand("AA", talkbackId, talkbackApiKey);
				break;
            case R.id.buttonFoff:
                GrowmatUtils.sendCommand("B0", talkbackId, talkbackApiKey);
				break;
            case R.id.buttonFon:
                GrowmatUtils.sendCommand("B1", talkbackId, talkbackApiKey);
				break;
            case R.id.buttonFauto:
                GrowmatUtils.sendCommand("BA", talkbackId, talkbackApiKey);
				break;
			case R.id.buttonSave:
				//SharedPreferences.Editor editor = getSharedPreferences(PREFS_NAME, MODE_PRIVATE).edit();
				editor.putString(PREFS_NAME_CHANNEL_NO, channelNo);
				editor.putString(PREFS_NAME_TALKBACK_ID, talkbackId);
				editor.putString(PREFS_NAME_TALKBACK_API_KEY, talkbackApiKey);
				editor.apply();
				break;
        }
	}

	public void OnRefresh(View view) {
		refresh();
	}

    @Override
	protected void onStart() {
		super.onStart();
	}

    public void startRepeatingTimer(View view) {
    	Context context = this.getApplicationContext();
    	if(alarm != null){
    		alarm.SetAlarm(context);
    	}else{
    		Toast.makeText(context, "Alarm is null", Toast.LENGTH_SHORT).show();
    	}
    }
    
    public void cancelRepeatingTimer(View view){
    	Context context = this.getApplicationContext();
    	if(alarm != null){
    		alarm.CancelAlarm(context);
    	}else{
    		Toast.makeText(context, "Alarm is null", Toast.LENGTH_SHORT).show();
    	}
    }
    
    public void onetimeTimer(View view){
    	Context context = this.getApplicationContext();
    	if(alarm != null){
    		alarm.setOnetimeTimer(context);
    	}else{
    		Toast.makeText(context, "Alarm is null", Toast.LENGTH_SHORT).show();
    	}
    }
/*
	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_widget_alarm_manager, menu);
        return true;
    }*/
}
