package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.Scanner;


public class Frame {
    private String command;
    private HashMap<String, String> headers = new HashMap<>();
    private String body = "";

public Frame(String _command, HashMap<String,String> _headers, String _body){
    this.command=_command;
    this.headers=_headers;
    this.body=_body;
}

public Frame(String str){
    Scanner scanner = new Scanner(str);
    this.command = scanner.nextLine();
    while(scanner.hasNextLine()){
        String nextLine = scanner.nextLine();
        if(!nextLine.equals("")){
            String[] nextHeader = nextLine.split(":");
            String key = nextHeader[0];
            String value = nextHeader[1];
            this.headers.put(key,value);
        }
        else
           break;
    }
    while(scanner.hasNextLine()){
        String nextLine = scanner.nextLine();
        if(!nextLine.equals("\u0000")){
            this.body+=nextLine+"\n";
        }
    }
    scanner.close();
}

public String frameToString(){
    String result = this.command + '\n';
    for(String header : this.headers.keySet()){
        result += header + ":" + this.headers.get(header); 
        result += '\n';
    }
    result += "" + '\n'; //blank line to indicate end of headers
    result += this.body + '\n';
    result += '\u0000'; //mark end of frame.
    return result;
}


public String getCommand(){
    return command;
}

public String getBody(){
    return body;
}
public HashMap<String, String> getHeaders(){
    return headers;
}

}