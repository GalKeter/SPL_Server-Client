package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.BaseServer;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
       int port = Integer.parseInt(args[0]);//??????
       String serverType = args[1];//???????????
    if(serverType.equals("tpc"))
        Server.threadPerClient(port, () -> new StompMessagingProtocolImpl(), () -> new StompEncDecImpl()).serve();
    else
        Server.reactor(Runtime.getRuntime().availableProcessors(),port, () -> new StompMessagingProtocolImpl(), () -> new StompEncDecImpl()).serve();      
}
}
