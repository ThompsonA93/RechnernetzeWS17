package Client;

import Message.Message;
import java.util.Scanner;

public class POP3Client {
    private Scanner sc;
    private ClientConnection clientConnection;

    private boolean useSSL = true;
    private final String USER_EMAIL = "asdffdas@airmail.cc";
    private final String USER_PASS = "asdf1234";
    private final int POP3_SSL_PORT = 995;
    private final int POP3_PORT = 110;
    private final String POP3_SERVER_HOST = "mail.cock.li";

    public static void main(String[] args) {
        POP3Client client = new POP3Client();
        client.run();
    }

    private void run() {
        sc = new Scanner(System.in);
        showMenu();
    }

    private void showMenu() {
        while (true) {
            System.out.println("POP3Client online.");
            System.out.println("Select option:");
            System.out.println("\t1 - Login");
            System.out.println("\t2 - Quit");

            int option = sc.nextInt();

            if (option == 1) {
                loginPrompt();
            } else if (option == 2) {
                System.out.println("Goodbye.");
                return;
            } else {
                System.out.println("Incorrect input.");
                continue;
            }

            break;
        }
    }

    private void loginPrompt() {
        initClient();
        String user, pass;
        while (true) {
            System.out.println("\nUsername: ");
            user = sc.next();
            if (!checkResponse(clientConnection.user(user))) {
                System.out.println("Error! Username incorrect.");
                continue;
            }

            System.out.println("Password: ");
            pass = sc.next();
            if (!checkResponse(clientConnection.pass(pass))) {
                System.out.println("Error! Password incorrect");
                continue;
            }

            System.out.println("Login successful.");
            loggedIn();
            break;
        }
    }

    private void loggedIn() {
        while (true) {
            System.out.println("Select Option:");
            System.out.println("\t1 - List all Mail");
            System.out.println("\t2 - Get Mail");
            System.out.println("\t3 - Delete Mail");
            System.out.println("\t4 - Reset changes");
            System.out.println("\t5 - Quit");

            int option = sc.nextInt();
            if (option == 1) {
                getMail();
            } else if (option == 2) {
                System.out.println("Enter Mail ID: ");
                getMail(sc.nextInt());
            } else if (option == 3) {
                System.out.println("Enter Mail ID to delete: ");
                deleteMail(sc.nextInt());
            } else if (option == 4) {
                resetChanges();
            } else if (option == 5) {
                disconnect();
                return;
            } else {
                System.out.println("Incorrect input.");
            }
        }
    }

    private void doQuickLogin() {
        initClient();
        clientConnection.user(USER_EMAIL);
        clientConnection.pass(USER_PASS);
        System.out.println("Login successful.");
        loggedIn();
    }

    private void getMail(int msg) {
        String retMsg = clientConnection.retr(msg);
        Message message;
        if (checkResponse(retMsg)) {
            message = new Message(retMsg);
        } else {
            System.out.println("Error while downloading mail");
            return;
        }

        System.out.println("--- Begin Message " + msg + "---");
        System.out.println("To: " + message.getHeaderElement("To"));
        System.out.println("From: " + message.getHeaderElement("From"));
        System.out.println("Date: " + message.getHeaderElement("Date"));
        System.out.println("Subject: " + message.getHeaderElement("Subject"));
        System.out.println("\n------------\n");
        System.out.println(message.getBody());
        System.out.println("--- End Message " + msg + " ---");

    }

    private void getMail() {
        int mailAmount = getMessageCount();
        for (int i = 1; i <= mailAmount; i++) {
            getMail(i);
            System.out.println();
        }
    }

    private void deleteMail(int msg) {
        if (checkResponse(clientConnection.dele(msg))) {
            System.out.println("Mail deleted.");
        } else {
            System.out.println("Something went wrong.");
        }
    }

    private int getMessageCount() {
        // split the STAT response at spaces. As per the POP3 protocol, the second field is the amount of mail the user has
        String mailAmountString = clientConnection.stat().split(" ")[1];
        return Integer.parseInt(mailAmountString);
    }

    private void resetChanges() {
        if (checkResponse(clientConnection.rset())) {
            System.out.println("Reset successful.");
        }
    }

    private boolean checkResponse(String response) {
        if (!response.startsWith("+OK")) {
            System.out.println("Error: " + response);
            return false;
        }
        return true;
    }

    private void initClient() {
        if (useSSL) clientConnection = new ClientConnection(POP3_SSL_PORT, POP3_SERVER_HOST);
        else clientConnection = new ClientConnection(POP3_PORT, POP3_SERVER_HOST);
        clientConnection.initialize(useSSL);
    }

    private void disconnect() {
        System.out.println("Disconnecting.");
        clientConnection.quit();
        clientConnection.disconnect();
    }
}
