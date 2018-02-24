package Server;

import Database.SampleDataBase;
import Message.Message;

import java.io.*;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Iterator;

public class ServerThread extends Thread {

    private BufferedReader socketReader;
    private BufferedOutputStream socketOutputStream;
    private Socket clientSocket;
    private STATE serverState;
    private ArrayList<Message> mailList;
    private boolean isCon = false;

    private final String RESPONSE_OK = "+OK ";
    private final String RESPONSE_ERR = "-ERR ";
    private final String POP3_NEWLINE = "\r\n";

    private enum STATE {
        Authentication,
        UserAccepted, // after USER command has been successfully issued, but PASS has not yet been received
        Transaction,
        Update
    }

    public ServerThread(Socket socket) {
        clientSocket = socket;
        try {
            clientSocket.setSoTimeout(120 * 1000); // 120 second socket timeout
        } catch (SocketException e) {
            e.printStackTrace();
        }
        serverState = STATE.Authentication;
        System.out.println("Client connected!");
    }

    /**
     * Run the server thread
     */
    public void run() {
        initServer();
    }

    private void initServer() {
        initStreams();
        serverHello();
        awaitResponse();
    }

    private void initStreams() {
        try {
            socketReader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            socketOutputStream = new BufferedOutputStream(clientSocket.getOutputStream());
            isCon = true;

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Load the static maildrop once the user is authenticated
     */
    private void loadMaildrop() {
        ArrayList<String> messages = SampleDataBase.messages;
        mailList = new ArrayList<>();

        // Load all available messages, starting with 1 as described in the POP3 standard
        int i = 1;
        for (String msg : messages) {
            mailList.add(new Message(msg, i));
            i++;
        }
    }

    /**
     * Await a client's request
     */
    private void awaitResponse() {
        String input;
        try {
            while (isConnected() &&
                    (input = socketReader.readLine()) != null) {
                System.out.println(input);
                parseRequest(input);
            }
        } catch (SocketTimeoutException e) {
            System.out.println("Read timeout.");
            disconnect();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Parse the command and parameters from the client's request
     * @param input client's request
     */
    private void parseRequest(String input) {
        String request;
        int requestIndex = input.indexOf(' '); // request is split by spaces. String before first space is request type
        String[] parameters = null;
        // command with no argument
        if (requestIndex == -1) {
            request = input;
        } else { // command with at least one argument
            request = input.substring(0, requestIndex);
            parameters = input.substring(requestIndex + 1).split(" "); // split the rest in parameters
        }

        delegateRequest(request, parameters);
    }

    private void delegateRequest(String request, String[] parameters) {

        switch (request) {
            case "QUIT":
                requestQuit();
                break;
            case "DELE":
                requestDele(parameters);
                break;
            case "LIST":
                requestList(parameters);
                break;
            case "RETR":
                requestRetr(parameters);
                break;
            case "TOP":
                requestTop(parameters);
                break;
            case "STAT":
                requestStat();
                break;
            case "PASS":
                requestPass(parameters);
                break;
            case "USER":
                requestUser(parameters);
                break;
            case "CAPA":
                requestCapa();
                break;
            case "RSET":
                requestRset();
                break;
            default:
                invalidRequest();
        }
    }

    private void requestQuit() {
        // QUIT responds differently in Authentication and Update state
        if (serverState == STATE.Authentication) {
            sendLine(RESPONSE_OK + "Goodbye.");
            disconnect();
            return;
        }
        // QUIT if in Update state
        serverState = STATE.Update;
        updateMaildrop();
        sendLine(RESPONSE_OK + "Maildrop updated. Goodbye.");
        disconnect();
    }

    private void requestDele(String[] parameters) {
        if (serverState != STATE.Transaction) {
            respondWrongState();
            return;
        }

        int toDelete = Integer.parseInt(parameters[0]);
        if (serverState == STATE.Transaction) {
            for (Message msg : mailList) {
                if (!msg.isFlaggedForDeletion() && msg.getMsgID() == toDelete) {
                    msg.setFlaggedForDeletion(true);
                    sendLine(RESPONSE_OK + "Message flagged for deletion.");
                    return;
                }
            }
            //no corresponding message was found
            sendLine(RESPONSE_ERR + "Message ID not found.");
        } else {
            sendLine(RESPONSE_ERR + "Not possible in this state.");
        }
    }

    private void requestList(String[] parameters) {
        if (serverState != STATE.Transaction) {
            respondWrongState();
            return;
        }
        if (parameters == null) {
            ArrayList<String> activeMail = new ArrayList<>();
            int maildropSize = 0;

            for (int i = 0; i < mailList.size(); i++) {
                if (!mailList.get(i).isFlaggedForDeletion()) {
                    maildropSize += mailList.get(i).getSize();
                    activeMail.add(i + 1 + " " + String.valueOf(mailList.get(i).getSize()));
                }
            }
            sendLine(constructMultiLineResponse(activeMail, activeMail.size() + " messages. (" + maildropSize + " Octets)"));
        } else { // has parameter, only return one
            for (Message msg : mailList) {
                if (!msg.isFlaggedForDeletion() && msg.getMsgID() == Integer.parseInt(parameters[0])) {
                    sendLine(RESPONSE_OK + msg.getMsgID() + " " + msg.getSize());
                    return;
                }
            }
            // message to list not found
            sendLine(RESPONSE_ERR + "Message not found.");
        }

    }

    private void requestRetr(String[] parameters) {
        if (serverState != STATE.Transaction) {
            respondWrongState();
            return;
        }

        // check if there is a message that is not flagged for deletion and has the corresponding MsgID
        for (Message msg : mailList) {
            // deleted messages may not be retrieved
            if (!msg.isFlaggedForDeletion() && msg.getMsgID() == Integer.parseInt(parameters[0])) {
                transmitMessage(msg);
                return;
            }
        }
        // if no suitable message was found
        sendLine(RESPONSE_ERR + "Message not found.");
    }

    /**
     * Transmit an email message to the client
     * @param msg Email to transmit
     */
    private void transmitMessage(Message msg) {
        sendLine(constructMultiLineResponse(msg.getFullMessage().split("\n"), String.valueOf(msg.getSize())));
    }

    /**
     * Return the header and the top n lines of the body
     *
     * @param parameters
     */
    private void requestTop(String[] parameters) {
        if (serverState != STATE.Transaction) {
            respondWrongState();
            return;
        }

        if (parameters == null || parameters.length < 2) {
            sendLine(RESPONSE_ERR + "Invalid arguments.");
            return;
        }
        for (Message msg : mailList) {
            if (!msg.isFlaggedForDeletion() && msg.getMsgID() == Integer.parseInt(parameters[0])) {
                String topBody = getMessageTop(msg.getBody(), Integer.parseInt(parameters[1]));
                int bodyLength = Message.calculateMsgLength(topBody);
                sendLine(constructMultiLineResponse((msg.getFullHeaderString() + topBody).split("\n"), String.valueOf(bodyLength)));
            }
        }
    }

    /**
     * Request the STAT command
     */
    private void requestStat() {
        if (serverState != STATE.Transaction) {
            respondWrongState();
            return;
        }
        sendLine(RESPONSE_OK + mailList.size() + " " + getMaildropSize());
    }

    /**
     * Request the PASS command
     * @param parameters User's password
     */
    private void requestPass(String[] parameters) {
        if (serverState != STATE.UserAccepted) {
            respondWrongState();
            return;
        }
        if (parameters != null) { // if request had parameters
            // do complicated auth
            loadMaildrop(); // load maildrop for this user (in our case only one)
            serverState = STATE.Transaction;
            sendLine(RESPONSE_OK + "Logged in.");
        }

    }

    /**
     * Request the USER command
     * @param parameters User's username
     */
    private void requestUser(String[] parameters) {
        if (serverState != STATE.Authentication) {
            respondWrongState();
            return;
        }
        if (parameters != null) {
            // do complicated auth
            serverState = STATE.UserAccepted;
            sendLine(RESPONSE_OK);
        } else {
            sendLine(RESPONSE_ERR + "User empty");
        }
    }

    /**
     * Request the CAPA command
     * This server only implements the optional TOP command, and signals that USER is available for authentication
     */
    private void requestCapa() {
        sendLine(constructMultiLineResponse(new String[]{"TOP", "USER"}, "Capabilities:"));
    }

    /**
     * Reuquest the RSET command
     * All mail flagged for deletion will be unflagged
     */
    private void requestRset() {
        if (serverState != STATE.Transaction) {
            respondWrongState();
            return;
        }

        // remove deletion flag on all mail
        for (Message msg : mailList) {
            msg.setFlaggedForDeletion(false);
        }
        sendLine(RESPONSE_OK + "Messages recovered.");
    }

    /**
     * Get the top n lines of the message body
     * @param msg Message body
     * @param n number of lines to return
     * @return Message body containing only n lines
     */
    private String getMessageTop(String msg, int n) {
        String[] split = msg.split("\n");
        int maxLines = Math.min(split.length, n);
        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < maxLines; i++) {
            sb.append(split[i]).append('\n');
        }

        return sb.toString();
    }

    /**
     * Get the size of all mail bodies in the maildrop
     * @return maildrop size
     */
    private int getMaildropSize() {
        int maildropSize = 0;

        for (Message msg : mailList) {
            maildropSize += msg.getSize();
        }

        return maildropSize;
    }

    /**
     * Respond that a command was issued in the wrong state
     */
    private void respondWrongState() {
        sendLine(RESPONSE_ERR + "Wrong state for this command.");
    }

    /**
     * Update the maildrop after the user quit.
     * This does pretty much nothing in this implementation as the maildrop DB is static, but it's here for consistency
     */
    private void updateMaildrop() {

        // use Iterator to avoid concurrency exception when iterating and removing at the same time
        Iterator<Message> it = mailList.iterator();
        while (it.hasNext()) {
            Message msg = it.next();
            if (msg.isFlaggedForDeletion()) {
                it.remove();
            }
        }
    }

    /**
     * Calculate the amount of mail in the maildrop that is not flagged for deletion
     *
     * @return Amount of valid mail in the maildrop
     */
    private int mailNotFlaggedForDeletion() {
        int mailAmount = 0;
        for (Message msg : mailList) {
            if (!msg.isFlaggedForDeletion())
                mailAmount++;
        }

        return mailAmount;
    }

    /**
     * Check if the socket is still connected to the client
     * @return
     */
    private boolean isConnected() {
        return clientSocket != null && isCon;
    }

    /**
     * Disconnect from the client and close all streams
     */
    private void disconnect() {
        System.out.println("Client disconnecting...");
        isCon = false;
        try {
            socketReader.close();
            socketOutputStream.close();
            clientSocket.close();
        } catch (IOException e) {
            System.out.println("Error while disconnecting.");
            e.printStackTrace();
        }
    }


    /**
     * Respond with Greeting immediately after a client connected
     */
    private void serverHello() {
        sendLine(RESPONSE_OK + "POP3 Server v1.3.37 ready");
    }

    /**
     * Respond that the request was invalid
     */
    private void invalidRequest() {
        sendLine(RESPONSE_ERR + "Invalid request.");
    }

    /**
     * Write a String to the network socket
     *
     * @param input String to write
     */
    private void sendLine(String input) {

        try {
            socketOutputStream.write(input.getBytes());
            socketOutputStream.write(POP3_NEWLINE.getBytes());
            socketOutputStream.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Construct a proper POP3 multi-line response
     * @param input split String array
     * @param okMsg +OK message to transmit
     * @return multi-line response
     */
    private String constructMultiLineResponse(String[] input, String okMsg) {
        StringBuilder sb = new StringBuilder();
        sb.append(RESPONSE_OK).append(okMsg).append(POP3_NEWLINE); // construct +OK line
        for (String s : input) { // each line is terminated with \r\n
            sb.append(s).append(POP3_NEWLINE);
        }
        sb.append("."); // terminate with line containing only '.' No need for additional \r\n here, as sending will append it

        return sb.toString();
    }

    /**
     * Convert an ArrayList to an array and construct a proper POP3 multi-line response
     * @param input String ArrayList
     * @param okMsg +OK message to transmit
     * @return multi-line response
     */
    private String constructMultiLineResponse(ArrayList<String> input, String okMsg) {
        String[] multiLine = new String[input.size()];
        for (int i = 0; i < input.size(); i++) {
            multiLine[i] = input.get(i);
        }
        return constructMultiLineResponse(multiLine, okMsg);
    }

}
