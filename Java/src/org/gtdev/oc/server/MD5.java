package org.gtdev.oc.server;

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class MD5 {
    static final char[] Digit = new char[] {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    private byte[] digest = new byte[16];
    public String digestHexStr;

    public static long b2iu(byte b) {
        return b < (byte) 0 ? (long) (b & 255) : (long) b;
    }

    public byte[] getMD5(byte[] bArr, int i, int i2) {
        if (bArr == null || i2 == 0 || i < 0) {
            return null;
        }
        byte[] sysGetBufferMd5 = sysGetBufferMd5(bArr, i, i2);
        if (sysGetBufferMd5 != null) {
            this.digest = sysGetBufferMd5;
            return this.digest;
        }
        return null;
    }

    public byte[] getMD5(InputStream inputStream, long j) {
        if (inputStream == null || j < 0) {
            return null;
        }
        try {
            long available = (long) inputStream.available();
            if (j == 0 || (available != 0 && ((long) inputStream.available()) < j)) {
                j = (long) inputStream.available();
            }
            if (j == 0) {
                return null;
            }
            byte[] sysGetStremMd5 = sysGetStreamMd5(inputStream, j);
            if (sysGetStremMd5 != null) {
                this.digest = sysGetStremMd5;
                return this.digest;
            }
            return null;
        } catch (Exception e2) {
            e2.printStackTrace();
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e3) {
                    e3.printStackTrace();
                }
            }
            return null;
        }
    }

    public static String byteHEX(byte b) {
        return new String(new char[] {Digit[(b >>> 4) & 15], Digit[b & 15]});
    }

    public static byte[] toMD5Byte(byte[] bArr) {
        return toMD5Byte(bArr, 0, bArr.length);
    }

    public static byte[] toMD5Byte(byte[] bArr, int i, int i2) {
        return new MD5().getMD5(bArr, i, i2);
    }

    public static byte[] toMD5Byte(String str) {
        if (str == null) {
            return null;
        }
        byte[] bytes;
        try {
            bytes = str.getBytes("ISO8859_1");
        } catch (UnsupportedEncodingException e) {
            bytes = str.getBytes();
        }
        return new MD5().getMD5(bytes, 0, bytes.length);
    }

    public static byte[] toMD5Byte(InputStream inputStream, long j) {
        return new MD5().getMD5(inputStream, j);
    }

    public static String toMD5(byte[] bArr) {
        int i = 0;
        if (bArr == null || bArr.length == 0) {
            return null;
        }
        byte[] md5 = new MD5().getMD5(bArr, 0, bArr.length);
        StringBuilder stringBuilder = new StringBuilder(32);
        while (i < 16) {
            stringBuilder.append(Digit[(md5[i] >>> 4) & 15]);
            stringBuilder.append(Digit[md5[i] & 15]);
            i++;
        }
        return stringBuilder.toString();
    }

    public static String toMD5(String str) {
        if (str == null) {
            return null;
        }
        byte[] bytes;
        try {
            bytes = str.getBytes("ISO8859_1");
        } catch (UnsupportedEncodingException e) {
            bytes = str.getBytes();
        }
        byte[] md5 = new MD5().getMD5(bytes, 0, bytes.length);
        StringBuilder stringBuilder = new StringBuilder(32);
        if (md5 == null) {
            return "";
        }
        for (int i = 0; i < 16; i++) {
            stringBuilder.append(Digit[(md5[i] >>> 4) & 15]);
            stringBuilder.append(Digit[md5[i] & 15]);
        }
        return stringBuilder.toString();
    }

    public static byte[] sysGetStreamMd5(InputStream inputStream, long len) {
        byte[] bArr = null;
        if (inputStream != null && len != 0) {
            try {
                MessageDigest instance = MessageDigest.getInstance("MD5");
                byte[] bArr2 = new byte[16384];
                int length = bArr2.length;
                long j2 = 0;
                while (j2 < len) {
                    if (((long) bArr2.length) + j2 > len) {
                        length = (int) (len - j2);
                    }
                    int read = inputStream.read(bArr2, 0, length);
                    if (read < 0) {
                        break;
                    }
                    instance.update(bArr2, 0, read);
                    j2 += (long) read;
                    length = read;
                }
                try {
                    inputStream.close();
                } catch (Exception e) {
                }
                bArr = instance.digest();
            } catch (NoSuchAlgorithmException e2) {
                e2.printStackTrace();
            } catch (IOException e3) {
                e3.printStackTrace();
            }
        }
        return bArr;
    }

    public static byte[] sysGetBufferMd5(byte[] bArr, int offset, int len) {
        byte[] bArr2 = null;
        if (!(bArr == null || len == 0)) {
            try {
                MessageDigest instance = MessageDigest.getInstance("MD5");
                instance.update(bArr, offset, len);
                bArr2 = instance.digest();
            } catch (NoSuchAlgorithmException e) {
                e.printStackTrace();
            }
        }
        return bArr2;
    }
}
