import java.nio.ByteBuffer;

/** TODO
 * Not implemented yet.
 */
public class Ue1_3 {
    ByteBuffer bb;

    public void init(){
        boolean isData = true;
        boolean isUrgent = false;
        int sequenceNumber = 312;
        byte[] payload = new byte[20];


        createMsg(isData, isUrgent, sequenceNumber, payload);
    }

    private void createMsg(boolean isData, boolean isUrgent, int sequenceNumber, byte[] payload) throws IllegalArgumentException {
        bb = ByteBuffer.allocate(4 + 4 + (payload.length/31));

        // Field 1
        byte[] field = new byte[2];
        field[0] = (byte) 0x02;
        field[1] = (byte) 0x00;
        bb.put(field);
        if(sequenceNumber > Math.pow(2, (16-1))) throw new IllegalArgumentException();
        bb.put((byte) sequenceNumber).array();

        // Field 2
        bb.putInt((byte)payload.length).array();

        // Field 3
        for(byte b : payload){
            bb.put(b);
        }

        printMsg();
    }

    private void printMsg(){
        bb.rewind();
        while (bb.hasRemaining())
            System.out.println((int) bb.get());
    }
}
