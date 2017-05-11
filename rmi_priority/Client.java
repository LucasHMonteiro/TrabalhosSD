import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Client {

    private Client() {}

    public static void main(String[] args) {
        boolean isReader = true;
        int fileKey = 0;
        String contentToWrite = "";
        if (args.length > 0) {
            fileKey = Integer.parseInt(args[0]);
        }
        if (args.length > 1) {
            isReader = false;
            contentToWrite = args[1];
        }

        try {
            Registry registry = LocateRegistry.getRegistry();
            Resource stub = (Resource) registry.lookup("Resource");
            if (isReader) {
                String response = stub.read(fileKey);
                System.out.println("Lido do arquivo ("+fileKey+"): " + response);
            } else {
                stub.write(fileKey, contentToWrite);
                System.out.println("Comando de escrita executado no arquivo ("+fileKey+").");
            }
        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}
