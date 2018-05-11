package org.gtdev.oc.server.protocol;

import java.util.Map;

import org.gtdev.oc.server.protocol.ProtoInputStream.HeadData;

public final class ProtoDisplayer {
    private StringBuilder	sb;
    private int				_level = 0;
    private ProtoInputStream pis;

    private void ps(int tag, String fieldName) {
        for(int i = 0; i < _level; ++i)
            sb.append('\t');
        if(fieldName != null)
            sb.append(tag).append(',').append(fieldName).append(": ");
    }

    public ProtoDisplayer(StringBuilder sb, int level) {
        this.sb = sb;
        this._level = level;
    }

    public ProtoDisplayer(StringBuilder sb) {
        this.sb = sb;
    }

    public void display(ProtoInputStream pis_) {
        this.pis = pis_;
        while(pis.getBs().hasRemaining()) {
            if(!do_display())
                break;
        }
    }

    private String getNamebyType(byte type) {
        switch(type) {
        case ProtoStruct.BYTE:
            return "BYTE";
        case ProtoStruct.DOUBLE:
            return "DOUBLE";
        case ProtoStruct.FLOAT:
            return "FLOAT";
        case ProtoStruct.INT:
            return "INT";
        case ProtoStruct.LIST:
            return "LIST";
        case ProtoStruct.LONG:
            return "LONG";
        case ProtoStruct.MAP:
            return "MAP";
        case ProtoStruct.SHORT:
            return "SHORT";
        case ProtoStruct.SIMPLE_LIST:
            return "SIMPLE_LIST";
        case ProtoStruct.STRING1:
            return "STRING1";
        case ProtoStruct.STRING4:
            return "STRING4";
        case ProtoStruct.STRUCT_BEGIN:
            return "STRUCT_BEGIN";
        case ProtoStruct.STRUCT_END:
            return "STRUCT_END";
        case ProtoStruct.ZERO_TAG:
            return "ZERO_TAG";
        }
        return "UNKNOWN";
    }

    private boolean do_display() {
        HeadData hd = new HeadData();
        pis.peakHead(hd);
        ps(hd.tag, getNamebyType(hd.type));
        String s = "";
        switch( hd.type ) {
        case ProtoStruct.BYTE:
            byte a = 0;
            a = pis.read(a, hd.tag, false);
            sb.append(a).append('\n');
            break;
        case ProtoStruct.DOUBLE:
            double b = 0;
            b = pis.read(b, hd.tag, false);
            sb.append(b).append('\n');
            break;
        case ProtoStruct.FLOAT:
            float c = 0;
            c = pis.read(c, hd.tag, false);
            sb.append(c).append('\n');
            break;
        case ProtoStruct.INT:
            int d = 0;
            d = pis.read(d, hd.tag, false);
            sb.append(d).append('\n');
            break;
        case ProtoStruct.LIST:
            pis.skipField();
            sb.append("[LIST]\n");
            break;
        case ProtoStruct.LONG:
            long f = 0;
            f = pis.read(f, hd.tag, false);
            sb.append(f).append('\n');
            break;
        case ProtoStruct.MAP:
            Map<String,String> m;
            m = pis.readStringMap(hd.tag, false);
            sb.append(m).append('\n');
            break;
        case ProtoStruct.SHORT:
            short h = 0;
            h = pis.read(h, hd.tag, false);
            sb.append(h).append('\n');
            break;
        case ProtoStruct.SIMPLE_LIST:
            byte[] i = null;
            i = pis.read(i, hd.tag, false);
            if ( i == null ) {
                sb.append("null").append('\n');
                break;
            }
            if(i.length == 0) {
                sb.append(i.length).append(", []").append('\n');
            }
            sb.append(i.length).append(", [");
            sb.append(i[0]);
            for(int j=1; j<i.length; j++)
                sb.append(',').append(i[j]);
            sb.append("]\n");
            break;
        case ProtoStruct.STRING1:
            s = pis.read(s, hd.tag, false);
            sb.append(s).append('\n');
            break;
        case ProtoStruct.STRING4:
            s = pis.read(s, hd.tag, false);
            sb.append(s).append('\n');
            break;
        case ProtoStruct.STRUCT_BEGIN:
            sb.append('\n');
            ProtoDisplayer jd = new ProtoDisplayer(sb, _level + 1);
            pis.skipField();
            break;
        case ProtoStruct.STRUCT_END:
            sb.append('\n');
            pis.skipField();
            //return false;
            break;
        case ProtoStruct.ZERO_TAG:
            sb.append("ZERO_TAG").append('\n');
            break;
        }
        return true;
    }

    @Override
    public String toString() {
        return sb.toString();
    }

}
