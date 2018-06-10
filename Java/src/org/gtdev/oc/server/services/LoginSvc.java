package org.gtdev.oc.server.services;

import org.gtdev.oc.server.Log;
import org.gtdev.oc.server.channel.AppPort;
import org.gtdev.oc.server.database.AccountMgrImpl;
import org.gtdev.oc.server.protocol.*;

public class LoginSvc extends AppPort.AppPortImpl {
	static final String TAG = "LoginSvc";

    public LoginSvc() {
    }

    @Override
    public boolean transact(ProtoInputStream data, ProtoOutputStream reply, int flags) {
        super.transact(data, reply, flags);
        AccountMgrImpl db = AccountMgrImpl.getInstance();
        long UID = 0;
        String cmd = "";
        //UID = data.read(UID, 0, true);
        cmd = data.read(cmd, 1, true);
        byte login_type = -1;
        login_type = data.read(login_type, PROTO_APP_FIRST_TAG, true);
        String pwdMD5 = "";
        int result = -1;
        if(login_type == 0) { // UID
            UID = data.read(UID, PROTO_APP_FIRST_TAG+1, true);
            pwdMD5 = data.readString(PROTO_APP_FIRST_TAG+2, true);
            result = db.AuthPassword(UID, pwdMD5);
            Log.d(TAG, "UID: "+UID);
            Log.d(TAG, "pwdMD5: "+pwdMD5);
            Log.d(TAG, "Result: "+result);
        } else if(login_type == 1) { // Email
            String Email = "";
            Email = data.readString(PROTO_APP_FIRST_TAG+1, true);
            pwdMD5 = data.readString(PROTO_APP_FIRST_TAG+2, true);
            UID = db.getUIDbyEmail(Email);
            result = db.AuthPassword(UID, pwdMD5);
            Log.d(TAG, "Email: "+Email);
            Log.d(TAG, "UID: "+UID);
            Log.d(TAG, "pwdMD5: "+pwdMD5);
            Log.d(TAG, "Result: "+result);
        }
        
        return true;
    }

}
