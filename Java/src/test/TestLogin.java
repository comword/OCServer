package test;

import org.gtdev.oc.server.LoginApp;
import org.gtdev.oc.server.channel.AppPort;
import org.gtdev.oc.server.protocol.*;
import org.junit.Test;

public class TestLogin {
	
	LoginApp la = new LoginApp();
	static ProtoInputStream Logindata;
	
	static {
		ProtoOutputStream w = new ProtoOutputStream();
		w.write((long)1, AppPort.PROTO_APP_FIRST_TAG);
		w.write("abcd", AppPort.PROTO_APP_FIRST_TAG + 1);
		Logindata = new ProtoInputStream(w.getByteBuffer().flip());
	}
	
	@Test
    public void doLogin() {
		int code = LoginApp.CODE_LOGIN;
		ProtoOutputStream reply = new ProtoOutputStream();
		boolean res = la.transact(code, Logindata, reply, 0);
		
    }
}
