package test;

import org.junit.Test;
import org.gtdev.oc.server.JNInterface;
import org.gtdev.oc.server.protocol.*;

/*
  * Test data generated from cpp
  1,BYTE: 1
  2,BYTE: -1
  3,SHORT: 256
  4,SHORT: 257
  5,SHORT: -129
  6,SHORT: -130
  7,INT: 65536
  8,INT: 65537
  9,INT: -32769
  10,INT: -32770
  11,LONG: 4294967296
  12,LONG: 4294967297
  13,LONG: -2147483649
  14,LONG: -2147483650
  15,FLOAT: 1.1
  16,FLOAT: -2.2
  17,DOUBLE: 1.1
  18,DOUBLE: -2.2
  19,STRING1: ABCDEFGHIJKLMNOPQESTUVWXYZabcdefghijklmnopqrestuvwxyz1234567890.
  20,MAP: {12345:67890,abcde:ABCDE}
  21,SIMPLE_LIST: \x30\x31\x32\x33\x34\x35\x36\x37\x38\x39
  22,LIST: [65535,65536,65537,65538,65539,65540,65541,65542,65543,65544]
  23,STRUCT_BEGIN: 
  	1,INT: 12345678
  	2,STRING1: HelloWorld
  	3,SIMPLE_LIST: \x07\x7d\x02\x0d
  23,STRUCT_END
  24,STRING1: END
 */

public class ProtoTest {

    //! Test data generated from cpp
    public String GDat =
        "100120ff31010041010151ff7f61ff7e7200010000820001000192ffff7fffa2ffff7ffeb300" +
        "00000100000000c30000000100000001d3ffffffff7fffffffe3ffffffff7ffffffef40f3f8c" +
        "cccdf410c00ccccdf5113ff19999a0000000f512c0019999a0000000f6134041424344454647" +
        "48494a4b4c4d4e4f505145535455565758595a6162636465666768696a6b6c6d6e6f70717265" +
        "737475767778797a313233343536373839302ef8140002060531323334351605363738393006" +
        "05616263646516054142434445fd1500000a30313233343536373839f916000a020000ffff02" +
        "0001000002000100010200010002020001000302000100040200010005020001000602000100" +
        "070200010008fa171200bc614e260a48656c6c6f576f726c643d000004077d020dfb17f61803" +
        "454e44";
    @Test
    public void proto_input() {
        byte[] data = Utils.hexStr2Bytes(GDat);
        ProtoInputStream st = new ProtoInputStream(data);
        StringBuilder sb = new StringBuilder();
        ProtoDisplayer pd = new ProtoDisplayer(sb,0);
        pd.display(st);
        System.out.println(pd);
    }
    
    @Test
    public void proto_cppdisplay() {
    	byte[] data = Utils.hexStr2Bytes(GDat);
    	String res = JNInterface.displayProto(data);
    	System.out.println(res);
    }

}
