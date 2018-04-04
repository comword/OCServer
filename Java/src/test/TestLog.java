package test;

import org.junit.Test;
import static org.junit.Assert.assertEquals;

import org.gtdev.oc.server.Log;

public class TestLog {
    @Test
    public void Logout_test() {
        String TAG = "TESTING";
        Log.d(TAG,"This is native debug log.");
        Log.i(TAG,"This is native information log.");
        Log.w(TAG,"This is native warning log.");
        Log.e(TAG,"This is native error log.");
    }
}
