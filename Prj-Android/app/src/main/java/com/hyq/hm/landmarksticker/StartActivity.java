package com.hyq.hm.landmarksticker;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class StartActivity extends AppCompatActivity implements View.OnClickListener{



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_start);
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.button) {
            Intent intent = new Intent(this,MainActivity.class);
            startActivity(intent);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (MainActivity.mMultiTrack106 != null) {
            MainActivity.mMultiTrack106.release();
            MainActivity.mMultiTrack106 = null;
        }
    }
}
