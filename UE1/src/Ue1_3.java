import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/** TODO
 * Not implemented yet.
 */
public class Ue1_3 {
    private final byte version = 2;

    public void init(){
        boolean isData = true;
        boolean isUrgent = false;
        int sequenceNumber = 312;
        byte[] payload = new byte[20];


        byte[] msg = createMsg(isData, isUrgent, sequenceNumber, payload);
        printMsg(msg);
    }

    private byte[] createMsg(boolean isData, boolean isUrgent, int sequenceNumber, byte[] payload) throws IllegalArgumentException {
        // check for invalid input
        if (payload.length < 0) throw new IllegalArgumentException();
        if (sequenceNumber < 0 || sequenceNumber > Math.pow(2, 16) - 1) throw new IllegalArgumentException();

        ByteBuffer bb = ByteBuffer.allocate(4 + 4 + (Byte.BYTES * payload.length));
        bb.order(ByteOrder.BIG_ENDIAN);

        // insert protocol Version
        byte byteVersion = version << 3;
        bb.put(byteVersion);               // Does not like version << 3

        // insert D and U fields
        byte flags = 0;
        if (isData) flags = (byte) ((flags | 1) << 1);  // set 1 first & shiftleft
        if (isUrgent) flags = (byte)(flags | 1);        // Set second
        bb.put(flags);

        // put sequence number
        bb.putShort((short)sequenceNumber);

        // put payload length
        bb.putInt(payload.length);

        // put payload
        bb.put(payload);
        return bb.array();
    }

    private void printMsg(byte[] message) {
        System.out.println("Printing Message");
        for (byte b : message) {
            System.out.print(b + " ");
        }
        System.out.println();
    }
}
