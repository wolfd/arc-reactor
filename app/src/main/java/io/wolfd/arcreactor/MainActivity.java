package io.wolfd.arcreactor;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.rarepebble.colorpicker.ColorPickerView;

import java.util.Arrays;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends AppCompatActivity implements BluetoothLeUart.Callback {
    private static final String TAG = "MainActivity";
    private static final int PERMISSIONS_REQUEST_ACCESS_COARSE_LOCATION = 900;
    @BindView(R.id.color_picker_view)
    ColorPickerView colorPickerView;

    private BluetoothLeUart uart;
    private long lastSendTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ButterKnife.bind(this);

        uart = new BluetoothLeUart(getApplicationContext());

        colorPickerView.showAlpha(false);

        colorPickerView.addColorObserver(observableColor -> {
            Log.d(TAG, "Color set to: " + observableColor.getColor());
            int rgb = observableColor.getColor();

            int r = Color.red(rgb);
            int g = Color.green(rgb);
            int b = Color.blue(rgb);

            sendSolidColor(r, g, b);
        });

        requestCoarseLocation();
    }

    private void requestCoarseLocation() {
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                    PERMISSIONS_REQUEST_ACCESS_COARSE_LOCATION);
        }

    }

    private void sendSolidColor(int r, int g, int b) {
        if (uart.isConnected() && System.currentTimeMillis() > lastSendTime + 5) {
            lastSendTime = System.currentTimeMillis();
            uart.send(new byte[]{0, (byte) r, (byte) g, (byte) b});
        }
    }

    private void sendSingleLed(int n, int r, int g, int b) {
        if (uart.isConnected() && System.currentTimeMillis() > lastSendTime + 5) {
            lastSendTime = System.currentTimeMillis();
            uart.send(new byte[]{1, (byte) r, (byte) g, (byte) b});
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "Scanning for devices...");
        uart.registerCallback(this);
        uart.connectFirstAvailable();
    }

    @Override
    protected void onStop() {
        super.onStop();

        uart.unregisterCallback(this);
        uart.disconnect();
    }

    @Override
    public void onConnected(BluetoothLeUart uart) {
        Log.i(TAG, "Connected to " + uart.getDeviceInfo());
    }

    @Override
    public void onConnectFailed(BluetoothLeUart uart) {
        Log.e(TAG, "Failed to connect to " + uart.getDeviceInfo());
    }

    @Override
    public void onDisconnected(BluetoothLeUart uart) {
        Log.i(TAG, "Disconnected from " + uart.getDeviceInfo());
    }

    @Override
    public void onReceive(BluetoothLeUart uart, BluetoothGattCharacteristic rx) {
        Log.i(TAG, "Received from " + uart.getDeviceInfo() + ": " + Arrays.toString(rx.getValue()));
    }

    @Override
    public void onDeviceFound(BluetoothDevice device) {
        Log.i(TAG, "Found device: " + device.getName() + " at " + device.getAddress());
    }

    @Override
    public void onDeviceInfoAvailable() {
        Log.i(TAG, "Device info available: " + uart.getDeviceInfo());
    }
}
