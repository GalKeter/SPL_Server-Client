package bgu.spl.net.srv;
import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    String connect(String login, String passcode, int connectionId);

    String subscribe(String Channel, int subId, int connectionId);

    String unsubscribe(int subId, int connectionId);

    ConcurrentHashMap<Integer, Integer> getChannelUsers(String destination);

    int generateMessageId();

    void registerConnectionHandler(ConnectionHandler<T> handler, int clientConnectionId);

    boolean checkSubscription(String destination, int connectionId);
}
