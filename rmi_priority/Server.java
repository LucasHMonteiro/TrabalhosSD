import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.HashMap;
import java.util.Map;

public class Server implements IOController {

	private Map<Integer, Resource> resources = new HashMap<>();

	public Server() {}

	public String read(int id, int duration) throws InterruptedException {
		return resources.get(id).read(duration);
	}

	public void write(int id, String content, int duration) throws InterruptedException {
		if (!resources.containsKey(id)) {
			resources.put(id, new Resource(id));
		}
		resources.get(id).write(content, duration);
	}

	public static void main(String args[]) {
		try {
			Server obj = new Server();
			IOController stub = (IOController) UnicastRemoteObject.exportObject(obj, 0);
			Registry registry = LocateRegistry.getRegistry();
			registry.bind("IOController", stub);
			System.err.println("Server ready");
		} catch (Exception e) {
			System.err.println("Server exception: " + e.toString());
			e.printStackTrace();
		}
	}
}
