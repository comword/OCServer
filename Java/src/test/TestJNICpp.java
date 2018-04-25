package test;

import org.junit.Test;
import static org.junit.Assert.assertEquals;

import java.util.HashMap;

import org.gtdev.oc.server.JNInterface;
import org.gtdev.oc.server.Log;

public class TestJNICpp {
    @Test
    public void Log_test() {
        String TAG = "TESTING";
        Log.d(TAG,"This is native debug log.");
        Log.i(TAG,"This is native information log.");
        Log.w(TAG,"This is native warning log.");
        Log.e(TAG,"This is native error log.");
    }
    @Test
    public void Config_test() {
    	HashMap<String,String> m = new HashMap<String,String>();
    	JNInterface.getAllPaths(m);
    	System.out.println(m);
    }
}
