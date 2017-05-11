import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import static java.lang.Thread.sleep;

public class Server implements Resource {

	private Map<Integer, String> resources;
	private static int activeWriters = 0, waitingWriters = 0, activeReaders = 0, waitingReaders = 0;
	private static Lock countersLock = new ReentrantLock();
	private static Object readersQueue = new Object();
	private static Object writersQueue = new Object();
	private static String file = "content";

	public Server() {
		this.resources = new HashMap<>();
		this.resources.put(1, "Initial content 1");
		this.resources.put(2, "Initial content 2");
		this.resources.put(3, "Initial content 3");
	}

	public String read(int fd) throws InterruptedException {
		countersLock.lock();
		System.out.println("Pedido de leitura.");

		while (activeWriters > 0) {
			waitingReaders++;
			countersLock.unlock();
			synchronized (readersQueue) {
				readersQueue.wait();
			}
			countersLock.lock();
			waitingReaders--;
		}

		activeReaders++;

		countersLock.unlock();

		System.out.println("Lendo...");
		String lido = file;
		sleep(10000);

		countersLock.lock();

		activeReaders--;

		if (activeReaders > 0 || waitingReaders > 0) {
			synchronized (readersQueue) {
				readersQueue.notifyAll();
			}
		} else if (waitingWriters > 0) {
			synchronized (writersQueue) {
				writersQueue.notify();
			}
		}

		countersLock.unlock();

		return lido;
	}

	public void write(int fd, String content) throws InterruptedException {
		countersLock.lock();
		System.out.println("Pedido de escrita.");

		while (activeReaders > 0 || waitingReaders > 0 || activeWriters > 0) {
			waitingWriters++;
			countersLock.unlock();
			synchronized (writersQueue) {
				writersQueue.wait();
			}
			countersLock.lock();
			waitingWriters--;
		}

		activeWriters++;

		countersLock.unlock();

		System.out.println("Escrevendo...");
		file += "\n" + content;

		countersLock.lock();

		activeWriters--;

		if (activeReaders > 0 || waitingReaders > 0) {
			synchronized (readersQueue) {
				readersQueue.notifyAll();
			}
		}
		else if (waitingWriters > 0) {
			synchronized (writersQueue) {
				writersQueue.notify();
			}
		}

		countersLock.unlock();
	}

	public static void main(String args[]) {
		try {
			Server obj = new Server();
			Resource stub = (Resource) UnicastRemoteObject.exportObject(obj, 0);
			Registry registry = LocateRegistry.getRegistry();
			registry.bind("Resource", stub);

			System.err.println("Server ready");
		} catch (Exception e) {
			System.err.println("Server exception: " + e.toString());
			e.printStackTrace();
		}
	}
}
