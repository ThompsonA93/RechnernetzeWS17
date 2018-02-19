package Message;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class Message {
    private String body;
    private Map<String, String> headerValues;
    private boolean flaggedForDeletion;
    private int msgID;
    private ArrayList<String> headerElements;
    private int size;

    // Clientside
    public Message(String input) {
        headerValues = new HashMap<>();
        headerElements = new ArrayList<>();
        parseMessage(input);
        flaggedForDeletion = false;
    }

    // Serverside
    public Message(String input, int msgID) {
        headerValues = new HashMap<>();
        headerElements = new ArrayList<>();
        parseMessage(input);
        flaggedForDeletion = false;
        this.msgID = msgID;
    }

    /**
     * Basic parsing
     * @param input Message String
     */
    public void parseMessage(String input) {
        String[] splitMessage = input.split("\n");
        String headerElement = ""; // The header attribute. Used as key in the header HashMap
        int colonIndex, index = 0;
        String s;

        // start at 1 to ignore server's +OK response
        for (int i = 1; i < splitMessage.length; i++) {
            s = splitMessage[i];

            // string is a continuation of the previous header entry, so combine with previous
            if (s.startsWith("\t") || s.startsWith(" ")) {
                String tmp = headerValues.get(headerElement);
                tmp = tmp.concat("\n").concat(s);
                headerValues.put(headerElement, tmp);
                index++;
                continue;
            }

            colonIndex = s.indexOf(':');
            // first string that is not a continuation and doesn't have a header element
            // therefore it should be the beginning of the message body
            // checking for an empty line is probably the proper way, but this works too
            if (colonIndex == -1) {
                index++;
                parseBody(splitMessage, index);
                return;
            }
            headerElement = s.substring(0, colonIndex);
            // end is exclusive, begin is inclusive. Don't save colon, so use +2
            headerValues.put(headerElement, s.substring(colonIndex + 1, s.length()));
            headerElements.add(headerElement); // store the header element in a separate list to access all of them later
            index++;
        }
    }

    /**
     * Parse the Body part of the message
     * @param input String array containing the whole message
     * @param index Index of the first body line
     */
    private void parseBody(String[] input, int index) {
        StringBuilder sb = new StringBuilder();

        for (int i = index; i < input.length; i++) {
            sb.append(input[i]).append("\n");
        }

        body = sb.toString();
        size = calculateMsgLength(body);
    }

    /**
     * Calculate the message length.
     * As per the POP3 standard, only the length of the message body is calculated.
     * new lines are stored as only '\n' on the server, but the POP3 RFC requires the length to be calculated
     * with 2 Byte "\r\n"
     * @param body The message body
     */
    public static int calculateMsgLength(String body) {
        int msgSize = body.length();
        char[] msgBody = body.toCharArray();

        // simply increment size by one if we encounter a newline as there shouldn't be any \r\n in the message.
        for (int i = 0; i < msgBody.length; i++) {
            if (msgBody[i] == '\n') {
                msgSize++;
            }
        }
        return msgSize;
    }


    public String getBody() {
        return body;
    }

    /**
     * Return the value to the given header attribute key
     * @param element
     * @return
     */
    public String getHeaderElement(String element) {
        return headerValues.get(element);
    }

    public Map<String, String> getFullHeaderMap() {
        return headerValues;
    }

    /**
     * Build a string containing the whole header
     * @return Header string
     */
    public String getFullHeaderString() {
        StringBuilder sb = new StringBuilder();
        // get the value to all header attributes
        for (String headerElement:headerElements) {
            sb.append(headerElement).append(":").append(headerValues.get(headerElement)).append('\n');
        }
        return sb.toString();
    }

    public String getFullMessage() {
        return getFullHeaderString() + getBody();
    }
    public boolean isFlaggedForDeletion() {
        return flaggedForDeletion;
    }
    public void setFlaggedForDeletion(boolean flaggedForDeletion) {
        this.flaggedForDeletion = flaggedForDeletion;
    }
    public int getMsgID() {
        return msgID;
    }
    public int getSize() {
        return size;
    }
}
