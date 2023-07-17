package bgu.spl.net.srv;

import java.util.concurrent.ConcurrentHashMap;
import java.util.LinkedList;
import java.util.Set;

public class ConnectionsImpl<T> implements Connections<T> {

    private ConcurrentHashMap<String, String> userToPasscode = new ConcurrentHashMap<>();
    private LinkedList<String> activeUsers = new LinkedList<>();
    private ConcurrentHashMap<Integer, String> connectionIdToUser = new ConcurrentHashMap<>();
    private ConcurrentHashMap<String, LinkedList<Integer>> channelToConnectionId = new ConcurrentHashMap<>();
    private ConcurrentHashMap<Integer, ConcurrentHashMap<String, Integer>> ConnectionIdToChannelToSubId = new ConcurrentHashMap<>();
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connectionIdToconnectionHandler = new ConcurrentHashMap<>();
    private int messageId = 0;

    public boolean send(int connectionId, T msg) {
        connectionIdToconnectionHandler.get(connectionId).send(msg);
        return true;
    }

    public void send(String channel, T msg) {
    }

    public void registerConnectionHandler(ConnectionHandler<T> handler, int connectionId) {
        connectionIdToconnectionHandler.put(connectionId, handler);
    }

    public boolean checkSubscription(String channel, int connectionId) {
        if (channelToConnectionId.get(channel) != null)
            return channelToConnectionId.get(channel).contains((Integer) connectionId);
        return false;
    }

    public ConcurrentHashMap<Integer, Integer> getChannelUsers(String channel) {
        ConcurrentHashMap<Integer, Integer> usersToSubId = new ConcurrentHashMap<>();
        LinkedList<Integer> subscribedUsers = channelToConnectionId.get(channel);
        for (Integer user : subscribedUsers) {
            int subId = ConnectionIdToChannelToSubId.get(user).get(channel);
            usersToSubId.put(user, subId);
        }
        return usersToSubId;
    }

    public void disconnect(int connectionId) {
        synchronized (activeUsers) {
            activeUsers.remove(connectionIdToUser.get(connectionId));
        } // remove client from active users list
        connectionIdToUser.remove(connectionId);
        ConnectionIdToChannelToSubId.remove(connectionId); // remove client from map
        for (LinkedList<Integer> chan : channelToConnectionId.values()) { // remove client from each channel
            chan.remove((Integer) connectionId);
        }
        connectionIdToconnectionHandler.remove(connectionId); // remove connection handler
        // ***continue
    }

    public String connect(String user, String passcode, int connectionId) {

        if (userToPasscode.containsKey(user)) {
            if (userToPasscode.get(user).equals(passcode)) {
                synchronized (activeUsers) {
                    if (activeUsers.contains(user))
                        return "User already logged in";
                    else {
                        activeUsers.add(user);
                        connectionIdToUser.put(connectionId, user);
                        return "Login successful";
                    }
                }
            } else
                return "Wrong password";
        } else {
            userToPasscode.put(user, passcode);
            synchronized (activeUsers) {
                activeUsers.add(user);
            }
            connectionIdToUser.put(connectionId, user);
            return "Login successful";
        }
    }

    public String subscribe(String Channel, int subId, int connectionId) {
        if (!channelToConnectionId.containsKey(Channel)) { // create new channel
            LinkedList<Integer> users = new LinkedList<>();
            users.add(connectionId);
            channelToConnectionId.put(Channel, users);
        } else { // channel already exists
            if (channelToConnectionId.get(Channel).contains(connectionId)) // check if user already subscribed to this
                                                                           // channel
                return "User already subscribed to this channel";
            else {
                channelToConnectionId.get(Channel).add(connectionId);
            }
        }
        if (ConnectionIdToChannelToSubId.containsKey(connectionId)) //
            ConnectionIdToChannelToSubId.get(connectionId).put(Channel, subId);
        else {
            ConcurrentHashMap<String, Integer> userSubs = new ConcurrentHashMap<>();
            userSubs.put(Channel, subId);
            ConnectionIdToChannelToSubId.put(connectionId, userSubs);
        }
        return "Success";
    }

    public String unsubscribe(int subId, int connectionId) {
        ConcurrentHashMap<String, Integer> userSubs = ConnectionIdToChannelToSubId.get(connectionId);
        if (!userSubs.containsValue(subId))
            return "User is not subscribed to this channel";
        else {
            Set<String> userChannels = userSubs.keySet();
            for (String channel : userChannels) {
                if (userSubs.get(channel).equals(subId)) { // we dont delete the user connection id in case he has no
                                                           // more subs, just the record in the inner hashmap
                    userSubs.remove(channel);
                    channelToConnectionId.get(channel).remove((Integer) connectionId);
                    break;
                }
            }
            return "Success";
        }
    }

    public synchronized int generateMessageId() {
        messageId++;
        return messageId;
    }

}
