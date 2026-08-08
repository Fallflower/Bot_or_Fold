package io.github.fallflower.botorfold;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

/** Holdem-owned Android entry point; native/runtime behavior is supplied by EUI-NEO. */
public class MainActivity extends com.sudoevolve.euineo.MainActivity {
    private volatile boolean gamePresentation = false;

    @Override
    public void setOrientationBis(int width, int height, boolean resizable, String hint) {
        // SDL marks its single Android window as resizable and may overwrite our
        // page-specific policy. Keep this app as the sole owner: the setup page
        // follows the user's device orientation while the table stays landscape.
        runOnUiThread(this::applyPresentationOrientation);
    }

    public void requestGamePresentation(boolean game) {
        gamePresentation = game;
        runOnUiThread(() -> {
            hideSoftKeyboard();
            applyPresentationOrientation();
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        applyPresentationOrientation();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // SDL handles the resized surface in super. Re-assert our page orientation
        // afterwards so a delayed sensor/configuration callback cannot undo it.
        getWindow().getDecorView().post(this::applyPresentationOrientation);
    }

    private void applyPresentationOrientation() {
        final int requestedOrientation = gamePresentation
                ? ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
                : ActivityInfo.SCREEN_ORIENTATION_FULL_USER;
        setRequestedOrientation(requestedOrientation);
    }

    private void hideSoftKeyboard() {
        final View decorView = getWindow().getDecorView();
        decorView.clearFocus();
        final InputMethodManager inputMethodManager =
                (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        if (inputMethodManager != null) {
            inputMethodManager.hideSoftInputFromWindow(decorView.getWindowToken(), 0);
        }
    }
}
