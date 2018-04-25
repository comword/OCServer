package org.gtdev.oc.server;

import java.io.File;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.HashMap;

public class JNInterface {
    static {
        try {
            addLibraryPath(System.getProperty("user.dir")+File.separatorChar
                           +".."+File.separatorChar+"Core");
            System.loadLibrary("OCSCore");
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(-1);
        }
    }

    /**
    * Adds the specified path to the java library path
    *
    * @param pathToAdd the path to add
    * @throws Exception
    */
    public static void addLibraryPath(String pathToAdd) throws Exception {
        final Field usrPathsField = ClassLoader.class.getDeclaredField("usr_paths");
        usrPathsField.setAccessible(true);

        //get array of paths
        final String[] paths = (String[])usrPathsField.get(null);

        //check if the path to add is already present
        for(String path : paths) {
            if(path.equals(pathToAdd)) {
                return;
            }
        }

        //add the new path
        final String[] newPaths = Arrays.copyOf(paths, paths.length + 1);
        newPaths[newPaths.length-1] = pathToAdd;
        usrPathsField.set(null, newPaths);
    }

    public static native int println(int priority, String tag, String msg);
    public static native boolean loadConf(HashMap<String,String> m);
    public static native String getPath(String key);
    public static native void getAllPaths(HashMap<String,String> m);
    public static native String displayProto(byte[] d);
    public static native boolean doTransact(int code, byte[] data, byte[] reply, int flags);
}
