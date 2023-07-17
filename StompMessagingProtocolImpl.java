package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private Connections<String> connections;
    private int connectionId;

    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(String message) {
        Frame frameToProcess = new Frame(message);
        String command = frameToProcess.getCommand();
        if (command.equals("CONNECT")) {
            processConnect(frameToProcess);
        } else if (command.equals("SEND")) {
            processSend(frameToProcess);
        } else if (command.equals("SUBSCRIBE")) {
            processSubscribe(frameToProcess);
        } else if (command.equals("UNSUBSCRIBE")) {
            processUnsubscribe(frameToProcess);
        } else if (command.equals("DISCONNECT")) {
            processDisconnect(frameToProcess);
        } else {
            Frame frame = createErrorFrame("the frame is not valid",frameToProcess.getHeaders().get("receipt"));
            connections.send(connectionId, frame.frameToString());
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }
    }

    private void processConnect(Frame frameToProcess) {
        String login = frameToProcess.getHeaders().get("login");
        String passcode = frameToProcess.getHeaders().get("passcode");
        String acceptVersion = frameToProcess.getHeaders().get("accept-version");
        String host = frameToProcess.getHeaders().get("host");
        if (login != null && passcode != null && acceptVersion != null && host != null) {
            String message = connections.connect(login, passcode, connectionId);
            if (message.equals("Login successful")) {
                HashMap<String, String> hash = new HashMap<>();
                hash.put("version", "1.2");
                Frame frame = new Frame("CONNECTED", hash, "");
                connections.send(connectionId, frame.frameToString());
                if (checkReceipt(frameToProcess) != null) {
                    connections.send(connectionId, (checkReceipt(frameToProcess)).frameToString());
                }
            }
            if (message.equals("Wrong password")) {
                Frame frame = createErrorFrame(message,frameToProcess.getHeaders().get("receipt"));
                connections.send(connectionId, frame.frameToString());
                connections.disconnect(connectionId);
                shouldTerminate = true;
            }
            if (message.equals("User already logged in")) {
                Frame frame = createErrorFrame(message,frameToProcess.getHeaders().get("receipt"));
                connections.send(connectionId, frame.frameToString());
                connections.disconnect(connectionId);
                shouldTerminate = true;
            }
        } 
        else {
            // error: frame is not valid
            Frame frame = createErrorFrame("the frame is not valid",frameToProcess.getHeaders().get("receipt"));
            connections.send(connectionId, frame.frameToString());
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }
    }

    private void processSend(Frame frameToProcess) {
        String destination = frameToProcess.getHeaders().get("destination");
        String body = frameToProcess.getBody();
        if (destination != null) {
            if (connections.checkSubscription(destination, connectionId)) {
                ConcurrentHashMap<Integer, Integer> usersToSubId = connections.getChannelUsers(destination);
                String messageId = Integer.toString(connections.generateMessageId());
                for (Integer user : usersToSubId.keySet()) {
                    HashMap<String, String> hash = new HashMap<>();
                    hash.put("subscription", Integer.toString(usersToSubId.get(user)));
                    hash.put("message-id", messageId); 
                    hash.put("destination", destination);
                    Frame messageFrame = new Frame("MESSAGE", hash, body);
                    connections.send(user, messageFrame.frameToString());
                }
                if (checkReceipt(frameToProcess) != null) {
                    connections.send(connectionId, (checkReceipt(frameToProcess)).frameToString());
                }
            } 
            else {
                Frame frame = createErrorFrame("the user is not subscribed to this channel",frameToProcess.getHeaders().get("receipt"));
                connections.send(connectionId, frame.frameToString());
                connections.disconnect(connectionId);
                shouldTerminate = true;
            }
        } 
        else {
            Frame frame = createErrorFrame("the frame is not valid",frameToProcess.getHeaders().get("receipt"));
            connections.send(connectionId, frame.frameToString());
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }
    }

    private void processSubscribe(Frame frameToProcess) {
        String destination = frameToProcess.getHeaders().get("destination");
        String id = frameToProcess.getHeaders().get("id");
        if (destination != null && id != null) {
            String message = connections.subscribe(destination, Integer.parseInt(id), connectionId);
            if (message.equals("Success")) {
                if (checkReceipt(frameToProcess) != null) {
                    connections.send(connectionId, (checkReceipt(frameToProcess)).frameToString());
                }
            }
            if (message.equals("User already subscribed to this channel")) {
                Frame frame = createErrorFrame(message,frameToProcess.getHeaders().get("receipt"));
                connections.send(connectionId, frame.frameToString());
                connections.disconnect(connectionId);
                shouldTerminate = true;
            }
        } 
        else {
            Frame frame = createErrorFrame("the frame is not valid", frameToProcess.getHeaders().get("receipt"));
            connections.send(connectionId, frame.frameToString());
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }
    }

    private void processUnsubscribe(Frame frameToProcess) {
        String id = frameToProcess.getHeaders().get("id");
        if (id != null) {
            String message = connections.unsubscribe(Integer.parseInt(id), connectionId);
            if (message.equals("Success")) {
                if (checkReceipt(frameToProcess) != null) {
                    connections.send(connectionId, (checkReceipt(frameToProcess)).frameToString());
                }
            }
            if (message.equals("User is not subscribed to this channel")) {
                Frame frame = createErrorFrame(message, frameToProcess.getHeaders().get("receipt"));
                connections.send(connectionId, frame.frameToString());
                connections.disconnect(connectionId);
                shouldTerminate = true;
            }
        } 
        else {
            Frame frame = createErrorFrame("the frame is not valid",frameToProcess.getHeaders().get("receipt"));
            connections.send(connectionId, frame.frameToString());
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }
    }

    private void processDisconnect(Frame frameToProcess) {
        if (checkReceipt(frameToProcess) != null) {
            connections.send(connectionId, (checkReceipt(frameToProcess)).frameToString());
            connections.disconnect(connectionId);
        } 
        else {
            Frame frame = createErrorFrame("the frame is not valid",frameToProcess.getHeaders().get("receipt"));
            connections.send(connectionId, frame.frameToString());
            connections.disconnect(connectionId); 
        }
        shouldTerminate = true; 
    }

    private Frame checkReceipt(Frame frameToProcess) {
        Frame returnFrame = null;
        String receipt = frameToProcess.getHeaders().get("receipt");
        if (receipt != null) {
            HashMap<String, String> hash = new HashMap<>();
            hash.put("receipt-id", receipt);
            returnFrame = new Frame("RECEIPT", hash, "");
        }
        return returnFrame;
    }

    private Frame createErrorFrame(String str, String receipt) {
        HashMap<String, String> hash = new HashMap<>();
        hash.put("message", str);
        if (receipt != null){
            hash.put("receipt-id", receipt);
        }
        Frame frame = new Frame("ERROR", hash, "");
        return frame;
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

}