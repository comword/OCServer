package org.gtdev.oc.server.protocol;


public abstract class ProtoStruct implements java.io.Serializable {
    public static final byte BYTE = 0;
    public static final byte SHORT = 1;
    public static final byte INT = 2;
    public static final byte LONG = 3;
    public static final byte FLOAT = 4;
    public static final byte DOUBLE = 5;
    public static final byte STRING1 = 6;
    public static final byte STRING4 = 7;
    public static final byte MAP = 8;
    public static final byte LIST = 9;
    public static final byte STRUCT_BEGIN = 10;
    public static final byte STRUCT_END = 11;
    public static final byte ZERO_TAG = 12;
    public static final byte SIMPLE_LIST = 13;

    public static final int MAX_STRING_LENGTH = 100 * 1024 * 1024;
    private Object tag;

    public abstract void writeTo(ProtoOutputStream os);
    public abstract void readFrom(ProtoInputStream is);
    public void display(StringBuilder sb, int level) {};

    public byte[] toByteArray() {
        ProtoOutputStream os = new ProtoOutputStream();
        writeTo(os);
        return os.toByteArray();
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        display(sb, 0);
        return sb.toString();
    }

    public Object getTag() {
        return tag;
    }

    public void setTag(final Object tag) {
        this.tag = tag;
    }

}
