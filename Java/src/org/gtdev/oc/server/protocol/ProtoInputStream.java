package org.gtdev.oc.server.protocol;

import java.io.UnsupportedEncodingException;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public final class ProtoInputStream {

    public class DecodeException extends RuntimeException {

        public DecodeException(String string) {
            super(string);
        }
    }

    private ByteBuffer bs;	// Buffer

    public static class HeadData {

        public byte type;
        public int tag;

        public void clear() {
            type = 0;
            tag = 0;
        }
    }

    public ProtoInputStream() {

    }

    public ProtoInputStream(ByteBuffer bs) {
        this.bs = bs;
    }

    public ProtoInputStream(byte[] bs) {
        this.bs = ByteBuffer.wrap(bs);
    }

    public ProtoInputStream(byte[] bs, int pos) {
        this.bs = ByteBuffer.wrap(bs);
        this.bs.position(pos);
    }

    public void wrap(byte[] bs) {
        this.bs = ByteBuffer.wrap(bs);
    }

    /**
     * Read data header
     * @param hd Header data to be read.
     * @param bb Data buffer
     * @return the number of bytes read from header.
     */
    public static int readHead(HeadData hd, ByteBuffer bb) {
        byte b = bb.get();
        hd.type = (byte) (b & 15);
        hd.tag = ((b & (15 << 4)) >> 4);
        if (hd.tag == 15) {
            hd.tag = (bb.get()&0x00ff);
            return 2;
        }
        return 1;
    }

    public void readHead(HeadData hd) {
        readHead(hd, bs);
    }

    //! This function read the header but not move the read position.
    public int peakHead(HeadData hd) {
        return readHead(hd, bs.duplicate());
    }

    private void skip(int len) {
        bs.position(bs.position() + len);
    }

    //! Seek to the position before a tag.
    public boolean skipToTag(int tag) {
        try {
            HeadData hd = new HeadData();
            while (true) {
                int len = peakHead(hd);
                if (hd.type == ProtoStruct.STRUCT_END) {
                    return false;
                }
                if (tag <= hd.tag)
                    return tag == hd.tag;
                skip(len);
                skipField(hd.type);
            }
        } catch (DecodeException e) {
        } catch (BufferUnderflowException e) {
        }
        return false;
    }

    public void skipToStructEnd() {
        HeadData hd = new HeadData();
        do {
            readHead(hd);
            skipField(hd.type);
        } while (hd.type != ProtoStruct.STRUCT_END);
    }

    public void skipField() {
        HeadData hd = new HeadData();
        readHead(hd);
        skipField(hd.type);
    }

    private void skipField(byte type) {
        switch (type) {
        case ProtoStruct.BYTE:
            skip(1);
            break;
        case ProtoStruct.SHORT:
            skip(2);
            break;
        case ProtoStruct.INT:
            skip(4);
            break;
        case ProtoStruct.LONG:
            skip(8);
            break;
        case ProtoStruct.FLOAT:
            skip(4);
            break;
        case ProtoStruct.DOUBLE:
            skip(8);
            break;
        case ProtoStruct.STRING1: {
            int len = bs.get();
            if (len < 0)
                len += 256;
            skip(len);
            break;
        }
        case ProtoStruct.STRING4: {
            skip(bs.getInt());
            break;
        }
        case ProtoStruct.MAP: {
            int size = read(0, 0, true);
            for (int i = 0; i < size * 2; ++i)
                skipField();
            break;
        }
        case ProtoStruct.LIST: {
            int size = read(0, 0, true);
            for (int i = 0; i < size; ++i)
                skipField();
            break;
        }
        case ProtoStruct.SIMPLE_LIST: {
            HeadData hd = new HeadData();
            readHead(hd);
            if(hd.type != ProtoStruct.BYTE) {
                throw new DecodeException("skipField with invalid type, type value: " + type + ", " + hd.type);
            }
            int size = read(0, 0, true);
            skip(size);
            break;
        }
        case ProtoStruct.STRUCT_BEGIN:
            skipToStructEnd();
            break;
        case ProtoStruct.STRUCT_END:
        case ProtoStruct.ZERO_TAG:
            break;
        default:
            throw new DecodeException("invalid type.");
        }
    }

    public boolean read(boolean b, int tag, boolean isRequire) {
        byte c = read((byte) 0x0, tag, isRequire);
        return c != 0;
    }

    public byte read(byte c, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.ZERO_TAG:
                c = 0x0;
                break;
            case ProtoStruct.BYTE:
                c = bs.get();
                break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return c;
    }

    public short read(short n, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.ZERO_TAG:
                n = 0;
                break;
            case ProtoStruct.BYTE:
                n = (short) bs.get();
                break;
            case ProtoStruct.SHORT:
                n = bs.getShort();
                break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return n;
    }

    public int read(int n, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.ZERO_TAG:
                n = 0;
                break;
            case ProtoStruct.BYTE:
                n = bs.get();
                break;
            case ProtoStruct.SHORT:
                n = bs.getShort();
                break;
            case ProtoStruct.INT:
                n = bs.getInt();
                break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return n;
    }

    public long read(long n, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.ZERO_TAG:
                n = 0;
                break;
            case ProtoStruct.BYTE:
                n = bs.get();
                break;
            case ProtoStruct.SHORT:
                n = bs.getShort();
                break;
            case ProtoStruct.INT:
                n = bs.getInt();
                break;
            case ProtoStruct.LONG:
                n = bs.getLong();
                break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return n;
    }

    public float read(float n, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.ZERO_TAG:
                n = 0;
                break;
            case ProtoStruct.FLOAT:
                n = bs.getFloat();
                break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return n;
    }

    public double read(double n, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.ZERO_TAG:
                n = 0;
                break;
            case ProtoStruct.FLOAT:
                n = bs.getFloat();
                break;
            case ProtoStruct.DOUBLE:
                n = bs.getDouble();
                break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return n;
    }

    public String readByteString(String s, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.STRING1: {
                int len = bs.get();
                if (len < 0)
                    len += 256;
                byte[] ss = new byte[len];
                bs.get(ss);
                s = Utils.bytes2HexStr(ss);
            }
            break;
            case ProtoStruct.STRING4: {
                int len = bs.getInt();
                if (len > ProtoStruct.MAX_STRING_LENGTH || len < 0 || len > bs.capacity())
                    throw new DecodeException("String too long: " + len);
                byte[] ss = new byte[len];
                bs.get(ss);
                s = Utils.bytes2HexStr(ss);
            }
            break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return s;
    }

    public String read(String s, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.STRING1: {
                int len = bs.get();
                if (len < 0)
                    len += 256;
                byte[] ss = new byte[len];
                bs.get(ss);
                try {
                    s = new String(ss, sServerEncoding);
                } catch (UnsupportedEncodingException e) {
                    s = new String(ss);
                }
            }
            break;
            case ProtoStruct.STRING4: {
                int len = bs.getInt();
                if (len > ProtoStruct.MAX_STRING_LENGTH || len < 0 || len > bs.capacity())
                    throw new DecodeException("String too long: " + len);
                byte[] ss = new byte[len];
                bs.get(ss);
                try {
                    s = new String(ss, sServerEncoding);
                } catch (UnsupportedEncodingException e) {
                    s = new String(ss);
                }
            }
            break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return s;
    }

    public String readString(int tag, boolean isRequire) {
        String s = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.STRING1: {
                int len = bs.get();
                if (len < 0)
                    len += 256;
                byte[] ss = new byte[len];
                bs.get(ss);
                try {
                    s = new String(ss, sServerEncoding);
                } catch (UnsupportedEncodingException e) {
                    s = new String(ss);
                }
            }
            break;
            case ProtoStruct.STRING4: {
                int len = bs.getInt();
                if (len > ProtoStruct.MAX_STRING_LENGTH || len < 0 || len > bs.capacity())
                    throw new DecodeException("String too long: " + len);
                byte[] ss = new byte[len];
                bs.get(ss);
                try {
                    s = new String(ss, sServerEncoding);
                } catch (UnsupportedEncodingException e) {
                    s = new String(ss);
                }
            }
            break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return s;
    }

    public String[] read(String[] s, int tag, boolean isRequire) {
        return readArray(s, tag, isRequire);
    }

    public Map<String, String> readStringMap(int tag, boolean isRequire) {
        HashMap<String, String> mr = new HashMap<String, String>();
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.MAP: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                for (int i = 0; i < size; ++i) {
                    String k = readString(0, true);
                    String v = readString(1, true);
                    mr.put(k, v);
                }
            }
            break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return mr;
    }

    public <K, V> HashMap<K, V> readMap(Map<K, V> m, int tag, boolean isRequire) {
        return (HashMap<K, V>) readMap(new HashMap<K, V>(), m, tag, isRequire);
    }

    @SuppressWarnings("unchecked")
    private <K, V> Map<K, V> readMap(Map<K, V> mr, Map<K, V> m, int tag, boolean isRequire) {
        if (m == null || m.isEmpty()) {
            return new HashMap<K, V>();
        }

        Iterator<Map.Entry<K, V>> it = m.entrySet().iterator();
        Map.Entry<K, V> en = it.next();
        K mk = en.getKey();
        V mv = en.getValue();

        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.MAP: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                for (int i = 0; i < size; ++i) {
                    K k = (K) read(mk, 0, true);
                    V v = (V) read(mv, 1, true);
                    mr.put(k, v);
                }
            }
            break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return mr;
    }

    @SuppressWarnings("unchecked")
    public List readList(int tag, boolean isRequire) {
        List lr = new ArrayList();
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                for (int i = 0; i < size; ++i) {
                    HeadData subH = new HeadData();
                    readHead(subH);
                    switch (subH.type) {
                    case ProtoStruct.BYTE:
                        skip(1);
                        break;
                    case ProtoStruct.SHORT:
                        skip(2);
                        break;
                    case ProtoStruct.INT:
                        skip(4);
                        break;
                    case ProtoStruct.LONG:
                        skip(8);
                        break;
                    case ProtoStruct.FLOAT:
                        skip(4);
                        break;
                    case ProtoStruct.DOUBLE:
                        skip(8);
                        break;
                    case ProtoStruct.STRING1: {
                        int len = bs.get();
                        if (len < 0)
                            len += 256;
                        skip(len);
                    }
                    break;
                    case ProtoStruct.STRING4: {
                        skip(bs.getInt());
                    }
                    break;
                    case ProtoStruct.MAP: {

                    }
                    break;
                    case ProtoStruct.LIST: {

                    }
                    break;
                    case ProtoStruct.STRUCT_BEGIN:
                        try {
                            Class<?> newoneClass = Class.forName(ProtoStruct.class.getName());
                            Constructor<?> cons = newoneClass.getConstructor();
                            ProtoStruct struct = (ProtoStruct) cons.newInstance();
                            struct.readFrom(this);
                            skipToStructEnd();
                            lr.add(struct);
                        } catch (Exception e) {
                            e.printStackTrace();
                            throw new DecodeException("type mismatch." + e);
                        }
                        break;
                    case ProtoStruct.ZERO_TAG:
                        lr.add(Integer.valueOf(0));
                        break;
                    default:
                        throw new DecodeException("type mismatch.");
                    }
                    // T t = read(mt, 0, true);
                    // lr.add(t);
                }
            }
            break;
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public boolean[] read(boolean[] l, int tag, boolean isRequire) {
        boolean[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                lr = new boolean[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public byte[] read(byte[] l, int tag, boolean isRequire) {
        byte[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.SIMPLE_LIST: {
                HeadData hh = new HeadData();
                readHead(hh);
                if(hh.type != ProtoStruct.BYTE) {
                    throw new DecodeException("type mismatch, tag: " + tag + ", type: " + hd.type + ", " + hh.type);
                }
                int size = read(0, 0, true);
                if(size < 0 || size > bs.capacity())
                    throw new DecodeException("invalid size, tag: " + tag + ", type: " + hd.type + ", " + hh.type + ", size: " + size);
                lr = new byte[size];
                bs.get(lr);
                break;
            }
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0 || size > bs.capacity())
                    throw new DecodeException("size invalid: " + size);
                lr = new byte[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public short[] read(short[] l, int tag, boolean isRequire) {
        short[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                lr = new short[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public int[] read(int[] l, int tag, boolean isRequire) {
        int[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                lr = new int[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public long[] read(long[] l, int tag, boolean isRequire) {
        long[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                lr = new long[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public float[] read(float[] l, int tag, boolean isRequire) {
        float[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                lr = new float[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public double[] read(double[] l, int tag, boolean isRequire) {
        double[] lr = null;
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                lr = new double[size];
                for (int i = 0; i < size; ++i)
                    lr[i] = read(lr[0], 0, true);
                break;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return lr;
    }

    public <T> T[] readArray(T[] l, int tag, boolean isRequire) {
        if (l == null || l.length == 0)
            throw new DecodeException("unable to get type of key and value.");
        return readArrayImpl(l[0], tag, isRequire);
    }

    public <T> List<T> readArray(List<T> l, int tag, boolean isRequire) {
        if (l == null || l.isEmpty()) {
            return new ArrayList<T>();
            //throw new TafProxyDecodeException("unable to get type of key and value.");
        }
        T[] tt = readArrayImpl(l.get(0), tag, isRequire);
        if(tt==null) return null;
        ArrayList<T> ll = new ArrayList<T>();
        for(int i = 0; i < tt.length; ++i)
            ll.add(tt[i]);
        return ll;
    }

    @SuppressWarnings("unchecked")
    private <T> T[] readArrayImpl(T mt, int tag, boolean isRequire) {
        if (skipToTag(tag)) {
            HeadData hd = new HeadData();
            readHead(hd);
            switch (hd.type) {
            case ProtoStruct.LIST: {
                int size = read(0, 0, true);
                if (size < 0)
                    throw new DecodeException("size invalid: " + size);
                T[] lr = (T[]) Array.newInstance(mt.getClass(), size);
                for (int i = 0; i < size; ++i) {
                    T t = (T) read(mt, 0, true);
                    lr[i] = t;
                }
                return lr;
            }
            default:
                throw new DecodeException("type mismatch.");
            }
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return null;
    }

    public ProtoStruct directRead(ProtoStruct o, int tag, boolean isRequire) {
        ProtoStruct ref = null;
        if (skipToTag(tag)) {
            try {
                ref = null;
            } catch (Exception e) {
                throw new DecodeException(e.getMessage());
            }

            HeadData hd = new HeadData();
            readHead(hd);
            if (hd.type != ProtoStruct.STRUCT_BEGIN)
                throw new DecodeException("type mismatch.");
            ref.readFrom(this);
            skipToStructEnd();
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return ref;
    }

    public ProtoStruct read(ProtoStruct o, int tag, boolean isRequire) {
        ProtoStruct ref = null;
        if (skipToTag(tag)) {
            try {
                ref = o.getClass().newInstance();
            } catch (Exception e) {
                throw new DecodeException(e.getMessage());
            }

            HeadData hd = new HeadData();
            readHead(hd);
            if (hd.type != ProtoStruct.STRUCT_BEGIN)
                throw new DecodeException("type mismatch.");
            ref.readFrom(this);
            skipToStructEnd();
        } else if (isRequire) {
            throw new DecodeException("require field not exist.");
        }
        return ref;
    }

    public ProtoStruct[] read(ProtoStruct[] o, int tag, boolean isRequire) {
        return readArray(o, tag, isRequire);
    }

    @SuppressWarnings("unchecked")
    public <T> Object read(T o, int tag, boolean isRequire) {
        if (o instanceof Byte) {
            return Byte.valueOf(read((byte) 0x0, tag, isRequire));
        } else if (o instanceof Boolean) {
            return Boolean.valueOf(read(false, tag, isRequire));
        } else if (o instanceof Short) {
            return Short.valueOf(read((short) 0, tag, isRequire));
        } else if (o instanceof Integer) {
            int i = read((int) 0, tag, isRequire);
            return Integer.valueOf(i);
        } else if (o instanceof Long) {
            return Long.valueOf(read((long) 0, tag, isRequire));
        } else if (o instanceof Float) {
            return Float.valueOf(read((float) 0, tag, isRequire));
        } else if (o instanceof Double) {
            return Double.valueOf(read((double) 0, tag, isRequire));
        } else if (o instanceof String) {
            return readString(tag, isRequire);
        } else if (o instanceof Map) {
            return readMap((Map) o, tag, isRequire);
        } else if (o instanceof List) {
            return readArray((List) o, tag, isRequire);
        } else if (o instanceof ProtoStruct) {
            return read((ProtoStruct) o, tag, isRequire);
        } else if (o.getClass().isArray()) {
            if(o instanceof byte[] || o instanceof Byte[]) {
                return read((byte[]) null, tag, isRequire);
            } else if(o instanceof boolean[]) {
                return read((boolean[]) null, tag, isRequire);
            } else if(o instanceof short[]) {
                return read((short[]) null, tag, isRequire);
            } else if(o instanceof int[]) {
                return read((int[]) null, tag, isRequire);
            } else if(o instanceof long[]) {
                return read((long[]) null, tag, isRequire);
            } else if(o instanceof float[]) {
                return read((float[]) null, tag, isRequire);
            } else if(o instanceof double[]) {
                return read((double[]) null, tag, isRequire);
            } else {
                return readArray((Object[])o, tag, isRequire);
            }
        } else {
            throw new DecodeException("read object error: unsupport type.");
        }
    }

    protected String sServerEncoding = "UTF-8";
    public int setServerEncoding(String se) {
        sServerEncoding = se;
        return 0;
    }


    public ByteBuffer getBs() {
        return bs;
    }
}
