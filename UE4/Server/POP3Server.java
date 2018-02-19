package Server;

import java.io.IOException;
import java.net.ServerSocket;

public class POP3Server {

    private ServerSocket serverSocket;

    private final int SERVER_PORT = 6789; // choose an arbitrary port as ports below 1024 are privileged on linux

    public static void main(String[] args) {
        POP3Server server = new POP3Server();
        server.init();
        server.listen();
    }

    private void init() {
        try {
            serverSocket = new ServerSocket(SERVER_PORT);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void listen() {
        System.out.println("Server listening on port " + SERVER_PORT);
        System.out.println("\ttelnet localhost 6789\n\tCommands:\n\tRETR CAPA TOP RSET DELE QUIT");
        // server listens forever and creates new ServerThread thread on client connection
        while (isListening()) {
            try {
                new ServerThread(serverSocket.accept()).start(); // start() to create a new thread
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private boolean isListening() {
        return serverSocket != null && serverSocket.isBound();
    }
}
