import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Resource {
	private int id;
	private int activeWriters = 0, waitingWriters = 0, activeReaders = 0, waitingReaders = 0;
	private Lock countersLock = new ReentrantLock();
	private Object readersQueue = new Object();
	private Object writersQueue = new Object();
	private String file = "first line";

	public Resource(int id) {
		this.id = id;
	}

	public String read(int duration) throws InterruptedException {
		countersLock.lock();

		while (activeWriters > 0) {
			this.log("waiting...");
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

		this.log("reading...");
		String read = file;
		Thread.sleep(duration * 1000);
		this.log("reading finished.");
		this.log("Contents: \n" + read);

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

		return read;
	}

	public void write(String content, int duration) throws InterruptedException {
		countersLock.lock();

		while (activeReaders > 0 || waitingReaders > 0 || activeWriters > 0) {
			this.log("waiting...");
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

		this.log("writing...");
		file += "\n" + content;
		Thread.sleep(duration * 1000);
		this.log("writing finished.");

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

	private void log(String message) {
		System.out.println("\nThread ("+Thread.currentThread().getId()+") --> Resource ("+this.id+"): " + message);
	}
}
