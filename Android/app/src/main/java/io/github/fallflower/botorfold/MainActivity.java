package io.github.fallflower.botorfold;

import android.content.pm.ActivityInfo;

/** Holdem-owned Android entry point; native/runtime behavior is supplied by EUI-NEO. */
public class MainActivity extends com.sudoevolve.euineo.MainActivity {
    public void requestGamePresentation(boolean game) {
        final int requestedOrientation = game
                ? ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
                : ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT;
        runOnUiThread(() -> {
            if (getRequestedOrientation() != requestedOrientation) {
                setRequestedOrientation(requestedOrientation);
            }
        });
    }
}
