package org.gtdev.oc.server.channel;

public abstract class BaseRemoteCommand {

    private String cmd;

    public BaseRemoteCommand(String cmd) {
        this.cmd = cmd;
    }

    public boolean isSynchronized() {
        return true;
    }

    public String getCmd() {
        return this.cmd;
    }

    public String toString() {
        return "[cmd:" + this.cmd + ", sync:" + isSynchronized() + "]";
    }
}