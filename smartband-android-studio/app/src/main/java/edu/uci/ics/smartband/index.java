package edu.uci.ics.smartband;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class index extends ActionBarActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.index);
    }


    public void goToStep(View view) {
        Intent goToIntent = new Intent(this, step.class);
        startActivity(goToIntent);
    }

    public void goToTemperature(View view) {
        Intent goToIntent = new Intent(this, temperature.class);
        startActivity(goToIntent);
    }
    public void goToCarbon(View view) {
        Intent goToIntent = new Intent(this, carbon.class);
        startActivity(goToIntent);
    }
    public void goToHeartbeat(View view) {
        Intent goToIntent = new Intent(this, heartbeat.class);
        startActivity(goToIntent);
    }
    public void goToUV(View view) {
        Intent goToIntent = new Intent(this, UV.class);
        startActivity(goToIntent);
    }

}
