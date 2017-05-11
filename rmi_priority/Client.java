import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Client {

    private Client() {}

    public static void main(String[] args) {
        boolean isReader;
        int fileKey;
        String contentToWrite = "";
        int duration;
        if (args.length == 2) {
            isReader = true;
            fileKey = Integer.parseInt(args[0]);
            duration = Integer.parseInt(args[1]);
        }
        else if (args.length == 3) {
            isReader = false;
            fileKey = Integer.parseInt(args[0]);
            contentToWrite = args[1];
            duration = Integer.parseInt(args[2]);
        }
        else {
            throw new IllegalArgumentException("Invalid number of arguments. Use 2 or 3.");
        }

        try {
            Registry registry = LocateRegistry.getRegistry();
            IOController stub = (IOController) registry.lookup("IOController");
            if (isReader) {
                stub.read(fileKey, duration);
            } else {
                stub.write(fileKey, contentToWrite, duration);
            }
        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}
