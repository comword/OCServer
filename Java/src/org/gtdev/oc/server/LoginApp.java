package org.gtdev.oc.server;

import org.gtdev.oc.server.channel.AppPort;
import org.gtdev.oc.server.database.AccountMgrImpl;
import org.gtdev.oc.server.protocol.*;

public class LoginApp extends AppPort.AppPortImpl {
	
	public static final int CODE_LOGIN = 1;
	public static final int CODE_LOGOUT = 2;
	public static final int CODE_REGISTER = 3;
	public static final int CODE_CLHELLO = 4;

    @Override
    public byte[] dump() {
        return null;
    }

    @Override
    public boolean transact(int code, ProtoInputStream data, ProtoOutputStream reply, int flags) {
        if(super.transact(code, data, reply, flags))
            return true;
        switch(code) {
        case CODE_LOGIN: //Login with ID
            long UID = 0;
            UID = data.read(UID, PROTO_APP_FIRST_TAG, true);
            String pwdMD5 = "";
            pwdMD5 = data.read(pwdMD5, PROTO_APP_FIRST_TAG + 1, true);
            int res = -1;
            res = AccountMgrImpl.AuthPassword(UID, pwdMD5);
            String token = "";
            reply.write(res, PROTO_APP_FIRST_TAG);
            reply.write(token,PROTO_APP_FIRST_TAG + 1);
            return true;
        case CODE_LOGOUT: //Logout

            return true;
        case CODE_REGISTER: //Register

            return true;
        case CODE_CLHELLO:
        	
        }
        return false;
    }

}
