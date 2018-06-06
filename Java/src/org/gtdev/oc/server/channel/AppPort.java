package org.gtdev.oc.server.channel;

import java.nio.ByteBuffer;

import org.gtdev.oc.server.protocol.*;

public interface AppPort {
    int PROTO_APP_FIRST_TAG = 4;

    public abstract class AppPortImpl implements AppPort {

        public boolean transact(ProtoInputStream data, ProtoOutputStream reply, int flags) {
            return false;
        }

        //This method is CPP entry point.
        @SuppressWarnings("unused")
        private boolean execTransact(ByteBuffer dataObj, ByteBuffer replyObj, int flags) {
            ProtoInputStream is = new ProtoInputStream(dataObj);
            ProtoOutputStream os = new ProtoOutputStream(replyObj);
            boolean res = transact(is,os,flags);
            return res;
        }
    }
    boolean transact(ProtoInputStream data, ProtoOutputStream reply, int flags);
}
