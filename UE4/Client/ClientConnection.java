package Client;

import javax.net.ssl.SSLSocketFactory;
import java.io.*;
import java.net.Socket;


public class ClientConnection {
    private final int POP3_PORT;
    private final String HOSTNAME;
    private final String POP3_NEWLINE = "\r\n";

    private Socket networkSocket;
    private BufferedReader socketReader;
    private OutputStream outputStream;

    public ClientConnection(int port, String hostname) {
        POP3_PORT = port;
        HOSTNAME = hostname;
    }

    public String initialize(boolean useSSL) {
        if (isConnected()) {
            throw new IllegalStateException("ClientConnection already connected.");
        }
        connect(useSSL);
        initStreams();
        return readLine();
    }

    /**
     * Establish an SSL connection to the host
     * Note: A user certificate and keystore are not required unless the host server explicitly requests it.
     */
    private void connect(boolean useSSL) {
        try {
            if (useSSL) {
                SSLSocketFactory ssf = (SSLSocketFactory) SSLSocketFactory.getDefault();
                networkSocket = ssf.createSocket(HOSTNAME, POP3_PORT);
            } else {
                networkSocket = new Socket(HOSTNAME, POP3_PORT);
            }
        } catch (IOException e) {
            System.out.println("IOException during connect");
        }
    }

    /**
     * Initialize reader/writer for the network socket
     */
    private void initStreams() {
        try {
            socketReader = new BufferedReader(new InputStreamReader(networkSocket.getInputStream()));
            outputStream = networkSocket.getOutputStream();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Read a single line from the network socket
     *
     * @return line read from the socket
     */
    private String readLine() {
        String readLine = null;
        try {
            readLine = socketReader.readLine();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return readLine;
    }

    /**
     * Read a multi-line String that ends with CRLF.CRLF
     *
     * @return Single String containing all lines, separated by \n
     */
    private String readMultiLine() {
        StringBuilder sb = new StringBuilder();
        String readline;

        // If the first line contains -ERR, it can't be a multi line response.
        // This is used for RETR with invalid msgID
        readline = readLine();
        if (readline.startsWith("-ERR")) {
            return readline;
        }
        sb.append(readline);

        // POP3 multi-line responses are terminated by a termination sequence of CRLF.CRLF
        // we can ignore the CRLFs here and only check for a single '.' as Reader.readLine() takes care of handling these
        while (!(readline = readLine()).equals(".")) {
            sb.append(readline);
            sb.append("\n");
        }
        sb.deleteCharAt(sb.lastIndexOf("\n")); // remove trailing newline
        return sb.toString();
    }

    /**
     * Write a String to the network socket
     *
     * @param input String to write
     */
    private void sendLine(String input) {

        try {
            outputStream.write(appendNewLine(input).getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String appendNewLine(String input) {
        return input + POP3_NEWLINE;
    }

    /**
     * Check if networkSocket is connected
     *
     * @return Connection status of the socket
     */
    private boolean isConnected() {
        return networkSocket != null && networkSocket.isConnected();
    }

    public String list() {
        sendLine("LIST");
        return readMultiLine();
    }

    public String list(int msg) {
        sendLine("LIST " + msg);
        return readLine();
    }

    public String user(String user) {
        sendLine("USER " + user);
        return readLine();
    }

    public String pass(String pass) {
        sendLine("PASS " + pass);
        return readLine();
    }

    public String retr(int msg) {
        sendLine("RETR " + msg);
        return readMultiLine();
    }

    public String dele(int msg) {
        sendLine("DELE " + msg);
        return readLine();
    }

    public String rset() {
        sendLine("RSET");
        return readLine();
    }

    public String noop() {
        sendLine("NOOP");
        return readLine();
    }

    public String quit() {
        sendLine("QUIT");
        return readLine();
    }

    public String stat() {
        sendLine("STAT");
        return readLine();
    }

    /**
     * Disconnect from the server and close open streams and socket
     */
    public void disconnect() {
        if (!isConnected()) {
            throw new IllegalStateException("ClientConnection not connected");
        }

        try {
            socketReader.close();
            outputStream.flush();
            outputStream.close();
            networkSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
