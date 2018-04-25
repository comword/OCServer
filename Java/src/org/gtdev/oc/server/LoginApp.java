package org.gtdev.oc.server;

import org.gtdev.oc.server.channel.AppPort;
import org.gtdev.oc.server.protocol.*;

public class LoginApp extends AppPort.AppPortImpl {

	@Override
	public byte[] dump() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean transact(int code, ProtoInputStream data, ProtoOutputStream reply, int flags) {
		switch(code) {
			case 1: //Login
			case 2: //Logout
			case 3: 
		}
		return false;
	}

}
